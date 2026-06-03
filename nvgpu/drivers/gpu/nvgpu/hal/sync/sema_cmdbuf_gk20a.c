// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/semaphore.h>
#include <nvgpu/priv_cmdbuf.h>

#include "sema_cmdbuf_gk20a.h"

u32 gk20a_sema_get_wait_cmd_size(void)
{
	return 8U;
}

u32 gk20a_sema_get_incr_cmd_size(void)
{
	return 10U;
}

static void gk20a_sema_add_header(struct gk20a *g,
		struct priv_cmd_entry *cmd, u64 sema_va)
{
	u32 data[] = {
		/* semaphore_a */
		0x20010004U,
		/* offset_upper */
		(u32)(sema_va >> 32) & 0xffU,
		/* semaphore_b */
		0x20010005U,
		/* offset */
		(u32)sema_va & 0xffffffff,
	};

	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}

void gk20a_sema_add_wait_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va)
{
	u32 data[] = {
		/* semaphore_c */
		0x20010006U,
		/* payload */
		nvgpu_semaphore_get_value(s),
		/* semaphore_d */
		0x20010007U,
		/* operation: acq_geq, switch_en */
		0x4U | BIT32(12),
	};

	nvgpu_log_fn(g, " ");

	gk20a_sema_add_header(g, cmd, sema_va);
	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}

void gk20a_sema_add_incr_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va,
		bool wfi)
{
	u32 data[] = {
		/* semaphore_c */
		0x20010006U,
		/* payload */
		nvgpu_semaphore_get_value(s),
		/* semaphore_d */
		0x20010007U,
		/* operation: release, wfi */
		0x2UL | (u32)((wfi ? 0x0UL : 0x1UL) << 20),
		/* non_stall_int */
		0x20010008U,
		/* ignored */
		0U,
	};

	nvgpu_log_fn(g, " ");

	gk20a_sema_add_header(g, cmd, sema_va);
	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}
