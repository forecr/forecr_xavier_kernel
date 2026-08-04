/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVJPG_H
#define NVGPU_NVJPG_H

#include <nvgpu/riscv.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/device.h>

/* NVJPG sw data */
struct nvgpu_nvjpg {

	/* NVJPG nvriscv descriptor info needed for BCR config */
	u32 nvriscv_firmware_desc[NVGPU_NVRISCV_INDEX_MAX];

	/* Falcon used to execute NVJPG ucode */
	struct nvgpu_falcon *nvjpg_riscv[NVJPG_INST_MAX];

	/** Memory to store ucode contents locally. */
	struct nvgpu_mem nvjpg_mem_desc;
};

struct gk20a;

s32 nvgpu_nvjpg_nvriscv_eb_boot(struct gk20a *g, u32 inst_id);
s32 nvgpu_nvjpg_sw_init(struct gk20a *g);
s32 nvgpu_nvjpg_sw_deinit(struct gk20a *g);
s32 nvgpu_nvjpg_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev);
void nvgpu_nvjpg_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx);

#endif
