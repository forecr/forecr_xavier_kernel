/* SPDX-License-Identifier: GPL-2.0-only */
/* SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 *
 * XDP/AF_XDP declarations for NvEthernet driver.
 */

#ifndef _ETHER_XDP_H_
#define _ETHER_XDP_H_

#include <linux/types.h>
#include <linux/bitops.h>

struct net_device;
struct netdev_bpf;
struct ether_priv_data;
struct xdp_frame;
struct xdp_buff;

#ifdef CONFIG_XDP_SOCKETS
#include <linux/bpf.h>
#define ETHER_XDP_RX_HEADROOM XDP_PACKET_HEADROOM
#else
#define ETHER_XDP_RX_HEADROOM 0U
#endif

/** tx_swcx->flags bit to identify XDP frame (buf_virt_addr is xdp_frame *) */
#define ETHER_TX_SWCX_XDP_FRAME  BIT(6)
/** tx_swcx->flags bit to identify XSK buffer (from AF_XDP TX ring) */
#define ETHER_TX_SWCX_XSK_BUFF   BIT(7)
/** tx_swcx->flags bit to identify RX-origin XDP_TX frame */
#define ETHER_TX_SWCX_XDP_TX     BIT(8)
/** tx_swcx->flags bit to identify page-pool DMA that must not be unmapped */
#define ETHER_TX_SWCX_XDP_TX_PAGE_POOL BIT(9)
/** tx_swcx->flags bit to identify XSK DMA that must not be unmapped */
#define ETHER_TX_SWCX_XDP_TX_XSK BIT(10)
/** tx_swcx->flags bits that identify RX-origin DMA owned outside Tx */
#define ETHER_TX_SWCX_XDP_TX_DMA_REUSE \
	(ETHER_TX_SWCX_XDP_TX_PAGE_POOL | ETHER_TX_SWCX_XDP_TX_XSK)

/**
 * enum ether_xdp_action - XDP action returned from ether_xdp_run_prog
 *
 * @ETHER_XDP_PASS: Continue to normal SKB path
 * @ETHER_XDP_DROP: Drop packet; caller must free buffer
 * @ETHER_XDP_CONSUMED: Drop packet; buffer already returned by callee
 * @ETHER_XDP_TX: Packet transmitted back out; caller must not free buffer
 * @ETHER_XDP_REDIRECT: Redirected; caller must not free buffer;
 *   xdp_do_flush() must be called at end of NAPI poll
 */
enum ether_xdp_action {
	ETHER_XDP_PASS,
	ETHER_XDP_DROP,
	ETHER_XDP_CONSUMED,
	ETHER_XDP_TX,
	ETHER_XDP_REDIRECT,
};

/** Number of per-channel XDP counters exported through ethtool. */
#define ETHER_XDP_STATS_COUNT 3U

/**
 * ether_bpf - Handle an XDP setup request from the networking core
 * @dev: Network device receiving the request
 * @bpf: XDP setup command and associated parameters
 *
 * Installs or removes the native XDP program.
 *
 * @return 0 on success, or a negative errno on failure
 */
int ether_bpf(struct net_device *dev, struct netdev_bpf *bpf);

/**
 * ether_xdp_rxq_setup - Register XDP receive queues with the networking core
 * @pdata: NvEthernet private data
 *
 * Registers the active DMA channels and their initial RX memory model during
 * interface open.
 *
 * @return 0 on success, or a negative errno on failure
 */
int ether_xdp_rxq_setup(struct ether_priv_data *pdata);

/**
 * ether_xdp_rxq_teardown - Unregister all XDP receive queues
 * @pdata: NvEthernet private data
 *
 * Unregisters RX queue metadata and memory models during interface close.
 */
void ether_xdp_rxq_teardown(struct ether_priv_data *pdata);

/**
 * ether_xdp_rxq_teardown_chan - Unregister one XDP receive queue
 * @pdata: NvEthernet private data
 * @chan: Hardware DMA channel to unregister
 *
 * The RX descriptor buffers are returned before this function is called, so
 * the queue memory model can be unregistered before its page pool is freed.
 */
void ether_xdp_rxq_teardown_chan(struct ether_priv_data *pdata,
				 unsigned int chan);

/**
 * ether_xdp_xmit - Transmit redirected XDP frames
 * @dev: Network device used for transmission
 * @n: Number of frames in @frames
 * @frames: Array of XDP frames to transmit
 * @flags: XDP transmit flags, such as XDP_XMIT_FLUSH
 *
 * @return Number of frames accepted, or a negative errno on failure
 */
int ether_xdp_xmit(struct net_device *dev, int n, struct xdp_frame **frames,
		   u32 flags);

/**
 * ether_xdp_tx_wake_queue - Wake the TX queue associated with a DMA channel
 * @pdata: NvEthernet private data
 * @chan: Hardware DMA channel whose queue should be woken
 */
void ether_xdp_tx_wake_queue(struct ether_priv_data *pdata, unsigned int chan);

/**
 * @brief ether_xdp_run_prog - Run XDP program on received packet
 *
 * @param pdata: Driver private data
 * @param chan: RX channel (for xdp_rxq_info)
 * @param data: Packet data pointer
 * @param len: Packet length
 * @param frame_sz: Buffer frame size (e.g. PAGE_SIZE)
 * @param headroom: Offset from data_hard_start to packet data
 * @param allow_redirect: Whether backing memory can be redirected safely
 * @param pass_data: Updated data pointer when action is XDP_PASS
 * @param pass_len: Updated packet length when action is XDP_PASS
 * @param prog_run: Set when an XDP program was executed
 * @return ETHER_XDP_PASS, ETHER_XDP_DROP, ETHER_XDP_CONSUMED,
 * ETHER_XDP_TX, or ETHER_XDP_REDIRECT
 */
enum ether_xdp_action
ether_xdp_run_prog(struct ether_priv_data *pdata, unsigned int chan,
		   void *data, unsigned int len, unsigned int frame_sz,
		   unsigned int headroom, bool allow_redirect, void **pass_data,
		   unsigned int *pass_len, bool *prog_run);

/**
 * @brief ether_xdp_run_prog_xdp - Run XDP program on xdp_buff (for XSK)
 *
 * Same as ether_xdp_run_prog but takes pre-allocated xdp_buff from xsk_buff_alloc.
 * Used when XSK pool is active.
 *
 * @param pdata: Driver private data
 * @param chan: RX channel
 * @param xdp: xdp_buff from xsk_buff_alloc (size already set via xsk_buff_set_size)
 * @param act_len: Packet length after the XDP program action
 * @return ETHER_XDP_PASS, ETHER_XDP_DROP, ETHER_XDP_CONSUMED,
 * ETHER_XDP_TX, or ETHER_XDP_REDIRECT
 */
enum ether_xdp_action
ether_xdp_run_prog_xdp(struct ether_priv_data *pdata, unsigned int chan,
		       struct xdp_buff *xdp, unsigned int *act_len);

int ether_xsk_wakeup(struct net_device *dev, u32 queue_id, u32 flags);
void ether_xsk_tx_wakeup_all(struct ether_priv_data *pdata);
int ether_xsk_tx(struct ether_priv_data *pdata, unsigned int chan,
		 unsigned int budget);
void ether_xsk_rx_refill(struct ether_priv_data *pdata, unsigned int chan);

/**
 * ether_xsk_validate_rx_buf_len - Validate attached XSK pool capacities
 * @pdata: Driver private data
 * @rx_buf_len: Required receive buffer length
 *
 * Verifies that every attached XSK pool can hold the complete receive buffer
 * required by the configured MTU.
 *
 * @return 0 on success, or a negative errno if a pool is too small
 */
int ether_xsk_validate_rx_buf_len(struct ether_priv_data *pdata,
				  unsigned int rx_buf_len);

/** DMA attribute for XSK pool map/unmap */
#define ETHER_XDP_RX_DMA_ATTR 0

#endif /* _ETHER_XDP_H_ */
