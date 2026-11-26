/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FALCON_GA10B_H
#define NVGPU_FALCON_GA10B_H

#include <nvgpu/falcon.h>

#define FALCON_DMEM_BLKSIZE2	8U
u32 ga10b_falcon_dmemc_blk_mask(void);
u32 ga10b_falcon_imemc_blk_field(u32 blk);
bool ga10b_falcon_is_cpu_halted(struct nvgpu_falcon *flcn);
void ga10b_falcon_set_bcr(struct nvgpu_falcon *flcn);
void ga10b_falcon_bootstrap(struct nvgpu_falcon *flcn, u64 boot_vector);
void ga10b_falcon_dump_brom_stats(struct nvgpu_falcon *flcn);
u32  ga10b_falcon_get_brom_retcode(struct nvgpu_falcon *flcn);
bool ga10b_falcon_is_priv_lockdown(struct nvgpu_falcon *flcn);
bool ga10b_falcon_check_brom_passed(u32 retcode);
bool ga10b_falcon_check_brom_failed(u32 retcode);
void ga10b_falcon_brom_config(struct nvgpu_falcon *flcn, u64 fmc_code_addr,
		u64 fmc_data_addr, u64 manifest_addr);
#ifdef CONFIG_NVGPU_FALCON_DEBUG
void ga10b_falcon_dump_stats(struct nvgpu_falcon *flcn);
#endif /* CONFIG_NVGPU_FALCON_DEBUG */
bool ga10b_is_falcon_scrubbing_done(struct nvgpu_falcon *flcn);
bool ga10b_is_falcon_idle(struct nvgpu_falcon *flcn);
#endif /* NVGPU_FALCON_GA10B_H */
