// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/mm.h>

#include "perf_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

static int vgpu_sendrecv_perfbuf_cmd(struct gk20a *g, u64 offset, u32 size,
		u32 pma_channel_id)
{
	struct mm_gk20a *mm = &g->mm;
	struct vm_gk20a *vm = mm->perfbuf[0].vm;
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_perfbuf_mgt_params *p =
						&msg.params.perfbuf_management;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_PERFBUF_MGT;
	msg.handle = vgpu_get_handle(g);

	p->vm_handle = vm->handle;
	p->offset = offset;
	p->size = size;
	p->pma_channel_id = pma_channel_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_perfbuffer_enable(struct gk20a *g, u32 pma_channel_id, u64 offset,
			   u32 size)
{
	return vgpu_sendrecv_perfbuf_cmd(g, offset, size, pma_channel_id);
}

int vgpu_perfbuffer_disable(struct gk20a *g, u32 pma_channel_id)
{
	return vgpu_sendrecv_perfbuf_cmd(g, 0, 0, pma_channel_id);
}

static int vgpu_sendrecv_perfbuf_inst_block_cmd(struct gk20a *g, u32 mode)
{
	struct mm_gk20a *mm = &g->mm;
	struct vm_gk20a *vm = mm->perfbuf[0].vm;
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_perfbuf_inst_block_mgt_params *p =
				&msg.params.perfbuf_inst_block_management;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_PERFBUF_INST_BLOCK_MGT;
	msg.handle = vgpu_get_handle(g);

	p->vm_handle = vm->handle;
	p->mode = mode;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	return err;
}

int vgpu_perfbuffer_init_inst_block(struct gk20a *g, struct perfbuf *perfbuf,
					u32 pma_channel_id)
{
	return vgpu_sendrecv_perfbuf_inst_block_cmd(g,
			TEGRA_VGPU_PROF_PERFBUF_INST_BLOCK_INIT);
}

void vgpu_perfbuffer_deinit_inst_block(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 pma_channel_id)
{
	vgpu_sendrecv_perfbuf_inst_block_cmd(g,
			TEGRA_VGPU_PROF_PERFBUF_INST_BLOCK_DEINIT);
}

int vgpu_perf_update_get_put(struct gk20a *g, u32 pma_channel_id,
		u64 bytes_consumed, bool update_available_bytes, u64 *put_ptr,
		bool *overflowed)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_perf_update_get_put_params *p =
				&msg.params.perf_updat_get_put;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_PERF_UPDATE_GET_PUT;
	msg.handle = vgpu_get_handle(g);

	p->bytes_consumed = bytes_consumed;
	p->update_available_bytes = (u8)update_available_bytes;
	p->pma_channel_id = pma_channel_id;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;

	if (err == 0) {
		if (put_ptr != NULL) {
			*put_ptr = p->put_ptr;
		}
		if (overflowed != NULL) {
			*overflowed = (bool)p->overflowed;
		}
	}

	return err;
}
