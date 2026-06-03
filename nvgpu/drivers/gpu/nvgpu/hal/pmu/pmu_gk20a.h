/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_GK20A_H
#define PMU_GK20A_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_pmu;
struct pmu_mutexes;

#define PMU_MODE_MISMATCH_STATUS_MAILBOX_R  6U
#define PMU_MODE_MISMATCH_STATUS_VAL        0xDEADDEADU

void gk20a_pmu_isr(struct gk20a *g);
u32 gk20a_pmu_get_irqmask(struct gk20a *g);
void gk20a_pmu_set_mailbox1(struct gk20a *g, u32 val);
u32 gk20a_pmu_get_irqstat(struct gk20a *g);
void gk20a_pmu_set_irqsclr(struct gk20a *g, u32 intr);
void gk20a_pmu_set_irqsset(struct gk20a *g, u32 intr);
u32 gk20a_pmu_get_exterrstat(struct gk20a *g);
void gk20a_pmu_set_exterrstat(struct gk20a *g, u32 intr);
u32 gk20a_pmu_get_exterraddr(struct gk20a *g);
u32 gk20a_pmu_get_bar0_addr(struct gk20a *g);
u32 gk20a_pmu_get_bar0_data(struct gk20a *g);
u32 gk20a_pmu_get_bar0_timeout(struct gk20a *g);
u32 gk20a_pmu_get_bar0_ctl(struct gk20a *g);
u32 gk20a_pmu_get_bar0_error_status(struct gk20a *g);
void gk20a_pmu_set_bar0_error_status(struct gk20a *g, u32 val);
u32 gk20a_pmu_get_bar0_fecs_error(struct gk20a *g);
void gk20a_pmu_set_bar0_fecs_error(struct gk20a *g, u32 val);
u32 gk20a_pmu_get_mailbox(struct gk20a *g, u32 i);
u32 gk20a_pmu_get_pmu_debug(struct gk20a *g, u32 i);
u32 gk20a_pmu_get_mutex_reg(struct gk20a *g, u32 i);
void gk20a_pmu_set_mutex_reg(struct gk20a *g, u32 i, u32 data);
u32 gk20a_pmu_get_mutex_id(struct gk20a *g);
u32 gk20a_pmu_get_mutex_id_release(struct gk20a *g);
void gk20a_pmu_set_mutex_id_release(struct gk20a *g, u32 data);
u32 gk20a_pmu_get_pmu_msgq_head(struct gk20a *g);
void gk20a_pmu_set_pmu_msgq_head(struct gk20a *g, u32 data);
void gk20a_pmu_set_new_instblk(struct gk20a *g, u32 data);

#ifdef CONFIG_NVGPU_LS_PMU
void gk20a_pmu_dump_falcon_stats(struct nvgpu_pmu *pmu);
void gk20a_pmu_init_perfmon_counter(struct gk20a *g);
void gk20a_pmu_pg_idle_counter_config(struct gk20a *g, u32 pg_engine_id);
u32 gk20a_pmu_read_idle_counter(struct gk20a *g, u32 counter_id);
void gk20a_pmu_reset_idle_counter(struct gk20a *g, u32 counter_id);
u32 gk20a_pmu_read_idle_intr_status(struct gk20a *g);
void gk20a_pmu_clear_idle_intr_status(struct gk20a *g);
void gk20a_pmu_dump_elpg_stats(struct nvgpu_pmu *pmu);
u32 gk20a_pmu_mutex_owner(struct gk20a *g, struct pmu_mutexes *mutexes,
	u32 id);
int gk20a_pmu_mutex_acquire(struct gk20a *g, struct pmu_mutexes *mutexes,
	u32 id, u32 *token);
void gk20a_pmu_mutex_release(struct gk20a *g, struct pmu_mutexes *mutexes,
	u32 id, u32 *token);
int gk20a_pmu_queue_head(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *head, bool set);
int gk20a_pmu_queue_tail(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *tail, bool set);
void gk20a_pmu_msgq_tail(struct nvgpu_pmu *pmu, u32 *tail, bool set);
u32 gk20a_pmu_get_irqdest(struct gk20a *g);
void gk20a_pmu_enable_irq(struct nvgpu_pmu *pmu, bool enable);
void gk20a_pmu_handle_interrupts(struct gk20a *g, u32 intr);
bool gk20a_pmu_is_interrupted(struct nvgpu_pmu *pmu);
int gk20a_pmu_bar0_error_status(struct gk20a *g, u32 *bar0_status,
	u32 *etype);
int gk20a_pmu_ns_bootstrap(struct gk20a *g, struct nvgpu_pmu *pmu,
	u32 args_offset);
bool gk20a_pmu_is_engine_in_reset(struct gk20a *g);
void gk20a_pmu_engine_reset(struct gk20a *g, bool do_reset);
void gk20a_write_dmatrfbase(struct gk20a *g, u32 addr);
u32 gk20a_pmu_falcon_base_addr(void);
bool gk20a_is_pmu_supported(struct gk20a *g);
#endif

#endif /* PMU_GK20A_H */
