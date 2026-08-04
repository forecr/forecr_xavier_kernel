// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/pm_reservation.h>

#include "common/vgpu/ivc/comm_vgpu.h"

#include "perf_vf.h"

void vf_perf_reset_pm_trigger_masks(struct gk20a *g, u32 pma_channel_id,
			u32 gr_instance_id, u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_vf_reset_pm_trigger_mask *p =
				&msg.params.reset_pm_trigger_mask;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_RESET_PM_TRIGGER_MASK;
	msg.handle = vgpu_get_handle(g);

	p->pma_channel_id = pma_channel_id;
	p->reservation_id = reservation_id;
	p->gr_instance_id = gr_instance_id;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	return ;
}

void vf_perf_enable_pm_trigger(struct gk20a *g, u32 gr_instance_id,
			u32 pma_channel_id, u32 reservation_id)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_vf_enable_pm_trigger *p =
				&msg.params.enable_pm_trigger;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_VF_PROF_ENABLE_PM_TRIGGER;
	msg.handle = vgpu_get_handle(g);

	p->pma_channel_id = pma_channel_id;
	p->gr_instance_id = gr_instance_id;
	p->reservation_id = reservation_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	return ;
}

static int vf_perf_init_inst_block_mgmt(struct gk20a *g, u32 pma_channel_id,
			u32 inst_blk_ptr, u32 aperture, u32 mode)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_vf_perf_inst_block *p =
				&msg.params.inst_block_mgmt;
	int err;

	nvgpu_log_fn(g, " ");

	if (mode == TEGRA_VGPU_VF_PROF_PERF_INST_BLOCK_INIT) {
		p->aperture = aperture;
		p->iova = inst_blk_ptr;
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_PERF_INST_BLOCK_MGMT;
	msg.handle = vgpu_get_handle(g);

	p->pma_channel_id = pma_channel_id;
	p->mode = mode;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (err != 0)
		nvgpu_err(g,
			"Inst block bind/unbind failed - %d for pma channel - %u",
			err, pma_channel_id);

	return err;
}

void vf_perf_deinit_inst_block(struct gk20a *g, u32 pma_channel_id)
{
	(void) vf_perf_init_inst_block_mgmt(g, pma_channel_id, 0U, 0U,
			TEGRA_VGPU_VF_PROF_PERF_INST_BLOCK_DEINIT);
	return ;
}

int vf_perf_init_inst_block(struct gk20a *g, u32 pma_channel_id,
			u32 inst_blk_ptr, u32 aperture, u32 gfid)
{
	(void) gfid;
	return vf_perf_init_inst_block_mgmt(g, pma_channel_id, inst_blk_ptr,
			aperture, TEGRA_VGPU_VF_PROF_PERF_INST_BLOCK_INIT);
}
