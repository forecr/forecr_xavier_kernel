/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2020-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PCIE_EPF_TEGRA_DMA_H
#define PCIE_EPF_TEGRA_DMA_H

#include <nvidia/conftest.h>

#ifdef CONFIG_PAGE_POOL
#include <linux/skbuff.h>
#if defined(NV_NET_PAGE_POOL_H_PRESENT)
#include <net/page_pool.h>
#else
#include <net/page_pool/types.h>
#endif
#endif

#ifndef PCI_DEVICE_ID_NVIDIA_JETSON_AGX_NETWORK
#define PCI_DEVICE_ID_NVIDIA_JETSON_AGX_NETWORK     0x2296
#endif

#ifndef PCI_DEVICE_ID_NVIDIA_JETSON_THOR_NETWORK
#define PCI_DEVICE_ID_NVIDIA_JETSON_THOR_NETWORK    0x22D7
#endif

/* SoC ID enumeration for tegra_vnet driver */
enum tegra_vnet_soc_id {
	TEGRA_VNET_SOC_T234 = 0,
	TEGRA_VNET_SOC_T264 = 1,
	TEGRA_VNET_SOC_MAX
};

/* T264 specific syncpoint base */
#define T264_SHIM_BASE  0x81C0000000ULL

#define DMA_RD_CHNL_NUM			2
#define DMA_WR_CHNL_NUM			4

/* Enabled for Milestone: DMA support with tegra-pcie-dma wrapper */
#define ENABLE_DMA 1
#define DMA_WR_DATA_CH 0
#define DMA_RD_DATA_CH 0

/* Network link timeout 5 sec */
#define LINK_TIMEOUT 5000

#define TVNET_DEFAULT_MTU 64512
#define TVNET_MIN_MTU 68
#define TVNET_MAX_MTU TVNET_DEFAULT_MTU

#define RING_COUNT	2048

/* Allocate 100% extra desc to handle the drift between empty & full buffer */
#define DMA_DESC_COUNT (2 * RING_COUNT)

/*
 * Cookie space for RX empty-buffer tracking.
 *
 * The old implementation keyed tracking nodes by DMA/IOVA address via xarray.
 * To avoid per-packet xarray lookups, we attach a cookie to each posted EMPTY
 * buffer and have the peer echo it back in the FULL message.
 *
 * Cookie values must not be re-used while a buffer is still in flight.
 */
#define TVNET_RX_COOKIE_MAX	8192

enum irq_type {
	/* No IRQ available in this slot */
	IRQ_NOT_AVAILABLE = 0,
	/* Use irq_{addr,val} fields */
	IRQ_SIMPLE = 1,
	/* Perform a dummy DMA reading */
	IRQ_DUMMY_DMA = 2,
};

struct irq_md {
	u32 irq_type;
	/* Simple method: Write to this */
	/* Dummy DMA method: Read from this */
	u64 irq_addr;
	/* Simple method: Write this value */
	/* Dummy DMA method: Don’t use this value */
	u32 irq_val;
	u32 reserved[4];
};

enum ring_buf {
	H2EP_CTRL,
	EP2H_CTRL,
	EP2H_EMPTY_BUF,
	EP2H_FULL_BUF,
	H2EP_FULL_BUF,
	H2EP_EMPTY_BUF,
};

struct ring_buf_md {
	u32 h2ep_offset;
	u32 h2ep_size;
	u32 ep2h_offset;
	u32 ep2h_size;
};

struct bar_md {
	/* IRQ generation for control packets */
	struct irq_md irq_ctrl;
	/* IRQ generation for data packets */
	struct irq_md irq_data;
	/* Ring buffers counter offset */
	u32 ep_own_cnt_offset;
	u32 host_own_cnt_offset;
	/* Ring buffers location offset */
	struct ring_buf_md ctrl_md;
	struct ring_buf_md ep2h_md;
	struct ring_buf_md h2ep_md;
	/* RAM region for use by host when programming EP DMA controller */
	u32 host_dma_offset;
	u32 host_dma_size;
	/* Endpoint will map all RX packet buffers into this region */
	u64 bar0_base_phy;
	u32 ep_rx_pkt_offset;
	u32 ep_rx_pkt_size;
};

enum ctrl_msg_type {
	CTRL_MSG_RESERVED,
	CTRL_MSG_LINK_UP,
	CTRL_MSG_LINK_DOWN,
	CTRL_MSG_LINK_DOWN_ACK,
};

struct ctrl_msg {
	u32 msg_id; /* enum ctrl_msg_type */
	union {
		u32 reserved[7];
	} u;
};

enum data_msg_type {
	DATA_MSG_RESERVED,
	DATA_MSG_EMPTY_BUF,
	DATA_MSG_FULL_BUF,
};

struct data_msg {
	u32 msg_id; /* enum data_msg_type */
	union {
		struct {
			u32 buffer_len;
			u16 cookie;
			u16 reserved;
			u64 pcie_address;
		} empty_buffer;
		struct {
			u32 packet_size;
			u16 cookie;
			u16 reserved;
			u64 pcie_address;
		} full_buffer;
		u32 reserved[7];
	} u;
};

struct tvnet_counter {
	u32 *rd;
	u32 *wr;
};

struct ep_own_cnt {
	u32 h2ep_ctrl_rd_cnt;
	u32 ep2h_ctrl_wr_cnt;
	u32 ep2h_empty_rd_cnt;
	u32 ep2h_full_wr_cnt;
	u32 h2ep_full_rd_cnt;
	u32 h2ep_empty_wr_cnt;
};

struct ep_ring_buf {
	struct ep_own_cnt *ep_cnt;
	/* Endpoint written message buffers */
	struct ctrl_msg *ep2h_ctrl_msgs;
	struct data_msg *ep2h_full_msgs;
	struct data_msg *h2ep_empty_msgs;
};

struct host_own_cnt {
	u32 h2ep_ctrl_wr_cnt;
	u32 ep2h_ctrl_rd_cnt;
	u32 ep2h_empty_wr_cnt;
	u32 ep2h_full_rd_cnt;
	u32 h2ep_full_wr_cnt;
	u32 h2ep_empty_rd_cnt;
};

struct host_ring_buf {
	struct host_own_cnt *host_cnt;
	/* Host written message buffers */
	struct ctrl_msg *h2ep_ctrl_msgs;
	struct data_msg *ep2h_empty_msgs;
	struct data_msg *h2ep_full_msgs;
};

struct ep2h_empty_list {
	u32 len;
	dma_addr_t iova;
#if ENABLE_DMA
	/* Buffer base DMA address (e.g., for page_pool sync) */
	dma_addr_t dma_base;
	/* Data offset from dma_base that device writes into */
	u32 offset;
#endif
	struct sk_buff *skb;
	struct list_head list;
};

struct h2ep_empty_list {
	u32 size;
#if ENABLE_DMA
	struct sk_buff *skb;
	/* Buffer base DMA address (e.g., for page_pool sync) */
	dma_addr_t dma_base;
	/* Data offset from dma_base that device writes into */
	u32 offset;
#else
	struct page *page;
	void *virt;
#endif
	dma_addr_t iova;
	struct list_head list;
};

enum dir_link_state {
	DIR_LINK_STATE_DOWN,
	DIR_LINK_STATE_UP,
	DIR_LINK_STATE_SENT_DOWN,
};

enum os_link_state {
	OS_LINK_STATE_UP,
	OS_LINK_STATE_DOWN,
};

#if ENABLE_DMA
/*
 * TX batching size used by both Host (RP) and EP vnet drivers.
 * Keep this as a shared constant so both sides stay in sync.
 */
#define TVNET_TX_BATCH_MAX	64

/*
 * TX batch flush delay (microseconds).
 * - Normal traffic: keep latency small.
 * - netdev_xmit_more()==true: extend window slightly to accumulate more SKBs.
 */
#define TVNET_TX_BATCH_DELAY_US		200
#define TVNET_TX_BATCH_DELAY_MORE_US	400

#ifdef CONFIG_PAGE_POOL
/* Headroom used for page_pool-backed RX buffers */
#define TVNET_PP_HEADROOM (NET_SKB_PAD + NET_IP_ALIGN)

static inline void tvnet_pp_init_params(struct page_pool_params *pp,
					struct device *dev,
					struct net_device *ndev,
					unsigned int order)
{
	/*
	 * Keep this minimal and shared across host + EP:
	 * - DMA mapped pages, with DMA_SYNC_DEV enabled so recycled pages are
	 *   synced for device when reallocated from the pool.
	 * - offset defines where device writes packet data (after headroom).
	 */
	*pp = (struct page_pool_params){
		.flags		= PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV,
		.order		= order,
		.pool_size	= RING_COUNT * 2,
		.nid		= NUMA_NO_NODE,
		.dev		= dev,
		.dma_dir	= DMA_FROM_DEVICE,
		.max_len	= TVNET_MAX_MTU + ETH_HLEN,
		.offset		= TVNET_PP_HEADROOM,
#if defined(NV_PAGE_POOL_PARAMS_STRUCT_HAS_NETDEV) /* Linux v6.8 */
		.netdev		= ndev,
#endif
	};
}
#endif

struct dma_desc_cnt {
	u32 rd_cnt;
	u32 wr_cnt;
};
#endif

static inline bool tvnet_ivc_empty(struct tvnet_counter *counter)
{
	u32 rd, wr;

	wr = READ_ONCE(*counter->wr);
	rd = READ_ONCE(*counter->rd);

	if (wr - rd > RING_COUNT)
		return true;

	return wr == rd;
}

static inline bool tvnet_ivc_full(struct tvnet_counter *counter)
{
	u32 rd, wr;

	wr = READ_ONCE(*counter->wr);
	rd = READ_ONCE(*counter->rd);

	return wr - rd >= RING_COUNT;
}

static inline u32 tvnet_ivc_rd_available(struct tvnet_counter *counter)
{
	u32 rd, wr;

	wr = READ_ONCE(*counter->wr);
	rd = READ_ONCE(*counter->rd);

	return wr - rd;
}

static inline u32 tvnet_ivc_wr_available(struct tvnet_counter *counter)
{
	u32 rd, wr;

	wr = READ_ONCE(*counter->wr);
	rd = READ_ONCE(*counter->rd);

	return (RING_COUNT - (wr - rd));
}

static inline void tvnet_ivc_advance_wr(struct tvnet_counter *counter)
{
	WRITE_ONCE(*counter->wr, READ_ONCE(*counter->wr) + 1);

	/* BAR0 mmio address is wc mem, add mb to make sure cnts are updated */
	smp_mb();
}

static inline void tvnet_ivc_advance_rd(struct tvnet_counter *counter)
{
	WRITE_ONCE(*counter->rd, READ_ONCE(*counter->rd) + 1);

	/* BAR0 mmio address is wc mem, add mb to make sure cnts are updated */
	smp_mb();
}

static inline void tvnet_ivc_set_wr(struct tvnet_counter *counter, u32 val)
{
	WRITE_ONCE(*counter->wr, val);

	/* BAR0 mmio address is wc mem, add mb to make sure cnts are updated */
	smp_mb();
}

static inline void tvnet_ivc_set_rd(struct tvnet_counter *counter, u32 val)
{
	WRITE_ONCE(*counter->rd, val);

	/* BAR0 mmio address is wc mem, add mb to make sure cnts are updated */
	smp_mb();
}

static inline u32 tvnet_ivc_get_wr_cnt(struct tvnet_counter *counter)
{
	return READ_ONCE(*counter->wr);
}

static inline u32 tvnet_ivc_get_rd_cnt(struct tvnet_counter *counter)
{
	return READ_ONCE(*counter->rd);
}

#endif
