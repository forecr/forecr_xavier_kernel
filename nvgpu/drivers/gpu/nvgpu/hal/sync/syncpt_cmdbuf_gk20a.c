// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/priv_cmdbuf.h>

#include "syncpt_cmdbuf_gk20a.h"

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
void gk20a_syncpt_add_wait_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		u32 id, u32 thresh, u64 gpu_va_base)
{
	u32 data[] = {
		/* syncpoint_a */
		0x2001001CU,
		/* payload */
		thresh,
		/* syncpoint_b */
		0x2001001DU,
		/* syncpt_id, switch_en, wait */
		(id << 8U) | 0x10U,
	};

	(void)gpu_va_base;

	nvgpu_log_fn(g, " ");

	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}

u32 gk20a_syncpt_get_wait_cmd_size(void)
{
	return 4U;
}

u32 gk20a_syncpt_get_incr_per_release(void)
{
	return 2U;
}

void gk20a_syncpt_add_incr_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		u32 id, u64 gpu_va, bool wfi)
{
	u32 wfi_data[] = {
		/* wfi */
		0x2001001EU,
		/* handle, ignored */
		0x00000000U,
	};

	u32 incr_data[] = {
		/* syncpoint_a */
		0x2001001CU,
		/* payload, ignored */
		0U,
		/* syncpoint_b */
		0x2001001DU,
		/* syncpt_id, incr */
		(id << 8U) | 0x1U,
		/* syncpoint_b */
		0x2001001DU,
		/* syncpt_id, incr */
		(id << 8U) | 0x1U,
	};

	(void)gpu_va;

	nvgpu_log_fn(g, " ");

	if (wfi) {
		nvgpu_priv_cmdbuf_append(g, cmd, wfi_data,
				ARRAY_SIZE(wfi_data));
	}
	nvgpu_priv_cmdbuf_append(g, cmd, incr_data, ARRAY_SIZE(incr_data));
}

u32 gk20a_syncpt_get_incr_cmd_size(bool wfi_cmd)
{
	if (wfi_cmd) {
		return 8U;
	} else {
		return 6U;
	}
}
#endif

void gk20a_syncpt_free_buf(struct nvgpu_channel *c,
		struct nvgpu_mem *syncpt_buf)
{
	(void)c;
	(void)syncpt_buf;
}

int gk20a_syncpt_alloc_buf(struct nvgpu_channel *c,
		u32 syncpt_id, struct nvgpu_mem *syncpt_buf)
{
	(void)c;
	(void)syncpt_id;
	(void)syncpt_buf;
	return 0;
}
