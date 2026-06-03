/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_GV11B_H
#define PMU_GV11B_H

#include <nvgpu/nvgpu_err.h>
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_hw_err_inject_info;
struct nvgpu_hw_err_inject_info_desc;

bool gv11b_pmu_is_debug_mode_en(struct gk20a *g);
void gv11b_pmu_flcn_setup_boot_config(struct gk20a *g);
void gv11b_setup_apertures(struct gk20a *g);
bool gv11b_pmu_is_engine_in_reset(struct gk20a *g);
void gv11b_pmu_engine_reset(struct gk20a *g, bool do_reset);
u32 gv11b_pmu_falcon_base_addr(void);
bool gv11b_is_pmu_supported(struct gk20a *g);
void gv11b_pmu_handle_ext_irq(struct gk20a *g, u32 intr0);
u32 gv11b_pmu_get_ecc_address(struct gk20a *g);
u32 gv11b_pmu_get_ecc_status(struct gk20a *g);
void gv11b_pmu_set_ecc_status(struct gk20a *g, u32 val);

#ifdef CONFIG_NVGPU_LS_PMU
int gv11b_pmu_bootstrap(struct gk20a *g, struct nvgpu_pmu *pmu,
	u32 args_offset);
void gv11b_pmu_init_perfmon_counter(struct gk20a *g);
void gv11b_pmu_setup_elpg(struct gk20a *g);
void gv11b_secured_pmu_start(struct gk20a *g);
void gv11b_write_dmatrfbase(struct gk20a *g, u32 addr);
u32 gv11b_pmu_queue_head_r(u32 i);
u32 gv11b_pmu_queue_head__size_1_v(void);
u32 gv11b_pmu_queue_tail_r(u32 i);
u32 gv11b_pmu_queue_tail__size_1_v(void);
u32 gv11b_pmu_mutex__size_1_v(void);
#endif

void gv11b_clear_pmu_bar0_host_err_status(struct gk20a *g);
int gv11b_pmu_bar0_error_status(struct gk20a *g, u32 *bar0_status,
	u32 *etype);
bool gv11b_pmu_validate_mem_integrity(struct gk20a *g);

#ifdef CONFIG_NVGPU_INJECT_HWERR
struct nvgpu_hw_err_inject_info_desc * gv11b_pmu_intr_get_err_desc(struct gk20a *g);
void gv11b_pmu_inject_ecc_error(struct gk20a *g,
		struct nvgpu_hw_err_inject_info *err, u32 error_info);

#endif /* CONFIG_NVGPU_INJECT_HWERR */

int gv11b_pmu_ecc_init(struct gk20a *g);
void gv11b_pmu_ecc_free(struct gk20a *g);
u32 gv11b_pmu_get_irqdest(struct gk20a *g);
void gv11b_pmu_enable_irq(struct nvgpu_pmu *pmu, bool enable);

#endif /* PMU_GV11B_H */
