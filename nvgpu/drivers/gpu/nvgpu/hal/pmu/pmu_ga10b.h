/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_GA10B_H
#define NVGPU_PMU_GA10B_H

#include <nvgpu/types.h>

#define DMA_OFFSET_START	0U
#define DMEM_DATA_0             0x0U
#define DMEM_DATA_1             0x1U
#define right_shift_8bits(v)    (v >> 8U)
#define left_shift_8bits(v)     (v << 8U)

struct gk20a;
struct nvgpu_pmu;

bool ga10b_is_pmu_supported(struct gk20a *g);
u32 ga10b_pmu_falcon2_base_addr(void);

#ifdef CONFIG_NVGPU_LS_PMU
int ga10b_pmu_ns_bootstrap(struct gk20a *g, struct nvgpu_pmu *pmu,
		u32 args_offset);
u32 ga10b_pmu_get_inst_block_config(struct gk20a *g);
#endif /* CONFIG_NVGPU_LS_PMU */

void ga10b_pmu_dump_elpg_stats(struct nvgpu_pmu *pmu);
void ga10b_pmu_init_perfmon_counter(struct gk20a *g);
u32 ga10b_pmu_read_idle_counter(struct gk20a *g, u32 counter_id);
void ga10b_pmu_reset_idle_counter(struct gk20a *g, u32 counter_id);
u32 ga10b_pmu_get_irqmask(struct gk20a *g);
bool ga10b_pmu_is_debug_mode_en(struct gk20a *g);
void ga10b_pmu_handle_swgen1_irq(struct gk20a *g, u32 intr);
#ifdef CONFIG_NVGPU_LS_PMU
bool ga10b_pmu_is_interrupted(struct nvgpu_pmu *pmu);
#endif
void ga10b_pmu_enable_irq(struct nvgpu_pmu *pmu, bool enable);
void ga10b_pmu_handle_ext_irq(struct gk20a *g, u32 intr0);
unsigned long long ga10b_get_amap_extmem2_start(void);
#endif /* NVGPU_PMU_GA10B_H */
