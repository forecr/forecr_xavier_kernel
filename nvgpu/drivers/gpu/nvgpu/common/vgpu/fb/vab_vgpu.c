// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/tegra_vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"
#include "vab_vgpu.h"

int vgpu_fb_vab_reserve(struct gk20a *g, u32 vab_mode, u32 num_range_checkers,
			struct nvgpu_vab_range_checker *vab_range_checker)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_fb_vab_reserve_params *p = &msg.params.fb_vab_reserve;
	int err;
	void *oob;
	size_t size, oob_size;
	int ivc;

	ivc = vgpu_comm_alloc_ivc_oob(g, &oob, &oob_size);
	if (ivc < 0) {
		return ivc;
	}
	size = sizeof(*vab_range_checker) * num_range_checkers;
	if (oob_size < size) {
		err = -ENOMEM;
		goto done;
	}

	msg.cmd = TEGRA_VGPU_CMD_FB_VAB_RESERVE;
	msg.handle = vgpu_get_handle(g);
	p->vab_mode = vab_mode;
	p->num_range_checkers = num_range_checkers;

	nvgpu_memcpy((u8 *)oob, (u8 *)vab_range_checker, size);
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);

	err = err != 0 ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g, "fb vab reserve failed err %d", err);
	}

done:
	vgpu_comm_free_ivc(g, ivc);
    return err;
}

int vgpu_fb_vab_dump_and_clear(struct gk20a *g, u8 *user_buf,
				u64 user_buf_size)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_fb_vab_dump_and_clear_params *p =
			&msg.params.fb_vab_dump_and_clear;
	int err;
	void *oob;
	size_t oob_size;
	int ivc;

	ivc = vgpu_comm_alloc_ivc_oob(g, &oob, &oob_size);
	if (ivc < 0) {
		return ivc;
	}

	if (oob_size < user_buf_size) {
		err = -ENOMEM;
		goto done;
	}

	msg.cmd = TEGRA_VGPU_CMD_FB_VAB_DUMP_CLEAR;
	msg.handle = vgpu_get_handle(g);
	p->user_buf_size = user_buf_size;

	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);

	err = err != 0 ? err : msg.ret;
	if (err == 0) {
		nvgpu_memcpy(user_buf, (u8 *)oob, user_buf_size);
	} else {
		nvgpu_err(g, "fb vab flush state failed err %d", err);
	}

done:
	vgpu_comm_free_ivc(g, ivc);
	return err;
}

int vgpu_fb_vab_release(struct gk20a *g)
{
	struct tegra_vgpu_cmd_msg msg = {};
	int err;

	msg.cmd = TEGRA_VGPU_CMD_FB_VAB_RELEASE;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err != 0 ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g, "fb vab release failed err %d", err);
	}

    return err;
}
