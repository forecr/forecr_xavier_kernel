// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/log.h>
#include <nvgpu/semaphore.h>
#include <nvgpu/priv_cmdbuf.h>

#include "sema_cmdbuf_gv11b.h"

u32 gv11b_sema_get_wait_cmd_size(void)
{
	return 10U;
}

u32 gv11b_sema_get_incr_cmd_size(void)
{
	return 12U;
}

static void gv11b_sema_add_header(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va)
{
	u32 data[] = {
		/* sema_addr_lo */
		0x20010017,
		sema_va & 0xffffffffULL,

		/* sema_addr_hi */
		0x20010018,
		(sema_va >> 32ULL) & 0xffULL,

		/* payload_lo */
		0x20010019,
		nvgpu_semaphore_get_value(s),

		/* payload_hi : ignored */
		0x2001001a,
		0,
	};

	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}

void gv11b_sema_add_wait_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va)
{
	u32 data[] = {
		/* sema_execute : acq_circ_geq | switch_en */
		0x2001001b,
		U32(0x3) | BIT32(12U),
	};

	nvgpu_log_fn(g, " ");

	gv11b_sema_add_header(g, cmd, s, sema_va);
	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}

void gv11b_sema_add_incr_cmd(struct gk20a *g,
		struct priv_cmd_entry *cmd,
		struct nvgpu_semaphore *s, u64 sema_va,
		bool wfi)
{
	u32 data[] = {
		/* sema_execute : release | wfi | 32bit */
		0x2001001b,
		U32(0x1) | ((wfi ? U32(0x1) : U32(0x0)) << 20U),

		/* non_stall_int : payload is ignored */
		0x20010008,
		0,
	};

	nvgpu_log_fn(g, " ");

	gv11b_sema_add_header(g, cmd, s, sema_va);
	nvgpu_priv_cmdbuf_append(g, cmd, data, ARRAY_SIZE(data));
}
