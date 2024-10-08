// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2021-2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvidia/conftest.h>

#include <linux/etherdevice.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of_platform.h>
#include <linux/pci-epc.h>
#include <linux/pci-epf.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/tegra_vnet.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

#define BAR0_SIZE SZ_4M
#define APPL_INTR_EN_L1_8_0                     0x44
#define APPL_INTR_EN_L1_8_EDMA_INT_EN           BIT(6)

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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
	void __iomem *dma_base;
	struct bar0_amap bar0_amap[AMAP_MAX];
	struct bar_md *bar_md;
	dma_addr_t bar0_iova;
	struct net_device *ndev;
	struct napi_struct napi;
	bool pcie_link_status;
	struct ep_ring_buf ep_ring_buf;
	struct host_ring_buf host_ring_buf;
	enum dir_link_state tx_link_state;
	enum dir_link_state rx_link_state;
	enum os_link_state os_link_state;
	/* To synchronize network link state machine*/
	struct mutex link_state_lock;
	wait_queue_head_t link_state_wq;
	struct list_head h2ep_empty_list;
#if ENABLE_DMA
	struct dma_desc_cnt desc_cnt;
#endif
	/* To protect h2ep empty list */
	spinlock_t h2ep_empty_lock;
	dma_addr_t rx_buf_iova;
	unsigned long *rx_buf_bitmap;
	int rx_num_pages;
	void __iomem *tx_dst_va;
	phys_addr_t tx_dst_pci_addr;
	void *ep_dma_virt;
	dma_addr_t ep_dma_iova;
	struct irqsp_data *ctrl_irqsp;
	struct irqsp_data *data_irqsp;
	struct work_struct raise_irq_work;

	struct tvnet_counter h2ep_ctrl;
	struct tvnet_counter ep2h_ctrl;
	struct tvnet_counter h2ep_empty;
	struct tvnet_counter h2ep_full;
	struct tvnet_counter ep2h_empty;
	struct tvnet_counter ep2h_full;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	/* DRV_MODE specific.*/
	struct pci_epc *epc;
	struct platform_device *host1x_pdev;
	atomic_t core_initialized;
	/* IOVA alloc abstraction.*/
	struct iova_domain *iovad;
	struct iova *iova;
	struct resource *appl_res;
	void __iomem *appl_base;
#endif
};

static void tvnet_ep_raise_irq_work_function(struct work_struct *work)
{
	struct pci_epf_tvnet *tvnet =
		container_of(work, struct pci_epf_tvnet, raise_irq_work);

	struct pci_epc *epc = tvnet->epf->epc;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	struct pci_epf *epf = tvnet->epf;
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
	lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 0);
	lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 1);
#else
	lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 0);
	lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 1);
#endif
#else
	pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 0);
	pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 1);
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
	struct pci_epc *epc = tvnet->epf->epc;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	struct pci_epf *epf = tvnet->epf;
#endif
	u32 idx;

	if (tvnet_ivc_full(&tvnet->ep2h_ctrl)) {
		/* Raise an interrupt to let host process EP2H ring */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
		lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 0);
#else
		lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 0);
#endif
#else
		pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 0);
#endif
		dev_dbg(tvnet->fdev, "%s: EP2H ctrl ring full\n", __func__);
		return -EAGAIN;
	}

	idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_ctrl) % RING_COUNT;
	memcpy(&ctrl_msg[idx], msg, sizeof(*msg));
	tvnet_ivc_advance_wr(&tvnet->ep2h_ctrl);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
	lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 0);
#else
	lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 0);
#endif
#else
	pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 0);
#endif

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
		return DMA_ERROR_CODE;
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
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	struct pci_epf *epf = tvnet->epf;
#endif
	struct device *cdev = epc->dev.parent;
	struct data_msg *h2ep_empty_msg = ep_ring_buf->h2ep_empty_msgs;
	struct h2ep_empty_list *h2ep_empty_ptr;
#if ENABLE_DMA
	struct net_device *ndev = tvnet->ndev;
#else
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	int ret = 0;
#endif

	while (!tvnet_ivc_full(&tvnet->h2ep_empty)) {
		dma_addr_t iova;
#if ENABLE_DMA
		struct sk_buff *skb;
		int len = ndev->mtu + ETH_HLEN;
#else
		struct page *page;
		void *virt;
#endif
		u32 idx;
		unsigned long flags;

#if ENABLE_DMA
		skb = netdev_alloc_skb(ndev, len);
		if (!skb) {
			pr_err("%s: alloc skb failed\n", __func__);
			break;
		}

		iova = dma_map_single(cdev, skb->data, len, DMA_FROM_DEVICE);
		if (dma_mapping_error(cdev, iova)) {
			pr_err("%s: dma map failed\n", __func__);
			dev_kfree_skb_any(skb);
			break;
		}

		/* The PCIe link is stable and dependable,
		 * so it's not necessary to perform a software checksum.
		 */
		skb->ip_summed = CHECKSUM_UNNECESSARY;
#else
		iova = tvnet_ivoa_alloc(tvnet);
		if (iova == DMA_ERROR_CODE) {
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

		ret = iommu_map(domain, iova, page_to_phys(page), PAGE_SIZE,
#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
				IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#else
				IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE);
#endif
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
			dma_unmap_single(cdev, iova, len, DMA_FROM_DEVICE);
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
#else
		h2ep_empty_ptr->page = page;
		h2ep_empty_ptr->virt = virt;
		h2ep_empty_ptr->size = PAGE_SIZE;
#endif
		h2ep_empty_ptr->iova = iova;
		spin_lock_irqsave(&tvnet->h2ep_empty_lock, flags);
		list_add_tail(&h2ep_empty_ptr->list, &tvnet->h2ep_empty_list);
		spin_unlock_irqrestore(&tvnet->h2ep_empty_lock, flags);

		idx = tvnet_ivc_get_wr_cnt(&tvnet->h2ep_empty) % RING_COUNT;
		h2ep_empty_msg[idx].u.empty_buffer.pcie_address = iova;
		h2ep_empty_msg[idx].u.empty_buffer.buffer_len = PAGE_SIZE;
		tvnet_ivc_advance_wr(&tvnet->h2ep_empty);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
		lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 0);
#else
		lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 0);
#endif
#else
		pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 0);
#endif
	}
}

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

	spin_lock_irqsave(&tvnet->h2ep_empty_lock, flags);
	list_for_each_entry_safe(h2ep_empty_ptr, temp, &tvnet->h2ep_empty_list,
				 list) {
		list_del(&h2ep_empty_ptr->list);
#if ENABLE_DMA
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
	spin_unlock_irqrestore(&tvnet->h2ep_empty_lock, flags);
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

	if ((tvnet->rx_link_state == DIR_LINK_STATE_UP) &&
	    (tvnet->tx_link_state == DIR_LINK_STATE_UP))
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

static netdev_tx_t tvnet_ep_start_xmit(struct sk_buff *skb,
				    struct net_device *ndev)
{
	struct device *fdev = ndev->dev.parent;
	struct pci_epf_tvnet *tvnet = dev_get_drvdata(fdev);
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct data_msg *ep2h_full_msg = ep_ring_buf->ep2h_full_msgs;
	struct skb_shared_info *info = skb_shinfo(skb);
	struct data_msg *ep2h_empty_msg = host_ring_buf->ep2h_empty_msgs;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
#if ENABLE_DMA
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	struct tvnet_dma_desc *ep_dma_virt =
				(struct tvnet_dma_desc *)tvnet->ep_dma_virt;
	u32 desc_widx, desc_ridx, val, ctrl_d;
	unsigned long timeout;
#else
	int ret;
#endif
	dma_addr_t src_iova;
	u32 rd_idx, wr_idx;
	u64 dst_masked, dst_off, dst_iova;
	int dst_len, len;

	/*TODO Not expecting skb frags, remove this after testing */
	WARN_ON(info->nr_frags);

	/* Check if EP2H_EMPTY_BUF available to read */
	if (!tvnet_ivc_rd_available(&tvnet->ep2h_empty)) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
		lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 0);
#else
		lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 0);
#endif
#else
		pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 0);
#endif
		dev_dbg(fdev, "%s: No EP2H empty msg, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

	/* Check if EP2H_FULL_BUF available to write */
	if (tvnet_ivc_full(&tvnet->ep2h_full)) {
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
#if defined(PCI_EPC_IRQ_TYPE_ENUM_PRESENT) /* Dropped from Linux 6.8 */
		lpci_epc_raise_irq(epc, epf->func_no, PCI_EPC_IRQ_MSIX, 1);
#else
		lpci_epc_raise_irq(epc, epf->func_no, PCI_IRQ_MSIX, 1);
#endif
#else
		pci_epc_raise_irq(epc, PCI_EPC_IRQ_MSIX, 1);
#endif
		dev_dbg(fdev, "%s: No EP2H full buf, stop tx\n", __func__);
		netif_stop_queue(ndev);
		return NETDEV_TX_BUSY;
	}

#if ENABLE_DMA
	/* Check if dma desc available */
	if ((desc_cnt->wr_cnt - desc_cnt->rd_cnt) >= DMA_DESC_COUNT) {
		dev_dbg(fdev, "%s: dma descs are not available\n", __func__);
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
	dst_iova = ep2h_empty_msg[rd_idx].u.empty_buffer.pcie_address;
	dst_len = ep2h_empty_msg[rd_idx].u.empty_buffer.buffer_len;

	/*
	 * Map host dst mem to local PCIe address range.
	 * PCIe address range is SZ_64K aligned.
	 */
	dst_masked = (dst_iova & ~(SZ_64K - 1));
	dst_off = (dst_iova & (SZ_64K - 1));

#if !ENABLE_DMA
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
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
	/* Trigger DMA write from src_iova to dst_iova */
	desc_widx = desc_cnt->wr_cnt % DMA_DESC_COUNT;
	ep_dma_virt[desc_widx].size = len;
	ep_dma_virt[desc_widx].sar_low = lower_32_bits(src_iova);
	ep_dma_virt[desc_widx].sar_high = upper_32_bits(src_iova);
	ep_dma_virt[desc_widx].dar_low = lower_32_bits(dst_iova);
	ep_dma_virt[desc_widx].dar_high = upper_32_bits(dst_iova);
	/* CB bit should be set at the end */
	mb();
	ctrl_d = DMA_CH_CONTROL1_OFF_WRCH_LIE;
	ctrl_d |= DMA_CH_CONTROL1_OFF_WRCH_CB;
	ep_dma_virt[desc_widx].ctrl_reg.ctrl_d = ctrl_d;

	/* DMA write should not go out of order wrt CB bit set */
	mb();

	timeout = jiffies + msecs_to_jiffies(1000);
	dma_common_wr8(tvnet->dma_base, DMA_WR_DATA_CH, DMA_WRITE_DOORBELL_OFF);
	desc_cnt->wr_cnt++;

	while (true) {
		val = dma_common_rd(tvnet->dma_base, DMA_WRITE_INT_STATUS_OFF);
		if (val == BIT(DMA_WR_DATA_CH)) {
			dma_common_wr(tvnet->dma_base, val,
				      DMA_WRITE_INT_CLEAR_OFF);
			break;
		}
		if (time_after(jiffies, timeout)) {
			dev_err(fdev, "dma took more time, reset dma engine\n");
			dma_common_wr(tvnet->dma_base,
				      DMA_WRITE_ENGINE_EN_OFF_DISABLE,
				      DMA_WRITE_ENGINE_EN_OFF);
			mdelay(1);
			dma_common_wr(tvnet->dma_base,
				      DMA_WRITE_ENGINE_EN_OFF_ENABLE,
				      DMA_WRITE_ENGINE_EN_OFF);
			desc_cnt->wr_cnt--;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
			lpci_epc_unmap_addr(epc, epf->func_no, tvnet->tx_dst_pci_addr);
#else
			pci_epc_unmap_addr(epc, tvnet->tx_dst_pci_addr);
#endif
			dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
			return NETDEV_TX_BUSY;
		}
	}

	desc_ridx = tvnet->desc_cnt.rd_cnt % DMA_DESC_COUNT;
	/* Clear DMA cycle bit and increment rd_cnt */
	ep_dma_virt[desc_ridx].ctrl_reg.ctrl_e.cb = 0;
	mb();

	tvnet->desc_cnt.rd_cnt++;
#else
	/* Copy skb->data to host dst address, use CPU virt addr */
	memcpy((void *)(tvnet->tx_dst_va + dst_off), skb->data, len);
	/*
	 * tx_dst_va is ioremap_wc() mem, add mb to make sure complete skb->data
	 * written to dst before adding it to full buffer
	 */
	mb();
#endif

	/* Push dst to EP2H full ring */
	wr_idx = tvnet_ivc_get_wr_cnt(&tvnet->ep2h_full) % RING_COUNT;
	ep2h_full_msg[wr_idx].u.full_buffer.packet_size = len;
	ep2h_full_msg[wr_idx].u.full_buffer.pcie_address = dst_iova;
	tvnet_ivc_advance_wr(&tvnet->ep2h_full);

	/* Free temp src and skb */
#if !ENABLE_DMA
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	lpci_epc_unmap_addr(epc, epf->func_no, tvnet->tx_dst_pci_addr);
#else
	pci_epc_unmap_addr(epc, tvnet->tx_dst_pci_addr);
#endif
#endif
	dma_unmap_single(cdev, src_iova, len, DMA_TO_DEVICE);
	dev_kfree_skb_any(skb);
	schedule_work(&tvnet->raise_irq_work);

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

static int tvnet_ep_process_h2ep_msg(struct pci_epf_tvnet *tvnet)
{
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct data_msg *data_msg = host_ring_buf->h2ep_full_msgs;
	struct pci_epf *epf = tvnet->epf;
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
	struct h2ep_empty_list *h2ep_empty_ptr;
	struct net_device *ndev = tvnet->ndev;
#if !ENABLE_DMA
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
#endif
	int count = 0;

	while ((count < TVNET_NAPI_WEIGHT) &&
	       tvnet_ivc_rd_available(&tvnet->h2ep_full)) {
		struct sk_buff *skb;
		int idx, found = 0;
		u32 len;
		u64 pcie_address;
		unsigned long flags;

		/* Read H2EP full msg */
		idx = tvnet_ivc_get_rd_cnt(&tvnet->h2ep_full) % RING_COUNT;
		len = data_msg[idx].u.full_buffer.packet_size;
		pcie_address = data_msg[idx].u.full_buffer.pcie_address;

		/* Get H2EP msg pointer from saved list */
		spin_lock_irqsave(&tvnet->h2ep_empty_lock, flags);
		list_for_each_entry(h2ep_empty_ptr, &tvnet->h2ep_empty_list,
				    list) {
			if (h2ep_empty_ptr->iova == pcie_address) {
				list_del(&h2ep_empty_ptr->list);
				found = 1;
				break;
			}
		}
		spin_unlock_irqrestore(&tvnet->h2ep_empty_lock, flags);

		/* Advance H2EP full buffer after search in local list */
		tvnet_ivc_advance_rd(&tvnet->h2ep_full);
		if (WARN_ON(!found))
			continue;
#if ENABLE_DMA
		dma_unmap_single(cdev, pcie_address, ndev->mtu + ETH_HLEN,
				 DMA_FROM_DEVICE);
		skb = h2ep_empty_ptr->skb;
		skb_put(skb, len);
		skb->protocol = eth_type_trans(skb, ndev);
		napi_gro_receive(&tvnet->napi, skb);
#else
		/* Alloc new skb and copy data from full buffer */
		skb = netdev_alloc_skb(ndev, len);
		memcpy(skb->data, h2ep_empty_ptr->virt, len);
		skb_put(skb, len);
		skb->protocol = eth_type_trans(skb, ndev);
		napi_gro_receive(&tvnet->napi, skb);

		/* Free H2EP dst msg */
		vunmap(h2ep_empty_ptr->virt);
		iommu_unmap(domain, h2ep_empty_ptr->iova, PAGE_SIZE);
		__free_pages(h2ep_empty_ptr->page, 1);
		tvnet_ep_iova_dealloc(tvnet, h2ep_empty_ptr->iova);
#endif

		kfree(h2ep_empty_ptr);
		count++;
	}

	return count;
}

#if ENABLE_DMA
static void tvnet_ep_setup_dma(struct pci_epf_tvnet *tvnet)
{
	dma_addr_t iova = tvnet->bar0_amap[HOST_DMA].iova;
	struct dma_desc_cnt *desc_cnt = &tvnet->desc_cnt;
	u32 val;

	desc_cnt->rd_cnt = desc_cnt->wr_cnt = 0;

	/* Enable linked list mode and set CCS for write channel-0 */
	val = dma_channel_rd(tvnet->dma_base, DMA_WR_DATA_CH,
			     DMA_CH_CONTROL1_OFF_WRCH);
	val |= DMA_CH_CONTROL1_OFF_WRCH_LLE;
	val |= DMA_CH_CONTROL1_OFF_WRCH_CCS;
	dma_channel_wr(tvnet->dma_base, DMA_WR_DATA_CH, val,
		       DMA_CH_CONTROL1_OFF_WRCH);

	/* Unmask write channel-0 done irq to enable LIE */
	val = dma_common_rd(tvnet->dma_base, DMA_WRITE_INT_MASK_OFF);
	val &= ~0x1;
	dma_common_wr(tvnet->dma_base, val, DMA_WRITE_INT_MASK_OFF);

	/* Enable write channel-0 local abort irq */
	val = dma_common_rd(tvnet->dma_base, DMA_WRITE_LINKED_LIST_ERR_EN_OFF);
	val |= (0x1 << 16);
	dma_common_wr(tvnet->dma_base, val, DMA_WRITE_LINKED_LIST_ERR_EN_OFF);

	/* Program DMA write linked list base address to DMA LLP register */
	dma_channel_wr(tvnet->dma_base, DMA_WR_DATA_CH,
		       lower_32_bits(tvnet->ep_dma_iova),
		       DMA_LLP_LOW_OFF_WRCH);
	dma_channel_wr(tvnet->dma_base, DMA_WR_DATA_CH,
		       upper_32_bits(tvnet->ep_dma_iova),
		       DMA_LLP_HIGH_OFF_WRCH);

	/* Enable DMA write engine */
	dma_common_wr(tvnet->dma_base, DMA_WRITE_ENGINE_EN_OFF_ENABLE,
		      DMA_WRITE_ENGINE_EN_OFF);

	/* Enable linked list mode and set CCS for read channel-0 */
	val = dma_channel_rd(tvnet->dma_base, DMA_RD_DATA_CH,
			     DMA_CH_CONTROL1_OFF_RDCH);
	val |= DMA_CH_CONTROL1_OFF_RDCH_LLE;
	val |= DMA_CH_CONTROL1_OFF_RDCH_CCS;
	dma_channel_wr(tvnet->dma_base, DMA_RD_DATA_CH, val,
		       DMA_CH_CONTROL1_OFF_RDCH);

	/* Mask read channel-0 done irq to enable RIE */
	val = dma_common_rd(tvnet->dma_base, DMA_READ_INT_MASK_OFF);
	val |= 0x1;
	dma_common_wr(tvnet->dma_base, val, DMA_READ_INT_MASK_OFF);

	val = dma_common_rd(tvnet->dma_base, DMA_READ_LINKED_LIST_ERR_EN_OFF);
	/* Enable read channel-0 remote abort irq */
	val |= 0x1;
	dma_common_wr(tvnet->dma_base, val, DMA_READ_LINKED_LIST_ERR_EN_OFF);

	/* Program DMA read linked list base address to DMA LLP register */
	dma_channel_wr(tvnet->dma_base, DMA_RD_DATA_CH,
		       lower_32_bits(iova), DMA_LLP_LOW_OFF_RDCH);
	dma_channel_wr(tvnet->dma_base, DMA_RD_DATA_CH,
		       upper_32_bits(iova), DMA_LLP_HIGH_OFF_RDCH);

	/* Enable DMA read engine */
	dma_common_wr(tvnet->dma_base, DMA_READ_ENGINE_EN_OFF_ENABLE,
		      DMA_READ_ENGINE_EN_OFF);
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
/* Returns aperture offset of syncpoint on SHIM_BASE. */
static inline u64 get_syncpt_shim_offset(u32 id)
{
	return (SHIM_BASE + ((u64)id * SP_SIZE));
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
	struct net_device *ndev = tvnet->ndev;

	if (netif_queue_stopped(ndev)) {
		if ((tvnet->os_link_state == OS_LINK_STATE_UP) &&
		    tvnet_ivc_rd_available(&tvnet->ep2h_empty) &&
		    !tvnet_ivc_full(&tvnet->ep2h_full)) {
			netif_wake_queue(ndev);
		}
	}

	if (tvnet_ivc_rd_available(&tvnet->h2ep_ctrl))
		tvnet_ep_process_ctrl_msg(tvnet);

	if (!tvnet_ivc_full(&tvnet->h2ep_empty) &&
	    (tvnet->os_link_state == OS_LINK_STATE_UP))
		tvnet_ep_alloc_empty_buffers(tvnet);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
	schedule_work(&data_irqsp->reprime_work);
#endif
}

static void tvnet_ep_data_irqsp_work(struct work_struct *work)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
	else
		schedule_work(&data_irqsp->reprime_work);
#endif
}

static int tvnet_ep_poll(struct napi_struct *napi, int budget)
{
	struct pci_epf_tvnet *tvnet = container_of(napi, struct pci_epf_tvnet,
						   napi);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
	struct irqsp_data *data_irqsp = tvnet->data_irqsp;
#endif
	int work_done;

	work_done = tvnet_ep_process_h2ep_msg(tvnet);
	if (work_done < budget) {
		napi_complete(napi);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

	syncpt_addr = get_syncpt_shim_offset(ctrl_irqsp->syncpt.id);
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
	ret = iommu_map(domain, amap->iova, syncpt_addr, PAGE_SIZE,
#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#else
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE);
#endif
	if (ret < 0) {
		dev_err(fdev, "%s: iommu_map of ctrlsp mem failed: %d\n",
			__func__, ret);
		goto free_data_sp;
	}

	irq = &tvnet->bar_md->irq_ctrl;
	irq->irq_addr = PAGE_SIZE;
	irq->irq_type = IRQ_SIMPLE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	syncpt_addr = get_syncpt_shim_offset(data_irqsp->syncpt.id);
	data_irqsp->syncpt.phy_addr = syncpt_addr;
	data_irqsp->syncpt.size = PAGE_SIZE;
#else
	syncpt_addr = nvhost_interrupt_syncpt_get_syncpt_addr(data_irqsp->is);
#endif
	ret = iommu_map(domain, amap->iova + PAGE_SIZE, syncpt_addr, PAGE_SIZE,
#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#else
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE);
#endif
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

	ret = iommu_map(domain, amap->iova, page_to_phys(amap->page), PAGE_SIZE,
#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#else
			IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE);
#endif
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

	ret = iommu_map(domain, amap->iova, page_to_phys(amap->page),
#if defined(NV_IOMMU_MAP_HAS_GFP_ARG)
			amap->size, IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE, GFP_KERNEL);
#else
			amap->size, IOMMU_CACHE | IOMMU_READ | IOMMU_WRITE);
#endif
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
	__free_pages(amap->page, page_count);
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
	int page_count = amap->size >> PAGE_SHIFT;

	iommu_unmap(domain, amap->iova, page_count);
	vfree(amap->virt);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
static int tvnet_ep_pci_epf_core_init(struct pci_epf *epf)
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	struct pci_epc *epc = epf->epc;
	struct pci_epf_header *header = epf->header;
	struct device *fdev = &epf->dev;
	struct pci_epf_bar *epf_bar = &epf->bar[BAR_0];
	int ret;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

	/* TODO Update it to 64-bit prefetch type */
	epf_bar->phys_addr = tvnet->bar0_iova;
	epf_bar->addr = tvnet->bar_md;
	epf_bar->size = BAR0_SIZE;
	epf_bar->barno = BAR_0;
	epf_bar->flags |= PCI_BASE_ADDRESS_SPACE_MEMORY |
				PCI_BASE_ADDRESS_MEM_TYPE_32;

	ret = lpci_epc_set_bar(epc, epf->func_no, epf_bar);
	if (ret) {
		dev_err(fdev, "pci_epc_set_bar() failed: %d\n", ret);
		return ret;
	}

	ret = lpci_epc_set_msi(epc, epf->func_no, epf->msi_interrupts);
	if (ret) {
		dev_err(fdev, "pci_epc_set_msi() failed: %d\n", ret);
		return ret;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	atomic_set(&tvnet->core_initialized, 1);

#endif
	return 0;
}
#endif

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0) && \
	LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
static int tvnet_ep_pci_epf_linkup(struct pci_epf *epf)
#else
static void tvnet_ep_pci_epf_linkup(struct pci_epf *epf)
#endif
{
	struct pci_epf_tvnet *tvnet = epf_get_drvdata(epf);
	u32 val;
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	val = readl(tvnet->appl_base + APPL_INTR_EN_L1_8_0);
	if (val & APPL_INTR_EN_L1_8_EDMA_INT_EN)
		writel(val & ~APPL_INTR_EN_L1_8_EDMA_INT_EN,
			tvnet->appl_base + APPL_INTR_EN_L1_8_0);

	return 0;
#endif
}
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_CORE_DEINIT)
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
#endif

static const struct pci_epc_event_ops tvnet_event_ops = {
	.core_init = tvnet_ep_pci_epf_core_init,
#if defined(NV_PCI_EPC_EVENT_OPS_STRUCT_HAS_CORE_DEINIT)
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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
	struct pci_epf_header *header = epf->header;
#endif
	struct device *fdev = &epf->dev;
	struct device *cdev = epc->dev.parent;
	struct iommu_domain *domain = iommu_get_domain_for_dev(cdev);
	struct platform_device *pdev = of_find_device_by_node(cdev->of_node);
	struct ep_ring_buf *ep_ring_buf = &tvnet->ep_ring_buf;
	struct host_ring_buf *host_ring_buf = &tvnet->host_ring_buf;
	struct net_device *ndev;
	struct bar_md *bar_md;
	struct resource *res;
	struct bar0_amap *amap;
	struct tvnet_dma_desc *dma_desc;
	int ret, size, bitmap_size;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	unsigned long shift;
#endif
	if (!domain) {
		dev_err(fdev, "IOMMU domain not found\n");
		ret = -ENXIO;
		goto fail;
	}

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
	ret = pci_epc_write_header(epc, header);
	if (ret < 0) {
		dev_err(fdev, "pci_epc_write_header() failed: %d\n", ret);
		goto fail;
	}
#endif

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "atu_dma");
	if (!res) {
		dev_err(fdev, "missing atu_dma resource in DT\n");
		ret = PTR_ERR(res);
		goto fail;
	}

	tvnet->dma_base = devm_ioremap(fdev, res->start + DMA_OFFSET,
				       resource_size(res) - DMA_OFFSET);
	if (IS_ERR(tvnet->dma_base)) {
		ret = PTR_ERR(tvnet->dma_base);
		dev_err(fdev, "dma region map failed: %d\n", ret);
		goto fail;
	}

	tvnet->appl_res = platform_get_resource_byname(pdev, IORESOURCE_MEM,
							"appl");
	if (!tvnet->appl_res) {
		dev_err(fdev, "Failed to find \"appl\" region\n");
		goto fail;
	}

	tvnet->appl_base = devm_ioremap(fdev, tvnet->appl_res->start,
					PAGE_SIZE);
	if (IS_ERR(tvnet->appl_base))
		goto fail;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
	size = ((DMA_DESC_COUNT + 1) * sizeof(struct tvnet_dma_desc));
	amap->size = PAGE_ALIGN(size);
	ret = tvnet_ep_alloc_multi_page_bar0_mem(epf, HOST_DMA);
	if (ret < 0) {
		dev_err(fdev, "BAR0 host dma mem alloc failed: %d\n", ret);
		goto free_host_mem;
	}

	/* Set link list pointer to create a dma desc ring */
	memset(amap->virt, 0, amap->size);
	dma_desc = (struct tvnet_dma_desc *)amap->virt;
	dma_desc[DMA_DESC_COUNT].sar_low = (amap->iova & 0xffffffff);
	dma_desc[DMA_DESC_COUNT].sar_high = ((amap->iova >> 32) & 0xffffffff);
	dma_desc[DMA_DESC_COUNT].ctrl_reg.ctrl_e.llp = 1;

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
	tvnet->tx_dst_va = pci_epc_mem_alloc_addr(epc,
						     &tvnet->tx_dst_pci_addr,
						     SZ_64K);
	if (!tvnet->tx_dst_va) {
		dev_err(fdev, "failed to allocate dst PCIe address\n");
		ret = -ENOMEM;
		goto free_host_dma;
	}

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
#if defined(NV_NETIF_NAPI_ADD_WEIGHT_PRESENT) /* Linux v6.1 */
	netif_napi_add_weight(ndev, &tvnet->napi, tvnet_ep_poll, TVNET_NAPI_WEIGHT);
#else
	netif_napi_add(ndev, &tvnet->napi, tvnet_ep_poll, TVNET_NAPI_WEIGHT);
#endif
	ndev->mtu = TVNET_DEFAULT_MTU;

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

	INIT_LIST_HEAD(&tvnet->h2ep_empty_list);
	spin_lock_init(&tvnet->h2ep_empty_lock);

	INIT_WORK(&tvnet->raise_irq_work, tvnet_ep_raise_irq_work_function);

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
	/* TODO Update it to 64-bit prefetch type */
	ret = pci_epc_set_bar(epc, BAR_0, tvnet->bar0_iova, BAR0_SIZE,
			      PCI_BASE_ADDRESS_SPACE_MEMORY |
			      PCI_BASE_ADDRESS_MEM_TYPE_32);
	if (ret < 0) {
		dev_err(fdev, "pci_epc_set_bar() failed: %d\n", ret);
		goto fail_unreg_netdev;
	}
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
	ret = pci_epc_set_msi(epc, epf->msi_interrupts);
	if (ret) {
		dev_err(fdev, "pci_epc_set_msi() failed: %d\n", ret);
		goto fail_clear_bar;
	}
#endif

	/* Allocate local memory for DMA write link list elements */
	size = ((DMA_DESC_COUNT + 1) * sizeof(struct tvnet_dma_desc));
	tvnet->ep_dma_virt = dma_alloc_coherent(cdev, size,
						&tvnet->ep_dma_iova,
						GFP_KERNEL);
	if (!tvnet->ep_dma_virt) {
		dev_err(fdev, "%s ep dma mem alloc failed\n", __func__);
		ret = -ENOMEM;
		goto fail_clear_bar;
	}

	/* Set link list pointer to create a dma desc ring */
	memset(tvnet->ep_dma_virt, 0, size);
	dma_desc = (struct tvnet_dma_desc *)tvnet->ep_dma_virt;
	dma_desc[DMA_DESC_COUNT].sar_low = (tvnet->ep_dma_iova & 0xffffffff);
	dma_desc[DMA_DESC_COUNT].sar_high = ((tvnet->ep_dma_iova >> 32) &
					     0xffffffff);
	dma_desc[DMA_DESC_COUNT].ctrl_reg.ctrl_e.llp = 1;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0))
	nvhost_interrupt_syncpt_prime(tvnet->ctrl_irqsp->is);
	nvhost_interrupt_syncpt_prime(tvnet->data_irqsp->is);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	epf->nb.notifier_call = tvnet_ep_pci_epf_notifier;
	pci_epc_register_notifier(epc, &epf->nb);
#endif
#endif

	return 0;

fail_clear_bar:
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
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
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	__free_iova(tvnet->iovad, tvnet->iova);
#else
	iommu_dma_free_iova(cdev, tvnet->bar0_iova, BAR0_SIZE);
#endif
fail:
	return ret;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	struct pci_epf_bar *epf_bar = &epf->bar[BAR_0];
#endif
	struct pci_epc *epc = epf->epc;
	struct device *cdev = epc->dev.parent;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	struct syncpt_t *syncpt = NULL;

	syncpt = &tvnet->ctrl_irqsp->syncpt;
	free_fence_resource(syncpt);
	cancel_work_sync(&syncpt->work);

	syncpt = &tvnet->data_irqsp->syncpt;
	free_fence_resource(syncpt);
	cancel_work_sync(&syncpt->work);
#endif
	pci_epc_stop(epc);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(4, 15, 0))
	lpci_epc_clear_bar(epc, epf->func_no, epf_bar);
#else
	pci_epc_clear_bar(epc, BAR_0);
#endif
	dma_free_coherent(cdev,
			  ((RING_COUNT + 1) * sizeof(struct tvnet_dma_desc)),
			  tvnet->ep_dma_virt, tvnet->ep_dma_iova);
	unregister_netdev(tvnet->ndev);
	netif_napi_del(&tvnet->napi);
	free_netdev(tvnet->ndev);
	pci_epc_mem_free_addr(epc, tvnet->tx_dst_pci_addr, tvnet->tx_dst_va,
			      SZ_64K);
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_DMA);
	tvnet_ep_free_multi_page_bar0_mem(epf, HOST_MEM);
	tvnet_ep_free_multi_page_bar0_mem(epf, EP_MEM);
	tvnet_ep_pci_epf_destroy_irqsp(tvnet);
	tvnet_ep_free_single_page_bar0_mem(epf, META_DATA);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	__free_iova(tvnet->iovad, tvnet->iova);
	tvnet->bar0_iova = 0x0;
#else
	iommu_dma_free_iova(cdev, tvnet->bar0_iova, BAR0_SIZE);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
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

	tvnet = devm_kzalloc(fdev, sizeof(*tvnet), GFP_KERNEL);
	if (!tvnet)
		return -ENOMEM;

	epf_set_drvdata(epf, tvnet);
	tvnet->fdev = fdev;
	tvnet->epf = epf;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 14, 0))
	epf->event_ops = &tvnet_event_ops;
#endif
	tvnet->header.vendorid = PCI_VENDOR_ID_NVIDIA;
	tvnet->header.deviceid = PCI_DEVICE_ID_NVIDIA_JETSON_AGX_NETWORK;
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
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4, 15, 0))
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
