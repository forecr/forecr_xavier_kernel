// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/log.h>
#include <nvgpu/errno.h>
#include <nvgpu/ptimer.h>

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>

#include "ptimer_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_read_ptimer(struct gk20a *g, u64 *value)
{
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_read_ptimer_params *p = &msg.params.read_ptimer;
	int err;

	nvgpu_log_fn(g, " ");

	msg.cmd = TEGRA_VGPU_CMD_READ_PTIMER;
	msg.handle = vgpu_get_handle(g);

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err == 0) {
		*value = p->time;
	} else {
		nvgpu_err(g, "vgpu read ptimer failed, err=%d", err);
	}

	return err;
}

int vgpu_get_timestamps_zipper(struct gk20a *g,
		u32 source_id, u32 count,
		struct nvgpu_cpu_time_correlation_sample *samples)
{
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_get_timestamps_zipper_params *p =
			&msg.params.get_timestamps_zipper;
	int err;
	u32 i;

	nvgpu_log_fn(g, " ");

	if (count > TEGRA_VGPU_GET_TIMESTAMPS_ZIPPER_MAX_COUNT) {
		nvgpu_err(g, "count %u overflow", count);
		return -EINVAL;
	}

	msg.cmd = TEGRA_VGPU_CMD_GET_TIMESTAMPS_ZIPPER;
	msg.handle = vgpu_get_handle(g);
	p->source_id = TEGRA_VGPU_GET_TIMESTAMPS_ZIPPER_SRC_ID_TSC;
	p->count = count;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g, "vgpu get timestamps zipper failed, err=%d", err);
		return err;
	}

	for (i = 0; i < count; i++) {
		samples[i].cpu_timestamp = p->samples[i].cpu_timestamp;
		samples[i].gpu_timestamp = p->samples[i].gpu_timestamp;
	}

	return err;
}
