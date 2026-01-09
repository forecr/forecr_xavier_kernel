/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVDEC_H
#define NVGPU_NVDEC_H

#include <nvgpu/riscv.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/device.h>

/* NVDEC sw data */
struct nvgpu_nvdec {

	/* NVDEC nvriscv descriptor info needed for BCR config */
	u32 nvriscv_firmware_desc[NVGPU_NVRISCV_INDEX_MAX];

	/* Falcon object used to execute NVDEC falcon ucode */
	struct nvgpu_falcon *nvdec_flcn;

	/* Falcon object used to execute NVDEC nvriscv ucode */
	struct nvgpu_falcon *nvdec_riscv[NVDEC_INST_MAX];

	/** Memory to store ucode contents locally. */
	struct nvgpu_mem nvdec_mem_desc;
};

struct gk20a;

s32 nvgpu_nvdec_nvriscv_br_boot(struct gk20a *g, u32 inst_id);
s32 nvgpu_nvdec_sw_init(struct gk20a *g);
s32 nvgpu_nvdec_sw_deinit(struct gk20a *g);
s32 nvgpu_nvdec_reset_and_boot(struct gk20a *g, const struct nvgpu_device *dev);
void nvgpu_nvdec_dump_status(struct gk20a *g, u32 inst_id,
			struct nvgpu_debug_context *dbg_ctx);

#endif
