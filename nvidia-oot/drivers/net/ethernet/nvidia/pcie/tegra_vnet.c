// SPDX-License-Identifier: GPL-2.0
/* SPDX-FileCopyrightText: Copyright (c) 2019-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <nvidia/conftest.h>

#include <linux/aer.h>
#include <linux/etherdevice.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/llist.h>
#include <linux/mempool.h>
#include <linux/module.h>
#include <linux/msi.h>
#include <linux/netdevice.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/tegra_vnet.h>
#include <linux/tegra-pcie-dma.h>
#include <linux/workqueue.h>
#if defined(NV_NET_PAGE_POOL_H_PRESENT)
#include <net/page_pool.h>
#else
#include <net/page_pool/helpers.h>
#endif

#if ENABLE_DMA
struct tvnet_dma_tx_ctx;
#endif

#define TVNET_NAPI_WEIGHT	64

struct tvnet_priv {
	struct net_device *ndev;
	struct napi_struct napi;
	struct pci_dev *pdev;
	void __iomem *mmio_base;
	void __iomem *msix_tbl;
	void __iomem *dma_base;
	struct bar_md *bar_md;
	struct ep_ring_buf ep_mem;
	struct host_ring_buf host_mem;
	/* RX empty buffers indexed by cookie (avoid per-packet xarray lookup). */
	spinlock_t ep2h_cookie_lock;
	DECLARE_BITMAP(ep2h_cookie_bmap, TVNET_RX_COOKIE_MAX);
	struct ep2h_empty_list *ep2h_cookie_map[TVNET_RX_COOKIE_MAX];
#if ENABLE_DMA
	struct dma_desc_cnt desc_cnt;
	void *dma_cookie;  /* tegra-pcie-dma library cookie */
	/* Deferred posting to H2EP full ring when it's temporarily full */
	struct llist_head tx_complete_list;
	struct delayed_work tx_complete_work;
	/* Timer-based flush to allow batching even when netdev_xmit_more() is always false */
	struct delayed_work tx_batch_flush_work;
	mempool_t *tx_ctx_pool;

	/* TX batching to reduce DMA submit/completion interrupt rate */
	mempool_t *tx_batch_pool;
	spinlock_t tx_batch_lock; /* protects the fields below */
	u8 tx_batch_cnt;
	struct hrtimer tx_batch_timer;
	struct tegra_pcie_dma_desc tx_batch_desc[TVNET_TX_BATCH_MAX];
	struct tvnet_dma_tx_ctx *tx_batch_ctx[TVNET_TX_BATCH_MAX];

	/* Lightweight instrumentation (for validating batching efficacy) */
	u64 tx_dma_submit_cnt;
	u64 tx_dma_submit_descs;
	u64 tx_dma_submit_batch_cnt; /* submit where nents > 1 */
	u64 tx_dma_submit_single_cnt; /* submit where nents == 1 */
	u64 tx_dma_flush_xmit_more_false;
	u64 tx_dma_flush_txq_stopped;
	u64 tx_dma_flush_batch_max;
	u64 tx_dma_flush_timer;
	u64 tx_dma_batch_alloc_fail;

#ifdef CONFIG_PAGE_POOL
	/* page_pool-backed RX buffer allocator (optional) */
	struct page_pool *rx_page_pool;
	spinlock_t rx_pp_lock; /* protects allocation side */
	unsigned int rx_pp_order;
#endif
#endif
	enum dir_link_state tx_link_state;
	enum dir_link_state rx_link_state;
	enum os_link_state os_link_state;

	/* Flag to track ndo_stop done by suspend */
	bool pm_closed;

	/* To synchronize network link state machine*/
	struct mutex link_state_lock;
	wait_queue_head_t link_state_wq;

	struct tvnet_counter h2ep_ctrl;
	struct tvnet_counter ep2h_ctrl;
	struct tvnet_counter h2ep_empty;
	struct tvnet_counter h2ep_full;
	struct tvnet_counter ep2h_empty;
	struct tvnet_counter ep2h_full;

	/* EP SoC identification */
	enum tegra_vnet_soc_id ep_soc_id;

	/* Refill buffers from process context (never from IRQ) */
	struct work_struct refill_work;
	/* Process control ring from process context (never from IRQ) */
	struct work_struct ctrl_work;
};

/* Forward declaration (used by ctrl work, defined later) */
static void tvnet_host_process_ctrl_msg(struct tvnet_priv *tvnet);

static int tvnet_host_ep2h_cookie_alloc(struct tvnet_priv *tvnet,
					struct ep2h_empty_list *ptr,
					u16 *cookie)
{
	unsigned long flags;
	unsigned int id;

	spin_lock_irqsave(&tvnet->ep2h_cookie_lock, flags);
	id = find_first_zero_bit(tvnet->ep2h_cookie_bmap, TVNET_RX_COOKIE_MAX);
	if (id >= TVNET_RX_COOKIE_MAX) {
		spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);
		return -ENOSPC;
	}
	__set_bit(id, tvnet->ep2h_cookie_bmap);
	tvnet->ep2h_cookie_map[id] = ptr;
	spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);

	*cookie = (u16)id;
	return 0;
}

static struct ep2h_empty_list *tvnet_host_ep2h_cookie_take(struct tvnet_priv *tvnet,
							   u16 cookie)
{
	unsigned long flags;
	struct ep2h_empty_list *ptr;

	if (cookie >= TVNET_RX_COOKIE_MAX)
		return NULL;

	spin_lock_irqsave(&tvnet->ep2h_cookie_lock, flags);
	ptr = tvnet->ep2h_cookie_map[cookie];
	tvnet->ep2h_cookie_map[cookie] = NULL;
	if (ptr)
		__clear_bit(cookie, tvnet->ep2h_cookie_bmap);
	spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);

	return ptr;
}

/* Slow-path recovery: if cookie in FULL msg is bad/corrupted, try to locate
 * the tracking node by IOVA. This avoids leaking RX buffers indefinitely.
 * This is O(TVNET_RX_COOKIE_MAX) and should only trigger on bugs/corruption.
 */
static struct ep2h_empty_list *tvnet_host_ep2h_cookie_take_by_iova(struct tvnet_priv *tvnet,
								   dma_addr_t iova)
{
	unsigned long flags;
	u16 cookie;
	struct ep2h_empty_list *ptr;

	spin_lock_irqsave(&tvnet->ep2h_cookie_lock, flags);
	for (cookie = 0; cookie < TVNET_RX_COOKIE_MAX; cookie++) {
		ptr = tvnet->ep2h_cookie_map[cookie];
		if (!ptr)
			continue;
		if (ptr->iova != iova)
			continue;
		tvnet->ep2h_cookie_map[cookie] = NULL;
		__clear_bit(cookie, tvnet->ep2h_cookie_bmap);
		spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);
		return ptr;
	}
	spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);

	return NULL;
}

#if ENABLE_DMA
/* Forward declaration */
static void tvnet_host_raise_ep_data_irq(struct tvnet_priv *tvnet);
/* Structure to hold context for DMA completion callback */
struct tvnet_dma_tx_ctx {
	struct llist_node llnode;
	struct sk_buff *skb;
	struct tvnet_priv *tvnet;
	dma_addr_t src_iova;
	dma_addr_t dst_iova;
	u32 len;
	u16 cookie;
};

struct tvnet_dma_tx_batch {
	struct tvnet_priv *tvnet;
	u8 nents;
	struct tegra_pcie_dma_desc desc[TVNET_TX_BATCH_MAX];
	struct tvnet_dma_tx_ctx *ctx[TVNET_TX_BATCH_MAX];
};

static bool tvnet_host_try_post_h2ep_full(struct tvnet_priv *tvnet,
					  u32 len, dma_addr_t dst_iova,
					  u16 cookie);
static void tvnet_host_dma_complete(void *priv, tegra_pcie_dma_status_t status);
static void tvnet_host_dma_complete_batch(void *priv, tegra_pcie_dma_status_t status);

static void tvnet_host_drain_tx_complete_list(struct tvnet_priv *tvnet)
{
	struct llist_node *node;
	struct tvnet_dma_tx_ctx *ctx, *n;

	node = llist_del_all(&tvnet->tx_complete_list);
	llist_for_each_entry_safe(ctx, n, node, llnode)
		mempool_free(ctx, tvnet->tx_ctx_pool);
}

static void tvnet_host_drop_tx_batch(struct tvnet_priv *tvnet)
{
	struct device *d = &tvnet->pdev->dev;
	struct tvnet_dma_tx_ctx *ctx[TVNET_TX_BATCH_MAX];
	u8 nents, i;
	unsigned long flags;

	spin_lock_irqsave(&tvnet->tx_batch_lock, flags);
	nents = tvnet->tx_batch_cnt;
	for (i = 0; i < nents; i++)
		ctx[i] = tvnet->tx_batch_ctx[i];
	tvnet->tx_batch_cnt = 0;
	spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);

	for (i = 0; i < nents; i++) {
		if (!ctx[i])
			continue;
		dma_unmap_single(d, ctx[i]->src_iova, ctx[i]->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(ctx[i]->skb);
		mempool_free(ctx[i], tvnet->tx_ctx_pool);
	}
}

static bool tvnet_host_try_post_h2ep_full(struct tvnet_priv *tvnet,
					  u32 len, dma_addr_t dst_iova, u16 cookie)
{
	struct host_ring_buf *host_mem = &tvnet->host_mem;
	struct data_msg *h2ep_full_msg = host_mem->h2ep_full_msgs;
	u32 wr_idx;

	if (tvnet_ivc_full(&tvnet->h2ep_full))
		return false;

	wr_idx = tvnet_ivc_get_wr_cnt(&tvnet->h2ep_full) % RING_COUNT;
	h2ep_full_msg[wr_idx].u.full_buffer.packet_size = len;
	h2ep_full_msg[wr_idx].u.full_buffer.pcie_address = (u64)dst_iova;
	h2ep_full_msg[wr_idx].u.full_buffer.cookie = cookie;
	h2ep_full_msg[wr_idx].msg_id = DATA_MSG_FULL_BUF;
	/* Ensure the msg is visible before advancing the ring counter.
	 * Note: tvnet_ivc_advance_wr() already has an smp_mb() after updating
	 * the counter; we still need an ordering point before the counter
	 * update (same pattern as the original code in this driver).
	 */
	mb();
	tvnet_ivc_advance_wr(&tvnet->h2ep_full);

	/* Note: Caller is responsible for raising DATA IRQ (to allow batching).
	 * This function only updates the ring and returns success/failure.
	 */
	return true;
}

static void tvnet_host_tx_complete_work(struct work_struct *work)
{
	struct tvnet_priv *tvnet = container_of(to_delayed_work(work),
						struct tvnet_priv,
						tx_complete_work);
	struct tvnet_dma_tx_ctx *ctx;
	struct tvnet_dma_tx_ctx *n;
	struct llist_node *node;
	unsigned int posted = 0;

	node = llist_del_all(&tvnet->tx_complete_list);
	if (!node)
		return;

	/* Preserve FIFO-ish behavior: reverse from LIFO to FIFO */
	node = llist_reverse_order(node);

	llist_for_each_entry_safe(ctx, n, node, llnode) {
		struct llist_node *remain, *last;

		if (tvnet_host_try_post_h2ep_full(tvnet, ctx->len, ctx->dst_iova, ctx->cookie)) {
			posted++;
			mempool_free(ctx, tvnet->tx_ctx_pool);
			continue;
		}

		/* Ring is full again; re-queue remaining items and retry ASAP.
		 * llist_del_all returns a chain (ctx->llnode.next is the remainder).
		 */
		remain = &ctx->llnode;
		last = remain;
		while (last->next)
			last = last->next;

		llist_add_batch(remain, last, &tvnet->tx_complete_list);

		/* Kick EP to drain FULL ring, then retry immediately */
		tvnet_host_raise_ep_data_irq(tvnet);
		queue_work(system_unbound_wq, &tvnet->tx_complete_work.work);
		return;
	}

	/* Raise DATA IRQ once if any entries were posted (coalesce) */
	if (posted)
		tvnet_host_raise_ep_data_irq(tvnet);
}

static enum hrtimer_restart tvnet_host_tx_batch_timer_fn(struct hrtimer *t)
{
	struct tvnet_priv *tvnet = container_of(t, struct tvnet_priv,
						tx_batch_timer);

	/* Flush in process context. */
	queue_delayed_work(system_highpri_wq, &tvnet->tx_batch_flush_work, 0);
	return HRTIMER_NORESTART;
}

static void tvnet_host_tx_batch_flush_work(struct work_struct *work)
{
	struct tvnet_priv *tvnet = container_of(to_delayed_work(work),
						struct tvnet_priv,
						tx_batch_flush_work);
	struct net_device *ndev = tvnet->ndev;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct device *d = &tvnet->pdev->dev;
	unsigned long flags;
	struct tegra_pcie_dma_xfer_info xfer_info = {0};
	struct tvnet_dma_tx_batch *batch;
	tegra_pcie_dma_status_t dma_status;
	struct tvnet_dma_tx_ctx *c;
	struct tegra_pcie_dma_desc desc;
	u8 nents, i;

	/* Avoid large on-stack snapshot arrays (frame-larger-than).
	 * Allocate the batch container first, then snapshot under lock
	 * directly into it.
	 */
	if (!READ_ONCE(tvnet->tx_batch_cnt))
		return;

	batch = mempool_alloc(tvnet->tx_batch_pool, GFP_KERNEL);
	if (!batch)
		tvnet->tx_dma_batch_alloc_fail++;

	spin_lock_irqsave(&tvnet->tx_batch_lock, flags);
	nents = tvnet->tx_batch_cnt;
	if (!nents) {
		tvnet->tx_batch_cnt = 0;
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
		if (batch)
			mempool_free(batch, tvnet->tx_batch_pool);
		return;
	}

	if (batch) {
		tvnet->tx_batch_cnt = 0;
		for (i = 0; i < nents; i++) {
			batch->ctx[i] = tvnet->tx_batch_ctx[i];
			batch->desc[i] = tvnet->tx_batch_desc[i];
		}
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
	} else {
		/* Rare fallback: submit one-by-one without large stack arrays.
		 * Preserve FIFO order by shifting the remaining entries.
		 */
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);

		for (;;) {
			spin_lock_irqsave(&tvnet->tx_batch_lock, flags);
			if (!tvnet->tx_batch_cnt) {
				spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
				break;
			}

			c = tvnet->tx_batch_ctx[0];
			desc = tvnet->tx_batch_desc[0];
			tvnet->tx_batch_cnt--;
			if (tvnet->tx_batch_cnt) {
				memmove(&tvnet->tx_batch_ctx[0], &tvnet->tx_batch_ctx[1],
					tvnet->tx_batch_cnt * sizeof(tvnet->tx_batch_ctx[0]));
				memmove(&tvnet->tx_batch_desc[0], &tvnet->tx_batch_desc[1],
					tvnet->tx_batch_cnt * sizeof(tvnet->tx_batch_desc[0]));
			}
			spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);

			if (!c)
				continue;

			xfer_info.type = TEGRA_PCIE_DMA_READ;
			xfer_info.channel_num = 0;
			xfer_info.desc = &desc;
			xfer_info.nents = 1;
			xfer_info.complete = tvnet_host_dma_complete;
			xfer_info.priv = c;

			/* Update WR before submit to avoid rd_cnt overtaking wr_cnt
			 * in fast-completion cases (inflight underflow in trace).
			 */
			desc_cnt->wr_cnt++;
			dma_status = tegra_pcie_dma_submit_xfer(tvnet->dma_cookie, &xfer_info);
			if (dma_status != TEGRA_PCIE_DMA_SUCCESS) {
				desc_cnt->wr_cnt--;
				ndev->stats.tx_errors++;
				dma_unmap_single(d, c->src_iova, c->len, DMA_TO_DEVICE);
				dev_kfree_skb_any(c->skb);
				mempool_free(c, tvnet->tx_ctx_pool);
				continue;
			}

			tvnet->tx_dma_submit_cnt++;
			tvnet->tx_dma_submit_descs++;
			tvnet->tx_dma_submit_single_cnt++;
		}

		return;
	}

	batch->tvnet = tvnet;
	batch->nents = nents;

	tvnet->tx_dma_flush_timer++;

	xfer_info.type = TEGRA_PCIE_DMA_READ;
	xfer_info.channel_num = 0;
	xfer_info.desc = batch->desc;
	xfer_info.nents = batch->nents;
	xfer_info.complete = tvnet_host_dma_complete_batch;
	xfer_info.priv = batch;

	/* Update WR before submit to avoid rd_cnt overtaking wr_cnt. */
	desc_cnt->wr_cnt += batch->nents;
	dma_status = tegra_pcie_dma_submit_xfer(tvnet->dma_cookie, &xfer_info);
	if (dma_status != TEGRA_PCIE_DMA_SUCCESS) {
		desc_cnt->wr_cnt -= batch->nents;
		for (i = 0; i < batch->nents; i++) {
			struct tvnet_dma_tx_ctx *c = batch->ctx[i];

			ndev->stats.tx_errors++;
			dma_unmap_single(d, c->src_iova, c->len, DMA_TO_DEVICE);
			dev_kfree_skb_any(c->skb);
			mempool_free(c, tvnet->tx_ctx_pool);
		}
		mempool_free(batch, tvnet->tx_batch_pool);
		return;
	}

	tvnet->tx_dma_submit_cnt++;
	tvnet->tx_dma_submit_descs += batch->nents;
	if (batch->nents > 1)
		tvnet->tx_dma_submit_batch_cnt++;
	else
		tvnet->tx_dma_submit_single_cnt++;

	/* We may have stopped the queue due to our software batch being full.
	 * Wake as soon as we flush the batch (do not wait for DMA completion).
	 */
	if (netif_queue_stopped(ndev) && tvnet->os_link_state == OS_LINK_STATE_UP) {
		if (tvnet_ivc_rd_available(&tvnet->h2ep_empty) &&
		    !tvnet_ivc_full(&tvnet->h2ep_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}
}

/* DMA completion callback for async transfers */
static void tvnet_host_dma_complete(void *priv, tegra_pcie_dma_status_t status)
{
	struct tvnet_dma_tx_ctx *ctx = (struct tvnet_dma_tx_ctx *)priv;
	struct sk_buff *skb = ctx->skb;
	struct net_device *ndev = skb->dev;
	struct tvnet_priv *tvnet = ctx->tvnet;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct device *d = &tvnet->pdev->dev;

	if (status != TEGRA_PCIE_DMA_SUCCESS) {
		dev_err(d, "DMA transfer failed: %d\n", status);
		ndev->stats.tx_errors++;
	} else {
		ndev->stats.tx_packets++;
		ndev->stats.tx_bytes += ctx->len;
	}

	/* Unmap and free skb */
	dma_unmap_single(d, ctx->src_iova, ctx->len, DMA_TO_DEVICE);
	dev_kfree_skb_any(skb);
	ctx->skb = NULL;
	ctx->src_iova = 0;

	/* Update descriptor count */
	desc_cnt->rd_cnt++;

	/* Post to H2EP full ring. If it's temporarily full, do brief retry
	 * (catches transient full during EP drain), then defer if still full.
	 */
	if (status == TEGRA_PCIE_DMA_SUCCESS) {
		if (tvnet_host_try_post_h2ep_full(tvnet, ctx->len, ctx->dst_iova, ctx->cookie)) {
			/* Posted successfully → notify EP to drain */
			tvnet_host_raise_ep_data_irq(tvnet);
			mempool_free(ctx, tvnet->tx_ctx_pool);
		} else {
			/* Ring full → defer to tx_complete_work for retry.
			 * No IRQ needed here; deferred worker will retry and raise
			 * IRQ when it succeeds.
			 */
			if (llist_add(&ctx->llnode, &tvnet->tx_complete_list))
				queue_work(system_unbound_wq,
					   &tvnet->tx_complete_work.work);
		}
	} else {
		mempool_free(ctx, tvnet->tx_ctx_pool);
	}

	/* Wake queue if stopped */
	if (netif_queue_stopped(ndev) && tvnet->os_link_state == OS_LINK_STATE_UP) {
		if (tvnet_ivc_rd_available(&tvnet->h2ep_empty) &&
		    !tvnet_ivc_full(&tvnet->h2ep_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}
}

static void tvnet_host_dma_complete_batch(void *priv,
					  tegra_pcie_dma_status_t status)
{
	struct tvnet_dma_tx_batch *batch = priv;
	struct tvnet_priv *tvnet = batch->tvnet;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct device *d = &tvnet->pdev->dev;
	struct net_device *ndev = NULL;
	unsigned int posted = 0;
	int i;

	for (i = 0; i < batch->nents; i++) {
		struct tvnet_dma_tx_ctx *ctx = batch->ctx[i];
		struct sk_buff *skb = ctx->skb;

		ndev = skb->dev;

		if (status != TEGRA_PCIE_DMA_SUCCESS) {
			ndev->stats.tx_errors++;
		} else {
			ndev->stats.tx_packets++;
			ndev->stats.tx_bytes += ctx->len;
		}

		dma_unmap_single(d, ctx->src_iova, ctx->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
		ctx->skb = NULL;
		ctx->src_iova = 0;

		if (status == TEGRA_PCIE_DMA_SUCCESS) {
			if (tvnet_host_try_post_h2ep_full(tvnet, ctx->len,
							  ctx->dst_iova,
							  ctx->cookie)) {
				posted++;
				mempool_free(ctx, tvnet->tx_ctx_pool);
			} else {
				/* Defer to tx_complete_work for retry */
				if (llist_add(&ctx->llnode, &tvnet->tx_complete_list))
					queue_work(system_unbound_wq,
						   &tvnet->tx_complete_work.work);
			}
		} else {
			mempool_free(ctx, tvnet->tx_ctx_pool);
		}
	}

	desc_cnt->rd_cnt += batch->nents;

	/* Raise DATA IRQ once per batch if any entries were posted.
	 * This coalesces up to 64 ring updates into a single interrupt.
	 */
	if (posted)
		tvnet_host_raise_ep_data_irq(tvnet);

	if (status != TEGRA_PCIE_DMA_SUCCESS)
		dev_err(d, "DMA transfer failed (batch nents=%u): %d\n",
			batch->nents, status);

	if (ndev && netif_queue_stopped(ndev) &&
	    tvnet->os_link_state == OS_LINK_STATE_UP) {
		if (tvnet_ivc_rd_available(&tvnet->h2ep_empty) &&
		    !tvnet_ivc_full(&tvnet->h2ep_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}

	mempool_free(batch, tvnet->tx_batch_pool);
}

/* Initialize DMA using tegra-pcie-dma wrapper library for remote DMA read */
static int tvnet_host_dma_init(struct tvnet_priv *tvnet)
{
	struct tegra_pcie_dma_init_info dma_info = {0};
	struct tegra_pcie_dma_remote_info remote_info = {0};
	tegra_pcie_dma_status_t status;
	struct device *dev = &tvnet->pdev->dev;
	int desc_bar;
	u64 msi_addr = 0;
	u32 msi_data = 0;

	/* Setup remote DMA info - EP's DMA controller */
	remote_info.dma_phy_base = pci_resource_start(tvnet->pdev,
						      (tvnet->ep_soc_id ==
						       TEGRA_VNET_SOC_T264) ?
						      0 : 4);
	remote_info.dma_size = pci_resource_len(tvnet->pdev,
						(tvnet->ep_soc_id ==
						 TEGRA_VNET_SOC_T264) ?
						0 : 4);
	dma_info.remote = &remote_info;

	/* Configure RX channel 0 (read channel for RP->EP transfers) */
	dma_info.rx[0].ch_type = TEGRA_PCIE_DMA_CHAN_XFER_ASYNC;
	dma_info.rx[0].num_descriptors = DMA_DESC_COUNT;
	/* Descriptor physical base from host's perspective: BAR base + offset */
	desc_bar = (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) ? 2 : 0;
	dma_info.rx[0].desc_phy_base = pci_resource_start(tvnet->pdev, desc_bar) +
				       tvnet->bar_md->host_dma_offset;
	dma_info.rx[0].desc_iova = tvnet->bar_md->bar0_base_phy +
				   tvnet->bar_md->host_dma_offset;

	dev_info(dev,
		 "DMA desc config: num=%d, phy_base=0x%llx, iova=0x%llx, offset=0x%x, size=%d\n",
		 dma_info.rx[0].num_descriptors,
		 dma_info.rx[0].desc_phy_base,
		 dma_info.rx[0].desc_iova,
		 tvnet->bar_md->host_dma_offset,
		 tvnet->bar_md->host_dma_size);

	dma_info.dev = dev;
	dma_info.soc = (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) ?
			NVPCIE_DMA_SOC_T264 :
			NVPCIE_DMA_SOC_T234;

	/* MSI configuration */
	if (tvnet->ep_soc_id == TEGRA_VNET_SOC_T234 && tvnet->msix_tbl) {
		/* T234: Read MSI-X table for DMA interrupt configuration */
		msi_addr = readl(tvnet->msix_tbl + PCI_MSIX_ENTRY_UPPER_ADDR);
		msi_addr = (msi_addr << 32) | readl(tvnet->msix_tbl + PCI_MSIX_ENTRY_LOWER_ADDR);
		msi_data = readw(tvnet->msix_tbl + PCI_MSIX_ENTRY_DATA);
		dma_info.msi_addr = msi_addr;
		dma_info.msi_data = msi_data;
		dev_info(dev, "T234 MSI-X: addr=0x%llx data=0x%x\n", msi_addr, msi_data);
	} else if (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) {
		/* T264: MSI configuration will be set after init using set_msi API */
		dma_info.msi_addr = 0;
		dma_info.msi_data = 0;
		dma_info.msi_irq = pci_irq_vector(tvnet->pdev, TEGRA264_PCIE_DMA_MSI_REMOTE_VEC);
		dev_info(dev, "T264 MSI will be configured after init\n");
	}

	/* Initialize DMA library */
	status = tegra_pcie_dma_initialize(&dma_info, &tvnet->dma_cookie);
	if (status != TEGRA_PCIE_DMA_SUCCESS) {
		dev_err(dev, "tegra_pcie_dma_initialize() failed: %d\n", status);
		return -EIO;
	}

	/* For T264, configure MSI using set_msi API */
	if (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) {
		struct msi_msg msg;

		/* Get MSI message for vector 1 (data interrupt) */
		get_cached_msi_msg(dma_info.msi_irq, &msg);
		msi_addr = ((u64)msg.address_hi << 32) | msg.address_lo;
		msi_data = msg.data + TEGRA264_PCIE_DMA_MSI_REMOTE_VEC;

		dev_info(dev, "T264 MSI: addr=0x%llx data=0x%x\n", msi_addr, msi_data);

		status = tegra_pcie_dma_set_msi(tvnet->dma_cookie, msi_addr, msi_data);
		if (status != TEGRA_PCIE_DMA_SUCCESS) {
			dev_err(dev, "tegra_pcie_dma_set_msi() failed: %d\n", status);
			tegra_pcie_dma_deinit(&tvnet->dma_cookie);
			return -EIO;
		}
	}

	dev_info(dev, "DMA library initialized (remote %s DMA)\n",
		 (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) ? "T264" : "T234");

	return 0;
}
#else
/* Non-DMA mode helper used from ndo_start_xmit(). Must NOT sleep.
 * (This codepath is currently disabled by ENABLE_DMA=1 in tegra_vnet.h,
 * but keep it buildable.)
 */
static int tvnet_host_poll_h2ep_full_space(struct tvnet_priv *tvnet,
					   struct net_device *ndev)
{
	if (tvnet_ivc_full(&tvnet->h2ep_full)) {
		/* Nudge endpoint to drain the ring */
		tvnet_host_raise_ep_data_irq(tvnet);
		ndev->stats.tx_dropped++;
		return -ENOSPC;
	}

	return 0;
}
#endif

static void tvnet_host_raise_ep_ctrl_irq(struct tvnet_priv *tvnet)
{
	struct irq_md *irq = &tvnet->bar_md->irq_ctrl;

	if (irq->irq_type == IRQ_SIMPLE) {
		/* Can write any value to generate sync point irq */
		writel(0x1, tvnet->mmio_base + irq->irq_addr);
		/* BAR0 mmio address is wc mem, add mb to make sure
		 * multiple interrupt writes are not combined.
		 */
		mb();
	} else {
		pr_err("%s: invalid irq type: %d\n", __func__, irq->irq_type);
	}
}

static void tvnet_host_raise_ep_data_irq(struct tvnet_priv *tvnet)
{
	struct irq_md *irq = &tvnet->bar_md->irq_data;

	if (irq->irq_type == IRQ_SIMPLE) {
		/* Can write any value to generate sync point irq */
		writel(0x1, tvnet->mmio_base + irq->irq_addr);
		/* BAR0 mmio address is wc mem, add mb to make sure
		 * multiple interrupt writes are not combined.
		 */
		mb();
	} else {
		pr_err("%s: invalid irq type: %d\n", __func__, irq->irq_type);
	}
}

static void tvnet_host_read_ctrl_msg(struct tvnet_priv *tvnet,
				     struct ctrl_msg *msg)
{
	struct ep_ring_buf *ep_mem = &tvnet->ep_mem;
	struct ctrl_msg *ctrl_msg = ep_mem->ep2h_ctrl_msgs;
	u32 idx;

	if (tvnet_ivc_empty(&tvnet->ep2h_ctrl)) {
		pr_debug("%s: EP2H ctrl ring is empty\n", __func__);
		return;
	}

	idx = tvnet_ivc_get_rd_cnt(&tvnet->ep2h_ctrl) % RING_COUNT;
	memcpy(msg, &ctrl_msg[idx], sizeof(*msg));
	tvnet_ivc_advance_rd(&tvnet->ep2h_ctrl);
}

/* TODO Handle error case */
static int tvnet_host_write_ctrl_msg(struct tvnet_priv *tvnet,
				     struct ctrl_msg *msg)
{
	struct host_ring_buf *host_mem = &tvnet->host_mem;
	struct ctrl_msg *ctrl_msg = host_mem->h2ep_ctrl_msgs;
	u32 idx;

	if (tvnet_ivc_full(&tvnet->h2ep_ctrl)) {
		/* Raise an interrupt to let EP process H2EP ring */
		tvnet_host_raise_ep_ctrl_irq(tvnet);
		pr_info("%s: H2EP ctrl ring is full\n", __func__);
		return -EAGAIN;
	}

	idx = tvnet_ivc_get_wr_cnt(&tvnet->h2ep_ctrl) % RING_COUNT;
	memcpy(&ctrl_msg[idx], msg, sizeof(*msg));
	/* BAR0 mmio address is wc mem, add mb to make sure ctrl msg is written
	 * before updating counters.
	 */
	mb();
	tvnet_ivc_advance_wr(&tvnet->h2ep_ctrl);
	tvnet_host_raise_ep_ctrl_irq(tvnet);

	return 0;
}

static void tvnet_host_alloc_empty_buffers(struct tvnet_priv *tvnet)
{
	struct net_device *ndev = tvnet->ndev;
	struct host_ring_buf *host_mem = &tvnet->host_mem;
	struct data_msg *ep2h_empty_msg = host_mem->ep2h_empty_msgs;
	struct ep2h_empty_list *ep2h_empty_ptr;
	struct device *d = &tvnet->pdev->dev;
	unsigned int iter = 0;
	unsigned int posted = 0;
	bool kicked = false;

	while (!tvnet_ivc_full(&tvnet->ep2h_empty)) {
		struct sk_buff *skb;
		dma_addr_t iova;
		u32 len = ndev->mtu + ETH_HLEN;
		u32 idx;
		u16 cookie;
		bool pp_alloc = false;

		skb = NULL;
		iova = 0;

#if ENABLE_DMA && defined(CONFIG_PAGE_POOL)
		if (tvnet->rx_page_pool) {
			struct page *page;
			void *va;
			unsigned int buflen = PAGE_SIZE << tvnet->rx_pp_order;
			dma_addr_t dma_base;

			spin_lock_bh(&tvnet->rx_pp_lock);
			page = page_pool_dev_alloc_pages(tvnet->rx_page_pool);
			spin_unlock_bh(&tvnet->rx_pp_lock);

			if (!page) {
				pr_err("%s: page_pool alloc failed\n", __func__);
				break;
			}

			va = page_address(page);
			skb = build_skb(va, buflen);
			if (!skb) {
				page_pool_put_full_page(tvnet->rx_page_pool, page, false);
				pr_err("%s: build_skb failed\n", __func__);
				break;
			}

			skb_mark_for_recycle(skb);
			skb_reserve(skb, TVNET_PP_HEADROOM);

			dma_base = page_pool_get_dma_addr(page);
			iova = dma_base + TVNET_PP_HEADROOM;
			pp_alloc = true;
		}
#endif

		if (!skb) {
			skb = __netdev_alloc_skb(ndev, len, GFP_KERNEL);
			if (!skb)
				break;
			iova = dma_map_single(d, skb->data, len, DMA_FROM_DEVICE);
			if (dma_mapping_error(d, iova)) {
				pr_err("%s: dma map failed\n", __func__);
				dev_kfree_skb_any(skb);
				break;
			}
		}

		/* The PCIe link is stable and dependable,
		 * so it's not necessary to perform a software checksum.
		 */
		skb->ip_summed = CHECKSUM_UNNECESSARY;

		ep2h_empty_ptr = kmalloc(sizeof(*ep2h_empty_ptr), GFP_KERNEL);
		if (!ep2h_empty_ptr) {
			if (!pp_alloc)
				dma_unmap_single(d, iova, len, DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			break;
		}
		ep2h_empty_ptr->skb = skb;
		ep2h_empty_ptr->iova = iova;
		ep2h_empty_ptr->len = len;
#if ENABLE_DMA
		/* If page_pool is used, iova points to (dma_base + TVNET_PP_HEADROOM).
		 * Otherwise dma_base==iova and offset==0.
		 */
#ifdef CONFIG_PAGE_POOL
		ep2h_empty_ptr->dma_base = pp_alloc ? (iova - TVNET_PP_HEADROOM) : iova;
		ep2h_empty_ptr->offset = pp_alloc ? TVNET_PP_HEADROOM : 0;
#else
		ep2h_empty_ptr->dma_base = iova;
		ep2h_empty_ptr->offset = 0;
#endif
#endif
		if (tvnet_host_ep2h_cookie_alloc(tvnet, ep2h_empty_ptr, &cookie)) {
			pr_err("%s: rx cookie alloc failed\n", __func__);
			if (!pp_alloc)
				dma_unmap_single(d, iova, len, DMA_FROM_DEVICE);
			dev_kfree_skb_any(skb);
			kfree(ep2h_empty_ptr);
			break;
		}

		idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_empty) %
					RING_COUNT;
		ep2h_empty_msg[idx].u.empty_buffer.pcie_address = iova;
		ep2h_empty_msg[idx].u.empty_buffer.buffer_len = len;
		ep2h_empty_msg[idx].u.empty_buffer.cookie = cookie;
		/* BAR0 mmio address is wc mem, add mb to make sure that empty
		 * buffers are updated before updating counters.
		 */
		mb();
		tvnet_ivc_advance_wr(&tvnet->ep2h_empty);
		posted++;
		/* Coalesce doorbells: ring once immediately after the first
		 * posted EMPTY, so the peer can start consuming without waiting
		 * for the full refill loop to finish.
		 */
		if (!kicked) {
			tvnet_host_raise_ep_ctrl_irq(tvnet);
			kicked = true;
		}

		/* Avoid long uninterrupted CPU hogging during large refills */
		if (!(++iter & 0x1f))
			cond_resched();
	}

	/* Trace what actually limits EP->Host TX (EP consumes EP2H_EMPTY).
	 * If this runs rarely or posts very few entries, EP will print stop_no_empty.
	 */
}

static void tvnet_host_refill_work(struct work_struct *work)
{
	struct tvnet_priv *tvnet = container_of(work, struct tvnet_priv,
					       refill_work);

	if (tvnet->os_link_state != OS_LINK_STATE_UP)
		return;

	tvnet_host_alloc_empty_buffers(tvnet);
}

static void tvnet_host_ctrl_work(struct work_struct *work)
{
	struct tvnet_priv *tvnet = container_of(work, struct tvnet_priv, ctrl_work);
	struct net_device *ndev = tvnet->ndev;

	/* Wake TX queue if we have space again */
	if (netif_queue_stopped(ndev)) {
		if (tvnet->os_link_state == OS_LINK_STATE_UP &&
		    tvnet_ivc_rd_available(&tvnet->h2ep_empty) &&
		    !tvnet_ivc_full(&tvnet->h2ep_full)) {
			pr_debug("%s: wake net tx queue\n", __func__);
			netif_wake_queue(ndev);
		}
	}

	/* Process control ring (may run link-state machine & free buffers) */
	if (tvnet_ivc_rd_available(&tvnet->ep2h_ctrl))
		tvnet_host_process_ctrl_msg(tvnet);

	/* Refill RX empty buffers eagerly.
	 * Don't check tvnet_ivc_full() here because the counter read can be
	 * stale (EP may have consumed empties after our read). The refill
	 * worker itself will stop when it encounters a truly full ring.
	 */
	if (tvnet->os_link_state == OS_LINK_STATE_UP)
		queue_work(system_unbound_wq, &tvnet->refill_work);

#if ENABLE_DMA
	/* If we have deferred TX completions because the FULL ring was
	 * temporarily full, retry immediately when we get control activity.
	 */
	if (READ_ONCE(tvnet->tx_complete_list.first) &&
	    !tvnet_ivc_full(&tvnet->h2ep_full))
		queue_work(system_unbound_wq, &tvnet->tx_complete_work.work);
#endif
}

static void tvnet_host_free_empty_buffers(struct tvnet_priv *tvnet)
{
	struct ep2h_empty_list *ep2h_empty_ptr, *temp;
	struct device *d = &tvnet->pdev->dev;
	unsigned long flags;
	u16 cookie;
	LIST_HEAD(local);

	/* Drain cookie map under lock; free/unmap outside lock. */
	spin_lock_irqsave(&tvnet->ep2h_cookie_lock, flags);
	for (cookie = 0; cookie < TVNET_RX_COOKIE_MAX; cookie++) {
		ep2h_empty_ptr = tvnet->ep2h_cookie_map[cookie];
		if (!ep2h_empty_ptr)
			continue;
		tvnet->ep2h_cookie_map[cookie] = NULL;
		__clear_bit(cookie, tvnet->ep2h_cookie_bmap);
		list_add(&ep2h_empty_ptr->list, &local);
	}
	spin_unlock_irqrestore(&tvnet->ep2h_cookie_lock, flags);

	list_for_each_entry_safe(ep2h_empty_ptr, temp, &local, list) {
		list_del(&ep2h_empty_ptr->list);
		if (!ep2h_empty_ptr->offset)
			dma_unmap_single(d, ep2h_empty_ptr->iova, ep2h_empty_ptr->len,
					 DMA_FROM_DEVICE);
		dev_kfree_skb_any(ep2h_empty_ptr->skb);
		kfree(ep2h_empty_ptr);
	}
}

static void tvnet_host_stop_tx_queue(struct tvnet_priv *tvnet)
{
	struct net_device *ndev = tvnet->ndev;

	netif_stop_queue(ndev);
	/* Get tx lock to make sure that there is no ongoing xmit */
	netif_tx_lock(ndev);
	netif_tx_unlock(ndev);
}

static void tvnet_host_stop_rx_work(struct tvnet_priv *tvnet)
{
	/* wait for interrupt handle to return to ensure rx is stopped */
	synchronize_irq(pci_irq_vector(tvnet->pdev, 1));
}

static void tvnet_host_clear_data_msg_counters(struct tvnet_priv *tvnet)
{
	struct host_ring_buf *host_mem = &tvnet->host_mem;
	struct host_own_cnt *host_cnt = host_mem->host_cnt;
	struct ep_ring_buf *ep_mem = &tvnet->ep_mem;
	struct ep_own_cnt *ep_cnt = ep_mem->ep_cnt;

	host_cnt->ep2h_empty_wr_cnt = 0;
	ep_cnt->ep2h_empty_rd_cnt = 0;
	host_cnt->h2ep_full_wr_cnt = 0;
	ep_cnt->h2ep_full_rd_cnt = 0;
}

static void tvnet_host_update_link_state(struct net_device *ndev,
					 enum os_link_state state)
{
	if (state == OS_LINK_STATE_UP) {
		netif_start_queue(ndev);
		netif_carrier_on(ndev);
	} else if (state == OS_LINK_STATE_DOWN) {
		netif_carrier_off(ndev);
		netif_stop_queue(ndev);
	} else {
		pr_err("%s: invalid state: %d\n", __func__, state);
	}
}

/* OS link state machine */
static void tvnet_host_update_link_sm(struct tvnet_priv *tvnet)
{
	struct net_device *ndev = tvnet->ndev;
	enum os_link_state old_state = tvnet->os_link_state;

	if (tvnet->rx_link_state == DIR_LINK_STATE_UP &&
	    tvnet->tx_link_state == DIR_LINK_STATE_UP)
		tvnet->os_link_state = OS_LINK_STATE_UP;
	else
		tvnet->os_link_state = OS_LINK_STATE_DOWN;

	if (tvnet->os_link_state != old_state)
		tvnet_host_update_link_state(ndev, tvnet->os_link_state);
}

/* One way link state machine*/
static void tvnet_host_user_link_up_req(struct tvnet_priv *tvnet)
{
	struct ctrl_msg msg = {};

	tvnet_host_clear_data_msg_counters(tvnet);
	tvnet_host_alloc_empty_buffers(tvnet);
	msg.msg_id = CTRL_MSG_LINK_UP;
	tvnet_host_write_ctrl_msg(tvnet, &msg);
	tvnet->rx_link_state = DIR_LINK_STATE_UP;
	tvnet_host_update_link_sm(tvnet);
}

static void tvnet_host_user_link_down_req(struct tvnet_priv *tvnet)
{
	struct ctrl_msg msg = {};

	tvnet->rx_link_state = DIR_LINK_STATE_SENT_DOWN;
	msg.msg_id = CTRL_MSG_LINK_DOWN;
	tvnet_host_write_ctrl_msg(tvnet, &msg);
	tvnet_host_update_link_sm(tvnet);
}

static void tvnet_host_rcv_link_up_msg(struct tvnet_priv *tvnet)
{
	tvnet->tx_link_state = DIR_LINK_STATE_UP;
	tvnet_host_update_link_sm(tvnet);
}

static void tvnet_host_rcv_link_down_msg(struct tvnet_priv *tvnet)
{
	struct ctrl_msg msg = {};

	/* Stop using empty buffers of remote system */
	tvnet_host_stop_tx_queue(tvnet);
	msg.msg_id = CTRL_MSG_LINK_DOWN_ACK;
	tvnet_host_write_ctrl_msg(tvnet, &msg);
	tvnet->tx_link_state = DIR_LINK_STATE_DOWN;
	tvnet_host_update_link_sm(tvnet);
}

static void tvnet_host_rcv_link_down_ack(struct tvnet_priv *tvnet)
{
	/* Stop using empty buffers(which are full in rx) of local system */
	tvnet_host_stop_rx_work(tvnet);
	tvnet_host_free_empty_buffers(tvnet);
	tvnet->rx_link_state = DIR_LINK_STATE_DOWN;
	wake_up_interruptible(&tvnet->link_state_wq);
	tvnet_host_update_link_sm(tvnet);
}

static int tvnet_host_open(struct net_device *ndev)
{
	struct tvnet_priv *tvnet = netdev_priv(ndev);

	mutex_lock(&tvnet->link_state_lock);
	if (tvnet->rx_link_state == DIR_LINK_STATE_DOWN)
		tvnet_host_user_link_up_req(tvnet);
	napi_enable(&tvnet->napi);
	mutex_unlock(&tvnet->link_state_lock);

	return 0;
}

static int tvnet_host_close(struct net_device *ndev)
{
	struct tvnet_priv *tvnet = netdev_priv(ndev);
	int ret = 0;

	mutex_lock(&tvnet->link_state_lock);
	napi_disable(&tvnet->napi);
	if (tvnet->rx_link_state == DIR_LINK_STATE_UP)
		tvnet_host_user_link_down_req(tvnet);

	ret = wait_event_interruptible_timeout(tvnet->link_state_wq,
					       (tvnet->rx_link_state ==
					       DIR_LINK_STATE_DOWN),
					       msecs_to_jiffies(LINK_TIMEOUT));
	ret = (ret > 0) ? 0 : -ETIMEDOUT;
	if (ret < 0) {
		pr_err("%s: link state machine failed: tx_state: %d rx_state: %d err: %d",
		       __func__, tvnet->tx_link_state, tvnet->rx_link_state,
		       ret);
		tvnet->rx_link_state = DIR_LINK_STATE_UP;
	}
	mutex_unlock(&tvnet->link_state_lock);

	return ret;
}

static int tvnet_host_change_mtu(struct net_device *ndev, int new_mtu)
{
	bool set_down = false;

	if (new_mtu > TVNET_MAX_MTU || new_mtu < TVNET_MIN_MTU) {
		pr_err("MTU range is %d to %d\n", TVNET_MIN_MTU,
		       TVNET_MAX_MTU);
		return -EINVAL;
	}

	if (netif_running(ndev)) {
		set_down = true;
		tvnet_host_close(ndev);
	}

	pr_info("changing MTU from %d to %d\n", ndev->mtu, new_mtu);
	ndev->mtu = new_mtu;

	if (set_down)
		tvnet_host_open(ndev);

	return 0;
}

static netdev_tx_t tvnet_host_start_xmit(struct sk_buff *skb,
					 struct net_device *ndev)
{
	struct tvnet_priv *tvnet = netdev_priv(ndev);
	struct skb_shared_info *info = skb_shinfo(skb);
	struct ep_ring_buf *ep_mem = &tvnet->ep_mem;
	struct data_msg *h2ep_empty_msg = ep_mem->h2ep_empty_msgs;
	struct device *d = &tvnet->pdev->dev;
#if ENABLE_DMA
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct tvnet_dma_tx_ctx *tx_ctx;
	struct netdev_queue *txq;
	unsigned long flags;
	bool flush_now;
	bool flush_reason_stopped;
	bool flush_reason_max;
	u8 batch_pending;
	u32 delay_us;
#endif
	dma_addr_t src_iova;
	dma_addr_t dst_iova;
	u32 rd_idx;
	u16 dst_cookie;
	void *dst_virt;
	u32 len;

	/* TODO Not expecting skb frags, remove this after testing */
	WARN_ON(info->nr_frags);

	/* Check if H2EP_EMPTY_BUF available to read */
	if (!tvnet_ivc_rd_available(&tvnet->h2ep_empty)) {
		tvnet_host_raise_ep_ctrl_irq(tvnet);
		pr_debug("%s: No H2EP empty msg, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	/* Check if H2EP_FULL_BUF available to write */
	if (tvnet_ivc_full(&tvnet->h2ep_full)) {
		tvnet_host_raise_ep_ctrl_irq(tvnet);
		pr_debug("%s: No H2EP full buf, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

#if ENABLE_DMA
	/* Check if dma desc available */
	batch_pending = READ_ONCE(tvnet->tx_batch_cnt);
	if (batch_pending >= TVNET_TX_BATCH_MAX) {
		/* Batch array is full; flush first to avoid OOB. */
		queue_delayed_work(system_highpri_wq,
				   &tvnet->tx_batch_flush_work, 0);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}
	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + batch_pending) >= DMA_DESC_COUNT) {
		pr_debug("%s: dma descriptors are not available\n", __func__);
		/* Flush any pending batch so completions can start immediately. */
		if (batch_pending)
			queue_delayed_work(system_highpri_wq,
					   &tvnet->tx_batch_flush_work, 0);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	/* Guard total consumed EMPTY buffers from EP.
	 * EP provided RING_COUNT empties initially. We've consumed empties for:
	 * - batch_pending (queued, not yet DMA submitted)
	 * - dma_inflight (submitted to DMA, not yet completed)
	 * - h2ep_full ring entries (DMA completed, waiting for EP to drain)
	 * Total consumed must stay < RING_COUNT or we'll exhaust EP's EMPTY pool.
	 */
	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + batch_pending +
	     tvnet_ivc_rd_available(&tvnet->h2ep_full) + 1) >= RING_COUNT) {
		pr_debug("%s: Total consumed EMPTYs would exceed RING_COUNT\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}
#endif

	len = skb_headlen(skb);

	src_iova = dma_map_single(d, skb->data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(d, src_iova)) {
		pr_err("%s: dma_map_single failed\n", __func__);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* Get H2EP empty msg */
	rd_idx = tvnet_ivc_get_rd_cnt(&tvnet->h2ep_empty) %
				RING_COUNT;
	dst_iova = h2ep_empty_msg[rd_idx].u.empty_buffer.pcie_address;
	dst_cookie = h2ep_empty_msg[rd_idx].u.empty_buffer.cookie;
	dst_virt = (__force void *)tvnet->mmio_base + (dst_iova - tvnet->bar_md->bar0_base_phy);
	/* Advance read count after all failure cases completed, to avoid
	 * dangling buffer at endpoint.
	 */
	tvnet_ivc_advance_rd(&tvnet->h2ep_empty);
	/* Raise an interrupt to let EP populate H2EP_EMPTY_BUF ring */
	tvnet_host_raise_ep_ctrl_irq(tvnet);

#if ENABLE_DMA
	/* Submit DMA transfer using tegra-pcie-dma wrapper */
	/* Allocate context for callback */
	tx_ctx = mempool_alloc(tvnet->tx_ctx_pool, GFP_ATOMIC);
	if (!tx_ctx) {
		pr_err("%s: Failed to allocate TX context\n", __func__);
		dma_unmap_single(d, src_iova, len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	tx_ctx->skb = skb;
	tx_ctx->tvnet = tvnet;
	tx_ctx->src_iova = src_iova;
	tx_ctx->dst_iova = dst_iova;
	tx_ctx->len = len;
	tx_ctx->cookie = dst_cookie;

	spin_lock_irqsave(&tvnet->tx_batch_lock, flags);
	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + tvnet->tx_batch_cnt)
		>= DMA_DESC_COUNT) {
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
		mempool_free(tx_ctx, tvnet->tx_ctx_pool);
		dma_unmap_single(d, src_iova, len, DMA_TO_DEVICE);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	tvnet->tx_batch_ctx[tvnet->tx_batch_cnt] = tx_ctx;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].src = src_iova;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].dst = dst_iova;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].sz = len;
	tvnet->tx_batch_cnt++;
	if (tvnet->tx_batch_cnt == 1) {
		/* If the stack indicates a burst is coming (xmit_more),
		 * give it a bit more time to accumulate so we avoid
		 * tiny (nents=1) submits in the middle of a bulk stream.
		 */
		delay_us = netdev_xmit_more() ?
			TVNET_TX_BATCH_DELAY_MORE_US : TVNET_TX_BATCH_DELAY_US;
		hrtimer_start(&tvnet->tx_batch_timer,
			      ns_to_ktime(delay_us * NSEC_PER_USEC),
			      HRTIMER_MODE_REL);
	}

	txq = netdev_get_tx_queue(ndev, skb_get_queue_mapping(skb));
	flush_reason_stopped = netif_xmit_stopped(txq);

	flush_reason_max = tvnet->tx_batch_cnt >= TVNET_TX_BATCH_MAX;
	flush_now = (flush_reason_stopped || flush_reason_max);
	if (!flush_now) {
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
		return NETDEV_TX_OK;
	}

	/* Best-effort cancel; OK if already fired. */
	hrtimer_try_to_cancel(&tvnet->tx_batch_timer);

	if (flush_reason_stopped)
		tvnet->tx_dma_flush_txq_stopped++;
	if (flush_reason_max)
		tvnet->tx_dma_flush_batch_max++;

	spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
	queue_delayed_work(system_highpri_wq, &tvnet->tx_batch_flush_work, 0);
#else
	/* Copy skb->data to endpoint dst address, use CPU virt addr */
	memcpy(dst_virt, skb->data, len);
	/* BAR0 mmio address is wc mem, add mb to make sure that complete
	 * skb->data is written before updating counters.
	 */
	mb();

	/* For non-DMA mode, push dst to H2EP full ring immediately after memcpy */
	/* Poll for space in h2ep_full ring before writing */
	if (tvnet_host_poll_h2ep_full_space(tvnet, ndev) == 0) {
		struct host_ring_buf *host_mem = &tvnet->host_mem;
		struct data_msg *h2ep_full_msg = host_mem->h2ep_full_msgs;
		u32 wr_idx;

		wr_idx = tvnet_ivc_get_wr_cnt(&tvnet->h2ep_full) %
					RING_COUNT;
		h2ep_full_msg[wr_idx].u.full_buffer.packet_size = len;
		h2ep_full_msg[wr_idx].u.full_buffer.pcie_address = dst_iova;
		h2ep_full_msg[wr_idx].u.full_buffer.cookie = dst_cookie;
		h2ep_full_msg[wr_idx].msg_id = DATA_MSG_FULL_BUF;
		/* BAR0 mmio address is wc mem, add mb to make sure that full
		 * buffer is written before updating counters.
		 */
		mb();
		tvnet_ivc_advance_wr(&tvnet->h2ep_full);
		tvnet_host_raise_ep_data_irq(tvnet);
	}
	/* else: error already logged and tx_dropped incremented by poll function */

	/* Free skb - for non-DMA mode */
	dma_unmap_single(d, src_iova, len, DMA_TO_DEVICE);
	dev_kfree_skb_any(skb);
#endif
	return NETDEV_TX_OK;
}

static const struct net_device_ops tvnet_host_netdev_ops = {
	.ndo_open = tvnet_host_open,
	.ndo_stop = tvnet_host_close,
	.ndo_start_xmit	= tvnet_host_start_xmit,
	.ndo_change_mtu = tvnet_host_change_mtu,
};

static void tvnet_host_setup_bar0_md(struct tvnet_priv *tvnet)
{
	struct ep_ring_buf *ep_mem = &tvnet->ep_mem;
	struct host_ring_buf *host_mem = &tvnet->host_mem;

	tvnet->bar_md = (__force struct bar_md *)tvnet->mmio_base;

	ep_mem->ep_cnt = (__force struct ep_own_cnt *)(tvnet->mmio_base +
					tvnet->bar_md->ep_own_cnt_offset);
	ep_mem->ep2h_ctrl_msgs = (__force struct ctrl_msg *)(tvnet->mmio_base +
					tvnet->bar_md->ctrl_md.ep2h_offset);
	ep_mem->ep2h_full_msgs = (__force struct data_msg *)(tvnet->mmio_base +
					tvnet->bar_md->ep2h_md.ep2h_offset);
	ep_mem->h2ep_empty_msgs = (__force struct data_msg *)(tvnet->mmio_base +
					tvnet->bar_md->h2ep_md.ep2h_offset);

	host_mem->host_cnt = (__force struct host_own_cnt *)(tvnet->mmio_base +
					tvnet->bar_md->host_own_cnt_offset);
	host_mem->h2ep_ctrl_msgs = (__force struct ctrl_msg *)(tvnet->mmio_base +
					tvnet->bar_md->ctrl_md.h2ep_offset);
	host_mem->ep2h_empty_msgs = (__force struct data_msg *)(tvnet->mmio_base +
					tvnet->bar_md->ep2h_md.h2ep_offset);
	host_mem->h2ep_full_msgs = (__force struct data_msg *)(tvnet->mmio_base +
					tvnet->bar_md->h2ep_md.h2ep_offset);

	tvnet->h2ep_ctrl.rd = &ep_mem->ep_cnt->h2ep_ctrl_rd_cnt;
	tvnet->h2ep_ctrl.wr = &host_mem->host_cnt->h2ep_ctrl_wr_cnt;
	tvnet->ep2h_ctrl.rd = &host_mem->host_cnt->ep2h_ctrl_rd_cnt;
	tvnet->ep2h_ctrl.wr = &ep_mem->ep_cnt->ep2h_ctrl_wr_cnt;
	tvnet->h2ep_empty.rd = &host_mem->host_cnt->h2ep_empty_rd_cnt;
	tvnet->h2ep_empty.wr = &ep_mem->ep_cnt->h2ep_empty_wr_cnt;
	tvnet->h2ep_full.rd = &ep_mem->ep_cnt->h2ep_full_rd_cnt;
	tvnet->h2ep_full.wr = &host_mem->host_cnt->h2ep_full_wr_cnt;
	tvnet->ep2h_empty.rd = &ep_mem->ep_cnt->ep2h_empty_rd_cnt;
	tvnet->ep2h_empty.wr = &host_mem->host_cnt->ep2h_empty_wr_cnt;
	tvnet->ep2h_full.rd = &host_mem->host_cnt->ep2h_full_rd_cnt;
	tvnet->ep2h_full.wr = &ep_mem->ep_cnt->ep2h_full_wr_cnt;
}

static void tvnet_host_process_ctrl_msg(struct tvnet_priv *tvnet)
{
	struct ctrl_msg msg;

	while (tvnet_ivc_rd_available(&tvnet->ep2h_ctrl)) {
		tvnet_host_read_ctrl_msg(tvnet, &msg);
		if (msg.msg_id == CTRL_MSG_LINK_UP)
			tvnet_host_rcv_link_up_msg(tvnet);
		else if (msg.msg_id == CTRL_MSG_LINK_DOWN)
			tvnet_host_rcv_link_down_msg(tvnet);
		else if (msg.msg_id == CTRL_MSG_LINK_DOWN_ACK)
			tvnet_host_rcv_link_down_ack(tvnet);
	}
}

static int tvnet_host_process_ep2h_msg(struct tvnet_priv *tvnet)
{
	struct ep_ring_buf *ep_mem = &tvnet->ep_mem;
	struct data_msg *data_msg = ep_mem->ep2h_full_msgs;
	struct device *d = &tvnet->pdev->dev;
	struct ep2h_empty_list *ep2h_empty_ptr;
	struct net_device *ndev = tvnet->ndev;
	int count = 0;
	bool kick_ctrl = false;

	while ((count < TVNET_NAPI_WEIGHT) &&
	       tvnet_ivc_rd_available(&tvnet->ep2h_full)) {
		struct sk_buff *skb;
		u64 pcie_address;
		u32 len;
		u16 cookie;
		int idx;

		/* Read EP2H full msg */
		idx = tvnet_ivc_get_rd_cnt(&tvnet->ep2h_full) %
					RING_COUNT;
		len = data_msg[idx].u.full_buffer.packet_size;
		pcie_address = data_msg[idx].u.full_buffer.pcie_address;
		cookie = data_msg[idx].u.full_buffer.cookie;

		/* Advance H2EP full buffer after search in local list */
		tvnet_ivc_advance_rd(&tvnet->ep2h_full);

		ep2h_empty_ptr = tvnet_host_ep2h_cookie_take(tvnet, cookie);
		if (unlikely(!ep2h_empty_ptr)) {
			ep2h_empty_ptr =
				tvnet_host_ep2h_cookie_take_by_iova(tvnet,
								    (dma_addr_t)pcie_address);
			/* recovered */
		}
		if (WARN_ON(!ep2h_empty_ptr))
			continue;

		/* Nudge EP after draining FULL ring entries (coalesced). */
		kick_ctrl = true;

		if (ep2h_empty_ptr->offset) {
			dma_sync_single_range_for_cpu(d,
						      ep2h_empty_ptr->dma_base,
						      ep2h_empty_ptr->offset,
						      len, DMA_FROM_DEVICE);
		} else {
			dma_unmap_single(d, ep2h_empty_ptr->iova, ep2h_empty_ptr->len,
					 DMA_FROM_DEVICE);
		}
		skb = ep2h_empty_ptr->skb;
		skb_put(skb, len);
		skb->protocol = eth_type_trans(skb, ndev);
		napi_gro_receive(&tvnet->napi, skb);

		/* Free EP2H empty list element */
		kfree(ep2h_empty_ptr);
		count++;
	}

	if (kick_ctrl)
		tvnet_host_raise_ep_ctrl_irq(tvnet);

	return count;
}

static irqreturn_t tvnet_irq_ctrl(int irq, void *data)
{
	struct net_device *ndev = data;
	struct tvnet_priv *tvnet = netdev_priv(ndev);

	/* Control IRQ can lead to link-state transitions and buffer teardown,
	 * which may sleep (eg synchronize_irq()). Defer to process context.
	 */
	queue_work(system_unbound_wq, &tvnet->ctrl_work);

	return IRQ_HANDLED;
}

static irqreturn_t tvnet_irq_data(int irq, void *data)
{
	struct net_device *ndev = data;
	struct tvnet_priv *tvnet = netdev_priv(ndev);
	u32 avail = tvnet_ivc_rd_available(&tvnet->ep2h_full);

	if (avail) {
		disable_irq_nosync(pci_irq_vector(tvnet->pdev, 1));
		napi_schedule(&tvnet->napi);
	}

	return IRQ_HANDLED;
}

static int tvnet_host_poll(struct napi_struct *napi, int budget)
{
	struct tvnet_priv *tvnet = container_of(napi, struct tvnet_priv, napi);
	int work_done;

	work_done = tvnet_host_process_ep2h_msg(tvnet);
	if (work_done < budget) {
		napi_complete(napi);
		enable_irq(pci_irq_vector(tvnet->pdev, 1));
	}

	return work_done;
}

static int tvnet_host_probe(struct pci_dev *pdev,
			    const struct pci_device_id *pci_id)
{
	struct tvnet_priv *tvnet;
	struct net_device *ndev;
	int ret;

	dev_dbg(&pdev->dev, "%s: PCIe VID: 0x%x DID: 0x%x\n", __func__,
		pci_id->vendor, pci_id->device);
	ndev = alloc_etherdev(sizeof(struct tvnet_priv));
	if (!ndev) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "alloc_etherdev() failed");
		goto fail;
	}

	eth_hw_addr_random(ndev);
	SET_NETDEV_DEV(ndev, &pdev->dev);
	ndev->netdev_ops = &tvnet_host_netdev_ops;
	/* Enable software GRO to reduce per-packet RX cost(net_rx_action/NAPI).
	 * This is particularly important for high PPS workloads.
	 */
	ndev->hw_features |= NETIF_F_GRO;
	ndev->features |= NETIF_F_GRO;

	tvnet = netdev_priv(ndev);
	tvnet->ndev = ndev;
	tvnet->pdev = pdev;
	pci_set_drvdata(pdev, tvnet);

	/* Detect EP SoC type from device ID */
	if (pci_id->device == PCI_DEVICE_ID_NVIDIA_JETSON_THOR_NETWORK)
		tvnet->ep_soc_id = TEGRA_VNET_SOC_T264;
	else
		tvnet->ep_soc_id = TEGRA_VNET_SOC_T234;

	dev_info(&pdev->dev, "EP SoC: %s (Device ID: 0x%x)\n",
		 (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) ? "T264" : "T234", pci_id->device);

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "pci_enable_device() failed: %d\n", ret);
		goto free_netdev;
	}

#if defined(NV_PCI_ENABLE_PCIE_ERROR_REPORTING_PRESENT) /* Linux 6.5 */
	pci_enable_pcie_error_reporting(pdev);
#endif

	/* Map BARs based on EP SoC type
	 * T234 EP: BAR0=Shared mem, BAR2=MSI-X table, BAR4=DMA registers
	 * T264 EP: BAR0=DMA registers, BAR2=Shared mem, BAR4=Dummy
	 */
	/* Map shared memory BAR based on EP SoC type:
	 * T264: Host BAR2 = Shared memory
	 * T234: Host BAR0 = Shared memory
	 */
	{
		int bar_idx = (tvnet->ep_soc_id == TEGRA_VNET_SOC_T264) ? 2 : 0;

#if ENABLE_DMA
		tvnet->mmio_base = devm_ioremap(&pdev->dev,
						pci_resource_start(pdev, bar_idx),
						pci_resource_len(pdev, bar_idx));
#else
		tvnet->mmio_base = devm_ioremap_wc(&pdev->dev,
						   pci_resource_start(pdev, bar_idx),
						   pci_resource_len(pdev, bar_idx));
#endif
		if (!tvnet->mmio_base) {
			ret = -ENOMEM;
			dev_err(&pdev->dev, "BAR%d (shared mem) ioremap() failed\n", bar_idx);
			goto pci_disable;
		}
	}

	pci_set_master(pdev);
	pci_set_drvdata(pdev, tvnet);

	/* Setup BAR0 meta data */
	tvnet_host_setup_bar0_md(tvnet);

#if defined(NV_NETIF_NAPI_ADD_WEIGHT_PRESENT) /* Linux v6.1 */
	netif_napi_add_weight(ndev, &tvnet->napi, tvnet_host_poll, TVNET_NAPI_WEIGHT);
#else
	netif_napi_add(ndev, &tvnet->napi, tvnet_host_poll, TVNET_NAPI_WEIGHT);
#endif

	ndev->mtu = TVNET_DEFAULT_MTU;

	ret = register_netdev(ndev);
	if (ret) {
		dev_err(&pdev->dev, "register_netdev() fail: %d\n", ret);
		goto pci_disable;
	}
	netif_carrier_off(ndev);

	tvnet->rx_link_state = DIR_LINK_STATE_DOWN;
	tvnet->tx_link_state = DIR_LINK_STATE_DOWN;
	tvnet->os_link_state = OS_LINK_STATE_DOWN;
	mutex_init(&tvnet->link_state_lock);
	init_waitqueue_head(&tvnet->link_state_wq);

	ret = pci_alloc_irq_vectors(pdev, 8, 8, PCI_IRQ_MSIX | PCI_IRQ_MSI
								| PCI_IRQ_AFFINITY);
	if (ret <= 0) {
		dev_err(&pdev->dev, "pci_alloc_irq_vectors() fail: %d\n", ret);
		ret = -EIO;
		goto unreg_netdev;
	}

	ret = request_irq(pci_irq_vector(pdev, 0), tvnet_irq_ctrl, 0,
			  ndev->name, ndev);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq() fail: %d\n", ret);
		goto disable_msi;
	}

	ret = request_irq(pci_irq_vector(pdev, 1), tvnet_irq_data, 0,
			  ndev->name, ndev);
	if (ret < 0) {
		dev_err(&pdev->dev, "request_irq() fail: %d\n", ret);
		goto fail_request_irq_ctrl;
	}

#if ENABLE_DMA
	ret = tvnet_host_dma_init(tvnet);
	if (ret) {
		dev_err(&pdev->dev, "DMA initialization failed: %d\n", ret);
		goto fail_request_irq_data;
	}
#endif

	spin_lock_init(&tvnet->ep2h_cookie_lock);
	bitmap_zero(tvnet->ep2h_cookie_bmap, TVNET_RX_COOKIE_MAX);
	memset(tvnet->ep2h_cookie_map, 0, sizeof(tvnet->ep2h_cookie_map));

#if ENABLE_DMA
	init_llist_head(&tvnet->tx_complete_list);
	INIT_DELAYED_WORK(&tvnet->tx_complete_work, tvnet_host_tx_complete_work);
	INIT_DELAYED_WORK(&tvnet->tx_batch_flush_work, tvnet_host_tx_batch_flush_work);
#if defined(NV_HRTIMER_SETUP_PRESENT) /* Linux v6.13 */
	hrtimer_setup(&tvnet->tx_batch_timer, &tvnet_host_tx_batch_timer_fn,
		      CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
	hrtimer_init(&tvnet->tx_batch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tvnet->tx_batch_timer.function = tvnet_host_tx_batch_timer_fn;
#endif
	tvnet->tx_ctx_pool = mempool_create_kmalloc_pool(DMA_DESC_COUNT,
							 sizeof(struct tvnet_dma_tx_ctx));
	if (!tvnet->tx_ctx_pool) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "Failed to create tx_ctx_pool\n");
		goto fail_dma;
	}

	spin_lock_init(&tvnet->tx_batch_lock);
	tvnet->tx_batch_cnt = 0;
	tvnet->tx_batch_pool = mempool_create_kmalloc_pool(DMA_DESC_COUNT,
							   sizeof(struct tvnet_dma_tx_batch));
	if (!tvnet->tx_batch_pool) {
		ret = -ENOMEM;
		dev_err(&pdev->dev, "Failed to create tx_batch_pool\n");
		goto fail_tx_ctx_pool;
	}

#ifdef CONFIG_PAGE_POOL
	spin_lock_init(&tvnet->rx_pp_lock);
	tvnet->rx_pp_order = get_order(TVNET_MAX_MTU + ETH_HLEN +
				       TVNET_PP_HEADROOM +
				       SKB_DATA_ALIGN(sizeof(struct skb_shared_info)));
	{
		struct page_pool_params pp;

		tvnet_pp_init_params(&pp, &pdev->dev, ndev, tvnet->rx_pp_order);
		tvnet->rx_page_pool = page_pool_create(&pp);
		if (!tvnet->rx_page_pool)
			dev_warn(&pdev->dev, "RX page_pool disabled (create failed)\n");
	}
#endif
#endif
	INIT_WORK(&tvnet->refill_work, tvnet_host_refill_work);
	INIT_WORK(&tvnet->ctrl_work, tvnet_host_ctrl_work);

	return 0;

#if ENABLE_DMA
fail_tx_ctx_pool:
	mempool_destroy(tvnet->tx_ctx_pool);
	tvnet->tx_ctx_pool = NULL;
fail_dma:
	/* DMA was initialized successfully in this probe() instance. */
	if (tvnet->dma_cookie) {
		tegra_pcie_dma_status_t status;

		status = tegra_pcie_dma_deinit(&tvnet->dma_cookie);
		if (status != TEGRA_PCIE_DMA_SUCCESS)
			dev_err(&pdev->dev, "DMA deinit failed: %d\n", status);
	}
#endif
fail_request_irq_data:
	free_irq(pci_irq_vector(pdev, 1), ndev);
fail_request_irq_ctrl:
	free_irq(pci_irq_vector(pdev, 0), ndev);
disable_msi:
	pci_free_irq_vectors(pdev);
unreg_netdev:
	unregister_netdev(ndev);
pci_disable:
	netif_napi_del(&tvnet->napi);
	pci_disable_device(pdev);
free_netdev:
	free_netdev(ndev);
fail:
	return ret;
}

static void tvnet_host_remove(struct pci_dev *pdev)
{
	int ret = -1;
	struct tvnet_priv *tvnet = pci_get_drvdata(pdev);

	/* Stop the endpoint from initiating further DMA before tearing down and
	 * freeing RX buffers (which are DMA targets for EP writes).
	 */
	pci_clear_master(pdev);

	cancel_work_sync(&tvnet->ctrl_work);
	cancel_work_sync(&tvnet->refill_work);
#if ENABLE_DMA
	cancel_delayed_work_sync(&tvnet->tx_complete_work);
	cancel_delayed_work_sync(&tvnet->tx_batch_flush_work);
	hrtimer_cancel(&tvnet->tx_batch_timer);
	tvnet_host_drop_tx_batch(tvnet);

#endif

	if (tvnet->rx_link_state == DIR_LINK_STATE_UP)
		tvnet_host_user_link_down_req(tvnet);

	ret = wait_event_interruptible_timeout(tvnet->link_state_wq,
					       (tvnet->rx_link_state ==
						DIR_LINK_STATE_DOWN),
						msecs_to_jiffies(LINK_TIMEOUT));
	ret = (ret > 0) ? 0 : -ETIMEDOUT;
	if (ret < 0) {
		pr_err("%s: failed: tx_state: %d rx_state: %d err: %d", __func__,
		       tvnet->tx_link_state, tvnet->rx_link_state, ret);
		tvnet->rx_link_state = DIR_LINK_STATE_UP;
	}

#if ENABLE_DMA
	/* Deinitialize DMA library */
	if (tvnet->dma_cookie) {
		tegra_pcie_dma_status_t status;

		status = tegra_pcie_dma_deinit(&tvnet->dma_cookie);
		if (status != TEGRA_PCIE_DMA_SUCCESS)
			dev_err(&pdev->dev, "DMA deinit failed: %d\n", status);
	}
#endif

	free_irq(pci_irq_vector(pdev, 0), tvnet->ndev);
	free_irq(pci_irq_vector(pdev, 1), tvnet->ndev);
	pci_free_irq_vectors(pdev);

	/* Drain/free any remaining RX empty buffers now that IRQs are disabled. */
	tvnet_host_free_empty_buffers(tvnet);

	/* Now that IRQ handlers can't re-queue work, cancel again and drain any
	 * pending deferred completions.
	 */
	cancel_work_sync(&tvnet->ctrl_work);
	cancel_work_sync(&tvnet->refill_work);
#if ENABLE_DMA
	cancel_delayed_work_sync(&tvnet->tx_complete_work);
	tvnet_host_drop_tx_batch(tvnet);
	tvnet_host_drain_tx_complete_list(tvnet);
	mempool_destroy(tvnet->tx_batch_pool);
	tvnet->tx_batch_pool = NULL;

	mempool_destroy(tvnet->tx_ctx_pool);
	tvnet->tx_ctx_pool = NULL;
#endif

	unregister_netdev(tvnet->ndev);
	netif_napi_del(&tvnet->napi);
#if ENABLE_DMA && defined(CONFIG_PAGE_POOL)
	if (tvnet->rx_page_pool) {
		page_pool_destroy(tvnet->rx_page_pool);
		tvnet->rx_page_pool = NULL;
	}
#endif
	pci_disable_device(pdev);
	free_netdev(tvnet->ndev);
}

static int tvnet_host_suspend(struct pci_dev *pdev, pm_message_t state)
{
	struct tvnet_priv *tvnet = pci_get_drvdata(pdev);

	disable_irq(pci_irq_vector(tvnet->pdev, 1));

	if (tvnet->rx_link_state == DIR_LINK_STATE_UP) {
		tvnet_host_close(tvnet->ndev);
		tvnet->pm_closed = true;
	}

	return 0;
}

static int tvnet_host_resume(struct pci_dev *pdev)
{
	struct tvnet_priv *tvnet = pci_get_drvdata(pdev);
#if ENABLE_DMA
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;

	desc_cnt->wr_cnt = 0;
	desc_cnt->rd_cnt = 0;
	/* DMA already initialized in probe, no need to reinit */
#endif

	if (tvnet->pm_closed) {
		tvnet_host_open(tvnet->ndev);
		tvnet->pm_closed = false;
	}

	enable_irq(pci_irq_vector(tvnet->pdev, 1));

	return 0;
}

static const struct pci_device_id tvnet_host_pci_tbl[] = {
	/* Match by PCI class: Network Other (class 0x02, subclass 0x80) */
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_JETSON_THOR_NETWORK, PCI_ANY_ID, PCI_ANY_ID,
	  PCI_CLASS_NETWORK_OTHER << 8, 0xFFFF00 },
	{ PCI_VENDOR_ID_NVIDIA, PCI_DEVICE_ID_NVIDIA_JETSON_AGX_NETWORK, PCI_ANY_ID, PCI_ANY_ID,
		PCI_CLASS_NETWORK_OTHER << 8, 0xFFFF00 },
	{0,},
};
MODULE_DEVICE_TABLE(pci, tvnet_host_pci_tbl);

static struct pci_driver tvnet_pci_driver = {
	.name		= "tvnet",
	.id_table	= tvnet_host_pci_tbl,
	.probe		= tvnet_host_probe,
	.remove		= tvnet_host_remove,
#ifdef CONFIG_PM
	.suspend        = tvnet_host_suspend,
	.resume         = tvnet_host_resume,
#endif
};

module_pci_driver(tvnet_pci_driver);

MODULE_DESCRIPTION("PCI TEGRA VIRTUAL NETWORK DRIVER");
MODULE_AUTHOR("Manikanta Maddireddy <mmaddireddy@nvidia.com>");
MODULE_LICENSE("GPL v2");
