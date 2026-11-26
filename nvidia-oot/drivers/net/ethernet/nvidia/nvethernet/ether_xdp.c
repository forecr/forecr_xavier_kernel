// SPDX-License-Identifier: GPL-2.0-only
/* SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *
 * XDP/AF_XDP support for NvEthernet driver.
 */

#include "ether_xdp.h"
#include "ether_linux.h"

#ifdef CONFIG_XDP_SOCKETS
#include <linux/bpf.h>
#include <linux/moduleparam.h>
#include <linux/rtnetlink.h>
#include <linux/filter.h>
#include <linux/if_vlan.h>
#include <linux/smp.h>
#include <net/xdp.h>
#include <net/xdp_sock_drv.h>
#include <linux/netdevice.h>

/* xdpsock (libbpf/libxdp) puts TX payload at frame base; desc.addr DMA must
 * not add headroom. Set to 1 to add headroom (for apps that put payload past
 * headroom per AF_XDP frame layout).
 */
static bool xsk_tx_add_headroom;
module_param_named(xsk_tx_add_headroom, xsk_tx_add_headroom, bool, 0644);
MODULE_PARM_DESC(xsk_tx_add_headroom, "Add headroom to AF_XDP TX DMA (default: no, xdpsock uses frame base)");

static unsigned int xsk_tx_budget = 64;
module_param_named(xsk_tx_budget, xsk_tx_budget, uint, 0644);
MODULE_PARM_DESC(xsk_tx_budget, "Maximum AF_XDP TX descriptors submitted per TX NAPI poll (0 = NAPI budget)");

static bool ether_xdp_tx_len_valid(unsigned int len)
{
	return len > 0U && len <= ETHER_TX_MAX_BUFF_SIZE;
}

static int ether_xsk_restore_open(struct ether_priv_data *pdata, int err)
{
	int open_err;

	open_err = dev_open(pdata->ndev, NULL);
	if (open_err < 0) {
		netdev_err(pdata->ndev,
			   "Failed to restore interface after XSK setup error %d: %d\n",
			   err, open_err);
		return open_err;
	}

	return err;
}

int ether_xsk_validate_rx_buf_len(struct ether_priv_data *pdata,
				  unsigned int rx_buf_len)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int i;

	for (i = 0; i < osi_dma->num_dma_chans; i++) {
		struct ether_rx_napi *rx_napi;
		struct xsk_buff_pool *pool;
		unsigned int frame_size;
		unsigned int chan;

		chan = osi_dma->dma_chans[i];
		if (chan == ETHER_INVALID_CHAN_NUM ||
		    chan >= OSI_MGBE_MAX_NUM_CHANS)
			continue;

		rx_napi = pdata->rx_napi[chan];
		if (!rx_napi)
			continue;

		pool = READ_ONCE(rx_napi->xsk_pool);
		if (!pool)
			continue;

		frame_size = xsk_pool_get_rx_frame_size(pool);
		if (frame_size < rx_buf_len) {
			netdev_err(pdata->ndev,
				   "queue %u XSK frame size %u smaller than required RX buffer length %u\n",
				   i, frame_size, rx_buf_len);
			return -EOPNOTSUPP;
		}
	}

	return 0;
}

static void ether_xdp_tx_rollback(struct osi_tx_ring *tx_ring,
				  unsigned int tx_idx,
				  unsigned int desc_cnt)
{
	struct osi_tx_swcx *tx_swcx = tx_ring->tx_swcx + tx_idx;
	struct osi_tx_desc *tx_desc = tx_ring->tx_desc + tx_idx;

	tx_ring->cur_tx_idx = tx_idx;
	tx_ring->desc_cnt = desc_cnt;
	memset(&tx_ring->tx_pkt_cx, 0, sizeof(tx_ring->tx_pkt_cx));
	memset(tx_desc, 0, sizeof(*tx_desc));
	*tx_swcx = (struct osi_tx_swcx) {0};
}

static void ether_xdp_tx_rollback_batch(struct osi_tx_ring *tx_ring,
					unsigned int tx_idx,
					unsigned int ring_sz,
					unsigned int desc_cnt,
					unsigned int ndesc)
{
	unsigned int i;

	tx_ring->cur_tx_idx = tx_idx;
	tx_ring->desc_cnt = desc_cnt;
	memset(&tx_ring->tx_pkt_cx, 0, sizeof(tx_ring->tx_pkt_cx));

	for (i = 0; i < ndesc; i++) {
		unsigned int idx = (tx_idx + i) % ring_sz;

		memset(tx_ring->tx_desc + idx, 0, sizeof(*tx_ring->tx_desc));
		tx_ring->tx_swcx[idx] = (struct osi_tx_swcx) {0};
	}
}

static int ether_xdp_submit_frame(struct ether_priv_data *pdata,
				  unsigned int chan, struct xdp_frame *xdpf,
				  u32 swcx_flags, bool page_pool_dma,
				  bool xsk_dma, dma_addr_t xsk_dma_addr,
				  bool flush)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	struct osi_tx_ring *tx_ring;
	struct netdev_queue *txq;
	struct osi_tx_swcx *tx_swcx;
	struct osi_tx_pkt_cx *tx_pkt_cx;
	dma_addr_t dma_addr;
	unsigned int tx_idx;
	unsigned int txq_idx;
	unsigned int desc_cnt;
	int avail;

	if (unlikely(chan >= OSI_MGBE_MAX_NUM_CHANS ||
		     !ether_xdp_tx_len_valid(xdpf->len)))
		return -EINVAL;

	if (!pdata->tx_napi[chan])
		return -EINVAL;

	/* Use the netdev TX queue mapped to this DMA channel. This is the same
	 * queue lock that the network stack holds around ndo_start_xmit(), so
	 * XDP redirect/TX cannot race with normal SKB TX on the same queue.
	 */
	txq_idx = pdata->tx_napi[chan]->ndev_queue_index;
	txq = netdev_get_tx_queue(pdata->ndev, txq_idx);
	__netif_tx_lock_bh(txq);
	txq_trans_cond_update(txq);

	/* ether_close() marks the device down and takes every TX queue lock
	 * before freeing DMA rings. Check state and acquire the ring only after
	 * taking the same lock so teardown cannot pass its quiesce point while
	 * this submission still owns a ring reference.
	 */
	if (unlikely(!netif_running(pdata->ndev) ||
		     !netif_carrier_ok(pdata->ndev))) {
		__netif_tx_unlock_bh(txq);
		return -ENETDOWN;
	}

	tx_ring = READ_ONCE(osi_dma->tx_ring[chan]);
	if (unlikely(!tx_ring || !tx_ring->tx_swcx || !tx_ring->tx_desc)) {
		__netif_tx_unlock_bh(txq);
		return -ENETDOWN;
	}

	avail = ether_avail_txdesc_cnt(osi_dma, tx_ring);
	if (avail <= 0) {
		__netif_tx_unlock_bh(txq);
		return -EBUSY;
	}

	if (page_pool_dma) {
		struct page *page = virt_to_page(xdpf->data);

		dma_addr = page_pool_get_dma_addr(page) + offset_in_page(xdpf->data);
		dma_sync_single_for_device(pdata->dev, dma_addr, xdpf->len,
					   DMA_BIDIRECTIONAL);
		swcx_flags |= ETHER_TX_SWCX_XDP_TX_PAGE_POOL;
	} else if (xsk_dma) {
		dma_addr = xsk_dma_addr;
		if (unlikely(!dma_addr)) {
			__netif_tx_unlock_bh(txq);
			return -EINVAL;
		}
		swcx_flags |= ETHER_TX_SWCX_XDP_TX_XSK;
	} else {
		dma_addr = dma_map_single(pdata->dev, xdpf->data, xdpf->len,
					  DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(pdata->dev, dma_addr))) {
			__netif_tx_unlock_bh(txq);
			return -ENOMEM;
		}
	}

	tx_idx = tx_ring->cur_tx_idx;
	desc_cnt = tx_ring->desc_cnt;
	tx_swcx = tx_ring->tx_swcx + tx_idx;
	tx_pkt_cx = &tx_ring->tx_pkt_cx;

	memset(tx_pkt_cx, 0, sizeof(*tx_pkt_cx));
	tx_pkt_cx->flags |= OSI_PKT_CX_LEN;
	tx_pkt_cx->payload_len = xdpf->len;
	tx_pkt_cx->desc_cnt = 1;

	CLEAR_TX_SWCX_FLAGS(tx_swcx, 0);
	tx_swcx->flags |= swcx_flags;
	tx_swcx->buf_virt_addr = xdpf;
	tx_swcx->buf_phy_addr = dma_addr;
	tx_swcx->len = xdpf->len;
	tx_swcx->chan = chan;

	if (osi_hw_transmit(osi_dma, chan) < 0) {
		if (!page_pool_dma && !xsk_dma)
			dma_unmap_single(pdata->dev, dma_addr, xdpf->len,
					 DMA_TO_DEVICE);
		ether_xdp_tx_rollback(tx_ring, tx_idx, desc_cnt);
		__netif_tx_unlock_bh(txq);
		return -EIO;
	}

	if (flush &&
	    osi_dma->use_tx_usecs == OSI_ENABLE &&
	    atomic_read(&pdata->tx_napi[chan]->tx_usecs_timer_armed) ==
	    OSI_DISABLE) {
		atomic_set(&pdata->tx_napi[chan]->tx_usecs_timer_armed,
			   OSI_ENABLE);
		hrtimer_start(&pdata->tx_napi[chan]->tx_usecs_timer,
			      osi_dma->tx_usecs * NSEC_PER_USEC,
			      HRTIMER_MODE_REL);
	}

	__netif_tx_unlock_bh(txq);
	return 0;
}

void ether_xdp_tx_wake_queue(struct ether_priv_data *pdata, unsigned int chan)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	struct osi_tx_ring *tx_ring;
	struct netdev_queue *txq;
	unsigned int txq_idx;

	if (chan >= OSI_MGBE_MAX_NUM_CHANS || !pdata->tx_napi[chan])
		return;

	tx_ring = READ_ONCE(osi_dma->tx_ring[chan]);
	if (!tx_ring)
		return;
	txq_idx = pdata->tx_napi[chan]->ndev_queue_index;
	txq = netdev_get_tx_queue(pdata->ndev, txq_idx);

	if (netif_tx_queue_stopped(txq) &&
	    ether_avail_txdesc_cnt(osi_dma, tx_ring) > ETHER_TX_DESC_THRESHOLD) {
		netif_tx_wake_queue(txq);
		netdev_dbg(pdata->ndev, "Tx ring[%d] - waking Txq\n", chan);
	}
}

static enum ether_xdp_action ether_xdp_tx_buff(struct ether_priv_data *pdata,
					       unsigned int chan,
					       struct xdp_buff *xdp,
					       bool account_drop)
{
	struct ether_rx_napi *rx_napi = pdata->rx_napi[chan];
	struct xsk_buff_pool *xsk_pool = NULL;
	struct xdp_frame *xdpf;
	bool page_pool_dma = xdp->rxq->mem.type == MEM_TYPE_PAGE_POOL;
	bool xsk_dma = xdp->rxq->mem.type == MEM_TYPE_XSK_BUFF_POOL;
	dma_addr_t xsk_dma_addr = 0;

	if (xsk_dma) {
		xsk_pool = rx_napi->xsk_pool;
		if (unlikely(!xsk_pool)) {
			if (account_drop)
				ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
			return ETHER_XDP_DROP;
		}
		xsk_dma_addr = xsk_buff_xdp_get_dma(xdp);
		if (unlikely(!xsk_dma_addr)) {
			if (account_drop)
				ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
			return ETHER_XDP_DROP;
		}
	}

	xdpf = xdp_convert_buff_to_frame(xdp);
	if (unlikely(!xdpf)) {
		if (account_drop)
			ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
		return ETHER_XDP_DROP;
	}

	if (xsk_dma)
		xsk_buff_raw_dma_sync_for_device(xsk_pool, xsk_dma_addr,
						 xdpf->len);

	if (ether_xdp_submit_frame(pdata, chan, xdpf,
				   ETHER_TX_SWCX_XDP_FRAME |
				   ETHER_TX_SWCX_XDP_TX,
				   page_pool_dma, xsk_dma, xsk_dma_addr,
				   true) < 0) {
		xdp_return_frame_rx_napi(xdpf);
		if (account_drop)
			ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
		return ETHER_XDP_CONSUMED;
	}

	return ETHER_XDP_TX;
}

/**
 * @brief ether_xsk_pool_enable - Enable XSK pool for a queue
 *
 * DMA-maps the pool, switches xdp_rxq_info to MEM_TYPE_XSK_BUFF_POOL,
 * and stores the pool pointer. Caller must hold RTNL.
 * When interface is up, performs internal restart (dev_close, enable, dev_open)
 * so XSK can be attached without requiring userspace to bring interface down.
 *
 * @param pdata: Driver private data
 * @param pool: XSK buffer pool to enable
 * @param queue_id: Netdev queue index
 * @return 0 on success, negative errno on failure
 */
static int ether_xsk_pool_enable(struct ether_priv_data *pdata,
				 struct xsk_buff_pool *pool, u16 queue_id)
{
#ifndef ETHER_PAGE_POOL
	return -EOPNOTSUPP;
#else
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	struct ether_rx_napi *rx_napi;
	unsigned int chan;
	u32 frame_size;
	bool if_running;
	int err;

	if (queue_id >= osi_dma->num_dma_chans)
		return -EINVAL;

	chan = osi_dma->dma_chans[queue_id];
	rx_napi = pdata->rx_napi[chan];
	if (rx_napi->xsk_pool) {
		if (rx_napi->xsk_pool == pool)
			return 0;
		return -EBUSY;
	}

	/* MTU can change while the interface is down. Refresh the cached RX
	 * length before validating a pool that is attached before the next open.
	 */
	err = osi_set_rx_buf_len(osi_dma);
	if (err < 0)
		return -EINVAL;

	frame_size = xsk_pool_get_rx_frame_size(pool);
	if (frame_size < osi_dma->rx_buf_len) {
		netdev_err(pdata->ndev,
			   "XSK frame size %u smaller than RX buffer length %u\n",
			   frame_size, osi_dma->rx_buf_len);
		return -EOPNOTSUPP;
	}

	/* Stop a running interface only after all non-disruptive validation. */
	if_running = netif_running(pdata->ndev);
	if (if_running)
		dev_close(pdata->ndev);

	err = xsk_pool_dma_map(pool, pdata->dev, ETHER_XDP_RX_DMA_ATTR);
	if (err) {
		netdev_err(pdata->ndev, "Failed to map xsk pool\n");
		if (if_running)
			return ether_xsk_restore_open(pdata, err);
		return err;
	}

	/* xdp_rxq_info is registered in ether_open and unregistered in ether_close.
	 * After dev_close (if interface was up) or when interface is down, it is
	 * unregistered. Register it here so we can set the mem model.
	 */
	if (!xdp_rxq_info_is_reg(&rx_napi->xdp_rxq)) {
		err = xdp_rxq_info_reg(&rx_napi->xdp_rxq, pdata->ndev, queue_id,
				       rx_napi->napi.napi_id);
		if (err < 0) {
			xsk_pool_dma_unmap(pool, ETHER_XDP_RX_DMA_ATTR);
			if (if_running)
				return ether_xsk_restore_open(pdata, err);
			return err;
		}
	}

	xdp_rxq_info_unreg_mem_model(&rx_napi->xdp_rxq);
	err = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
					 MEM_TYPE_XSK_BUFF_POOL, NULL);
	if (err) {
		xsk_pool_dma_unmap(pool, ETHER_XDP_RX_DMA_ATTR);
		if (if_running)
			return ether_xsk_restore_open(pdata, err);
		return err;
	}

	xsk_pool_set_rxq_info(pool, &rx_napi->xdp_rxq);
	rx_napi->xsk_pool = pool;

	if (if_running) {
		err = dev_open(pdata->ndev, NULL);
		if (err < 0) {
			netdev_err(pdata->ndev,
				   "Failed to bring interface back up after XSK attach: %d\n",
				   err);
			if (rx_napi->xsk_pool == pool) {
				rx_napi->xsk_pool = NULL;
				xdp_rxq_info_unreg_mem_model(&rx_napi->xdp_rxq);
				xsk_pool_dma_unmap(pool, ETHER_XDP_RX_DMA_ATTR);
			}
			return ether_xsk_restore_open(pdata, err);
		}
	}

	return 0;
#endif
}

/**
 * @brief ether_xsk_pool_disable - Disable XSK pool for a queue
 *
 * Clears pool pointer, switches xdp_rxq_info back to page-pool memory,
 * and DMA-unmaps the pool.
 *
 * @param pdata: Driver private data
 * @param queue_id: Netdev queue index
 * @return 0 on success, negative errno on failure
 */
static int ether_xsk_pool_disable(struct ether_priv_data *pdata, u16 queue_id)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	struct ether_rx_napi *rx_napi;
	struct xsk_buff_pool *pool;
	unsigned int chan;
	int restore_err;
	int err = 0;

	if (queue_id >= osi_dma->num_dma_chans)
		return -EINVAL;

	chan = osi_dma->dma_chans[queue_id];
	rx_napi = pdata->rx_napi[chan];
	pool = rx_napi->xsk_pool;
	if (!pool)
		return 0;

	/* When interface is up, include down/up in the path. dev_close frees RX
	 * buffers; explicit XSK detach owns the pool pointer and DMA unmap so
	 * suspend/resume RX buffer handling does not silently detach the pool.
	 */
	if (netif_running(pdata->ndev)) {
		dev_close(pdata->ndev);
		rx_napi->xsk_pool = NULL;
		err = dev_open(pdata->ndev, NULL);
		if (err < 0) {
			netdev_err(pdata->ndev,
				   "Failed to bring interface back up after XSK detach: %d\n",
				   err);
			rx_napi->xsk_pool = pool;
			restore_err = dev_open(pdata->ndev, NULL);
			if (restore_err < 0)
				netdev_err(pdata->ndev,
					   "Failed to restore XSK attachment after detach error %d: %d\n",
					   err, restore_err);
			return err;
		}
		xsk_pool_dma_unmap(pool, ETHER_XDP_RX_DMA_ATTR);
		return 0;
	}

	if (!xdp_rxq_info_is_reg(&rx_napi->xdp_rxq))
		goto commit_detach;

	xdp_rxq_info_unreg_mem_model(&rx_napi->xdp_rxq);
	if (pdata->page_pool[chan])
		err = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
						 MEM_TYPE_PAGE_POOL,
						 pdata->page_pool[chan]);
	else
		err = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
						 MEM_TYPE_PAGE_SHARED, NULL);
	if (err) {
		restore_err =
			xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
						   MEM_TYPE_XSK_BUFF_POOL, NULL);
		if (restore_err == 0)
			xsk_pool_set_rxq_info(pool, &rx_napi->xdp_rxq);
		else
			netdev_err(pdata->ndev,
				   "Failed to restore XSK memory model after detach error %d: %d\n",
				   err, restore_err);
		return err;
	}

commit_detach:
	rx_napi->xsk_pool = NULL;
	xsk_pool_dma_unmap(pool, ETHER_XDP_RX_DMA_ATTR);

	return err;
}

/**
 * @brief ether_bpf - Handle XDP/BPF setup commands (ndo_bpf callback)
 *
 * Handles XDP_SETUP_PROG with hot-swap (no interface restart).
 * Handles XDP_SETUP_XSK_POOL for AF_XDP zero-copy RX.
 * Returns -EOPNOTSUPP for XDP_SETUP_PROG_HW.
 *
 * @param dev: Network device
 * @param bpf: BPF command and parameters
 * @return 0 on success, negative errno on failure
 */
int ether_bpf(struct net_device *dev, struct netdev_bpf *bpf)
{
	struct ether_priv_data *pdata = netdev_priv(dev);
	struct bpf_prog *old_prog;

	switch (bpf->command) {
	case XDP_SETUP_PROG:
		/* Hot-swap: replace prog under RCU; caller holds RTNL */
		old_prog = rtnl_dereference(pdata->xdp_prog);
		rcu_assign_pointer(pdata->xdp_prog, bpf->prog);
		if (old_prog)
			bpf_prog_put(old_prog);
		return 0;
	case XDP_SETUP_XSK_POOL:
		if (bpf->xsk.pool)
			return ether_xsk_pool_enable(pdata, bpf->xsk.pool,
						     bpf->xsk.queue_id);
		return ether_xsk_pool_disable(pdata, bpf->xsk.queue_id);
	case XDP_SETUP_PROG_HW:
		return -EOPNOTSUPP;
	default:
		return -EOPNOTSUPP;
	}
}

/**
 * @brief ether_xsk_wakeup - Wake up NAPI for XSK processing (ndo_xsk_wakeup callback)
 *
 * Wakes the given queue after user space has added buffers or descriptors.
 * RX refill still runs from RX NAPI. For TX, drain the XSK TX ring inline so
 * sendto(MSG_DONTWAIT) does not merely schedule TX NAPI and leave a control-loop
 * reply waiting for the next packet/interrupt before it reaches hardware.
 *
 * @param dev: Network device
 * @param queue_id: Queue index
 * @param flags: XDP_WAKEUP_RX and/or XDP_WAKEUP_TX
 * @return 0 on success
 */
int ether_xsk_wakeup(struct net_device *dev, u32 queue_id, u32 flags)
{
	struct ether_priv_data *pdata = netdev_priv(dev);
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int chan;

	if (queue_id >= osi_dma->num_dma_chans)
		return 0;

	if (!netif_running(dev))
		return 0;

	chan = osi_dma->dma_chans[queue_id];
	if (chan == ETHER_INVALID_CHAN_NUM)
		return 0;

	if ((flags & XDP_WAKEUP_RX) && pdata->rx_napi[chan])
		napi_schedule(&pdata->rx_napi[chan]->napi);

	if ((flags & XDP_WAKEUP_TX) && pdata->tx_napi[chan]) {
		if (ether_xsk_tx(pdata, chan, NAPI_POLL_WEIGHT) > 0)
			napi_schedule(&pdata->tx_napi[chan]->napi);
	}

	return 0;
}

void ether_xsk_tx_wakeup_all(struct ether_priv_data *pdata)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int chan;
	unsigned int i;

	for (i = 0; i < osi_dma->num_dma_chans; i++) {
		chan = osi_dma->dma_chans[i];
		if (chan == ETHER_INVALID_CHAN_NUM ||
		    chan >= OSI_MGBE_MAX_NUM_CHANS || !pdata->rx_napi[chan])
			continue;
		if (!READ_ONCE(pdata->rx_napi[chan]->xsk_pool))
			continue;

		ether_xsk_wakeup(pdata->ndev, i, XDP_WAKEUP_TX);
	}
}

/**
 * @brief ether_xdp_rxq_setup - Register xdp_rxq_info for all RX queues
 *
 * Called from ether_open() after DMA resources are allocated.
 * Registers each RX queue with the XDP subsystem for driver-mode XDP.
 * Uses MEM_TYPE_XSK_BUFF_POOL when AF_XDP zero-copy is attached,
 * or MEM_TYPE_PAGE_POOL for page-pool RX. The legacy SKB-backed path registers
 * MEM_TYPE_PAGE_SHARED only as RX queue metadata for XDP_PASS and XDP_DROP;
 * the SKB remains driver-owned and is never returned through that memory
 * model.
 *
 * @param pdata: Driver private data
 * @return 0 on success, negative errno on failure
 */
int ether_xdp_rxq_setup(struct ether_priv_data *pdata)
{
	struct net_device *ndev = pdata->ndev;
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int i;
	unsigned int chan;
	int ret;

	for (i = 0; i < osi_dma->num_dma_chans; i++) {
		struct ether_rx_napi *rx_napi;

		chan = osi_dma->dma_chans[i];
		rx_napi = pdata->rx_napi[chan];

		if (xdp_rxq_info_is_reg(&rx_napi->xdp_rxq)) {
			xdp_rxq_info_unreg_mem_model(&rx_napi->xdp_rxq);
			xdp_rxq_info_unreg(&rx_napi->xdp_rxq);
		}

		ret = xdp_rxq_info_reg(&rx_napi->xdp_rxq, ndev, i,
				       rx_napi->napi.napi_id);
		if (ret < 0) {
			dev_err(pdata->dev,
				"Failed to register xdp_rxq for queue %u: %d\n",
				i, ret);
			goto err_unreg;
		}

#ifdef ETHER_PAGE_POOL
		if (rx_napi->xsk_pool) {
			ret = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
							 MEM_TYPE_XSK_BUFF_POOL,
							 NULL);
			if (ret == 0)
				xsk_pool_set_rxq_info(rx_napi->xsk_pool,
						      &rx_napi->xdp_rxq);
		} else {
			ret = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
							 MEM_TYPE_PAGE_POOL,
							 pdata->page_pool[chan]);
		}
#else
		if (rx_napi->xsk_pool) {
			ret = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
							 MEM_TYPE_XSK_BUFF_POOL,
							 NULL);
			if (ret == 0)
				xsk_pool_set_rxq_info(rx_napi->xsk_pool,
						      &rx_napi->xdp_rxq);
		} else {
			ret = xdp_rxq_info_reg_mem_model(&rx_napi->xdp_rxq,
							 MEM_TYPE_PAGE_SHARED,
							 NULL);
		}
#endif
		if (ret < 0) {
			dev_err(pdata->dev,
				"Failed to register xdp_rxq mem model for queue %u: %d\n",
				i, ret);
			xdp_rxq_info_unreg(&rx_napi->xdp_rxq);
			goto err_unreg;
		}
	}

	return 0;

err_unreg:
	/* Unregister all queues we successfully registered (0 to i-1) */
	while (i > 0) {
		i--;
		chan = osi_dma->dma_chans[i];
		if (xdp_rxq_info_is_reg(&pdata->rx_napi[chan]->xdp_rxq)) {
			xdp_rxq_info_unreg_mem_model(&pdata->rx_napi[chan]->xdp_rxq);
			xdp_rxq_info_unreg(&pdata->rx_napi[chan]->xdp_rxq);
		}
	}
	return ret;
}

/**
 * @brief ether_xdp_rxq_teardown_chan - Unregister one xdp_rxq_info
 *
 * Called after RX descriptor buffers have been returned and before the
 * backing page pool is destroyed.
 *
 * @param pdata: Driver private data
 * @param chan: DMA channel
 */
void ether_xdp_rxq_teardown_chan(struct ether_priv_data *pdata,
				 unsigned int chan)
{
	struct ether_rx_napi *rx_napi;

	if (chan >= OSI_MGBE_MAX_NUM_CHANS)
		return;

	rx_napi = pdata->rx_napi[chan];

	if (!rx_napi)
		return;

	rx_napi->xdp_redirect_pending = false;
	if (xdp_rxq_info_is_reg(&rx_napi->xdp_rxq)) {
		xdp_rxq_info_unreg_mem_model(&rx_napi->xdp_rxq);
		xdp_rxq_info_unreg(&rx_napi->xdp_rxq);
	}
}

/**
 * @brief ether_xdp_rxq_teardown - Unregister xdp_rxq_info for all RX queues
 *
 * @param pdata: Driver private data
 */
void ether_xdp_rxq_teardown(struct ether_priv_data *pdata)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int i;
	unsigned int chan;

	for (i = 0; i < osi_dma->num_dma_chans; i++) {
		chan = osi_dma->dma_chans[i];
		ether_xdp_rxq_teardown_chan(pdata, chan);
	}
}

/**
 * @brief ether_xdp_run_prog - Run XDP program on received packet
 *
 * Call from RX path before building SKB. Return the action for the caller to
 * handle. For ETHER_XDP_REDIRECT, caller must not free the buffer;
 * xdp_do_flush() must be called at end of NAPI poll.
 *
 * @param pdata: Driver private data
 * @param chan: RX channel (for xdp_rxq_info)
 * @param data: Packet data_hard_start pointer
 * @param len: Packet length
 * @param frame_sz: Buffer frame size (e.g. PAGE_SIZE)
 * @param headroom: Offset from data_hard_start to packet data
 * @param allow_redirect: Whether backing memory can be redirected safely
 * @param pass_data: Updated data pointer when action is XDP_PASS
 * @param pass_len: Updated packet length after the XDP program action
 * @param prog_run: Set when an XDP program was executed
 * @return ETHER_XDP_PASS, ETHER_XDP_DROP, ETHER_XDP_CONSUMED,
 * ETHER_XDP_TX, or ETHER_XDP_REDIRECT
 */
enum ether_xdp_action
ether_xdp_run_prog(struct ether_priv_data *pdata, unsigned int chan,
		   void *data, unsigned int len, unsigned int frame_sz,
		   unsigned int headroom, bool allow_redirect, void **pass_data,
		   unsigned int *pass_len, bool *prog_run)
{
	struct ether_rx_napi *rx_napi = pdata->rx_napi[chan];
	struct bpf_prog *prog;
	struct xdp_buff xdp;
	unsigned int xdp_len;
	u32 act;

	*pass_data = (char *)data + headroom;
	*pass_len = len;
	*prog_run = false;

	/* RCU read in BH context; RX runs in NAPI softirq */
	prog = rcu_dereference_bh(pdata->xdp_prog);
	if (!prog)
		return ETHER_XDP_PASS;
	*prog_run = true;

	xdp_init_buff(&xdp, frame_sz, &rx_napi->xdp_rxq);
	xdp_prepare_buff(&xdp, data, headroom, len, false);

	act = bpf_prog_run_xdp(prog, &xdp);
	xdp_len = (unsigned int)((char *)xdp.data_end -
				 (char *)xdp.data);
	*pass_len = xdp_len;

	if (act == XDP_PASS) {
		if (unlikely(xdp_len < ETH_HLEN))
			return ETHER_XDP_DROP;
		*pass_data = xdp.data;
		return ETHER_XDP_PASS;
	}
	if (act == XDP_TX) {
		if (!allow_redirect)
			return ETHER_XDP_DROP;
		return ether_xdp_tx_buff(pdata, chan, &xdp, false);
	}
	if (act == XDP_REDIRECT) {
		if (!allow_redirect)
			return ETHER_XDP_DROP;
		if (unlikely(xdp_len < ETH_HLEN))
			return ETHER_XDP_DROP;
		*pass_len = xdp_len;
		if (xdp_do_redirect(pdata->ndev, &xdp, prog) < 0)
			return ETHER_XDP_DROP;
		rx_napi->xdp_redirect_pending = true;
		return ETHER_XDP_REDIRECT;
	}
	if (act != XDP_DROP)
		bpf_warn_invalid_xdp_action(pdata->ndev, prog, act);

	return ETHER_XDP_DROP;
}

/**
 * @brief ether_xdp_run_prog_xdp - Run XDP program on xdp_buff (for XSK)
 *
 * Used when buffer is from xsk_buff_alloc. xdp_buff must have size set
 * via xsk_buff_set_size before calling.
 *
 * @param pdata: Driver private data
 * @param chan: RX channel
 * @param xdp: xdp_buff from xsk_buff_alloc
 * @param act_len: Packet length after the XDP program action
 * @return ETHER_XDP_PASS, ETHER_XDP_DROP, ETHER_XDP_CONSUMED,
 * ETHER_XDP_TX, or ETHER_XDP_REDIRECT
 */
enum ether_xdp_action
ether_xdp_run_prog_xdp(struct ether_priv_data *pdata, unsigned int chan,
		       struct xdp_buff *xdp, unsigned int *act_len)
{
	struct ether_rx_napi *rx_napi = pdata->rx_napi[chan];
	struct bpf_prog *prog;
	u32 act;

	prog = rcu_dereference_bh(pdata->xdp_prog);
	if (!prog) {
		*act_len = (unsigned int)((char *)xdp->data_end -
					  (char *)xdp->data);
		return ETHER_XDP_PASS;
	}

	act = bpf_prog_run_xdp(prog, xdp);
	*act_len = (unsigned int)((char *)xdp->data_end -
				  (char *)xdp->data);

	switch (act) {
	case XDP_PASS:
		if (unlikely(*act_len < ETH_HLEN)) {
			ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
			return ETHER_XDP_DROP;
		}
		ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_pass);
		return ETHER_XDP_PASS;
	case XDP_DROP:
		ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
		return ETHER_XDP_DROP;
	case XDP_TX:
		return ether_xdp_tx_buff(pdata, chan, xdp, true);
	case XDP_REDIRECT:
		if (unlikely(*act_len < ETH_HLEN)) {
			ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
			return ETHER_XDP_DROP;
		}
		if (xdp_do_redirect(pdata->ndev, xdp, prog) < 0) {
			ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
			return ETHER_XDP_DROP;
		}
		ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_redirect);
		rx_napi->xdp_redirect_pending = true;
		return ETHER_XDP_REDIRECT;
	default:
		bpf_warn_invalid_xdp_action(pdata->ndev, prog, act);
		ether_xdp_stats_inc(rx_napi, &rx_napi->xdp_drop);
		return ETHER_XDP_DROP;
	}
}

/**
 * @brief ether_xsk_tx - Transmit packets from XSK TX ring
 *
 * Peeks descriptors from the XSK TX ring, programs HW descriptors with
 * xsk_buff_raw_get_dma, and submits. On completion, osd_transmit_complete
 * calls xsk_tx_completed. Sets xsk_set_tx_need_wakeup when the XSK TX ring is
 * empty or a hardware submission fails and no completion can retry the work.
 *
 * @param pdata: Driver private data
 * @param chan: DMA channel (queue)
 * @param budget: Max packets per poll (from NAPI; limits run length to avoid RCU starvation)
 * @return Number of frames submitted
 */
int ether_xsk_tx(struct ether_priv_data *pdata, unsigned int chan,
		 unsigned int budget)
{
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	struct xsk_buff_pool *pool;
	struct osi_tx_ring *tx_ring;
	struct netdev_queue *txq;
	unsigned int txq_idx;
	unsigned int work_done = 0;
	unsigned int xsk_peeked = 0;
	unsigned int xsk_dropped = 0;
	unsigned int batch_tx_idx;
	unsigned int batch_desc_cnt;
	int nxmit = 0;
	struct xdp_desc desc;
	enum {
		ETHER_XSK_TX_STOP_BUDGET,
		ETHER_XSK_TX_STOP_RING_EMPTY,
		ETHER_XSK_TX_STOP_HW_FULL,
	} stop = ETHER_XSK_TX_STOP_BUDGET;

	if (chan >= OSI_MGBE_MAX_NUM_CHANS || !pdata->rx_napi[chan] ||
	    !pdata->tx_napi[chan])
		return 0;

	pool = READ_ONCE(pdata->rx_napi[chan]->xsk_pool);
	if (!pool)
		return 0;
	if (xsk_tx_budget && budget > xsk_tx_budget)
		budget = xsk_tx_budget;

	txq_idx = pdata->tx_napi[chan]->ndev_queue_index;
	txq = netdev_get_tx_queue(pdata->ndev, txq_idx);
	__netif_tx_lock_bh(txq);
	txq_trans_cond_update(txq);
	if (unlikely(!netif_running(pdata->ndev) ||
		     !netif_carrier_ok(pdata->ndev))) {
		if (xsk_uses_need_wakeup(pool))
			xsk_set_tx_need_wakeup(pool);
		goto unlock;
	}

	tx_ring = READ_ONCE(osi_dma->tx_ring[chan]);
	if (unlikely(!tx_ring || !tx_ring->tx_swcx || !tx_ring->tx_desc))
		goto unlock;
	batch_tx_idx = tx_ring->cur_tx_idx;
	batch_desc_cnt = tx_ring->desc_cnt;

	/* Submit the XSK TX drain with one doorbell. osi_hw_transmit()
	 * still applies the descriptor write barrier and advances cur_tx_idx
	 * per packet, so TX completion cannot observe descriptors before they
	 * are visible. Only the posted TX tail MMIO write is deferred until
	 * the end of this locked batch.
	 */
	tx_ring->skip_doorbell = 1U;

	while (work_done < budget) {
		struct osi_tx_swcx *tx_swcx;
		struct osi_tx_pkt_cx *tx_pkt_cx;
		dma_addr_t dma_addr;
		unsigned int tx_idx;
		unsigned int desc_cnt;

		/* Check HW room before consuming an XSK descriptor.
		 * xsk_tx_peek_desc() advances the XSK consumer; breaking out
		 * after a peek without xsk_tx_release() would neither transmit
		 * the frame nor return it to the completion ring.
		 */
		if (ether_avail_txdesc_cnt(osi_dma, tx_ring) <= 0) {
			stop = ETHER_XSK_TX_STOP_HW_FULL;
			break;
		}

		if (!xsk_tx_peek_desc(pool, &desc)) {
			stop = ETHER_XSK_TX_STOP_RING_EMPTY;
			break;
		}

		xsk_peeked++;
		work_done++;
		if (unlikely(!ether_xdp_tx_len_valid(desc.len))) {
			pdata->ndev->stats.tx_dropped =
				update_stats_counter(pdata->ndev->stats.tx_dropped,
						     1UL);
			xsk_dropped++;
			continue;
		}

		dma_addr = xsk_buff_raw_get_dma(pool, desc.addr);
		if (unlikely(!dma_addr)) {
			pdata->ndev->stats.tx_dropped =
				update_stats_counter(pdata->ndev->stats.tx_dropped,
						     1UL);
			xsk_dropped++;
			continue;
		}
		/* xdpsock puts payload at frame base; do not add headroom by default.
		 * Use xsk_tx_add_headroom=1 for apps that use [headroom][payload] layout.
		 */
		if (xsk_tx_add_headroom)
			dma_addr += xsk_pool_get_headroom(pool);
		xsk_buff_raw_dma_sync_for_device(pool, dma_addr, desc.len);

		tx_idx = tx_ring->cur_tx_idx;
		desc_cnt = tx_ring->desc_cnt;
		tx_swcx = tx_ring->tx_swcx + tx_idx;
		tx_pkt_cx = &tx_ring->tx_pkt_cx;

		memset(tx_pkt_cx, 0, sizeof(*tx_pkt_cx));
		tx_pkt_cx->flags |= OSI_PKT_CX_LEN;
		tx_pkt_cx->payload_len = desc.len;
		tx_pkt_cx->desc_cnt = 1;

		CLEAR_TX_SWCX_FLAGS(tx_swcx, 0);
		tx_swcx->flags |= ETHER_TX_SWCX_XSK_BUFF;
		/* Retain the submitting pool until this descriptor completes. */
		tx_swcx->buf_virt_addr = pool;
		tx_swcx->buf_phy_addr = dma_addr;
		tx_swcx->len = desc.len;
		tx_swcx->chan = chan;

		if (osi_hw_transmit(osi_dma, chan) < 0) {
			tx_ring->skip_doorbell = 0U;
			ether_xdp_tx_rollback(tx_ring, tx_idx, desc_cnt);
			pdata->ndev->stats.tx_dropped =
				update_stats_counter(pdata->ndev->stats.tx_dropped,
						     1UL);
			xsk_dropped++;
			stop = ETHER_XSK_TX_STOP_HW_FULL;
			break;
		}

		nxmit++;
	}

	tx_ring->skip_doorbell = 0U;
	if (nxmit > 0) {
		if (osi_tx_doorbell(osi_dma, chan) < 0) {
			ether_xdp_tx_rollback_batch(tx_ring, batch_tx_idx,
						    osi_dma->tx_ring_sz,
						    batch_desc_cnt, nxmit);
			pdata->ndev->stats.tx_dropped =
				update_stats_counter(pdata->ndev->stats.tx_dropped,
						     (unsigned long)nxmit);
			xsk_dropped += (unsigned int)nxmit;
			nxmit = 0;
			stop = ETHER_XSK_TX_STOP_HW_FULL;
		}
	}

	/* Release XSK TX descriptors after the final doorbell succeeds. If the
	 * doorbell fails, the prepared HW descriptors are rolled back first and
	 * the peeked frames are completed as dropped, so user space is not left
	 * waiting for completions from descriptors that HW never saw.
	 */
	if (xsk_peeked > 0U)
		xsk_tx_release(pool);
	if (xsk_dropped > 0U)
		xsk_tx_completed(pool, xsk_dropped);

	if (xsk_uses_need_wakeup(pool)) {
		if (stop == ETHER_XSK_TX_STOP_BUDGET) {
			xsk_clear_tx_need_wakeup(pool);
			napi_schedule(&pdata->tx_napi[chan]->napi);
		} else if (stop == ETHER_XSK_TX_STOP_RING_EMPTY) {
			xsk_set_tx_need_wakeup(pool);
		} else {
			/* HW-full progress depends on Tx completions, not a user kick. */
			xsk_clear_tx_need_wakeup(pool);
		}
	}

	tx_ring->skip_doorbell = 0U;

	if (nxmit > 0 &&
	    osi_dma->use_tx_usecs == OSI_ENABLE &&
	    atomic_read(&pdata->tx_napi[chan]->tx_usecs_timer_armed) ==
	    OSI_DISABLE) {
		atomic_set(&pdata->tx_napi[chan]->tx_usecs_timer_armed,
			   OSI_ENABLE);
		hrtimer_start(&pdata->tx_napi[chan]->tx_usecs_timer,
			      osi_dma->tx_usecs * NSEC_PER_USEC,
			      HRTIMER_MODE_REL);
	}

unlock:
	__netif_tx_unlock_bh(txq);
	return nxmit;
}

/**
 * @brief ether_xdp_xmit - Transmit XDP frames (ndo_xdp_xmit callback)
 *
 * Receives redirected frames from other devices. Maps each xdp_frame to DMA,
 * sets ETHER_TX_SWCX_XDP_FRAME in tx_swcx->flags, stores xdpf in buf_virt_addr,
 * and submits via osi_hw_transmit. On completion, osd_transmit_complete calls
 * xdp_return_frame for each frame.
 * XDP_XMIT_FLUSH: arms TX coalescing timer to ensure completion processing.
 *
 * @param dev: Network device
 * @param n: Number of frames
 * @param frames: Array of xdp_frame pointers
 * @param flags: XDP_XMIT_FLUSH to trigger completion processing
 * @return Number of frames successfully transmitted
 */
int ether_xdp_xmit(struct net_device *dev, int n, struct xdp_frame **frames,
		   u32 flags)
{
	struct ether_priv_data *pdata = netdev_priv(dev);
	struct osi_dma_priv_data *osi_dma = pdata->osi_dma;
	unsigned int chan;
	unsigned int queue_id;
	unsigned int num_tx_queues;
	int i, nxmit = 0;

	if (unlikely(flags & ~XDP_XMIT_FLAGS_MASK))
		return -EINVAL;
	num_tx_queues = min_t(unsigned int, dev->real_num_tx_queues,
			      osi_dma->num_dma_chans);
	if (unlikely(num_tx_queues == 0U))
		return -EINVAL;

	queue_id = raw_smp_processor_id() % num_tx_queues;
	chan = osi_dma->dma_chans[queue_id];
	if (unlikely(chan == ETHER_INVALID_CHAN_NUM ||
		     chan >= OSI_MGBE_MAX_NUM_CHANS))
		return -EINVAL;

	for (i = 0; i < n; i++) {
		struct xdp_frame *xdpf = frames[i];

		if (!xdpf || !ether_xdp_tx_len_valid(xdpf->len))
			break;
		if (ether_xdp_submit_frame(pdata, chan, xdpf,
					   ETHER_TX_SWCX_XDP_FRAME,
					   false, false, 0,
					   flags & XDP_XMIT_FLUSH) < 0)
			break;
		nxmit++;
	}

	return nxmit;
}

#endif /* CONFIG_XDP_SOCKETS */
