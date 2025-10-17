// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "ltc_vgpu.h"

u64 vgpu_determine_L2_size_bytes(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	nvgpu_log_fn(g, " ");

	return priv->constants.l2_size;
}

void vgpu_ltc_init_fs_state(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct nvgpu_ltc *ltc = g->ltc;

	nvgpu_log_fn(g, " ");

	ltc->ltc_count = priv->constants.ltc_count;
	ltc->cacheline_size = priv->constants.cacheline_size;
	ltc->slices_per_ltc = priv->constants.slices_per_ltc;
}

#ifdef CONFIG_NVGPU_DEBUGGER

int vgpu_ltc_get_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 *num_ways)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_l2_max_ways_evict_last_params *p =
				&msg.params.l2_max_ways_evict_last;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_TSG_GET_L2_MAX_WAYS_EVICT_LAST;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (unlikely(err)) {
		nvgpu_err(g, "failed to get L2 max ways evict last, err %d",
			err);
	} else {
		*num_ways = p->num_ways;
	}

	return err;
}

int vgpu_ltc_set_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 num_ways)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_l2_max_ways_evict_last_params *p =
				&msg.params.l2_max_ways_evict_last;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_TSG_SET_L2_MAX_WAYS_EVICT_LAST;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	p->num_ways = num_ways;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (unlikely(err)) {
		nvgpu_err(g, "failed to set L2 max ways evict last, err %d",
			err);
	}

	return err;
}

int vgpu_ltc_set_sector_promotion(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 policy)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_l2_sector_promotion_params *p =
				&msg.params.l2_promotion;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_L2_SECTOR_PROMOTION;
	msg.handle = vgpu_get_handle(g);
	p->tsg_id = tsg->tsgid;
	p->policy = policy;
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (unlikely(err)) {
		nvgpu_err(g, "failed to set L2 sector promotion, err %d",
			err);
	}

	return err;
}

#endif
