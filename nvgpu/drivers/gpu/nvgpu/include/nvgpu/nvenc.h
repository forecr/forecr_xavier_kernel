/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVENC_H
#define NVGPU_NVENC_H

#include <nvgpu/riscv.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/device.h>

/* NVENC sw data */
struct nvgpu_nvenc {

	/* NVENC ucode header info needed for dma'ing the ucode */
	u32 ucode_header[MULTIMEDIA_UCODE_HEADER_SIZE];

	/* NVENC nvriscv descriptor info needed for BCR config */
	u32 nvriscv_firmware_desc[NVGPU_NVRISCV_INDEX_MAX];

	/* Falcon object used to execute NVENC falcon ucode */
	struct nvgpu_falcon *nvenc_flcn;

	/* Falcon object used to execute NVENC nvriscv ucode */
	struct nvgpu_falcon *nvenc_riscv[NVENC_INST_MAX];

	/** Memory to store ucode contents locally. */
	struct nvgpu_mem nvenc_mem_desc;

};

struct gk20a;
struct nvgpu_debug_context;

s32 nvgpu_nvenc_falcon_boot(struct gk20a *g, u32 inst_id);
s32 nvgpu_nvenc_sw_init(struct gk20a *g);
s32 nvgpu_nvenc_sw_deinit(struct gk20a *g);
s32 nvgpu_nvenc_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev);
s32 nvgpu_nvenc_nvriscv_eb_boot(struct gk20a *g, u32 inst_id);
void nvgpu_nvenc_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx);

#endif
