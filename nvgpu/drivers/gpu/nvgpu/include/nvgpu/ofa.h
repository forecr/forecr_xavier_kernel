/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OFA_H
#define NVGPU_OFA_H

#include <nvgpu/riscv.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/device.h>

/* OFA sw data */
struct nvgpu_ofa {

	/* OFA nvriscv descriptor info needed for BCR config */
	u32 nvriscv_firmware_desc[NVGPU_NVRISCV_INDEX_MAX];

	/* Falcon used to execute OFA ucode */
	struct nvgpu_falcon *ofa_riscv[OFA_INST_MAX];

	/** Memory to store ucode contents locally. */
	struct nvgpu_mem ofa_mem_desc;
};

struct gk20a;

s32 nvgpu_ofa_nvriscv_eb_boot(struct gk20a *g, u32 inst_id);
s32 nvgpu_ofa_sw_init(struct gk20a *g);
s32 nvgpu_ofa_sw_deinit(struct gk20a *g);
s32 nvgpu_ofa_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev);
void nvgpu_ofa_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx);

#endif
