// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2021-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvidia/conftest.h>

#include <linux/etherdevice.h>
#include <linux/hrtimer.h>
#include <linux/kernel.h>
#include <linux/llist.h>
#include <linux/mempool.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_platform.h>
#include <linux/pci-epc.h>
#include <linux/pci-epf.h>
#include <linux/platform_device.h>
#include <linux/ratelimit.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/tegra_vnet.h>
#include <linux/tegra-pcie-dma.h>
#include <linux/msi.h>
#if defined(NV_NET_PAGE_POOL_H_PRESENT)
#include <net/page_pool.h>
#else
#include <net/page_pool/helpers.h>
#endif
#include <soc/tegra/fuse-helper.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
#include <linux/dma-fence.h>
#include <linux/host1x-next.h>
#include <linux/iommu.h>
#include <linux/iova.h>
#include <linux/time.h>
#include "pci-epf-wrapper.h"
#else
#include <linux/dma-iommu.h>
#include <linux/nvhost.h>
#include <linux/nvhost_interrupt_syncpt.h>
#include <linux/nvhost_t194.h>
#endif

#if ENABLE_DMA
struct tvnet_ep_dma_tx_ctx;
#endif

#define TVNET_NAPI_WEIGHT	128

#define BAR0_SIZE SZ_64M
#define APPL_INTR_EN_L1_8_0                     0x44
#define APPL_INTR_EN_L1_8_EDMA_INT_EN           BIT(6)

/* Helper macro to get interrupt type based on SoC */
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
#define TVNET_IRQ_TYPE(tvnet)						\
	((tvnet)->soc_id == TEGRA_VNET_SOC_T264 ? PCI_EPC_IRQ_MSI :	\
	 PCI_EPC_IRQ_MSIX)
#else
#define TVNET_IRQ_TYPE(tvnet)						\
	((tvnet)->soc_id == TEGRA_VNET_SOC_T264 ? PCI_IRQ_MSI :		\
	 PCI_IRQ_MSIX)
#endif

enum bar0_amap_type {
	META_DATA,
	SIMPLE_IRQ,
	DMA_IRQ = SIMPLE_IRQ,
	EP_MEM,
	HOST_MEM,
	HOST_DMA,
	EP_RX_BUF,
	AMAP_MAX,
};

struct bar0_amap {
	int size;
	struct page *page;
	void *virt;
	dma_addr_t iova;
	dma_addr_t phy;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
/* syncpoint handling. */
struct syncpt_t {
	u32 id;
	u32 threshold;
	struct host1x_syncpt *sp;

	/* syncpoint physical address for stritching to PCIe BAR backing.*/
	size_t size;
	phys_addr_t phy_addr;

	bool host1x_cb_set;
	/* Lock to protect fences between callback and deinit. */
	struct mutex lock;
	/* Fence to specific Threshold. */
	struct dma_fence *fence;
	struct dma_fence_cb fence_cb;
	/* Work to notify and allocate new fence. */
	struct work_struct work;
	void (*notifier)(void *data);
	void *notifier_data;
	bool fence_release;
};

/* NvRmHost1xSyncpointShim have size: 64KB on Orin.*/
#define SP_SIZE                 (0x10000)

/*
 * Represents SyncpointShimBase on all T234.
 * Each syncpoint is offset at (syncpt_id * SP_SIZE) on SHIM_BASE.
 */
#define SHIM_BASE               (0x60000000)

#endif

struct irqsp_data {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	/* Notification. */
	struct syncpt_t syncpt;
#else
	struct nvhost_interrupt_syncpt *is;
	struct work_struct reprime_work;
#endif
	struct device *dev;
};

struct pci_epf_tvnet {
	struct pci_epf *epf;
	struct device *fdev;
	struct pci_epf_header header;
	struct bar0_amap bar0_amap[AMAP_MAX];
	struct bar_md *bar_md;
	dma_addr_t bar0_iova;
	struct net_device *ndev;
	struct napi_struct napi;
	bool pcie_link_status;
#if !ENABLE_DMA
	/* Work queue for deferred vunmap (cannot be called from softirq) */
	struct work_struct rx_cleanup_work;
	struct list_head rx_cleanup_list;
	spinlock_t rx_cleanup_lock; /* protects rx_cleanup_list */
#endif
	struct ep_ring_buf ep_ring_buf;
	struct host_ring_buf host_ring_buf;
	enum dir_link_state tx_link_state;
	enum dir_link_state rx_link_state;
	enum os_link_state os_link_state;
	/* To synchronize network link state machine*/
	struct mutex link_state_lock;
	wait_queue_head_t link_state_wq;
	/* RX empty buffers indexed by cookie (avoid per-packet xarray lookup). */
	spinlock_t h2ep_cookie_lock;
	DECLARE_BITMAP(h2ep_cookie_bmap, TVNET_RX_COOKIE_MAX);
	struct h2ep_empty_list *h2ep_cookie_map[TVNET_RX_COOKIE_MAX];
#if ENABLE_DMA
	struct dma_desc_cnt desc_cnt;
	void *dma_cookie;  /* tegra-pcie-dma library cookie */
	int dma_msi_irq;   /* Platform MSI IRQ for T264 local DMA */
	u64 msi_addr;
	u32 msi_data;
	/* Deferred posting to EP2H full ring when it's temporarily full */
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
	struct tvnet_ep_dma_tx_ctx *tx_batch_ctx[TVNET_TX_BATCH_MAX];

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
	dma_addr_t rx_buf_iova;
	unsigned long *rx_buf_bitmap;
	int rx_num_pages;
	void __iomem *tx_dst_va;
	phys_addr_t tx_dst_pci_addr;
	struct irqsp_data *ctrl_irqsp;
	struct irqsp_data *data_irqsp;
	struct work_struct raise_irq_work;

	/* Refill buffers from process context (never from IRQ/syncpt callback) */
	struct work_struct refill_work;
	/* Process control ring from process context (never from IRQ/syncpt callback) */
	struct work_struct ctrl_work;

	/* SoC identification */
	enum tegra_vnet_soc_id soc_id;

	struct tvnet_counter h2ep_ctrl;
	struct tvnet_counter ep2h_ctrl;
	struct tvnet_counter h2ep_empty;
	struct tvnet_counter h2ep_full;
	struct tvnet_counter ep2h_empty;
	struct tvnet_counter ep2h_full;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	/* DRV_MODE specific.*/
	struct pci_epc *epc;
	struct platform_device *host1x_pdev;
	atomic_t core_initialized;
	/* IOVA alloc abstraction.*/
	struct iova_domain *iovad;
	struct iova *iova;
#endif
};

static int tvnet_ep_h2ep_cookie_alloc(struct pci_epf_tvnet *tvnet,
				      struct h2ep_empty_list *ptr,
				      u16 *cookie)
{
	unsigned long flags;
	unsigned int id;

	spin_lock_irqsave(&tvnet->h2ep_cookie_lock, flags);
	id = find_first_zero_bit(tvnet->h2ep_cookie_bmap, TVNET_RX_COOKIE_MAX);
	if (id >= TVNET_RX_COOKIE_MAX) {
		spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);
		return -ENOSPC;
	}
	__set_bit(id, tvnet->h2ep_cookie_bmap);
	tvnet->h2ep_cookie_map[id] = ptr;
	spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);

	*cookie = (u16)id;
	return 0;
}

static struct h2ep_empty_list *tvnet_ep_h2ep_cookie_take(struct pci_epf_tvnet *tvnet,
							 u16 cookie)
{
	unsigned long flags;
	struct h2ep_empty_list *ptr;

	if (cookie >= TVNET_RX_COOKIE_MAX)
		return NULL;

	spin_lock_irqsave(&tvnet->h2ep_cookie_lock, flags);
	ptr = tvnet->h2ep_cookie_map[cookie];
	tvnet->h2ep_cookie_map[cookie] = NULL;
	if (ptr)
		__clear_bit(cookie, tvnet->h2ep_cookie_bmap);
	spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);

	return ptr;
}

/*
 * Slow-path recovery: if cookie in FULL msg is bad/corrupted, try to locate
 * the tracking node by IOVA. This avoids leaking RX buffers indefinitely.
 * This is O(TVNET_RX_COOKIE_MAX) and should only trigger on bugs/corruption.
 */
static struct h2ep_empty_list *tvnet_ep_h2ep_cookie_take_by_iova(struct pci_epf_tvnet *tvnet,
								 dma_addr_t iova)
{
	unsigned long flags;
	u16 cookie;
	struct h2ep_empty_list *ptr;

	spin_lock_irqsave(&tvnet->h2ep_cookie_lock, flags);
	for (cookie = 0; cookie < TVNET_RX_COOKIE_MAX; cookie++) {
		ptr = tvnet->h2ep_cookie_map[cookie];
		if (!ptr)
			continue;
		if (ptr->iova != iova)
			continue;
		tvnet->h2ep_cookie_map[cookie] = NULL;
		__clear_bit(cookie, tvnet->h2ep_cookie_bmap);
		spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);
		return ptr;
	}
	spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);

	return NULL;
}

#if ENABLE_DMA
/* Forward declaration */
static void tvnet_ep_setup_dma(struct pci_epf_tvnet *tvnet);
static void tvnet_ep_dma_complete(void *priv, tegra_pcie_dma_status_t status);
static void tvnet_ep_dma_complete_batch(void *priv, tegra_pcie_dma_status_t status);
#endif

static void tvnet_ep_raise_irq_work_function(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet =
		container_of(work, struct pci_epf_tvnet, raise_irq_work);

	struct pci_epc *epc = tvnet->epf->epc;
	int irq_type = TVNET_IRQ_TYPE(tvnet);

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	struct pci_epf *epf = tvnet->epf;

	lpci_epc_raise_irq(epc, epf->func_no, irq_type, 0);
	lpci_epc_raise_irq(epc, epf->func_no, irq_type, 1);
#else
	pci_epc_raise_irq(epc, irq_type, 0);
	pci_epc_raise_irq(epc, irq_type, 1);
#endif
}

static void tvnet_ep_read_ctrl_msg(struct pci_epf_tvnet *tvnet,
				   struct ctrl_msg *msg)
{
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct ctrl_msg *ctrl_msg = host_ring_buf->h2ep_ctrl_msgs;
	u32 idx;

	if (tvnet_ivc_empty(&tvnet->h2ep_ctrl)) {
		dev_dbg(tvnet->fdev, "%s: H2EP ctrl ring empty\n", __func__);
		return;
	}

	idx = tvnet_ivc_get_rd_cnt(&tvnet->h2ep_ctrl) % RING_COUNT;
	memcpy(msg, &ctrl_msg[idx], sizeof(*msg));
	tvnet_ivc_advance_rd(&tvnet->h2ep_ctrl);
}

/* TODO Handle error case */
static int tvnet_ep_write_ctrl_msg(struct pci_epf_tvnet *tvnet,
				   struct ctrl_msg *msg)
{
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct ctrl_msg *ctrl_msg = ep_ring_buf->ep2h_ctrl_msgs;
	u32 idx;

	if (tvnet_ivc_full(&tvnet->ep2h_ctrl)) {
		/* Defer IRQ raising to workqueue (may be called from atomic context) */
		schedule_work(&tvnet->raise_irq_work);
		dev_dbg(tvnet->fdev, "%s: EP2H ctrl ring full\n", __func__);
		return -EAGAIN;
	}

	idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_ctrl) % RING_COUNT;
	memcpy(&ctrl_msg[idx], msg, sizeof(*msg));
	tvnet_ivc_advance_wr(&tvnet->ep2h_ctrl);
	/* Defer IRQ raising to workqueue (may be called from atomic context) */
	schedule_work(&tvnet->raise_irq_work);

	return 0;
}

#if !ENABLE_DMA
static dma_addr_t tvnet_ivoa_alloc(struct pci_epf_tvnet *tvnet)
{
	dma_addr_t iova;
	int pageno;

	pageno = bitmap_find_free_region(tvnet->rx_buf_bitmap,
					 tvnet->rx_num_pages, 0);
	if (pageno < 0) {
		dev_err(tvnet->fdev, "%s: Rx iova alloc fail, page: %d\n",
			__func__, pageno);
		return 0;  /* Return 0 on error (invalid DMA address) */
	}
	iova = tvnet->rx_buf_iova + (pageno << PAGE_SHIFT);

	return iova;
}

static void tvnet_ep_iova_dealloc(struct pci_epf_tvnet *tvnet, dma_addr_t iova)
{
	int pageno;

	pageno = (iova - tvnet->rx_buf_iova) >> PAGE_SHIFT;
	bitmap_release_region(tvnet->rx_buf_bitmap, pageno, 0);
}
#endif

static void tvnet_ep_alloc_empty_buffers(struct pci_epf_tvnet *tvnet)
{
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct pci_epc *epc = tvnet->epf->epc;
	struct device *cdev = epc->dev.parent;
	struct data_msg *h2ep_empty_msg = ep_ring_buf->h2ep_empty_msgs;
	struct h2ep_empty_list *h2ep_empty_ptr;
#if ENABLE_DMA
	struct net_device *ndev = tvnet->ndev;
	struct sk_buff *skb;
	u32 len = ndev->mtu + ETH_HLEN;
#ifdef CONFIG_PAGE_POOL
	bool pp_alloc = false;
#endif
#else
	struct page *page;
	void *virt;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	int ret = 0;
#endif
	unsigned int iter = 0;
	bool kicked = false;

	while (!tvnet_ivc_full(&tvnet->h2ep_empty)) {
		dma_addr_t iova;
		u32 idx;
		u16 cookie;

#if ENABLE_DMA
		skb = NULL;
		iova = 0;

#ifdef CONFIG_PAGE_POOL
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

			iova = dma_map_single(cdev, skb->data, len, DMA_FROM_DEVICE);
			if (dma_mapping_error(cdev, iova)) {
				pr_err("%s: dma map failed\n", __func__);
				dev_kfree_skb_any(skb);
				break;
			}
		}

		/* The PCIe link is stable and dependable,
		 * so it's not necessary to perform a software checksum.
		 */
		skb->ip_summed = CHECKSUM_UNNECESSARY;
#else
		iova = tvnet_ivoa_alloc(tvnet);
		if (!iova) {  /* Check for 0 (invalid DMA address) */
			dev_err(tvnet->fdev, "%s: iova alloc failed\n",
				__func__);
			break;
		}

		page = alloc_pages(GFP_KERNEL, 1);
		if (!page) {
			dev_err(tvnet->fdev, "%s: alloc_pages() failed\n",
				__func__);
			tvnet_ep_iova_dealloc(tvnet, iova);
			break;
		}

		{
			ulong prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;

#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
			ret = iommu_map(domain, iova, page_to_phys(page), PAGE_SIZE,
					prot, GFP_KERNEL);
#else
			ret = iommu_map(domain, iova, page_to_phys(page), PAGE_SIZE,
					prot);
#endif
		}
		if (ret < 0) {
			dev_err(tvnet->fdev, "%s: iommu_map(RAM) failed: %d\n",
				__func__, ret);
			__free_pages(page, 1);
			tvnet_ep_iova_dealloc(tvnet, iova);
			break;
		}

		virt = vmap(&page, 1, VM_MAP, PAGE_KERNEL);
		if (!virt) {
			dev_err(tvnet->fdev, "%s: vmap() failed\n", __func__);
			iommu_unmap(domain, iova, PAGE_SIZE);
			__free_pages(page, 1);
			tvnet_ep_iova_dealloc(tvnet, iova);
			break;
		}
#endif

		h2ep_empty_ptr = kmalloc(sizeof(*h2ep_empty_ptr), GFP_KERNEL);
		if (!h2ep_empty_ptr) {
#if ENABLE_DMA
#ifdef CONFIG_PAGE_POOL
			if (!pp_alloc)
				dma_unmap_single(cdev, iova, len, DMA_FROM_DEVICE);
#else
			dma_unmap_single(cdev, iova, len, DMA_FROM_DEVICE);
#endif
			dev_kfree_skb_any(skb);
#else
			vunmap(virt);
			iommu_unmap(domain, iova, PAGE_SIZE);
			__free_pages(page, 1);
			tvnet_ep_iova_dealloc(tvnet, iova);
#endif
			break;
		}

#if ENABLE_DMA
		h2ep_empty_ptr->skb = skb;
		h2ep_empty_ptr->size = len;
		/*
		 * If page_pool is used, iova points to (dma_base + TVNET_PP_HEADROOM).
		 * Otherwise dma_base==iova and offset==0.
		 */
#ifdef CONFIG_PAGE_POOL
		h2ep_empty_ptr->dma_base = pp_alloc ? (iova - TVNET_PP_HEADROOM) : iova;
		h2ep_empty_ptr->offset = pp_alloc ? TVNET_PP_HEADROOM : 0;
#else
		h2ep_empty_ptr->dma_base = iova;
		h2ep_empty_ptr->offset = 0;
#endif
#else
		h2ep_empty_ptr->page = page;
		h2ep_empty_ptr->virt = virt;
		h2ep_empty_ptr->size = PAGE_SIZE;
#endif
		h2ep_empty_ptr->iova = iova;
		if (tvnet_ep_h2ep_cookie_alloc(tvnet, h2ep_empty_ptr, &cookie)) {
			dev_err(tvnet->fdev, "%s: rx cookie alloc failed\n", __func__);
#if ENABLE_DMA
#ifdef CONFIG_PAGE_POOL
			if (!pp_alloc)
				dma_unmap_single(cdev, iova, len, DMA_FROM_DEVICE);
#else
			dma_unmap_single(cdev, iova, len, DMA_FROM_DEVICE);
#endif
			dev_kfree_skb_any(skb);
#else
			vunmap(virt);
			iommu_unmap(domain, iova, PAGE_SIZE);
			__free_pages(page, 1);
			tvnet_ep_iova_dealloc(tvnet, iova);
#endif
			kfree(h2ep_empty_ptr);
			break;
		}

		idx = tvnet_ivc_get_wr_cnt(&tvnet->h2ep_empty) % RING_COUNT;
		h2ep_empty_msg[idx].u.empty_buffer.pcie_address = iova;
		h2ep_empty_msg[idx].u.empty_buffer.buffer_len = len;
		h2ep_empty_msg[idx].u.empty_buffer.cookie = cookie;
		tvnet_ivc_advance_wr(&tvnet->h2ep_empty);

		/*
		 * Coalesce doorbells: queue once immediately after the first
		 * posted EMPTY, so the peer can start consuming without waiting
		 * for the full refill loop to finish.
		 */
		if (!kicked) {
			schedule_work(&tvnet->raise_irq_work);
			kicked = true;
		}

		/* Avoid long uninterrupted CPU hogging during large refills */
		if (!(++iter & 0x1f))
			cond_resched();
	}
}

#if !ENABLE_DMA
/* Work queue handler to cleanup RX buffers (vunmap cannot be called from softirq) */
static void tvnet_ep_rx_cleanup_work(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet = container_of(work, struct pci_epf_tvnet, rx_cleanup_work);
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct h2ep_empty_list *h2ep_empty_ptr, *temp;
	unsigned long flags;
	LIST_HEAD(local_list);

	/* Move items from cleanup list to local list */
	spin_lock_irqsave(&tvnet->rx_cleanup_lock, flags);
	list_splice_init(&tvnet->rx_cleanup_list, &local_list);
	spin_unlock_irqrestore(&tvnet->rx_cleanup_lock, flags);

	/* Process cleanup in process context (vunmap is allowed here) */
	list_for_each_entry_safe(h2ep_empty_ptr, temp, &local_list, list) {
		list_del(&h2ep_empty_ptr->list);
		vunmap(h2ep_empty_ptr->virt);
		iommu_unmap(domain, h2ep_empty_ptr->iova, PAGE_SIZE);
		__free_pages(h2ep_empty_ptr->page, 1);
		tvnet_ep_iova_dealloc(tvnet, h2ep_empty_ptr->iova);
		kfree(h2ep_empty_ptr);
	}
}
#endif

static void tvnet_ep_free_empty_buffers(struct pci_epf_tvnet *tvnet)
{
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
#if !ENABLE_DMA
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
#endif
	struct h2ep_empty_list *h2ep_empty_ptr, *temp;
	unsigned long flags;
	u16 cookie;
	LIST_HEAD(local);

	/* Drain cookie map under lock; free/unmap outside lock. */
	spin_lock_irqsave(&tvnet->h2ep_cookie_lock, flags);
	for (cookie = 0; cookie < TVNET_RX_COOKIE_MAX; cookie++) {
		h2ep_empty_ptr = tvnet->h2ep_cookie_map[cookie];
		if (!h2ep_empty_ptr)
			continue;
		tvnet->h2ep_cookie_map[cookie] = NULL;
		__clear_bit(cookie, tvnet->h2ep_cookie_bmap);
		list_add(&h2ep_empty_ptr->list, &local);
	}
	spin_unlock_irqrestore(&tvnet->h2ep_cookie_lock, flags);

	list_for_each_entry_safe(h2ep_empty_ptr, temp, &local, list) {
		list_del(&h2ep_empty_ptr->list);
#if ENABLE_DMA
		if (!h2ep_empty_ptr->offset)
			dma_unmap_single(cdev, h2ep_empty_ptr->iova,
					 h2ep_empty_ptr->size, DMA_FROM_DEVICE);
		dev_kfree_skb_any(h2ep_empty_ptr->skb);
#else
		vunmap(h2ep_empty_ptr->virt);
		iommu_unmap(domain, h2ep_empty_ptr->iova, PAGE_SIZE);
		__free_pages(h2ep_empty_ptr->page, 1);
		tvnet_ep_iova_dealloc(tvnet, h2ep_empty_ptr->iova);
#endif
		kfree(h2ep_empty_ptr);
	}
}

static void tvnet_ep_stop_tx_queue(struct pci_epf_tvnet *tvnet)
{
	struct net_device *ndev = tvnet->ndev;

	netif_stop_queue(ndev);
	/* Get tx lock to make sure that there is no ongoing xmit */
	netif_tx_lock(ndev);
	netif_tx_unlock(ndev);
}

static void tvnet_ep_stop_rx_work(struct pci_epf_tvnet *tvnet)
{
	/* TODO wait for syncpoint interrupt handlers */
}

static void tvnet_ep_clear_data_msg_counters(struct pci_epf_tvnet *tvnet)
{
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct host_own_cnt *host_cnt = host_ring_buf->host_cnt;
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct ep_own_cnt *ep_cnt = ep_ring_buf->ep_cnt;

	host_cnt->h2ep_empty_rd_cnt = 0;
	ep_cnt->h2ep_empty_wr_cnt = 0;
	ep_cnt->ep2h_full_wr_cnt = 0;
	host_cnt->ep2h_full_rd_cnt = 0;
}

static void tvnet_ep_update_link_state(struct net_device *ndev,
				       enum os_link_state state)
{
	if (state == OS_LINK_STATE_UP) {
		netif_start_queue(ndev);
		netif_carrier_on(ndev);
	} else if (state == OS_LINK_STATE_DOWN) {
		netif_carrier_off(ndev);
		netif_stop_queue(ndev);
	} else {
		pr_err("%s: invalid sate: %d\n", __func__, state);
	}
}

/* OS link state machine */
static void tvnet_ep_update_link_sm(struct pci_epf_tvnet *tvnet)
{
	struct net_device *ndev = tvnet->ndev;
	enum os_link_state old_state = tvnet->os_link_state;

	if (tvnet->rx_link_state == DIR_LINK_STATE_UP &&
	    tvnet->tx_link_state == DIR_LINK_STATE_UP)
		tvnet->os_link_state = OS_LINK_STATE_UP;
	else
		tvnet->os_link_state = OS_LINK_STATE_DOWN;

	if (tvnet->os_link_state != old_state)
		tvnet_ep_update_link_state(ndev, tvnet->os_link_state);
}

/* One way link state machine */
static void tvnet_ep_user_link_up_req(struct pci_epf_tvnet *tvnet)
{
	struct ctrl_msg msg;

	tvnet_ep_clear_data_msg_counters(tvnet);
	tvnet_ep_alloc_empty_buffers(tvnet);
	msg.msg_id = CTRL_MSG_LINK_UP;
	tvnet_ep_write_ctrl_msg(tvnet, &msg);
	tvnet->rx_link_state = DIR_LINK_STATE_UP;
	tvnet_ep_update_link_sm(tvnet);
}

static void tvnet_ep_user_link_down_req(struct pci_epf_tvnet *tvnet)
{
	struct ctrl_msg msg;

	tvnet->rx_link_state = DIR_LINK_STATE_SENT_DOWN;
	msg.msg_id = CTRL_MSG_LINK_DOWN;
	tvnet_ep_write_ctrl_msg(tvnet, &msg);
	tvnet_ep_update_link_sm(tvnet);
}

static void tvnet_ep_rcv_link_up_msg(struct pci_epf_tvnet *tvnet)
{
#if ENABLE_DMA
	tvnet_ep_setup_dma(tvnet);
#endif
	tvnet->tx_link_state = DIR_LINK_STATE_UP;
	tvnet_ep_update_link_sm(tvnet);
}

static void tvnet_ep_rcv_link_down_msg(struct pci_epf_tvnet *tvnet)
{
	struct ctrl_msg msg;

	/* Stop using empty buffers of remote system */
	tvnet_ep_stop_tx_queue(tvnet);
	msg.msg_id = CTRL_MSG_LINK_DOWN_ACK;
	tvnet_ep_write_ctrl_msg(tvnet, &msg);
	tvnet->tx_link_state = DIR_LINK_STATE_DOWN;
	tvnet_ep_update_link_sm(tvnet);
}

static void tvnet_ep_rcv_link_down_ack(struct pci_epf_tvnet *tvnet)
{
	/* Stop using empty buffers(which are full in rx) of local system */
	tvnet_ep_stop_rx_work(tvnet);
	tvnet_ep_free_empty_buffers(tvnet);
	tvnet->rx_link_state = DIR_LINK_STATE_DOWN;
	wake_up_interruptible(&tvnet->link_state_wq);
	tvnet_ep_update_link_sm(tvnet);
}

static int tvnet_ep_open(struct net_device *ndev)
{
	struct device *fdev = ndev->dev.parent;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(fdev);

	if (!tvnet->pcie_link_status) {
		dev_err(fdev, "%s: PCIe link is not up\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&tvnet->link_state_lock);
	if (tvnet->rx_link_state == DIR_LINK_STATE_DOWN)
		tvnet_ep_user_link_up_req(tvnet);
	napi_enable(&tvnet->napi);
	mutex_unlock(&tvnet->link_state_lock);

	return 0;
}

static int tvnet_ep_close(struct net_device *ndev)
{
	struct device *fdev = ndev->dev.parent;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(fdev);
	int ret = 0;

	mutex_lock(&tvnet->link_state_lock);
	napi_disable(&tvnet->napi);
	if (tvnet->rx_link_state == DIR_LINK_STATE_UP)
		tvnet_ep_user_link_down_req(tvnet);

	ret = wait_event_interruptible_timeout(tvnet->link_state_wq,
					       (tvnet->rx_link_state ==
					       DIR_LINK_STATE_DOWN),
					       msecs_to_jiffies(LINK_TIMEOUT));
	ret = (ret > 0) ? 0 : -ETIMEDOUT;
	if (ret < 0) {
		pr_err("%s: link state machine failed: tx_state: %d rx_state: %d err: %d\n",
		       __func__, tvnet->tx_link_state, tvnet->rx_link_state,
		       ret);
		tvnet->rx_link_state = DIR_LINK_STATE_UP;
	}
	mutex_unlock(&tvnet->link_state_lock);

	return 0;
}

static int tvnet_ep_change_mtu(struct net_device *ndev, int new_mtu)
{
	bool set_down = false;

	if (new_mtu > TVNET_MAX_MTU || new_mtu < TVNET_MIN_MTU) {
		pr_err("MTU range is %d to %d\n", TVNET_MIN_MTU, TVNET_MAX_MTU);
		return -EINVAL;
	}

	if (netif_running(ndev)) {
		set_down = true;
		tvnet_ep_close(ndev);
	}

	pr_info("changing MTU from %d to %d\n", ndev->mtu, new_mtu);

	ndev->mtu = new_mtu;

	if (set_down)
		tvnet_ep_open(ndev);

	return 0;
}

#if ENABLE_DMA
/* Structure to hold context for DMA completion callback */
struct tvnet_ep_dma_tx_ctx {
	struct llist_node llnode;
	struct sk_buff *skb;
	struct pci_epf_tvnet *tvnet;
	dma_addr_t src_iova;
	dma_addr_t dst_iova;
	u32 len;
	u16 cookie;
};

struct tvnet_ep_dma_tx_batch {
	struct pci_epf_tvnet *tvnet;
	u8 nents;
	struct tegra_pcie_dma_desc desc[TVNET_TX_BATCH_MAX];
	struct tvnet_ep_dma_tx_ctx *ctx[TVNET_TX_BATCH_MAX];
};

static void tvnet_ep_drop_tx_batch(struct pci_epf_tvnet *tvnet)
{
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	void *ctx[TVNET_TX_BATCH_MAX];
	u8 nents, i;
	unsigned long flags;

	spin_lock_irqsave(&tvnet->tx_batch_lock, flags);
	nents = tvnet->tx_batch_cnt;
	for (i = 0; i < nents; i++)
		ctx[i] = tvnet->tx_batch_ctx[i];
	tvnet->tx_batch_cnt = 0;
	spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);

	for (i = 0; i < nents; i++) {
		struct tvnet_ep_dma_tx_ctx *c = ctx[i];

		if (!c)
			continue;
		dma_unmap_single(cdev, c->src_iova, c->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(c->skb);
		mempool_free(c, tvnet->tx_ctx_pool);
	}
}

static bool tvnet_ep_try_post_ep2h_full(struct pci_epf_tvnet *tvnet,
					u32 len, dma_addr_t dst_iova, u16 cookie)
{
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct data_msg *ep2h_full_msg = ep_ring_buf->ep2h_full_msgs;
	u32 wr_idx;

	if (tvnet_ivc_full(&tvnet->ep2h_full))
		return false;

	wr_idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_full) % RING_COUNT;
	ep2h_full_msg[wr_idx].u.full_buffer.packet_size = len;
	ep2h_full_msg[wr_idx].u.full_buffer.pcie_address = (u64)dst_iova;
	ep2h_full_msg[wr_idx].u.full_buffer.cookie = cookie;
	/*
	 * Ensure the msg is visible before advancing the ring counter.
	 * tvnet_ivc_advance_wr() adds a barrier after updating the counter,
	 * but we still need ordering before the counter update.
	 */
	mb();
	tvnet_ivc_advance_wr(&tvnet->ep2h_full);

	/*
	 * Note: Caller is responsible for raising DATA IRQ (to allow batching).
	 * This function only updates the ring and returns success/failure.
	 */
	return true;
}

static enum hrtimer_restart tvnet_ep_tx_batch_timer_fn(struct hrtimer *t)
{
	struct pci_epf_tvnet *tvnet = container_of(t, struct pci_epf_tvnet,
						  tx_batch_timer);

	queue_delayed_work(system_highpri_wq, &tvnet->tx_batch_flush_work, 0);
	return HRTIMER_NORESTART;
}

static void tvnet_ep_tx_batch_flush_work(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet = container_of(to_delayed_work(work),
						   struct pci_epf_tvnet,
						   tx_batch_flush_work);
	struct net_device *ndev = tvnet->ndev;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	unsigned long flags;
	struct tegra_pcie_dma_xfer_info xfer_info = {0};
	struct tvnet_ep_dma_tx_batch *batch;
	tegra_pcie_dma_status_t dma_status;
	struct tvnet_ep_dma_tx_ctx *c;
	struct tegra_pcie_dma_desc desc;
	u8 nents, i;

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

			xfer_info.type = TEGRA_PCIE_DMA_WRITE;
			xfer_info.channel_num = 0;
			xfer_info.desc = &desc;
			xfer_info.nents = 1;
			xfer_info.complete = tvnet_ep_dma_complete;
			xfer_info.priv = c;

			/*
			 * Update WR before submit to avoid rd_cnt overtaking wr_cnt
			 * in fast-completion cases (inflight underflow in trace).
			 */
			desc_cnt->wr_cnt++;
			dma_status = tegra_pcie_dma_submit_xfer(tvnet->dma_cookie, &xfer_info);
			if (dma_status != TEGRA_PCIE_DMA_SUCCESS) {
				desc_cnt->wr_cnt--;
				ndev->stats.tx_errors++;
				dma_unmap_single(cdev, c->src_iova, c->len, DMA_TO_DEVICE);
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

	xfer_info.type = TEGRA_PCIE_DMA_WRITE;
	xfer_info.channel_num = 0;
	xfer_info.desc = batch->desc;
	xfer_info.nents = batch->nents;
	xfer_info.complete = tvnet_ep_dma_complete_batch;
	xfer_info.priv = batch;

	/* Update WR before submit to avoid rd_cnt overtaking wr_cnt. */
	desc_cnt->wr_cnt += batch->nents;
	dma_status = tegra_pcie_dma_submit_xfer(tvnet->dma_cookie, &xfer_info);
	if (dma_status != TEGRA_PCIE_DMA_SUCCESS) {
		desc_cnt->wr_cnt -= batch->nents;
		for (i = 0; i < batch->nents; i++) {
			struct tvnet_ep_dma_tx_ctx *c = batch->ctx[i];

			ndev->stats.tx_errors++;
			dma_unmap_single(cdev, c->src_iova, c->len, DMA_TO_DEVICE);
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

	/*
	 * We may have stopped the queue due to our software batch being full.
	 * Wake as soon as we flush the batch (do not wait for DMA completion).
	 */
	if (netif_queue_stopped(ndev) && tvnet->os_link_state == OS_LINK_STATE_UP) {
		if (tvnet_ivc_rd_available(&tvnet->ep2h_empty) &&
		    !tvnet_ivc_full(&tvnet->ep2h_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}
}

static void tvnet_ep_tx_complete_work(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet = container_of(to_delayed_work(work),
						  struct pci_epf_tvnet,
						  tx_complete_work);
	struct tvnet_ep_dma_tx_ctx *ctx;
	struct tvnet_ep_dma_tx_ctx *n;
	struct llist_node *node;
	unsigned int posted = 0;

	node = llist_del_all(&tvnet->tx_complete_list);
	if (!node)
		return;

	node = llist_reverse_order(node);

	llist_for_each_entry_safe(ctx, n, node, llnode) {
		struct llist_node *remain, *last;

		if (tvnet_ep_try_post_ep2h_full(tvnet, ctx->len, ctx->dst_iova, ctx->cookie)) {
			posted++;
			mempool_free(ctx, tvnet->tx_ctx_pool);
			continue;
		}

		remain = &ctx->llnode;
		last = remain;
		while (last->next)
			last = last->next;

		llist_add_batch(remain, last, &tvnet->tx_complete_list);

		/* Kick Host to drain FULL ring, then retry immediately */
		schedule_work(&tvnet->raise_irq_work);
		queue_work(system_unbound_wq, &tvnet->tx_complete_work.work);
		return;
	}

	/* Raise DATA IRQ once if any entries were posted (coalesce) */
	if (posted)
		schedule_work(&tvnet->raise_irq_work);
}

static void tvnet_ep_dma_complete_batch(void *priv, tegra_pcie_dma_status_t status)
{
	struct tvnet_ep_dma_tx_batch *batch = priv;
	struct pci_epf_tvnet *tvnet = batch->tvnet;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct device *fdev = tvnet->fdev;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct net_device *ndev = NULL;
	struct tvnet_ep_dma_tx_ctx *ctx;
	struct sk_buff *skb;
	unsigned int posted = 0;
	u8 i;

	for (i = 0; i < batch->nents; i++) {
		ctx = batch->ctx[i];
		skb = ctx->skb;

		ndev = skb->dev;

		if (status != TEGRA_PCIE_DMA_SUCCESS) {
			ndev->stats.tx_errors++;
		} else {
			ndev->stats.tx_packets++;
			ndev->stats.tx_bytes += ctx->len;
		}

		dma_unmap_single(cdev, ctx->src_iova, ctx->len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
		ctx->skb = NULL;
		ctx->src_iova = 0;

		if (status == TEGRA_PCIE_DMA_SUCCESS) {
			if (tvnet_ep_try_post_ep2h_full(tvnet, ctx->len,
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

	/*
	 * Raise DATA IRQ once per batch if any entries were posted.
	 * This coalesces up to 64 ring updates into a single interrupt.
	 */
	if (posted)
		schedule_work(&tvnet->raise_irq_work);

	if (status != TEGRA_PCIE_DMA_SUCCESS)
		dev_err(fdev, "DMA transfer failed (batch nents=%u): %d\n",
			batch->nents, status);

	if (ndev && netif_queue_stopped(ndev) && tvnet->os_link_state == OS_LINK_STATE_UP) {
		if (tvnet_ivc_rd_available(&tvnet->ep2h_empty) &&
		    !tvnet_ivc_full(&tvnet->ep2h_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}

	mempool_free(batch, tvnet->tx_batch_pool);
}

/* DMA completion callback for async local DMA write */
static void tvnet_ep_dma_complete(void *priv, tegra_pcie_dma_status_t status)
{
	struct tvnet_ep_dma_tx_ctx *ctx = (struct tvnet_ep_dma_tx_ctx *)priv;
	struct sk_buff *skb = ctx->skb;
	struct net_device *ndev = skb->dev;
	struct device *fdev = ndev->dev.parent;
	struct pci_epf_tvnet *tvnet = ctx->tvnet;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;

	if (status != TEGRA_PCIE_DMA_SUCCESS) {
		dev_err(fdev, "DMA transfer failed: %d\n", status);
		ndev->stats.tx_errors++;
	} else {
		ndev->stats.tx_packets++;
		ndev->stats.tx_bytes += ctx->len;
	}

	/* Unmap and free skb */
	dma_unmap_single(cdev, ctx->src_iova, ctx->len, DMA_TO_DEVICE);
	dev_kfree_skb_any(skb);
	ctx->skb = NULL;
	ctx->src_iova = 0;

	/* Update descriptor count */
	desc_cnt->rd_cnt++;

	/*
	 * Post to EP2H full ring. If it's temporarily full, defer to process
	 * context and kick Host to drain faster.
	 */
	if (status == TEGRA_PCIE_DMA_SUCCESS) {
		if (tvnet_ep_try_post_ep2h_full(tvnet, ctx->len, ctx->dst_iova, ctx->cookie)) {
			/* Posted successfully → notify Host to drain */
			schedule_work(&tvnet->raise_irq_work);
			mempool_free(ctx, tvnet->tx_ctx_pool);
		} else {
			/*
			 * Ring full → defer to tx_complete_work for retry.
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
		if (tvnet_ivc_rd_available(&tvnet->ep2h_empty) &&
		    !tvnet_ivc_full(&tvnet->ep2h_full) &&
		    ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) < DMA_DESC_COUNT)) {
			netif_wake_queue(ndev);
		}
	}
}

static void tvnet_ep_drain_tx_complete_list(struct pci_epf_tvnet *tvnet)
{
	struct llist_node *node;
	struct tvnet_ep_dma_tx_ctx *ctx, *n;

	node = llist_del_all(&tvnet->tx_complete_list);
	llist_for_each_entry_safe(ctx, n, node, llnode)
		mempool_free(ctx, tvnet->tx_ctx_pool);
}
#else
/*
 * Non-DMA mode helper used from ndo_start_xmit(). Must NOT sleep.
 * (This codepath is currently disabled by ENABLE_DMA=1 in tegra_vnet.h,
 * but keep it buildable.)
 */
static int tvnet_ep_poll_ep2h_full_space(struct pci_epf_tvnet *tvnet,
					 struct net_device *ndev)
{
	if (tvnet_ivc_full(&tvnet->ep2h_full)) {
		/* Nudge host to drain */
		schedule_work(&tvnet->raise_irq_work);
		ndev->stats.tx_dropped++;
		return -ENOSPC;
	}

	return 0;
}
#endif

static void tvnet_ep_refill_work(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet = container_of(work, struct pci_epf_tvnet,
						  refill_work);

	if (tvnet->os_link_state != OS_LINK_STATE_UP)
		return;

	tvnet_ep_alloc_empty_buffers(tvnet);
}

static netdev_tx_t tvnet_ep_start_xmit(struct sk_buff *skb,
				       struct net_device *ndev)
{
	struct device *fdev = ndev->dev.parent;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(fdev);
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct skb_shared_info *info = skb_shinfo(skb);
	struct data_msg *ep2h_empty_msg = host_ring_buf->ep2h_empty_msgs;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
#if ENABLE_DMA
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct tvnet_ep_dma_tx_ctx *tx_ctx;
	struct netdev_queue *txq;
	unsigned long flags;
	bool flush_now;
	bool flush_reason_stopped;
	bool flush_reason_max;
	u8 batch_pending;
	u32 delay_us;
#else
	int ret;
#endif
	dma_addr_t src_iova;
	u32 rd_idx;
	u16 dst_cookie;
	u64 dst_masked, dst_off, dst_addr;
	dma_addr_t dst_iova;
	u32 dst_len, len;

	/*TODO Not expecting skb frags, remove this after testing */
	WARN_ON(info->nr_frags);

	/* Check if EP2H_EMPTY_BUF available to read */
	if (!tvnet_ivc_rd_available(&tvnet->ep2h_empty)) {
		/* Defer IRQ raising to workqueue (cannot sleep in TX softirq) */
		schedule_work(&tvnet->raise_irq_work);
		dev_dbg(fdev, "%s: No EP2H empty msg, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	/* Check if EP2H_FULL_BUF available to write */
	if (tvnet_ivc_full(&tvnet->ep2h_full)) {
		/* Defer IRQ raising to workqueue (cannot sleep in TX softirq) */
		schedule_work(&tvnet->raise_irq_work);
		dev_dbg(fdev, "%s: No EP2H full buf, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

#if ENABLE_DMA
	/* Check if dma desc available */
	batch_pending = READ_ONCE(tvnet->tx_batch_cnt);
	if (batch_pending >= TVNET_TX_BATCH_MAX) {
		queue_delayed_work(system_highpri_wq,
				   &tvnet->tx_batch_flush_work, 0);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + batch_pending) >= DMA_DESC_COUNT) {
		dev_dbg(fdev, "%s: dma descs are not available\n", __func__);
		if (batch_pending)
			queue_delayed_work(system_highpri_wq,
					   &tvnet->tx_batch_flush_work, 0);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	/*
	 * Guard total consumed EMPTY buffers from Host.
	 * Host provided RING_COUNT empties initially. We've consumed empties for:
	 * - batch_pending (queued, not yet DMA submitted)
	 * - dma_inflight (submitted to DMA, not yet completed)
	 * - ep2h_full ring entries (DMA completed, waiting for Host to drain)
	 * Total consumed must stay < RING_COUNT or we'll exhaust Host's EMPTY pool.
	 */
	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + batch_pending +
	     tvnet_ivc_rd_available(&tvnet->ep2h_full) + 1) >= RING_COUNT) {
		dev_dbg(fdev, "%s: Total consumed EMPTYs would exceed RING_COUNT\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}
#endif

	len = skb_headlen(skb);

	src_iova = dma_map_single(cdev, skb->data, len, DMA_TO_DEVICE);
	if (dma_mapping_error(cdev, src_iova)) {
		dev_err(fdev, "%s: dma_map_single failed\n", __func__);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}

	/* Get EP2H empty msg */
	rd_idx = tvnet_ivc_get_rd_cnt(&tvnet->ep2h_empty) % RING_COUNT;
	dst_addr = ep2h_empty_msg[rd_idx].u.empty_buffer.pcie_address;
	dst_iova = (dma_addr_t)dst_addr;
	dst_len = ep2h_empty_msg[rd_idx].u.empty_buffer.buffer_len;
	dst_cookie = ep2h_empty_msg[rd_idx].u.empty_buffer.cookie;
	/*
	 * Map host dst mem to local PCIe address range.
	 * PCIe address range is SZ_64K aligned.
	 */
	dst_masked = (dst_addr & ~(SZ_64K - 1));
	dst_off = (dst_addr & (SZ_64K - 1));

#if !ENABLE_DMA
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	ret = lpci_epc_map_addr(epc, epf->func_no, tvnet->tx_dst_pci_addr,
				dst_masked, dst_len);
#else
	ret = pci_epc_map_addr(epc, tvnet->tx_dst_pci_addr, dst_masked,
			       dst_len);
#endif
	if (ret < 0) {
		dev_err(fdev, "failed to map dst addr to PCIe addr range\n");
		dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
		dev_kfree_skb_any(skb);
		return NETDEV_TX_OK;
	}
#endif
	/*
	 * Advance read count after all failure cases completed, to avoid
	 * dangling buffer at host.
	 */
	tvnet_ivc_advance_rd(&tvnet->ep2h_empty);

#if ENABLE_DMA
	/* Submit DMA transfer using tegra-pcie-dma wrapper */
	/* Allocate context for callback */
	tx_ctx = mempool_alloc(tvnet->tx_ctx_pool, GFP_ATOMIC);
	if (!tx_ctx) {
		dev_err(fdev, "%s: Failed to allocate TX context\n", __func__);
		dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
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
	if (((desc_cnt->wr_cnt - desc_cnt->rd_cnt) + tvnet->tx_batch_cnt) >= DMA_DESC_COUNT) {
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
		mempool_free(tx_ctx, tvnet->tx_ctx_pool);
		dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	tvnet->tx_batch_ctx[tvnet->tx_batch_cnt] = tx_ctx;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].src = src_iova;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].dst = dst_iova;
	tvnet->tx_batch_desc[tvnet->tx_batch_cnt].sz = len;
	tvnet->tx_batch_cnt++;
	if (tvnet->tx_batch_cnt == 1) {
		delay_us = netdev_xmit_more() ?
			TVNET_TX_BATCH_DELAY_MORE_US : TVNET_TX_BATCH_DELAY_US;
		hrtimer_start(&tvnet->tx_batch_timer,
			      ns_to_ktime(delay_us * NSEC_PER_USEC),
			      HRTIMER_MODE_REL);
	}

	txq = netdev_get_tx_queue(ndev, skb_get_queue_mapping(skb));
	flush_reason_stopped = netif_xmit_stopped(txq);
	flush_reason_max = (tvnet->tx_batch_cnt >= TVNET_TX_BATCH_MAX);
	flush_now = (flush_reason_stopped || flush_reason_max);
	if (!flush_now) {
		spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
		return NETDEV_TX_OK;
	}

	hrtimer_try_to_cancel(&tvnet->tx_batch_timer);

	if (flush_reason_stopped)
		tvnet->tx_dma_flush_txq_stopped++;
	if (flush_reason_max)
		tvnet->tx_dma_flush_batch_max++;

	spin_unlock_irqrestore(&tvnet->tx_batch_lock, flags);
	queue_delayed_work(system_highpri_wq, &tvnet->tx_batch_flush_work, 0);
#else
	/* Copy skb->data to host dst address, use CPU virt addr */
	memcpy_toio((void *)(tvnet->tx_dst_va + dst_off), skb->data, len);
	/*
	 * tx_dst_va is ioremap_wc() mem, add mb to make sure complete skb->data
	 * written to dst before adding it to full buffer
	 */
	mb();

	/* For non-DMA mode, push dst to EP2H full ring immediately after memcpy */
	/* Poll for space in ep2h_full ring before writing */
	if (tvnet_ep_poll_ep2h_full_space(tvnet, ndev) == 0) {
		struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
		struct data_msg *ep2h_full_msg = ep_ring_buf->ep2h_full_msgs;
		u32 wr_idx;

		wr_idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_full) % RING_COUNT;
		ep2h_full_msg[wr_idx].u.full_buffer.packet_size = len;
		ep2h_full_msg[wr_idx].u.full_buffer.pcie_address = dst_iova;
		ep2h_full_msg[wr_idx].u.full_buffer.cookie = dst_cookie;
		mb(); /* Ensure data written before counter increment */
		tvnet_ivc_advance_wr(&tvnet->ep2h_full);

		schedule_work(&tvnet->raise_irq_work);
	}
	/* else: error already logged and tx_dropped incremented by poll function */

	/* Free temp src and skb - for non-DMA mode */
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	lpci_epc_unmap_addr(epc, epf->func_no, tvnet->tx_dst_pci_addr);
#else
	pci_epc_unmap_addr(epc, tvnet->tx_dst_pci_addr);
#endif
	dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
	dev_kfree_skb_any(skb);
#endif
	return NETDEV_TX_OK;
}

static const struct net_device_ops tvnet_netdev_ops = {
	.ndo_open = tvnet_ep_open,
	.ndo_stop = tvnet_ep_close,
	.ndo_start_xmit = tvnet_ep_start_xmit,
	.ndo_change_mtu = tvnet_ep_change_mtu,
};

static void tvnet_ep_process_ctrl_msg(struct pci_epf_tvnet *tvnet)
{
	struct ctrl_msg msg;

	while (tvnet_ivc_rd_available(&tvnet->h2ep_ctrl)) {
		tvnet_ep_read_ctrl_msg(tvnet, &msg);
		if (msg.msg_id == CTRL_MSG_LINK_UP)
			tvnet_ep_rcv_link_up_msg(tvnet);
		else if (msg.msg_id == CTRL_MSG_LINK_DOWN)
			tvnet_ep_rcv_link_down_msg(tvnet);
		else if (msg.msg_id == CTRL_MSG_LINK_DOWN_ACK)
			tvnet_ep_rcv_link_down_ack(tvnet);
	}
}

static void tvnet_ep_ctrl_work(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet =
		container_of(work, struct pci_epf_tvnet, ctrl_work);
	struct net_device *ndev;

	if (!tvnet)
		return;

	ndev = tvnet->ndev;

	if (!ndev)
		return;

	if (netif_queue_stopped(ndev)) {
		if (tvnet->os_link_state == OS_LINK_STATE_UP &&
		    tvnet_ivc_rd_available(&tvnet->ep2h_empty) &&
		    !tvnet_ivc_full(&tvnet->ep2h_full)) {
			netif_wake_queue(ndev);
		}
	}

	if (tvnet_ivc_rd_available(&tvnet->h2ep_ctrl))
		tvnet_ep_process_ctrl_msg(tvnet);

	/*
	 * Refill RX empty buffers eagerly.
	 * Don't check tvnet_ivc_full() here because the counter read can be
	 * stale (Host may have consumed empties after our read). The refill
	 * worker itself will stop when it encounters a truly full ring.
	 */
	if (tvnet->os_link_state == OS_LINK_STATE_UP)
		queue_work(system_unbound_wq, &tvnet->refill_work);

#if ENABLE_DMA
	/*
	 * If we have deferred TX completions because the FULL ring was
	 * temporarily full, retry immediately when we get control activity.
	 */
	if (READ_ONCE(tvnet->tx_complete_list.first) &&
	    !tvnet_ivc_full(&tvnet->ep2h_full))
		queue_work(system_unbound_wq, &tvnet->tx_complete_work.work);
#endif
}

static int tvnet_ep_process_h2ep_msg(struct pci_epf_tvnet *tvnet)
{
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct data_msg *data_msg = host_ring_buf->h2ep_full_msgs;
	struct h2ep_empty_list *h2ep_empty_ptr;
	struct net_device *ndev = tvnet->ndev;
#if ENABLE_DMA
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct sk_buff *skb;
	int idx;
	u32 len;
	u64 pcie_address;
	u16 cookie;
#else
	unsigned long flags;
#endif
	int count = 0;

	while ((count < TVNET_NAPI_WEIGHT) &&
	       tvnet_ivc_rd_available(&tvnet->h2ep_full)) {
		/* Read H2EP full msg */
		idx = tvnet_ivc_get_rd_cnt(&tvnet->h2ep_full) % RING_COUNT;
		len = data_msg[idx].u.full_buffer.packet_size;
		pcie_address = data_msg[idx].u.full_buffer.pcie_address;
		cookie = data_msg[idx].u.full_buffer.cookie;

		/* Advance H2EP full buffer after removing from local map */
		tvnet_ivc_advance_rd(&tvnet->h2ep_full);

		h2ep_empty_ptr = tvnet_ep_h2ep_cookie_take(tvnet, cookie);
		if (unlikely(!h2ep_empty_ptr)) {
			h2ep_empty_ptr =
				tvnet_ep_h2ep_cookie_take_by_iova(tvnet,
								  (dma_addr_t)pcie_address);
			/* recovered */
		}
		if (WARN_ON(!h2ep_empty_ptr))
			continue;
		if (unlikely(h2ep_empty_ptr->iova != (dma_addr_t)pcie_address)) {
			/* mismatch */
		}
#if ENABLE_DMA
		if (h2ep_empty_ptr->offset) {
			dma_sync_single_range_for_cpu(cdev,
						      h2ep_empty_ptr->dma_base,
						      h2ep_empty_ptr->offset,
						      len, DMA_FROM_DEVICE);
		} else {
			dma_unmap_single(cdev, h2ep_empty_ptr->iova, h2ep_empty_ptr->size,
					 DMA_FROM_DEVICE);
		}
		skb = h2ep_empty_ptr->skb;
		skb_put(skb, len);
		skb->protocol = eth_type_trans(skb, ndev);
		napi_gro_receive(&tvnet->napi, skb);
		/* Buffer tracking node is no longer needed after handing skb up. */
		kfree(h2ep_empty_ptr);
#else
		/* Alloc new skb and copy data from full buffer */
		skb = netdev_alloc_skb(ndev, len);
		if (!skb) {
			dev_err(&ndev->dev, "%s: netdev_alloc_skb() failed\n", __func__);
			/* Queue for deferred cleanup - vunmap cannot be called from softirq */
			spin_lock_irqsave(&tvnet->rx_cleanup_lock, flags);
			list_add_tail(&h2ep_empty_ptr->list, &tvnet->rx_cleanup_list);
			spin_unlock_irqrestore(&tvnet->rx_cleanup_lock, flags);
			schedule_work(&tvnet->rx_cleanup_work);
			count++;
			continue;
		}
		memcpy(skb->data, h2ep_empty_ptr->virt, len);
		skb_put(skb, len);
		skb->protocol = eth_type_trans(skb, ndev);
		napi_gro_receive(&tvnet->napi, skb);

		/* Queue for deferred cleanup - vunmap cannot be called from softirq */
		spin_lock_irqsave(&tvnet->rx_cleanup_lock, flags);
		list_add_tail(&h2ep_empty_ptr->list, &tvnet->rx_cleanup_list);
		spin_unlock_irqrestore(&tvnet->rx_cleanup_lock, flags);
		schedule_work(&tvnet->rx_cleanup_work);
#endif

		count++;
	}

	return count;
}

#if ENABLE_DMA
/* Platform MSI write callback for T264 */
static void tvnet_ep_platform_msi_write(struct msi_desc *desc, struct msi_msg *msg)
{
	struct pci_epf_tvnet *tvnet = (struct pci_epf_tvnet *)dev_get_drvdata(desc->dev);

	if (!tvnet) {
		pr_err("%s: tvnet is NULL\n", __func__);
		return;
	}

	if (tvnet->msi_addr == 0) {
		tvnet->msi_addr = msg->address_hi;
		tvnet->msi_addr <<= 32;
		tvnet->msi_addr |= msg->address_lo;
		tvnet->msi_data = msg->data;
		pr_err("tvnet: %p MSI addr: 0x%llx, data: 0x%x desc_irq: %d\n",
		       tvnet, tvnet->msi_addr, tvnet->msi_data, desc->irq);
	}
}

static void tvnet_ep_platform_msi_free_irqs(struct device *cdev)
{
#if defined(NV_PLATFORM_DEVICE_MSI_INIT_AND_ALLOC_IRQS_PRESENT) /* Linux v6.9 */
	platform_device_msi_free_irqs_all(cdev);
#else
	platform_msi_domain_free_irqs(cdev);
#endif
}

/* Initialize DMA using tegra-pcie-dma wrapper library for local DMA write */
static int tvnet_ep_dma_init(struct pci_epf_tvnet *tvnet)
{
	struct tegra_pcie_dma_init_info dma_info = {0};
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *fdev = tvnet->fdev;
	struct device *cdev = epc->dev.parent;
	tegra_pcie_dma_status_t status;
	void *orig_drvdata;
	int ret = 0;

	/* Configure TX channel 0 (write channel for EP->RP transfers) */
	dma_info.tx[0].ch_type = TEGRA_PCIE_DMA_CHAN_XFER_ASYNC;
	dma_info.tx[0].num_descriptors = DMA_DESC_COUNT;

	dma_info.dev = cdev;
	dma_info.soc = (tvnet->soc_id == TEGRA_VNET_SOC_T264) ?
			NVPCIE_DMA_SOC_T264 :
			NVPCIE_DMA_SOC_T234;
	dma_info.remote = NULL;  /* Local DMA */

	/* Platform MSI allocation for T264 */
	if (tvnet->soc_id == TEGRA_VNET_SOC_T264) {
		/* Temporarily set tvnet as cdev driver data so MSI callback can retrieve it */
		orig_drvdata = dev_get_drvdata(cdev);
		dev_set_drvdata(cdev, tvnet);

		/* Allocate 8 platform MSIs on cdev (which has IOMMU domain for IOVAs) */
#if defined(NV_PLATFORM_DEVICE_MSI_INIT_AND_ALLOC_IRQS_PRESENT) /* Linux 6.9 */
		ret = platform_device_msi_init_and_alloc_irqs(cdev, 8,
						tvnet_ep_platform_msi_write);
#else
		ret = platform_msi_domain_alloc_irqs(cdev, 8, tvnet_ep_platform_msi_write);
#endif

		/* Get MSI IRQ for LOCAL DMA vector (vector 4) */
#if defined(NV_MSI_GET_VIRQ_PRESENT) /* Linux v6.1 */
		tvnet->dma_msi_irq = msi_get_virq(cdev, TEGRA264_PCIE_DMA_MSI_LOCAL_VEC);
#else
		{
			struct msi_desc *desc;
			tvnet->dma_msi_irq = -1;
			for_each_msi_entry(desc, cdev) {
				if (desc->platform.msi_index ==
					TEGRA264_PCIE_DMA_MSI_LOCAL_VEC) {
					tvnet->dma_msi_irq = desc->irq;
					break;
				}
			}
		}
#endif
		if (tvnet->dma_msi_irq < 0) {
			dev_err(fdev, "Failed to get MSI IRQ for LOCAL vector\n");
			tvnet_ep_platform_msi_free_irqs(cdev);
			return -EINVAL;
		}

		dma_info.msi_irq = tvnet->dma_msi_irq;
		dma_info.msi_addr = 0;  /* Will be set via set_msi API */
		dma_info.msi_data = 0;

		dev_info(fdev, "T264 platform MSI allocated: IRQ=%d\n", tvnet->dma_msi_irq);
	} else {
		/* T234 MSI-X configuration - not applicable for local DMA */
		dma_info.msi_irq = 0;
		dma_info.msi_addr = 0;
		dma_info.msi_data = 0;
	}

	/* Initialize DMA library */
	status = tegra_pcie_dma_initialize(&dma_info, &tvnet->dma_cookie);
	if (status != TEGRA_PCIE_DMA_SUCCESS) {
		dev_err(fdev, "tegra_pcie_dma_initialize() failed: %d\n", status);
		if (tvnet->soc_id == TEGRA_VNET_SOC_T264)
			tvnet_ep_platform_msi_free_irqs(cdev);
		return -EIO;
	}

	/* For T264, configure MSI using set_msi API */
	if (tvnet->soc_id == TEGRA_VNET_SOC_T264) {
		/* Restore original driver data */
		dev_set_drvdata(cdev, orig_drvdata);
		if (ret) {
			dev_err(fdev, "Failed to allocate platform MSI: %d\n", ret);
			tegra_pcie_dma_deinit(&tvnet->dma_cookie);
			tvnet_ep_platform_msi_free_irqs(cdev);
			return ret;
		}
		dev_info(fdev, "T264 DMA tvnet: %p MSI: addr=0x%llx data=0x%x\n",
			 tvnet, tvnet->msi_addr, tvnet->msi_data);

		status = tegra_pcie_dma_set_msi(tvnet->dma_cookie, tvnet->msi_addr,
						tvnet->msi_data);
		if (status != TEGRA_PCIE_DMA_SUCCESS) {
			dev_err(fdev, "tegra_pcie_dma_set_msi() failed: %d\n", status);
			tegra_pcie_dma_deinit(&tvnet->dma_cookie);
			tvnet_ep_platform_msi_free_irqs(cdev);
			return -EIO;
		}
	}

	tvnet->desc_cnt.rd_cnt = 0;
	tvnet->desc_cnt.wr_cnt = 0;

	dev_info(fdev, "DMA library initialized (local %s DMA write)\n",
		 (tvnet->soc_id == TEGRA_VNET_SOC_T264) ? "T264" : "T234");

	return 0;
}

static void tvnet_ep_setup_dma(struct pci_epf_tvnet *tvnet)
{
	int ret;

	/* Initialize DMA using wrapper library */
	ret = tvnet_ep_dma_init(tvnet);
	if (ret) {
		dev_err(tvnet->fdev, "DMA initialization failed: %d\n", ret);
		return;
	}
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
/* Returns aperture offset of syncpoint on SHIM_BASE. */
static inline u64 get_syncpt_shim_offset(u32 id, enum tegra_vnet_soc_id soc_id)
{
	u64 base = (soc_id == TEGRA_VNET_SOC_T264) ? T264_SHIM_BASE : SHIM_BASE;
	return (base + ((u64)id * SP_SIZE));
}

static void
host1x_cb_func(struct dma_fence *f, struct dma_fence_cb *cb)
{
	struct syncpt_t *syncpt = container_of(cb, struct syncpt_t, fence_cb);

	schedule_work(&syncpt->work);
}

static int
allocate_fence(struct syncpt_t *syncpt)
{
	int ret = 0;
	struct dma_fence *fence = NULL;

	mutex_lock(&syncpt->lock);
	fence = host1x_fence_create(syncpt->sp, ++syncpt->threshold, false);
	if (IS_ERR_OR_NULL(fence)) {
		ret = PTR_ERR(fence);
		pr_err("host1x_fence_create failed with: %d\n", ret);
		mutex_unlock(&syncpt->lock);
		return ret;
	}

	ret = dma_fence_add_callback(fence, &syncpt->fence_cb, host1x_cb_func);
	if (ret != 0) {
		/* If already expired. */
		if (ret == -ENOENT) {
			ret = 0;
			schedule_work(&syncpt->work);
		}
		goto put_fence;
	}
	syncpt->fence = fence;
	mutex_unlock(&syncpt->lock);

	return ret;

put_fence:
	dma_fence_put(fence);
	mutex_unlock(&syncpt->lock);
	return ret;
}

static void
fence_do_work(struct syncpt_t *syncpt)
{
	int ret = 0;

	if (syncpt->notifier) {
		syncpt->notifier(syncpt->notifier_data);
	}

	mutex_lock(&syncpt->lock);
	/* If deinit triggered, no need to proceed. */
	if (syncpt->fence_release) {
		mutex_unlock(&syncpt->lock);
		return;
	}

	if (syncpt->fence) {
		dma_fence_put(syncpt->fence);
		syncpt->fence = NULL;
	}
	mutex_unlock(&syncpt->lock);

	ret = allocate_fence(syncpt);
	if (ret != 0) {
		pr_err("allocate_fence failed with: %d\n", ret);
		return;
	}
}
#endif

static void tvnet_ep_ctrl_irqsp_work(struct work_struct *work)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	struct syncpt_t *syncpt =
		container_of(work, struct syncpt_t, work);

	fence_do_work(syncpt);
#else
	struct irqsp_data *data_irqsp =
		container_of(work, struct irqsp_data, reprime_work);
	nvhost_interrupt_syncpt_prime(data_irqsp->sp);
#endif
}

static void tvnet_ep_ctrl_irqsp_callback(void *private_data)
{
	struct irqsp_data *data_irqsp = private_data;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(data_irqsp->dev);

	/*
	 * Control notifications can lead to link-state transitions and buffer
	 * teardown; defer to process context (safe for both IRQ and work origins).
	 */
	queue_work(system_unbound_wq, &tvnet->ctrl_work);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
	schedule_work(&data_irqsp->reprime_work);
#endif
}

static void tvnet_ep_data_irqsp_work(struct work_struct *work)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	struct syncpt_t *syncpt =
		container_of(work, struct syncpt_t, work);

	fence_do_work(syncpt);
#else
	struct irqsp_data *data_irqsp =
		container_of(work, struct irqsp_data, reprime_work);

	nvhost_interrupt_syncpt_prime(data_irqsp->is);
#endif
}

static void tvnet_ep_data_irqsp_callback(void *private_data)
{
	struct irqsp_data *data_irqsp = private_data;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(data_irqsp->dev);

	if (tvnet_ivc_rd_available(&tvnet->h2ep_full))
		napi_schedule(&tvnet->napi);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
	else
		schedule_work(&data_irqsp->reprime_work);
#endif
}

static int tvnet_ep_poll(struct napi_struct *napi, int budget)
{
	struct pci_epf_tvnet *tvnet = container_of(napi, struct pci_epf_tvnet,
						   napi);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
	struct irqsp_data *data_irqsp = tvnet->data_irqsp;
#endif
	int work_done;

	work_done = tvnet_ep_process_h2ep_msg(tvnet);
	if (work_done < budget) {
		napi_complete(napi);
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
		schedule_work(&data_irqsp->reprime_work);
#endif
	}

	return work_done;
}

static int tvnet_ep_pci_epf_setup_irqsp(struct pci_epf_tvnet *tvnet)
{
	struct bar0_amap *amap = &tvnet->bar0_amap[SIMPLE_IRQ];
	struct irqsp_data *ctrl_irqsp, *data_irqsp;
	struct pci_epf *epf = tvnet->epf;
	struct device *fdev = tvnet->fdev;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct irq_md *irq;
	phys_addr_t syncpt_addr;
	int ret;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	struct host1x *host1x = NULL;
	struct syncpt_t *syncpt = NULL;

	host1x = platform_get_drvdata(tvnet->host1x_pdev);
	if (!host1x) {
		pr_err("Host1x handle is null.");
		return -EINVAL;
	}
#endif
	ctrl_irqsp = devm_kzalloc(fdev, sizeof(*ctrl_irqsp), GFP_KERNEL);
	if (!ctrl_irqsp) {
		ret = -ENOMEM;
		goto fail;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	syncpt = &ctrl_irqsp->syncpt;
	syncpt->sp = host1x_syncpt_alloc(host1x, HOST1X_SYNCPT_CLIENT_MANAGED,
					 "pcie-ep-vnet-ctrl");
	if (IS_ERR_OR_NULL(syncpt->sp)) {
		ret = -ENOMEM;
		pr_err("Failed to reserve comm notify syncpt\n");
		goto free_ctrl_sp;
	}

	syncpt->id = host1x_syncpt_id(syncpt->sp);
	ctrl_irqsp->dev = fdev;
	INIT_WORK(&syncpt->work, tvnet_ep_ctrl_irqsp_work);
	tvnet->ctrl_irqsp = ctrl_irqsp;

	syncpt->threshold = host1x_syncpt_read(syncpt->sp);

	/* enable syncpt notifications handling from peer.*/
	mutex_init(&syncpt->lock);
	syncpt->notifier = tvnet_ep_ctrl_irqsp_callback;
	syncpt->notifier_data = (void *)ctrl_irqsp;
	syncpt->host1x_cb_set = true;
	syncpt->fence_release = false;

	ret = allocate_fence(syncpt);
	if (ret != 0) {
		pr_err("allocate_fence failed with: %d\n", ret);
		goto free_ctrl_sp;
	}
#else
	ctrl_irqsp->is =
		nvhost_interrupt_syncpt_get(cdev->of_node,
					    tvnet_ep_ctrl_irqsp_callback,
					    ctrl_irqsp);
	if (IS_ERR(ctrl_irqsp->is)) {
		ret = PTR_ERR(ctrl_irqsp->is);
		dev_err(fdev, "failed to get ctrl syncpt irq: %d\n", ret);
		goto fail;
	}

	ctrl_irqsp->dev = fdev;
	INIT_WORK(&ctrl_irqsp->reprime_work, tvnet_ep_ctrl_irqsp_reprime_work);
	tvnet->ctrl_irqsp = ctrl_irqsp;
#endif
	data_irqsp = devm_kzalloc(fdev, sizeof(*data_irqsp), GFP_KERNEL);
	if (!data_irqsp) {
		ret = -ENOMEM;
		goto free_ctrl_sp;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	syncpt = &data_irqsp->syncpt;
	syncpt->sp = host1x_syncpt_alloc(host1x, HOST1X_SYNCPT_CLIENT_MANAGED,
					 "pcie-ep-vnet-data");
	if (IS_ERR_OR_NULL(syncpt->sp)) {
		ret = -ENOMEM;
		pr_err("Failed to reserve comm notify syncpt\n");
		goto free_ctrl_sp;
	}

	syncpt->id = host1x_syncpt_id(syncpt->sp);
	data_irqsp->dev = fdev;
	INIT_WORK(&syncpt->work, tvnet_ep_data_irqsp_work);
	tvnet->data_irqsp = data_irqsp;

	syncpt->threshold = host1x_syncpt_read(syncpt->sp);

	/* enable syncpt notifications handling from peer.*/
	mutex_init(&syncpt->lock);
	syncpt->notifier = tvnet_ep_data_irqsp_callback;
	syncpt->notifier_data = (void *)data_irqsp;
	syncpt->host1x_cb_set = true;
	syncpt->fence_release = false;

	ret = allocate_fence(syncpt);
	if (ret != 0) {
		pr_err("allocate_fence failed with: %d\n", ret);
		goto free_ctrl_sp;
	}

	syncpt_addr = get_syncpt_shim_offset(ctrl_irqsp->syncpt.id, tvnet->soc_id);
	ctrl_irqsp->syncpt.phy_addr = syncpt_addr;
	ctrl_irqsp->syncpt.size = PAGE_SIZE;
#else
	data_irqsp->is =
		nvhost_interrupt_syncpt_get(cdev->of_node,
					    tvnet_ep_data_irqsp_callback,
					    data_irqsp);
	if (IS_ERR(data_irqsp->is)) {
		ret = PTR_ERR(data_irqsp->is);
		dev_err(fdev, "failed to get data syncpt irq: %d\n", ret);
		goto free_ctrl_sp;
	}

	data_irqsp->dev = fdev;
	INIT_WORK(&data_irqsp->reprime_work, tvnet_ep_data_irqsp_reprime_work);
	tvnet->data_irqsp = data_irqsp;

	syncpt_addr = nvhost_interrupt_syncpt_get_syncpt_addr(ctrl_irqsp->is);
#endif
	{
		ulong prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;

#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
		ret = iommu_map(domain, amap->iova, syncpt_addr, PAGE_SIZE,
				prot, GFP_KERNEL);
#else
		ret = iommu_map(domain, amap->iova, syncpt_addr, PAGE_SIZE,
				prot);
#endif
	}
	if (ret < 0) {
		dev_err(fdev, "%s: iommu_map of ctrlsp mem failed: %d\n",
			__func__, ret);
		goto free_data_sp;
	}

	irq = &tvnet->bar_md->irq_ctrl;
	irq->irq_addr = PAGE_SIZE;
	irq->irq_type = IRQ_SIMPLE;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	syncpt_addr = get_syncpt_shim_offset(data_irqsp->syncpt.id, tvnet->soc_id);
	data_irqsp->syncpt.phy_addr = syncpt_addr;
	data_irqsp->syncpt.size = PAGE_SIZE;
#else
	syncpt_addr = nvhost_interrupt_syncpt_get_syncpt_addr(data_irqsp->is);
#endif
	{
		ulong prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;

#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
		ret = iommu_map(domain, amap->iova + PAGE_SIZE, syncpt_addr,
				PAGE_SIZE, prot, GFP_KERNEL);
#else
		ret = iommu_map(domain, amap->iova + PAGE_SIZE, syncpt_addr,
				PAGE_SIZE, prot);
#endif
	}
	if (ret < 0) {
		dev_err(fdev, "%s: iommu_map of datasp mem failed: %d\n",
			__func__, ret);
		goto free_ctrl_ivoa;
	}

	irq = &tvnet->bar_md->irq_data;
	irq->irq_addr = 2 * PAGE_SIZE;
	irq->irq_type = IRQ_SIMPLE;

	return 0;

free_ctrl_ivoa:
	iommu_unmap(domain, amap->iova, PAGE_SIZE);
free_data_sp:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	host1x_syncpt_put(data_irqsp->syncpt.sp);
free_ctrl_sp:
	host1x_syncpt_put(ctrl_irqsp->syncpt.sp);
#else
	nvhost_interrupt_syncpt_free(data_irqsp->is);
free_ctrl_sp:
	nvhost_interrupt_syncpt_free(ctrl_irqsp->is);
#endif
fail:
	return ret;
}

static void tvnet_ep_pci_epf_destroy_irqsp(struct pci_epf_tvnet *tvnet)
{
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);

	iommu_unmap(domain, tvnet->bar0_amap[SIMPLE_IRQ].iova + PAGE_SIZE,
		    PAGE_SIZE);
	iommu_unmap(domain, tvnet->bar0_amap[SIMPLE_IRQ].iova, PAGE_SIZE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	host1x_syncpt_put(tvnet->data_irqsp->syncpt.sp);
	host1x_syncpt_put(tvnet->ctrl_irqsp->syncpt.sp);
#else
	nvhost_interrupt_syncpt_free(tvnet->data_irqsp->is);
	nvhost_interrupt_syncpt_free(tvnet->ctrl_irqsp->is);
#endif
}

static int tvnet_ep_alloc_single_page_bar0_mem(struct pci_epf *epf,
					       enum bar0_amap_type type)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct bar0_amap *amap = &tvnet->bar0_amap[type];
	int ret = 0;

	amap->page = alloc_pages(GFP_KERNEL, 1);
	if (!amap->page) {
		dev_err(tvnet->fdev, "%s: type: %d alloc_pages() failed\n",
			__func__, type);
		ret = -ENOMEM;
		goto fail;
	}

	{
		ulong prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;

#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
		ret = iommu_map(domain, amap->iova, page_to_phys(amap->page),
				PAGE_SIZE, prot, GFP_KERNEL);
#else
		ret = iommu_map(domain, amap->iova, page_to_phys(amap->page),
				PAGE_SIZE, prot);
#endif
	}
	if (ret < 0) {
		dev_err(tvnet->fdev, "%s: type: %d iommu_map(RAM) failed: %d\n",
			__func__, type, ret);
		goto fail_free_pages;
	}

	amap->virt = vmap(&amap->page, 1, VM_MAP, PAGE_KERNEL);
	if (!amap->virt) {
		dev_err(tvnet->fdev, "%s: type: %d vmap() failed\n",
			__func__, type);
		ret = -ENOMEM;
		goto fail_unmap_iova;
	}

	return 0;

fail_unmap_iova:
	iommu_unmap(domain, amap->iova, PAGE_SIZE);
fail_free_pages:
	__free_pages(amap->page, 1);
fail:
	return ret;
}

static void tvnet_ep_free_single_page_bar0_mem(struct pci_epf *epf,
					       enum bar0_amap_type type)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct bar0_amap *amap = &tvnet->bar0_amap[type];

	vunmap(amap->virt);
	iommu_unmap(domain, amap->iova, PAGE_SIZE);
	__free_pages(amap->page, 1);
}

static int tvnet_ep_alloc_multi_page_bar0_mem(struct pci_epf *epf,
					      enum bar0_amap_type type)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct bar0_amap *amap = &tvnet->bar0_amap[type];
	struct page **map;
	int ret = 0, page_count, order, i;

	page_count = amap->size >> PAGE_SHIFT;
	order = get_order(amap->size);

	map = kmalloc(sizeof(struct page *) << order, GFP_KERNEL);
	if (!map)
		return -ENOMEM;

	amap->page = alloc_pages(GFP_KERNEL, order);
	if (!amap->page) {
		dev_err(tvnet->fdev, "%s: alloc_pages() failed\n", __func__);
		ret = -ENOMEM;
		goto fail;
	}

	split_page(amap->page, order);

	order = 1 << order;
	map[0] = amap->page;
	for (i = 1; i < page_count; i++)
		map[i] = amap->page + i;
	for (; i < order; i++)
		__free_page(amap->page + i);

	amap->virt = vmap(map, page_count, VM_MAP, PAGE_KERNEL);
	if (!amap->virt) {
		dev_err(tvnet->fdev, "%s: vmap() failed\n", __func__);
		ret = -ENOMEM;
		goto fail_free_pages;
	}

	{
		ulong prot = IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE;

#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
		ret = iommu_map(domain, amap->iova, page_to_phys(amap->page),
				amap->size, prot, GFP_KERNEL);
#else
		ret = iommu_map(domain, amap->iova, page_to_phys(amap->page),
				amap->size, prot);
#endif
	}
	if (ret < 0) {
		dev_err(tvnet->fdev, "%s: iommu_map(RAM) failed: %d\n",
			__func__, ret);
		goto fail_vunmap;
	}

	kfree(map);
	return 0;

fail_vunmap:
	vunmap(amap->virt);
fail_free_pages:
	__free_pages(amap->page, get_order(amap->size));
fail:
	kfree(map);

	return ret;
}

static void tvnet_ep_free_multi_page_bar0_mem(struct pci_epf *epf,
					      enum bar0_amap_type type)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct bar0_amap *amap = &tvnet->bar0_amap[type];

	iommu_unmap(domain, amap->iova, amap->size);
	vfree(amap->virt);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
static int tvnet_get_host1x_dev(struct pci_epf *epf)
{
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct platform_device *host1x_pdev = NULL;
	struct device_node *np = NULL;
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);

	if (WARN_ON(!tvnet || !cdev->of_node))
		return -EINVAL;

	np = of_parse_phandle(cdev->of_node, "nvidia,host1x", 0);
	if (!np) {
		dev_err(tvnet->fdev, "Failed to find host1x, syncpt support disabled");
		return -ENODATA;
	}

	host1x_pdev = of_find_device_by_node(np);
	if (!host1x_pdev) {
		dev_err(tvnet->fdev, "host1x device not available");
		return -EPROBE_DEFER;
	}

	tvnet->host1x_pdev = host1x_pdev;

	return 0;
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
static int tvnet_ep_pci_epf_core_init(struct pci_epf *epf)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct pci_epf_header *header = epf->header;
	struct device *fdev = &epf->dev;
	struct pci_epf_bar *epf_bar;
	int ret;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	if (atomic_read(&tvnet->core_initialized)) {
		pr_err("Received CORE_INIT callback again\n");
		return -EINVAL;
	}
#endif
	ret = lpci_epc_write_header(epc, epf->func_no, header);
	if (ret) {
		dev_err(fdev, "pci_epc_write_header() failed: %d\n", ret);
		return ret;
	}

	/* Set BARs based on SoC type */
	if (tvnet->soc_id == TEGRA_VNET_SOC_T264) {
		/* T264 BAR Layout:
		 * EPF BAR0 = DMA registers (hardware managed, no SW setup needed)
		 * EPF BAR1 = Shared memory (64-bit prefetchable, becomes Host BAR2)
		 * EPF BAR2 = Dummy BAR (size only, becomes Host BAR4)
		 */

		/* Set BAR1 (shared memory) */
		epf_bar = &epf->bar[BAR_1];
		epf_bar->phys_addr = tvnet->bar0_iova;
		epf_bar->addr = tvnet->bar_md;
		epf_bar->size = BAR0_SIZE;
		epf_bar->barno = BAR_1;
		epf_bar->flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
				 PCI_BASE_ADDRESS_MEM_TYPE_64 |
				 PCI_BASE_ADDRESS_MEM_PREFETCH;

		ret = lpci_epc_set_bar(epc, epf->func_no, epf_bar);
		if (ret) {
			dev_err(fdev, "Failed to set BAR1: %d\n", ret);
			return ret;
		}

		/* Set BAR2 (dummy BAR - size only, no memory) */
		epf_bar = &epf->bar[BAR_2];
		epf_bar->phys_addr = 0;
		epf_bar->addr = NULL;
		epf_bar->size = SZ_32M;
		epf_bar->barno = BAR_2;
		epf_bar->flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
		PCI_BASE_ADDRESS_MEM_TYPE_64 | PCI_BASE_ADDRESS_MEM_PREFETCH;

		ret = lpci_epc_set_bar(epc, epf->func_no, epf_bar);
		if (ret) {
			dev_err(fdev, "Failed to set BAR2: %d\n", ret);
			return ret;
		}

		/* Set MSI (16 vectors for T264) */
		ret = lpci_epc_set_msi(epc, epf->func_no, 16);
	} else {
		/* T234 BAR Layout (existing):
		 * EPF BAR0 = Shared memory (becomes Host BAR0)
		 * EPF BAR1 = MSI-X table (becomes Host BAR2)
		 * EPF BAR2 = DMA registers (becomes Host BAR4)
		 */

		/* Set BAR0 (shared memory) */
		epf_bar = &epf->bar[BAR_0];
		epf_bar->phys_addr = tvnet->bar0_iova;
		epf_bar->addr = tvnet->bar_md;
		epf_bar->size = BAR0_SIZE;
		epf_bar->barno = BAR_0;
		epf_bar->flags = PCI_BASE_ADDRESS_SPACE_MEMORY |
				 PCI_BASE_ADDRESS_MEM_TYPE_32;

		ret = lpci_epc_set_bar(epc, epf->func_no, epf_bar);
		if (ret) {
			dev_err(fdev, "Failed to set BAR0: %d\n", ret);
			return ret;
		}

		/* Set MSI-X */
		ret = lpci_epc_set_msi(epc, epf->func_no, epf->msi_interrupts);
	}

	if (ret) {
		dev_err(fdev, "pci_epc_set_msi() failed: %d\n", ret);
		return ret;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	atomic_set(&tvnet->core_initialized, 1);

#endif
	return 0;
}
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0) && \
	LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
static int tvnet_ep_pci_epf_notifier(struct notifier_block *nb,
				     unsigned long val, void *data)
{
	struct pci_epf *epf = container_of(nb, struct pci_epf, nb);
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	int ret;

	switch (val) {
	case CORE_INIT:
		ret = tvnet_ep_pci_epf_core_init(epf);
		if (ret)
			return NOTIFY_BAD;
		break;

	case LINK_UP:
#if ENABLE_DMA
		tvnet_ep_setup_dma(tvnet);
#endif

		/*
		 * If host goes through a suspend resume, it recycles EP2H
		 * empty buffer. Clear any pending EP2H full buffer by setting
		 * "wr_cnt = rd_cnt".
		 */
		tvnet_ivc_set_wr(&tvnet->ep2h_full,
				 tvnet_ivc_get_rd_cnt(&tvnet->ep2h_full));

		tvnet->pcie_link_status = true;
		break;

	default:
		dev_err(&epf->dev, "Invalid EPF test notifier event\n");
		return NOTIFY_BAD;
	}

	return NOTIFY_OK;
}
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
static int tvnet_ep_pci_epf_linkup(struct pci_epf *epf)
#else
static void tvnet_ep_pci_epf_linkup(struct pci_epf *epf)
#endif
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);

#if ENABLE_DMA
	tvnet_ep_setup_dma(tvnet);
#endif

	/*
	 * If host goes through a suspend resume, it recycles EP2H empty buffer.
	 * Clear any pending EP2H full buffer by setting "wr_cnt = rd_cnt".
	 */
	tvnet_ivc_set_wr(&tvnet->ep2h_full,
			 tvnet_ivc_get_rd_cnt(&tvnet->ep2h_full));

	tvnet->pcie_link_status = true;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	return 0;
#endif
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_EPC_DEINIT) || \
    defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_CORE_DEINIT) /* Linux v6.11 || Nvidia Internal */
static int tvnet_ep_pci_epf_core_deinit(struct pci_epf *epf)
{
	struct pci_epf_bar *epf_bar = &epf->bar[BAR_0];
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	if (!tvnet)
		return -EINVAL;

	if (atomic_read(&tvnet->core_initialized)) {
		lpci_epc_clear_bar(epf->epc, epf->func_no, epf_bar);
		/* no api to clear epf header.*/
		atomic_set(&tvnet->core_initialized, 0);
	}

	return 0;
}

#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_EPC_DEINIT)
static void tvnet_ep_pci_epf_epc_deinit(struct pci_epf *epf)
{
	WARN_ON(tvnet_ep_pci_epf_core_deinit(epf));
}
#endif
#endif

static const struct pci_epc_event_ops tvnet_event_ops = {
#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_EPC_INIT) /* Linux v6.11 */
	.epc_init = tvnet_ep_pci_epf_core_init,
#else
	.core_init = tvnet_ep_pci_epf_core_init,
#endif
#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_EPC_DEINIT) /* Linux v6.11 */
	.epc_deinit = tvnet_ep_pci_epf_epc_deinit,
#elif defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_CORE_DEINIT) /* Nvidia Internal */
	.core_deinit = tvnet_ep_pci_epf_core_deinit,
#endif
	.link_up = tvnet_ep_pci_epf_linkup,
};

enum iommu_dma_cookie_type {
	IOMMU_DMA_IOVA_COOKIE,
	IOMMU_DMA_MSI_COOKIE,
};

struct iommu_dma_cookie {
	enum iommu_dma_cookie_type      type;
	union {
		/* Full allocator for IOMMU_DMA_IOVA_COOKIE */
		struct iova_domain      iovad;
		/* Trivial linear page allocator for IOMMU_DMA_MSI_COOKIE */
		dma_addr_t              msi_iova;
	};
	struct list_head                msi_page_list;

	/* Domain for flush queue callback; NULL if flush queue not in use */
	struct iommu_domain             *fq_domain;
};
#endif

static int tvnet_ep_pci_epf_bind(struct pci_epf *epf)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	struct pci_epf_header *header = epf->header;
#endif
	struct device *fdev = &epf->dev;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	const struct pci_epc_features *epc_features;
	struct net_device *ndev;
	struct bar_md *bar_md;	struct bar0_amap *amap;
	int ret, size, bitmap_size;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	unsigned long shift;
#endif
	if (!domain) {
		dev_err(fdev, "IOMMU domain not found\n");
		ret = -ENXIO;
		goto fail;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	ret = pci_epc_write_header(epc, header);
	if (ret < 0) {
		dev_err(fdev, "pci_epc_write_header() failed: %d\n", ret);
		goto fail;
	}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	tvnet->iovad = (struct iova_domain *)&domain->iova_cookie->iovad;

	shift = iova_shift(tvnet->iovad);

	tvnet->iova = alloc_iova(tvnet->iovad, BAR0_SIZE >> shift,
				 cdev->coherent_dma_mask >> shift, true);
	if (!tvnet->iova) {
		dev_err(fdev, "iova region map failed\n");
		goto fail;
	}

	tvnet->bar0_iova = iova_dma_addr(tvnet->iovad, tvnet->iova);
	tvnet_get_host1x_dev(epf);
#else
	tvnet->bar0_iova = iommu_dma_alloc_iova(cdev, BAR0_SIZE,
						cdev->coherent_dma_mask);
	if (!tvnet->bar0_iova) {
		dev_err(fdev, "iommu_dma_alloc_iova() failed\n");
		ret = -ENOMEM;
		goto fail;
	}
#endif
	pr_debug("BAR0 IOVA: 0x%llx\n", tvnet->bar0_iova);

	/* BAR0 metadata memory allocation */
	tvnet->bar0_amap[META_DATA].iova = tvnet->bar0_iova;
	tvnet->bar0_amap[META_DATA].size = PAGE_SIZE;
	ret = tvnet_ep_alloc_single_page_bar0_mem(epf, META_DATA);
	if (ret < 0) {
		dev_err(fdev, "BAR0 metadata alloc failed: %d\n", ret);
		goto free_iova;
	}

	tvnet->bar_md = (struct bar_md *)tvnet->bar0_amap[META_DATA].virt;
	bar_md = tvnet->bar_md;

	/* BAR0 SIMPLE_IRQ setup: two interrupts required two pages */
	amap = &tvnet->bar0_amap[SIMPLE_IRQ];
	amap->iova = tvnet->bar0_amap[META_DATA].iova +
		tvnet->bar0_amap[META_DATA].size;
	amap->size = 2 * PAGE_SIZE;

	ret = tvnet_ep_pci_epf_setup_irqsp(tvnet);
	if (ret < 0) {
		dev_err(fdev, "irqsp setup failed: %d\n", ret);
		goto free_bar0_md;
	}

	/* BAR0 EP memory allocation */
	amap = &tvnet->bar0_amap[EP_MEM];
	amap->iova = tvnet->bar0_amap[SIMPLE_IRQ].iova +
		tvnet->bar0_amap[SIMPLE_IRQ].size;
	size = sizeof(struct ep_own_cnt) + (RING_COUNT *
		(sizeof(struct ctrl_msg) + 2 * sizeof(struct data_msg)));
	amap->size = PAGE_ALIGN(size);
	ret = tvnet_ep_alloc_multi_page_bar0_mem(epf, EP_MEM);
	if (ret < 0) {
		dev_err(fdev, "BAR0 EP mem alloc failed: %d\n", ret);
		goto free_irqsp;
	}

	ep_ring_buf->ep_cnt = (struct ep_own_cnt *)amap->virt;
	ep_ring_buf->ep2h_ctrl_msgs = (struct ctrl_msg *)
				(ep_ring_buf->ep_cnt + 1);
	ep_ring_buf->ep2h_full_msgs = (struct data_msg *)
				(ep_ring_buf->ep2h_ctrl_msgs + RING_COUNT);
	ep_ring_buf->h2ep_empty_msgs = (struct data_msg *)
				(ep_ring_buf->ep2h_full_msgs + RING_COUNT);
	/* Clear EP counters */
	memset(ep_ring_buf->ep_cnt, 0, sizeof(struct ep_own_cnt));

	/* BAR0 host memory allocation */
	amap = &tvnet->bar0_amap[HOST_MEM];
	amap->iova = tvnet->bar0_amap[EP_MEM].iova +
					tvnet->bar0_amap[EP_MEM].size;
	size = (sizeof(struct host_own_cnt)) + (RING_COUNT *
		(sizeof(struct ctrl_msg) + 2 * sizeof(struct data_msg)));
	amap->size = PAGE_ALIGN(size);
	ret = tvnet_ep_alloc_multi_page_bar0_mem(epf, HOST_MEM);
	if (ret < 0) {
		dev_err(fdev, "BAR0 host mem alloc failed: %d\n", ret);
		goto free_ep_mem;
	}

	host_ring_buf->host_cnt = (struct host_own_cnt *)amap->virt;
	host_ring_buf->h2ep_ctrl_msgs = (struct ctrl_msg *)
				(host_ring_buf->host_cnt + 1);
	host_ring_buf->ep2h_empty_msgs = (struct data_msg *)
				(host_ring_buf->h2ep_ctrl_msgs + RING_COUNT);
	host_ring_buf->h2ep_full_msgs = (struct data_msg *)
				(host_ring_buf->ep2h_empty_msgs + RING_COUNT);
	/* Clear host counters */
	memset(host_ring_buf->host_cnt, 0, sizeof(struct host_own_cnt));

	/*
	 * Allocate local memory for DMA read link list elements.
	 * This is exposed through BAR0 to initiate DMA read from host.
	 */
	amap = &tvnet->bar0_amap[HOST_DMA];
	amap->iova = tvnet->bar0_amap[HOST_MEM].iova +
					tvnet->bar0_amap[HOST_MEM].size;
	size = (DMA_DESC_COUNT + 1) * 32;
	amap->size = PAGE_ALIGN(size);
	ret = tvnet_ep_alloc_multi_page_bar0_mem(epf, HOST_DMA);
	if (ret < 0) {
		dev_err(fdev, "BAR0 host dma mem alloc failed: %d\n", ret);
		goto free_host_mem;
	}

	/* Set link list pointer to create a dma desc ring */
	memset(amap->virt, 0, amap->size);

	/* Update BAR metadata region with offsets */
	/* EP owned memory */
	bar_md->ep_own_cnt_offset = tvnet->bar0_amap[META_DATA].size +
					tvnet->bar0_amap[SIMPLE_IRQ].size;
	bar_md->ctrl_md.ep2h_offset = bar_md->ep_own_cnt_offset +
					sizeof(struct ep_own_cnt);
	bar_md->ctrl_md.ep2h_size = RING_COUNT;
	bar_md->ep2h_md.ep2h_offset = bar_md->ctrl_md.ep2h_offset +
					(RING_COUNT * sizeof(struct ctrl_msg));
	bar_md->ep2h_md.ep2h_size = RING_COUNT;
	bar_md->h2ep_md.ep2h_offset = bar_md->ep2h_md.ep2h_offset +
					(RING_COUNT * sizeof(struct data_msg));
	bar_md->h2ep_md.ep2h_size = RING_COUNT;

	/* Host owned memory */
	bar_md->host_own_cnt_offset = bar_md->ep_own_cnt_offset +
					tvnet->bar0_amap[EP_MEM].size;
	bar_md->ctrl_md.h2ep_offset = bar_md->host_own_cnt_offset +
					sizeof(struct host_own_cnt);
	bar_md->ctrl_md.h2ep_size = RING_COUNT;
	bar_md->ep2h_md.h2ep_offset = bar_md->ctrl_md.h2ep_offset +
					(RING_COUNT * sizeof(struct ctrl_msg));
	bar_md->ep2h_md.h2ep_size = RING_COUNT;
	bar_md->h2ep_md.h2ep_offset = bar_md->ep2h_md.h2ep_offset +
					(RING_COUNT * sizeof(struct data_msg));
	bar_md->h2ep_md.h2ep_size = RING_COUNT;

	tvnet->h2ep_ctrl.rd = &ep_ring_buf->ep_cnt->h2ep_ctrl_rd_cnt;
	tvnet->h2ep_ctrl.wr = &host_ring_buf->host_cnt->h2ep_ctrl_wr_cnt;
	tvnet->ep2h_ctrl.rd = &host_ring_buf->host_cnt->ep2h_ctrl_rd_cnt;
	tvnet->ep2h_ctrl.wr = &ep_ring_buf->ep_cnt->ep2h_ctrl_wr_cnt;
	tvnet->h2ep_empty.rd = &host_ring_buf->host_cnt->h2ep_empty_rd_cnt;
	tvnet->h2ep_empty.wr = &ep_ring_buf->ep_cnt->h2ep_empty_wr_cnt;
	tvnet->h2ep_full.rd = &ep_ring_buf->ep_cnt->h2ep_full_rd_cnt;
	tvnet->h2ep_full.wr = &host_ring_buf->host_cnt->h2ep_full_wr_cnt;
	tvnet->ep2h_empty.rd = &ep_ring_buf->ep_cnt->ep2h_empty_rd_cnt;
	tvnet->ep2h_empty.wr = &host_ring_buf->host_cnt->ep2h_empty_wr_cnt;
	tvnet->ep2h_full.rd = &host_ring_buf->host_cnt->ep2h_full_rd_cnt;
	tvnet->ep2h_full.wr = &ep_ring_buf->ep_cnt->ep2h_full_wr_cnt;

	/* RAM region for use by host when programming EP DMA controller */
	bar_md->host_dma_offset = bar_md->host_own_cnt_offset +
					tvnet->bar0_amap[HOST_MEM].size;
	bar_md->host_dma_size = tvnet->bar0_amap[HOST_DMA].size;

	/* EP Rx pkt IOVA range */
	tvnet->rx_buf_iova = tvnet->bar0_amap[HOST_DMA].iova +
					tvnet->bar0_amap[HOST_DMA].size;
	bar_md->bar0_base_phy = tvnet->bar0_iova;
	bar_md->ep_rx_pkt_offset = bar_md->host_dma_offset +
					tvnet->bar0_amap[HOST_DMA].size;
	bar_md->ep_rx_pkt_size = BAR0_SIZE -
					tvnet->bar0_amap[META_DATA].size -
					tvnet->bar0_amap[SIMPLE_IRQ].size -
					tvnet->bar0_amap[EP_MEM].size -
					tvnet->bar0_amap[HOST_MEM].size -
					tvnet->bar0_amap[HOST_DMA].size;

	/* Create bitmap for allocating RX buffers */
	tvnet->rx_num_pages = (bar_md->ep_rx_pkt_size >> PAGE_SHIFT);
	bitmap_size = BITS_TO_LONGS(tvnet->rx_num_pages) * sizeof(long);
	tvnet->rx_buf_bitmap = devm_kzalloc(fdev, bitmap_size, GFP_KERNEL);
	if (!tvnet->rx_buf_bitmap) {
		dev_err(fdev, "rx_bitmap mem alloc failed\n");
		ret = -ENOMEM;
		goto free_host_dma;
	}

	/* Allocate PCIe memory for RP's dst address during xmit */
	tvnet->tx_dst_va = pci_epc_mem_alloc_addr(epc, &tvnet->tx_dst_pci_addr,
						  SZ_64K);
	if (!tvnet->tx_dst_va) {
		dev_err(fdev, "failed to allocate dst PCIe address\n");
		ret = -ENOMEM;
		goto free_host_dma;
	}
	dev_info(fdev, "tx_dst_va=%p, tx_dst_pci_addr=0x%llx\n",
		 tvnet->tx_dst_va, tvnet->tx_dst_pci_addr);

	/* Register network device */
	ndev = alloc_etherdev(0);
	if (!ndev) {
		dev_err(fdev, "alloc_etherdev() failed\n");
		ret = -ENOMEM;
		goto free_pci_mem;
	}

	eth_hw_addr_random(ndev);
	tvnet->ndev = ndev;
	SET_NETDEV_DEV(ndev, fdev);
	ndev->netdev_ops = &tvnet_netdev_ops;
	/*
	 * Enable software GRO to reduce per-packet RX cost (net_rx_action/NAPI).
	 * This is particularly important for high PPS workloads.
	 */
	ndev->hw_features |= NETIF_F_GRO;
	ndev->features |= NETIF_F_GRO;

#if defined(NV_NETIF_NAPI_ADD_WEIGHT_PRESENT) /* Linux v6.1 */
	netif_napi_add_weight(ndev, &tvnet->napi, tvnet_ep_poll, TVNET_NAPI_WEIGHT);
#else
	netif_napi_add(ndev, &tvnet->napi, tvnet_ep_poll, TVNET_NAPI_WEIGHT);
#endif
	ndev->mtu = TVNET_DEFAULT_MTU;

#if ENABLE_DMA && defined(CONFIG_PAGE_POOL)
	spin_lock_init(&tvnet->rx_pp_lock);
	tvnet->rx_pp_order = get_order(TVNET_MAX_MTU + ETH_HLEN +
				       TVNET_PP_HEADROOM +
				       SKB_DATA_ALIGN(sizeof(struct skb_shared_info)));
	{
		struct page_pool_params pp;

		tvnet_pp_init_params(&pp, cdev, ndev, tvnet->rx_pp_order);
		tvnet->rx_page_pool = page_pool_create(&pp);
		if (!tvnet->rx_page_pool)
			dev_warn(fdev, "RX page_pool disabled (create failed)\n");
	}
#endif

	ret = register_netdev(ndev);
	if (ret < 0) {
		dev_err(fdev, "register_netdev() failed: %d\n", ret);
		goto fail_free_netdev;
	}
	netif_carrier_off(ndev);

	tvnet->rx_link_state = DIR_LINK_STATE_DOWN;
	tvnet->tx_link_state = DIR_LINK_STATE_DOWN;
	tvnet->os_link_state = OS_LINK_STATE_DOWN;
	mutex_init(&tvnet->link_state_lock);
	init_waitqueue_head(&tvnet->link_state_wq);

	spin_lock_init(&tvnet->h2ep_cookie_lock);
	bitmap_zero(tvnet->h2ep_cookie_bmap, TVNET_RX_COOKIE_MAX);
	memset(tvnet->h2ep_cookie_map, 0, sizeof(tvnet->h2ep_cookie_map));

#if !ENABLE_DMA
	/* Initialize RX cleanup work queue (vunmap cannot be called from softirq) */
	INIT_LIST_HEAD(&tvnet->rx_cleanup_list);
	spin_lock_init(&tvnet->rx_cleanup_lock);
	INIT_WORK(&tvnet->rx_cleanup_work, tvnet_ep_rx_cleanup_work);
#endif

	INIT_WORK(&tvnet->raise_irq_work, tvnet_ep_raise_irq_work_function);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	/* TODO Update it to 64-bit prefetch type */
	ret = pci_epc_set_bar(epc, BAR_0, tvnet->bar0_iova, BAR0_SIZE,
			      PCI_BASE_ADDRESS_SPACE_MEMORY |
			      PCI_BASE_ADDRESS_MEM_TYPE_32);
	if (ret < 0) {
		dev_err(fdev, "pci_epc_set_bar() failed: %d\n", ret);
		goto fail_unreg_netdev;
	}
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	ret = pci_epc_set_msi(epc, epf->msi_interrupts);
	if (ret) {
		dev_err(fdev, "pci_epc_set_msi() failed: %d\n", ret);
		goto fail_clear_bar;
	}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
	nvhost_interrupt_syncpt_prime(tvnet->ctrl_irqsp->is);
	nvhost_interrupt_syncpt_prime(tvnet->data_irqsp->is);

#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	epf->nb.notifier_call = tvnet_ep_pci_epf_notifier;
	pci_epc_register_notifier(epc, &epf->nb);
#endif
#endif
/* For T264: Call core_init in bind if controller doesn't support core_init notifier */
	epc_features = pci_epc_get_features(epc, epf->func_no, epf->vfunc_no);
	if (!epc_features) {
		dev_err(fdev, "epc_features not implemented\n");
		ret = -EOPNOTSUPP;
		goto fail_clear_bar;
	}

#if defined(NV_PCI_EPC_FEATURES_STRUCT_HAS_CORE_INIT_NOTIFIER)
	if (!epc_features->core_init_notifier) {
		dev_info(fdev, "T264: Calling core_init from bind (no notifier support)\n");
		ret = tvnet_ep_pci_epf_core_init(epf);
		if (ret) {
			dev_err(fdev, "core_init failed: %d\n", ret);
			goto fail_clear_bar;
		}
		tvnet->pcie_link_status = true;
	}
#endif

	return 0;

fail_clear_bar:
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	pci_epc_clear_bar(epc, BAR_0);
fail_unreg_netdev:
#endif
	unregister_netdev(ndev);
fail_free_netdev:
	netif_napi_del(&tvnet->napi);
	free_netdev(ndev);
free_pci_mem:
	pci_epc_mem_free_addr(epc, tvnet->tx_dst_pci_addr, tvnet->tx_dst_va,
			      SZ_64K);
free_host_dma:
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_DMA);
free_host_mem:
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_MEM);
free_ep_mem:
	tvnet_ep_free_multi_page_bar0_mem(epf, EP_MEM);
free_irqsp:
	tvnet_ep_pci_epf_destroy_irqsp(tvnet);
free_bar0_md:
	tvnet_ep_free_single_page_bar0_mem(epf, META_DATA);
free_iova:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	__free_iova(tvnet->iovad, tvnet->iova);
#else
	iommu_dma_free_iova(cdev, tvnet->bar0_iova, BAR0_SIZE);
#endif
fail:
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
static void free_fence_resource(struct syncpt_t *syncpt)
{
	int ret = 0;
	mutex_lock(&syncpt->lock);
	syncpt->fence_release = true;
	if (syncpt->fence) {
		ret = dma_fence_remove_callback(syncpt->fence,
						&syncpt->fence_cb);
		if (ret) {
			/*
			 * If dma_fence_remove_callback() returns true
			 * means callback is removed successfully.
			 * Cancel the fence to drop the refcount.
			 */
			host1x_fence_cancel(syncpt->fence);
		}
		dma_fence_put(syncpt->fence);
		syncpt->fence = NULL;
	}
	mutex_unlock(&syncpt->lock);
	mutex_destroy(&syncpt->lock);
}
#endif

static void tvnet_ep_pci_epf_unbind(struct pci_epf *epf)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	struct pci_epf_bar *epf_bar = &epf->bar[BAR_0];
#endif
	struct pci_epc *epc = epf->epc;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	struct syncpt_t *syncpt = NULL;

	syncpt = &tvnet->ctrl_irqsp->syncpt;
	free_fence_resource(syncpt);
	cancel_work_sync(&syncpt->work);

	syncpt = &tvnet->data_irqsp->syncpt;
	free_fence_resource(syncpt);
	cancel_work_sync(&syncpt->work);
#endif
	pci_epc_stop(epc);
#if LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0)
	lpci_epc_clear_bar(epc, epf->func_no, epf_bar);
#else
	pci_epc_clear_bar(epc, BAR_0);
#endif
#if ENABLE_DMA
	cancel_work_sync(&tvnet->ctrl_work);
	cancel_work_sync(&tvnet->refill_work);
	cancel_work_sync(&tvnet->raise_irq_work);
	cancel_delayed_work_sync(&tvnet->tx_complete_work);
	cancel_delayed_work_sync(&tvnet->tx_batch_flush_work);
	hrtimer_cancel(&tvnet->tx_batch_timer);
	tvnet_ep_drop_tx_batch(tvnet);

	/* Deinitialize DMA library */
	if (tvnet->dma_cookie) {
		tegra_pcie_dma_status_t status;
		status = tegra_pcie_dma_deinit(&tvnet->dma_cookie);
		if (status != TEGRA_PCIE_DMA_SUCCESS)
			dev_err(&epf->dev, "DMA deinit failed: %d\n", status);
	}
	/* Free platform MSI for T264 */
	if (tvnet->soc_id == TEGRA_VNET_SOC_T264) {
		struct pci_epc *epc = epf->epc;
		struct device *cdev = epc->dev.parent;

		tvnet_ep_platform_msi_free_irqs(cdev);
	}
#else
	/* Cancel RX cleanup work and flush any pending items */
	cancel_work_sync(&tvnet->rx_cleanup_work);
	cancel_work_sync(&tvnet->ctrl_work);
	cancel_work_sync(&tvnet->raise_irq_work);
#endif
	unregister_netdev(tvnet->ndev);
	netif_napi_del(&tvnet->napi);

	/* Drain/free any remaining RX empty buffers before freeing BAR backing. */
	tvnet_ep_free_empty_buffers(tvnet);

#if ENABLE_DMA
	/*
	 * After unregister_netdev() there shouldn't be new xmit activity.
	 * Cancel again and drain any pending deferred completions.
	 */
	cancel_work_sync(&tvnet->refill_work);
	cancel_delayed_work_sync(&tvnet->tx_complete_work);
	tvnet_ep_drop_tx_batch(tvnet);
	tvnet_ep_drain_tx_complete_list(tvnet);
	if (tvnet->tx_batch_pool) {
		mempool_destroy(tvnet->tx_batch_pool);
		tvnet->tx_batch_pool = NULL;
	}
	if (tvnet->tx_ctx_pool) {
		mempool_destroy(tvnet->tx_ctx_pool);
		tvnet->tx_ctx_pool = NULL;
	}
#ifdef CONFIG_PAGE_POOL
	if (tvnet->rx_page_pool) {
		page_pool_destroy(tvnet->rx_page_pool);
		tvnet->rx_page_pool = NULL;
	}
#endif
#endif

	free_netdev(tvnet->ndev);
	pci_epc_mem_free_addr(epc, tvnet->tx_dst_pci_addr, tvnet->tx_dst_va,
			      SZ_64K);
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_DMA);
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_MEM);
	tvnet_ep_free_multi_page_bar0_mem(epf, EP_MEM);
	tvnet_ep_pci_epf_destroy_irqsp(tvnet);
	tvnet_ep_free_single_page_bar0_mem(epf, META_DATA);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	__free_iova(tvnet->iovad, tvnet->iova);
	tvnet->bar0_iova = 0x0;
#else
	iommu_dma_free_iova(cdev, tvnet->bar0_iova, BAR0_SIZE);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
static const struct pci_epf_device_id tvnet_ep_epf_tvnet_ids[] = {
	{ .name = "pci_epf_tvnet", },
	{ },
};
#endif

#if defined(NV_PCI_EPF_DRIVER_STRUCT_PROBE_HAS_ID_ARG) /* Linux 6.4 */
static int tvnet_ep_epf_tvnet_probe(struct pci_epf *epf, const struct pci_epf_device_id *id)
#else
static int tvnet_ep_epf_tvnet_probe(struct pci_epf *epf)
#endif
{
	struct device *fdev = &epf->dev;
	struct pci_epf_tvnet *tvnet;
	u32 chip_id;

	tvnet = devm_kzalloc(fdev, sizeof(*tvnet), GFP_KERNEL);
	if (!tvnet)
		return -ENOMEM;

	epf_set_drvdata(epf, tvnet);
	tvnet->fdev = fdev;
	tvnet->epf = epf;

	INIT_WORK(&tvnet->refill_work, tvnet_ep_refill_work);
	INIT_WORK(&tvnet->ctrl_work, tvnet_ep_ctrl_work);
#if ENABLE_DMA
	init_llist_head(&tvnet->tx_complete_list);
	INIT_DELAYED_WORK(&tvnet->tx_complete_work, tvnet_ep_tx_complete_work);
	INIT_DELAYED_WORK(&tvnet->tx_batch_flush_work, tvnet_ep_tx_batch_flush_work);
#if defined(NV_HRTIMER_SETUP_PRESENT) /* Linux v6.13 */
	hrtimer_setup(&tvnet->tx_batch_timer, &tvnet_ep_tx_batch_timer_fn,
		      CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
	hrtimer_init(&tvnet->tx_batch_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	tvnet->tx_batch_timer.function = tvnet_ep_tx_batch_timer_fn;
#endif
	tvnet->tx_ctx_pool = mempool_create_kmalloc_pool(DMA_DESC_COUNT,
							 sizeof(struct tvnet_ep_dma_tx_ctx));
	if (!tvnet->tx_ctx_pool) {
		dev_err(fdev, "Failed to create tx_ctx_pool\n");
		return -ENOMEM;
	}

	spin_lock_init(&tvnet->tx_batch_lock);
	tvnet->tx_batch_cnt = 0;
	tvnet->tx_batch_pool = mempool_create_kmalloc_pool(DMA_DESC_COUNT,
							   sizeof(struct tvnet_ep_dma_tx_batch));
	if (!tvnet->tx_batch_pool) {
		dev_err(fdev, "Failed to create tx_batch_pool\n");
		mempool_destroy(tvnet->tx_ctx_pool);
		tvnet->tx_ctx_pool = NULL;
		return -ENOMEM;
	}
#endif

	/* Detect SoC type using chip ID */
	chip_id = __tegra_get_chip_id();
	if (chip_id == TEGRA264) {
		tvnet->soc_id = TEGRA_VNET_SOC_T264;
	} else {
		tvnet->soc_id = TEGRA_VNET_SOC_T234;
	}

	dev_info(fdev, "Detected SoC: %s (chip_id=0x%x)\n",
		 (tvnet->soc_id == TEGRA_VNET_SOC_T264) ? "T264" : "T234", chip_id);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0)
	epf->event_ops = &tvnet_event_ops;
#endif
	tvnet->header.vendorid = PCI_VENDOR_ID_NVIDIA;

	/* Set device ID based on SoC type */
	if (tvnet->soc_id == TEGRA_VNET_SOC_T264) {
		tvnet->header.deviceid = PCI_DEVICE_ID_NVIDIA_JETSON_THOR_NETWORK;
	} else {
		tvnet->header.deviceid = PCI_DEVICE_ID_NVIDIA_JETSON_AGX_NETWORK;
	}

	tvnet->header.revid = 0x0;
	tvnet->header.baseclass_code = PCI_BASE_CLASS_NETWORK;
	tvnet->header.subclass_code = (PCI_CLASS_NETWORK_OTHER & 0xff);
	tvnet->header.subsys_vendor_id = PCI_VENDOR_ID_NVIDIA;
	tvnet->header.subsys_id = 0x0;
	tvnet->header.interrupt_pin = PCI_INTERRUPT_INTA;
	epf->header = &tvnet->header;

	return 0;
}

static struct pci_epf_ops tvnet_ep_ops = {
	.bind		= tvnet_ep_pci_epf_bind,
	.unbind		= tvnet_ep_pci_epf_unbind,
#if LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0)
	.linkup		= tvnet_ep_pci_epf_linkup,
#endif
};

static struct pci_epf_driver tvnet_driver = {
	.driver.name	= "pci_epf_tvnet",
	.probe		= tvnet_ep_epf_tvnet_probe,
	.id_table	= tvnet_ep_epf_tvnet_ids,
	.ops		= &tvnet_ep_ops,
	.owner		= THIS_MODULE,
};

static int __init tvnet_ep_pci_epf_init(void)
{
	int ret;

	ret = pci_epf_register_driver(&tvnet_driver);
	if (ret < 0) {
		pr_err("Failed to register EPF Tegra vnet driver: %d\n", ret);
		return ret;
	}

	return 0;
}
module_init(tvnet_ep_pci_epf_init);

static void __exit tvnet_ep_pci_epf_exit(void)
{
	pci_epf_unregister_driver(&tvnet_driver);
}
module_exit(tvnet_ep_pci_epf_exit);

MODULE_DESCRIPTION("PCI EPF TEGRA VIRTUAL NETWORK DRIVER");
MODULE_AUTHOR("Manikanta Maddireddy <mmaddireddy@nvidia.com>");
MODULE_LICENSE("GPL v2");
