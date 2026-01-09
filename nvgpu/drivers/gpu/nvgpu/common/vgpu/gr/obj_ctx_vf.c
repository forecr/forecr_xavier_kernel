// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/obj_ctx.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>


#include "common/gr/obj_ctx_priv.h"
#include "common/vgpu/ivc/comm_vgpu.h"
#include "obj_ctx_vf.h"

int vgpu_vf_gr_obj_ctx_init_golden_image(struct gk20a *g)
{
	struct nvgpu_gr_obj_ctx_golden_image *golden_image =
					nvgpu_gr_get_golden_image_ptr(g);
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_golden_image_params *p =
					&msg.params.vf_golden_image;
	int ivc = -1;
	void *oob;
	size_t oob_size;
	int err;

	nvgpu_log_fn(g, " ");

	nvgpu_mutex_acquire(&golden_image->ctx_mutex);

	if (golden_image->ready) {
		err = 0;
		goto out;
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_GET_GOLDEN_IMAGE;
	msg.handle = vgpu_get_handle(g);
	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&oob, &oob_size);
	if (ivc < 0) {
		err = ivc;
		goto out;
	}
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
		goto free_ivc;
	}

	nvgpu_assert(p->size ==
		nvgpu_gr_obj_ctx_get_golden_image_size(golden_image));
	memcpy(nvgpu_gr_obj_ctx_get_local_golden_image_ptr(golden_image),
		oob, p->size);
	golden_image->ready = true;
free_ivc:
	vgpu_comm_free_ivc(g, ivc);
out:
	nvgpu_mutex_release(&golden_image->ctx_mutex);
	return err;
}
