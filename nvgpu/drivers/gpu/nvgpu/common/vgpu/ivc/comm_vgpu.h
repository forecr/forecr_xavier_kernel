/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef COMM_VGPU_H
#define COMM_VGPU_H

#include <nvgpu/vgpu/vgpu_ivc.h>

struct gk20a;
struct tegra_vgpu_cmd_msg;
struct tegra_hv_ivm_cookie;

/* Represents one IVC channel for IVC command.
 * One VGPU device may have one or more IVC channels.
 */
struct vgpu_ivc_cmd_ch {
	struct gk20a *g;
	/* IVC queue OS abstract */
	struct vgpu_ivc ivc;
	/* IVM mempool associated with IVC command queue */
	struct tegra_hv_ivm_cookie *ivm;
	/* CPU VA for the mempool */
	void *ivm_va;
};

struct vgpu_ivc_cmd_chs {
	int nr_ivc;
	struct vgpu_ivc_cmd_ch *ivc_chs; /* nr_ivc of ivc channels */

	struct nvgpu_sem sem_alloc; /* semaphore for nr_ivc */
	struct nvgpu_spinlock lock_alloc; /* lock for alloc_bitmap */
	unsigned long alloc_bitmap; /* bitmap for ivc channel allocation */
};

/**
 * Init vgpu comm layer. Creates nr_ivc of IVC channels.
 */
int vgpu_comm_init(struct gk20a *g, int nr_ivc);
/**
 * Deinit comm layer.
 */
void vgpu_comm_deinit(struct gk20a *g);
/**
 * Allocate one ivc channel to send ivc message.
 * @return ivc >= 0 if ivc channel is allocated successfully.
 * @return -EINTR for QNX if signal interrupted sem wait.
 * @return -ERESTARTSYS for Linux if killable signal interrupted sem wait.
 */
int vgpu_comm_alloc_ivc(struct gk20a *g);
/**
 * Free the ivc channel that is allocated with vgpu_comm_alloc_ivc.
 */
void vgpu_comm_free_ivc(struct gk20a *g, int ivc);
/**
 * Send one ivc message with the allocated ivc channel.
 */
int vgpu_comm_sendrecv_locked(struct gk20a *g, int ivc,
	struct tegra_vgpu_cmd_msg *msg);
/**
 * Get OOB info (mempool) associated with the ivc channel.
 */
void vgpu_comm_oob(struct gk20a *g, int ivc, void **va, size_t *size);
/**
 * Allocate an IVC channel and send one IVC message, then free the channel.
 */
int vgpu_comm_sendrecv(struct gk20a *g, struct tegra_vgpu_cmd_msg *msg);

static inline int vgpu_comm_alloc_ivc_oob(struct gk20a *g, void **va, size_t *size)
{
	int ivc = vgpu_comm_alloc_ivc(g);
	if (ivc < 0) {
		return ivc;
	}
	vgpu_comm_oob(g, ivc, va, size);
	return ivc;
}

#endif
