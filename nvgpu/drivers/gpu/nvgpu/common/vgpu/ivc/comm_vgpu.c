// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/utils.h>
#include <nvgpu/bug.h>
#include <nvgpu/string.h>
#include <nvgpu/kmem.h>
#include <nvgpu/nvgpu_ivm.h>
#include <nvgpu/vgpu/vgpu.h>

#include "comm_vgpu.h"

int vgpu_comm_init(struct gk20a *g, int nr_ivc)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	int err;

	if (nr_ivc <= 0 || (unsigned int)nr_ivc > BITS_PER_LONG) {
		nvgpu_err(g, "invalid cmd nr_ivc %d!", nr_ivc);
		return -EINVAL;
	}

	ivc_cmd_chs->nr_ivc = nr_ivc;
	ivc_cmd_chs->ivc_chs =
		(struct vgpu_ivc_cmd_ch *)nvgpu_kzalloc(g, sizeof(*ivc_cmd_chs->ivc_chs) * nr_ivc);
	if (ivc_cmd_chs->ivc_chs == NULL) {
		return -ENOMEM;
	}
	err = nvgpu_sem_init(&ivc_cmd_chs->sem_alloc, nr_ivc);
	if (err != 0) {
		nvgpu_err(g, "nvgpu_sem_init failed err=%d", err);
		nvgpu_kfree(g, ivc_cmd_chs->ivc_chs);
		ivc_cmd_chs->ivc_chs = NULL;
		return err;
	}
	nvgpu_spinlock_init(&ivc_cmd_chs->lock_alloc);
	ivc_cmd_chs->alloc_bitmap = GENMASK(nr_ivc - 1, 0);

	return 0;
}

void vgpu_comm_deinit(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;

	nvgpu_sem_destroy(&ivc_cmd_chs->sem_alloc);
	nvgpu_kfree(g, ivc_cmd_chs->ivc_chs);
	ivc_cmd_chs->ivc_chs = NULL;
}

int vgpu_comm_alloc_ivc(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	int ivc;
	int err;

	/* wait for one ivc channel to be available */
	err = nvgpu_sem_wait(&ivc_cmd_chs->sem_alloc);
	if (err != 0) {
		return err;
	}

	/* allocate one ivc channel */
	nvgpu_spinlock_acquire(&ivc_cmd_chs->lock_alloc);
	ivc = nvgpu_fls(ivc_cmd_chs->alloc_bitmap);
	nvgpu_assert(ivc != 0);
	ivc -= 1;
	nvgpu_clear_bit(ivc, &ivc_cmd_chs->alloc_bitmap);
	nvgpu_spinlock_release(&ivc_cmd_chs->lock_alloc);

	return ivc;
}

void vgpu_comm_free_ivc(struct gk20a *g, int ivc)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;

	nvgpu_assert(ivc >= 0 && ivc < ivc_cmd_chs->nr_ivc);
	/* free the ivc channel */
	nvgpu_spinlock_acquire(&ivc_cmd_chs->lock_alloc);
	nvgpu_set_bit(ivc, &ivc_cmd_chs->alloc_bitmap);
	nvgpu_spinlock_release(&ivc_cmd_chs->lock_alloc);

	/* release the ivc channel */
	nvgpu_sem_post(&ivc_cmd_chs->sem_alloc);
}

int vgpu_comm_sendrecv_locked(struct gk20a *g, int ivc,
	struct tegra_vgpu_cmd_msg *msg)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	struct vgpu_ivc_cmd_ch *cmd_ch = &ivc_cmd_chs->ivc_chs[ivc];
	int err;

	err = vgpu_ivc_send(&cmd_ch->ivc, msg, sizeof(*msg));
	if (err != 0) {
		nvgpu_err(g, "send msg failed, err=%d", err);
		return err;
	}

	err = vgpu_ivc_recv(&cmd_ch->ivc, msg, sizeof(*msg));
	if (err == 0 && msg->cmd == TEGRA_VGPU_CMD_DUMMY) {
		err = vgpu_ivc_recv(&cmd_ch->ivc, msg, sizeof(*msg));
	}
	if (err != 0) {
		nvgpu_err(g, "recv msg failed, err=%d", err);
		return err;
	}

	return 0;
}

void vgpu_comm_oob(struct gk20a *g, int ivc, void **va, size_t *size)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ivc_cmd_chs *ivc_cmd_chs = &priv->ivc_cmd_chs;
	struct vgpu_ivc_cmd_ch *cmd_ch = &ivc_cmd_chs->ivc_chs[ivc];

	*va = cmd_ch->ivm_va;
	*size = nvgpu_ivm_get_size(cmd_ch->ivm);
}

int vgpu_comm_sendrecv(struct gk20a *g, struct tegra_vgpu_cmd_msg *msg)
{
	int ivc;
	int err;

	ivc = vgpu_comm_alloc_ivc(g);
	if (ivc < 0) {
		return ivc;
	}
	err = vgpu_comm_sendrecv_locked(g, ivc, msg);
	vgpu_comm_free_ivc(g, ivc);

	return err;
}
