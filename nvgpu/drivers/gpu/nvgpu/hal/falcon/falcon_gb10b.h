/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FALCON_GB10B_H
#define NVGPU_FALCON_GB10B_H

#include <nvgpu/falcon.h>

#define FALCON_DMEM_BLKSIZE2	8U
#define GB10B_FECS_RISCV_BOOTPLUGIN                "g_rv_boot_plugin_fecs.text.bin"
#define GB10B_GPCCS_RISCV_BOOTPLUGIN               "g_rv_boot_plugin_gpccs.text.bin"

#ifdef CONFIG_NVGPU_FALCON_DEBUG
void gb10b_falcon_dump_info(struct nvgpu_falcon *flcn);
#endif
void gb10b_falcon_dump_debug_regs(struct nvgpu_falcon *flcn,
				struct nvgpu_debug_context *o);
u32 gb10b_falcon_debuginfo_offset(void);
void gb10b_falcon_write_dmatrfbase(struct nvgpu_falcon *flcn, u64 dma_base);
u32 gb10b_falcon_get_mem_size(struct nvgpu_falcon *flcn,
		enum falcon_mem_type mem_type);
bool gb10b_falcon_check_brom_init_done(u32 retcode);
void gb10b_get_bootplugin_fw_name(struct gk20a *g, const char **name, u32 falcon_id);

#endif /* NVGPU_FALCON_GB10B_H */
