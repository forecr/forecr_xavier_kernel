/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2022-2024, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __TEGRA_HV_IVC_H
#define __TEGRA_HV_IVC_H

#include <linux/of.h>
#include <linux/version.h>

struct tegra_hv_ivc_cookie {
	/* some fields that might be useful */
	int irq;
	int peer_vmid;
	int nframes;
	int frame_size;
	uint32_t *notify_va; /* address used to notify end-point */
};

struct tegra_hv_ivc_ops {
	/* called when data are received */
	void (*rx_rdy)(struct tegra_hv_ivc_cookie *ivck);
	/* called when space is available to write data */
	void (*tx_rdy)(struct tegra_hv_ivc_cookie *ivck);
};

struct tegra_hv_ivm_cookie {
	uint64_t ipa;
	uint64_t size;
	unsigned peer_vmid;
	void *reserved;
};

#if defined(CONFIG_TEGRA_VIRTUALIZATION)
bool is_tegra_hypervisor_mode(void);

/**
 * tegra_hv_ivc_reserve - Reserve an IVC queue for use
 * @dn:		Device node pointer to the queue in the DT
 *		If NULL, then operate on first HV device
 * @queue_id	Id number of the queue to use.
 * @ops		Ops structure or NULL (deprecated)
 *
 * Reserves the queue for use
 *
 * Returns a pointer to the ivc_dev to use or an ERR_PTR.
 * Note that returning EPROBE_DEFER means that the ivc driver
 * hasn't loaded yet and you should try again later in the
 * boot sequence.
 *
 * Note that @ops must be NULL for channels that handle reset.
 */
struct tegra_hv_ivc_cookie *tegra_hv_ivc_reserve(
		struct device_node *dn, int id,
		const struct tegra_hv_ivc_ops *ops);

/**
 * tegra_hv_ivc_unreserve - Unreserve an IVC queue used
 * @ivck	IVC cookie
 *
 * Unreserves the IVC channel
 *
 * Returns 0 on success and an error code otherwise
 */
int tegra_hv_ivc_unreserve(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_write - Writes a frame to the IVC queue
 * @ivck	IVC cookie of the queue
 * @buf		Pointer to the data to write
 * @size	Size of the data to write
 *
 * Write a number of bytes (as a single frame) from the queue.
 *
 * Returns size on success and an error code otherwise
 */
int tegra_hv_ivc_write(struct tegra_hv_ivc_cookie *ivck, const void *buf,
		int size);

/**
 * ivc_hv_ivc_write_user - Writes a frame to the IVC queue
 * @ivck	IVC cookie of the queue
 * @buf		Pointer to the userspace data to write
 * @size	Size of the data to write
 *
 * Write a number of bytes (as a single frame) from the queue.
 *
 * Returns size on success and an error code otherwise
 */
int tegra_hv_ivc_write_user(struct tegra_hv_ivc_cookie *ivck, const void __user *buf,
		int size);

/**
 * ivc_hv_ivc_read - Reads a frame from the IVC queue
 * @ivck	IVC cookie of the queue
 * @buf		Pointer to the data to read
 * @size	max size of the data to read
 *
 * Reads a number of bytes (as a single frame) from the queue.
 *
 * Returns size on success and an error code otherwise
 */
int tegra_hv_ivc_read(struct tegra_hv_ivc_cookie *ivck, void *buf, int size);

/**
 * ivc_hv_ivc_read_user - Reads a frame from the IVC queue
 * @ivck	IVC cookie of the queue
 * @buf		Pointer to the userspace data to read
 * @size	max size of the data to read
 *
 * Reads a number of bytes (as a single frame) from the queue.
 *
 * Returns size on success and an error code otherwise
 */
int tegra_hv_ivc_read_user(struct tegra_hv_ivc_cookie *ivck, void __user *buf, int size);

/**
 * ivc_hv_ivc_can_read - Test whether data are available
 * @ivck	IVC cookie of the queue
 *
 * Test wheter data to read are available
 *
 * Returns 1 if data are available in the rx queue, 0 if not
 */
int tegra_hv_ivc_can_read(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_can_write - Test whether data can be written
 * @ivck	IVC cookie of the queue
 *
 * Test wheter data can be written
 *
 * Returns 1 if data are can be written to the tx queue, 0 if not
 */
int tegra_hv_ivc_can_write(struct tegra_hv_ivc_cookie *ivck);

/**
 * tegra_ivc_tx_frames_available - gets number of free entries in tx queue
 * @ivc/@ivck	IVC channel or cookie
 *
 * Returns the number of unused entries in the tx queue. Assuming the caller
 * does not write any additional frames, this number may increase from the
 * value returned as the receiver consumes frames.
 *
*/
uint32_t tegra_hv_ivc_tx_frames_available(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_tx_empty - Test whether the tx queue is empty
 * @ivck	IVC cookie of the queue
 *
 * Test wheter the tx queue is completely empty
 *
 * Returns 1 if the queue is empty, zero otherwise
 */
int tegra_hv_ivc_tx_empty(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_loopback - Sets (or clears) loopback mode
 * @ivck	IVC cookie of the queue
 * @mode	Set loopback on/off (1 = on, 0 = off)
 *
 * Sets or clears loopback mode accordingly.
 *
 * When loopback is active any writes are ignored, while
 * reads do not return data.
 * Incoming data are copied immediately to the tx queue.
 *
 * Returns 0 on success, a negative error code otherwise
 */
int tegra_hv_ivc_set_loopback(struct tegra_hv_ivc_cookie *ivck, int mode);

/* debugging aid */
int tegra_hv_ivc_dump(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_read_peek - Peek (copying) data from a received frame
 * @ivck	IVC cookie of the queue
 * @buf		Buffer to receive the data
 * @off		Offset in the frame
 * @count	Count of bytes to copy
 *
 * Peek data from a received frame, copying to buf, without removing
 * the frame from the queue.
 *
 * Returns 0 on success, a negative error code otherwise
 */
int tegra_hv_ivc_read_peek(struct tegra_hv_ivc_cookie *ivck,
		void *buf, int off, int count);

/**
 * ivc_hv_ivc_read_get_next_frame - Peek at the next frame to receive
 * @ivck	IVC cookie of the queue
 *
 * Peek at the next frame to be received, without removing it from
 * the queue.
 *
 * Returns a pointer to the frame, or an error encoded pointer.
 */
void *tegra_hv_ivc_read_get_next_frame(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_read_advance - Advance the read queue
 * @ivck	IVC cookie of the queue
 *
 * Advance the read queue
 *
 * Returns 0, or a negative error value if failed.
 */
int tegra_hv_ivc_read_advance(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_write_poke - Poke data to a frame to be transmitted
 * @ivck	IVC cookie of the queue
 * @buf		Buffer to the data
 * @off		Offset in the frame
 * @count	Count of bytes to copy
 *
 * Copy data to a transmit frame, copying from buf, without advancing
 * the the transmit queue.
 *
 * Returns 0 on success, a negative error code otherwise
 */
int tegra_hv_ivc_write_poke(struct tegra_hv_ivc_cookie *ivck,
		const void *buf, int off, int count);

/**
 * ivc_hv_ivc_write_get_next_frame - Poke at the next frame to transmit
 * @ivck	IVC cookie of the queue
 *
 * Get access to the next frame.
 *
 * Returns a pointer to the frame, or an error encoded pointer.
 */
void *tegra_hv_ivc_write_get_next_frame(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_hv_ivc_write_advance - Advance the write queue
 * @ivck	IVC cookie of the queue
 *
 * Advance the write queue
 *
 * Returns 0, or a negative error value if failed.
 */
int tegra_hv_ivc_write_advance(struct tegra_hv_ivc_cookie *ivck);

/**
 * tegra_hv_mempool_reserve - reserve a mempool for use
 * @id		Id of the requested mempool.
 *
 * Returns a cookie representing the mempool on success, otherwise an ERR_PTR.
 */
struct tegra_hv_ivm_cookie *tegra_hv_mempool_reserve(unsigned id);

/**
 * tegra_hv_mempool_release - release a reserved mempool
 * @ck		Cookie returned by tegra_hv_mempool_reserve().
 *
 * Returns 0 on success or a negative error code otherwise.
 */
int tegra_hv_mempool_unreserve(struct tegra_hv_ivm_cookie *ck);

/**
 * ivc_channel_notified - handle internal messages
 * @ivck	IVC cookie of the queue
 *
 * This function must be called following every notification (interrupt or
 * callback invocation) for the tegra_hv_- version).
 *
 * Returns 0 if the channel is ready for communication, or -EAGAIN if a channel
 * reset is in progress.
 */
int tegra_hv_ivc_channel_notified(struct tegra_hv_ivc_cookie *ivck);

/**
 * ivc_channel_reset - initiates a reset of the shared memory state
 * @ivck	IVC cookie of the queue
 *
 * This function must be called after a channel is reserved before it is used
 * for communication. The channel will be ready for use when a subsequent call
 * to ivc_channel_notified() returns 0.
 */
void tegra_hv_ivc_channel_reset(struct tegra_hv_ivc_cookie *ivck);

/**
 * tegra_hv_ivc_get_info - Get info of Guest shared area
 * @ivck	IVC cookie of the queue
 * @pa		IPA of shared area
 * @size	Size of the shared area
 *
 * Get info (IPA and size) of Guest shared area
 *
 * Returns size on success and an error code otherwise
 */
int tegra_hv_ivc_get_info(struct tegra_hv_ivc_cookie *ivck, uint64_t *pa,
			  uint64_t *size);

/**
 * tegra_hv_ivc_notify - Notify remote guest
 * @ivck	IVC cookie of the queue
 *
 * Notify remote guest
 *
 */
void tegra_hv_ivc_notify(struct tegra_hv_ivc_cookie *ivck);

struct tegra_ivc *tegra_hv_ivc_convert_cookie(struct tegra_hv_ivc_cookie *ivck);
#else
static inline bool is_tegra_hypervisor_mode(void)
{
	return false;
}

static inline struct tegra_hv_ivc_cookie *tegra_hv_ivc_reserve(
		struct device_node *dn, int id,
		const struct tegra_hv_ivc_ops *ops)
{
	return ERR_PTR(-ENOTSUPP);
};

static inline int tegra_hv_ivc_unreserve(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_write(struct tegra_hv_ivc_cookie *ivck,
				     const void *buf, int size)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_write_user(struct tegra_hv_ivc_cookie *ivck,
					  const void __user *buf, int size)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_read(struct tegra_hv_ivc_cookie *ivck, void *buf,
				    int size)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_read_user(struct tegra_hv_ivc_cookie *ivck,
					 void __user *buf, int size)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_can_read(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_can_write(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline uint32_t tegra_hv_ivc_tx_frames_available(
		struct tegra_hv_ivc_cookie *ivck)
{
	return 0;
};

static inline int tegra_hv_ivc_tx_empty(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_set_loopback(struct tegra_hv_ivc_cookie *ivck,
					    int mode)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_dump(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_read_peek(struct tegra_hv_ivc_cookie *ivck,
					 void *buf, int off, int count)
{
	return -ENOTSUPP;
};

static inline void *tegra_hv_ivc_read_get_next_frame(
		struct tegra_hv_ivc_cookie *ivck)
{
	return ERR_PTR(-ENOTSUPP);
};

static inline int tegra_hv_ivc_read_advance(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_write_poke(struct tegra_hv_ivc_cookie *ivck,
					  const void *buf, int off, int count)
{
	return -ENOTSUPP;
};

static inline void *tegra_hv_ivc_write_get_next_frame(
		struct tegra_hv_ivc_cookie *ivck)
{
	return ERR_PTR(-ENOTSUPP);
};

static inline int tegra_hv_ivc_write_advance(struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline struct tegra_hv_ivm_cookie *tegra_hv_mempool_reserve(unsigned id)
{
	return ERR_PTR(-ENOTSUPP);
};

static inline int tegra_hv_mempool_unreserve(struct tegra_hv_ivm_cookie *ck)
{
	return -ENOTSUPP;
};

static inline int tegra_hv_ivc_channel_notified(
		struct tegra_hv_ivc_cookie *ivck)
{
	return -ENOTSUPP;
};

static inline void tegra_hv_ivc_channel_reset(struct tegra_hv_ivc_cookie *ivck)
{
	return;
};

static inline int tegra_hv_ivc_get_info(struct tegra_hv_ivc_cookie *ivck,
					uint64_t *pa, uint64_t *size)
{
	return -ENOTSUPP;
};

static inline void tegra_hv_ivc_notify(struct tegra_hv_ivc_cookie *ivck)
{
	return;
};

static inline struct tegra_ivc *tegra_hv_ivc_convert_cookie(
		struct tegra_hv_ivc_cookie *ivck)
{
	return ERR_PTR(-ENOTSUPP);
};
#endif /* defined(CONFIG_TEGRA_VIRTUALIZATION) */

#endif /* __TEGRA_HV_IVC_H */
