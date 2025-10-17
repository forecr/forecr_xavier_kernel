// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/gk20a.h>

#include <nvgpu/hw/gp10b/hw_pbdma_gp10b.h>

#include "pbdma_gp10b.h"

u32 gp10b_pbdma_channel_fatal_0_intr_descs(void)
{
	/*
	 * These are data parsing, framing errors or others which can be
	 * recovered from with intervention... or just resetting the
	 * channel
	 */
	u32 channel_fatal_0_intr_descs =
		pbdma_intr_0_gpfifo_pending_f() |
		pbdma_intr_0_gpptr_pending_f() |
		pbdma_intr_0_gpentry_pending_f() |
		pbdma_intr_0_gpcrc_pending_f() |
		pbdma_intr_0_pbptr_pending_f() |
		pbdma_intr_0_pbentry_pending_f() |
		pbdma_intr_0_pbcrc_pending_f() |
		pbdma_intr_0_method_pending_f() |
		pbdma_intr_0_methodcrc_pending_f() |
		pbdma_intr_0_pbseg_pending_f() |
		pbdma_intr_0_syncpoint_illegal_pending_f() |
		pbdma_intr_0_signature_pending_f();

	return channel_fatal_0_intr_descs;
}

u32 gp10b_pbdma_allowed_syncpoints_0_index_f(u32 syncpt)
{
	return pbdma_allowed_syncpoints_0_index_f(syncpt);
}

u32 gp10b_pbdma_allowed_syncpoints_0_valid_f(void)
{
	return pbdma_allowed_syncpoints_0_valid_f(1U);
}

u32 gp10b_pbdma_allowed_syncpoints_0_index_v(u32 offset)
{
	return pbdma_allowed_syncpoints_0_index_v(offset);
}
