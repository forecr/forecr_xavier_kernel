/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef PMU_PG_PMU_PG_H
#define PMU_PG_PMU_PG_H

/* state transition :
 * OFF => [OFF_ON_PENDING optional] => ON_PENDING => ON => OFF
 * ON => OFF is always synchronized
 */
/* elpg is off */
#define PMU_ELPG_STAT_OFF		0U
/* elpg is on */
#define PMU_ELPG_STAT_ON		1U
/* elpg is off, ALLOW cmd has been sent, wait for ack */
#define PMU_ELPG_STAT_ON_PENDING	2U
/* elpg is on, DISALLOW cmd has been sent, wait for ack */
#define PMU_ELPG_STAT_OFF_PENDING	3U
/* elpg is off, caller has requested on, but ALLOW
 * cmd hasn't been sent due to ENABLE_ALLOW delay
 */
#define PMU_ELPG_STAT_OFF_ON_PENDING	4U

/* elpg_ms is off */
#define PMU_ELPG_MS_STAT_OFF		0U
/* elpg_ms is on */
#define PMU_ELPG_MS_STAT_ON		1U
/* elpg_ms is off, ALLOW cmd has been sent, wait for ack */
#define PMU_ELPG_MS_STAT_ON_PENDING	2U
/* elpg_ms is on, DISALLOW cmd has been sent, wait for ack */
#define PMU_ELPG_MS_STAT_OFF_PENDING	3U
/* elpg_ms is off, caller has requested on, but ALLOW
 * cmd hasn't been sent due to ENABLE_ALLOW delay
 */
#define PMU_ELPG_MS_STAT_OFF_ON_PENDING	4U

#define PMU_PGENG_GR_BUFFER_IDX_INIT	0U
#define PMU_PGENG_GR_BUFFER_IDX_ZBC	1U
#define PMU_PGENG_GR_BUFFER_IDX_FECS	2U

void pmu_handle_pg_elpg_msg(struct gk20a *g, struct pmu_msg *msg,
			void *param, u32 status);
void pmu_handle_pg_stat_msg(struct gk20a *g, struct pmu_msg *msg,
			void *param, u32 status);
void pmu_handle_pg_buf_config_msg(struct gk20a *g, struct pmu_msg *msg,
			void *param, u32 status);

#endif /* PMU_PG_PMU_PG_H */
