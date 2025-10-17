/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMU_GB10B_H
#define NVGPU_PMU_GB10B_H

#include <nvgpu/nvgpu_err.h>

struct gk20a;
struct nvgpu_pmu;
struct nvgpu_hw_err_inject_info;
struct nvgpu_hw_err_inject_info_desc;

void gb10b_pmu_dump_elpg_stats(struct nvgpu_pmu *pmu);
u32 gb10b_pmu_falcon_base_addr(void);
u32 gb10b_pmu_falcon2_base_addr(void);
u32 gb10b_pmu_read_idle_counter(struct gk20a *g, u32 counter_id);
void gb10b_pmu_reset_idle_counter(struct gk20a *g, u32 counter_id);
u32 gb10b_pmu_read_idle_intr_status(struct gk20a *g);
void gb10b_pmu_clear_idle_intr_status(struct gk20a *g);
u32 gb10b_pmu_get_mutex_reg(struct gk20a *g, u32 i);
void gb10b_pmu_set_mutex_reg(struct gk20a *g, u32 i, u32 data);
u32 gb10b_pmu_get_mutex_id(struct gk20a *g);
u32 gb10b_pmu_get_mutex_id_release(struct gk20a *g);
void gb10b_pmu_set_mutex_id_release(struct gk20a *g, u32 data);

#ifdef CONFIG_NVGPU_LS_PMU
void gb10b_write_dmatrfbase(struct gk20a *g, u32 addr);
#endif
void gb10b_pmu_engine_reset(struct gk20a *g, bool do_reset);
bool gb10b_pmu_is_engine_in_reset(struct gk20a *g);
u32 gb10b_pmu_get_irqmask(struct gk20a *g);
u32 gb10b_pmu_get_irqstat(struct gk20a *g);
void gb10b_pmu_set_mailbox1(struct gk20a *g, u32 val);
u32 gb10b_pmu_get_ecc_address(struct gk20a *g);
u32 gb10b_pmu_get_ecc_status(struct gk20a *g);
void gb10b_pmu_set_ecc_status(struct gk20a *g, u32 val);
void gb10b_clear_pmu_bar0_host_err_status(struct gk20a *g);
void gb10b_pmu_set_irqsclr(struct gk20a *g, u32 intr);
void gb10b_pmu_set_irqsset(struct gk20a *g, u32 intr);
u32 gb10b_pmu_get_exterrstat(struct gk20a *g);
void gb10b_pmu_set_exterrstat(struct gk20a *g, u32 intr);
u32 gb10b_pmu_get_exterraddr(struct gk20a *g);
u32 gb10b_pmu_get_bar0_addr(struct gk20a *g);
u32 gb10b_pmu_get_bar0_data(struct gk20a *g);
u32 gb10b_pmu_get_bar0_timeout(struct gk20a *g);
u32 gb10b_pmu_get_bar0_ctl(struct gk20a *g);
u32 gb10b_pmu_get_bar0_error_status(struct gk20a *g);
void gb10b_pmu_set_bar0_error_status(struct gk20a *g, u32 val);
u32 gb10b_pmu_get_bar0_fecs_error(struct gk20a *g);
void gb10b_pmu_set_bar0_fecs_error(struct gk20a *g, u32 val);
u32 gb10b_pmu_get_mailbox(struct gk20a *g, u32 i);
u32 gb10b_pmu_get_pmu_debug(struct gk20a *g, u32 i);
void gb10b_pmu_init_perfmon_counter(struct gk20a *g);

#ifdef CONFIG_NVGPU_INJECT_HWERR
struct nvgpu_hw_err_inject_info_desc * gb10b_pmu_intr_get_err_desc(struct gk20a *g);
void gb10b_pmu_inject_ecc_error(struct gk20a *g,
	struct nvgpu_hw_err_inject_info *err, u32 error_info);

#endif /* CONFIG_NVGPU_INJECT_HWERR */
u32 gb10b_pmu_get_pmu_msgq_head(struct gk20a *g);
void gb10b_pmu_set_pmu_msgq_head(struct gk20a *g, u32 data);
u32 gb10b_pmu_queue_head_r(u32 i);
u32 gb10b_pmu_queue_head__size_1_v(void);
u32 gb10b_pmu_queue_tail_r(u32 i);
u32 gb10b_pmu_queue_tail__size_1_v(void);
void gb10b_pmu_pg_idle_counter_config(struct gk20a *g, u32 pg_engine_id);
void gb10b_pmu_set_new_instblk(struct gk20a *g, u32 data);
u32 gb10b_pmu_msgq_tail_r(void);
bool gb10b_is_cg_supported_by_pmu(struct gk20a *g);
int gb10b_pmu_elcg_init_idle_filters(struct gk20a *g);
void gb10b_pmu_init_elcg_mode(struct gk20a *g, u32 mode, u32 engine);
void gb10b_pmu_init_blcg_mode(struct gk20a *g, u32 mode, u32 engine);
void gb10b_pmu_get_intr_ctrl_msg(struct gk20a *g, bool enable,
		u32 *intr_ctrl_msg);
void gb10b_pmu_enable_irq(struct nvgpu_pmu *pmu, bool enable);
bool gb10b_pmu_is_debug_mode_en(struct gk20a *g);
void gb10b_pmu_ns_setup_apertures(struct gk20a *g);
unsigned long long gb10b_get_amap_extmem2_start(void);
#endif /* NVGPU_PMU_GB10B_H */
