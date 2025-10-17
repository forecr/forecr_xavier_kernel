/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POWER_FEATURES_PG_H
#define NVGPU_POWER_FEATURES_PG_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_POWER_PG
#define nvgpu_pg_elpg_protected_call(g, func) \
	({ \
		int rt = 0; \
		rt = nvgpu_pg_elpg_disable(g);\
		if (rt != 0) {\
			(void)nvgpu_pg_elpg_enable(g);\
		}\
		if (rt == 0) { \
			rt = (func); \
			(void)nvgpu_pg_elpg_enable(g);\
		} \
		rt; \
	})

#define nvgpu_pg_elpg_ms_protected_call(g, func) \
	({ \
		int status = 0; \
		status = nvgpu_pg_elpg_ms_disable(g);\
		if (status != 0) {\
			(void)nvgpu_pg_elpg_ms_enable(g);\
		} \
		if (status == 0) { \
			status = (func); \
			(void)nvgpu_pg_elpg_ms_enable(g);\
		} \
		status; \
	})
#else
#define nvgpu_pg_elpg_protected_call(g, func) func
#define nvgpu_pg_elpg_ms_protected_call(g, func) func
#endif

/* Feature source ID to LPWR_MON Index ID mapping */
#define PMU_LPWR_MON_SOURCE_ID_ELCG_GR_INDEX_ID                         0
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVENC0_INDEX_ID                     1
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVENC1_INDEX_ID                     2
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVDEC0_INDEX_ID                     3
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVDEC1_INDEX_ID                     4
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVJPG0_INDEX_ID                     5
#define PMU_LPWR_MON_SOURCE_ID_ELCG_NVJPG1_INDEX_ID                     6
#define PMU_LPWR_MON_SOURCE_ID_ELCG_OFA0_INDEX_ID                       7
#define PMU_LPWR_MON_SOURCE_ID_ELCG_SEC_INDEX_ID                        8
#define PMU_LPWR_MON_SOURCE_ID_ELCG_CE0_INDEX_ID                        9
#define PMU_LPWR_MON_SOURCE_ID_ELCG_CE1_INDEX_ID                        10
#define PMU_LPWR_MON_SOURCE_ID_ELCG_CE2_INDEX_ID                        11
#define PMU_LPWR_MON_SOURCE_ID_ELCG_CE3_INDEX_ID                        12
#define PMU_LPWR_MON_SOURCE_ID_FGPG_GR_INDEX_ID                         13
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVENC0_INDEX_ID                     14
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVENC1_INDEX_ID                     15
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVDEC0_INDEX_ID                     16
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVDEC1_INDEX_ID                     17
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVJPG0_INDEX_ID                     18
#define PMU_LPWR_MON_SOURCE_ID_FGPG_NVJPG1_INDEX_ID                     19
#define PMU_LPWR_MON_SOURCE_ID_FGPG_OFA0_INDEX_ID                       20
#define PMU_LPWR_MON_SOURCE_ID_FLCG_GPCCLK_INDEX_ID                     21
#define PMU_LPWR_MON_SOURCE_ID_FLCG_NVDCLK_INDEX_ID                     22

int nvgpu_pg_elpg_disable(struct gk20a *g);
int nvgpu_pg_elpg_enable(struct gk20a *g);
int nvgpu_pg_elpg_ms_disable(struct gk20a *g);
int nvgpu_pg_elpg_ms_enable(struct gk20a *g);
bool nvgpu_pg_elpg_is_enabled(struct gk20a *g);
bool nvgpu_pg_elpg_ms_is_enabled(struct gk20a *g);
int nvgpu_pg_elpg_set_elpg_enabled(struct gk20a *g, bool enable);
int nvgpu_pg_fgpg_set_fgpg_enabled(struct gk20a *g, bool enable);
int nvgpu_pg_fgpg_set_engine_fgpg_enable(struct gk20a *g,
	unsigned long new_engine_bitmask);
bool nvgpu_pg_feature_status(struct gk20a *g, u32 lpwr_mon_index);
s32 nvgpu_pg_lpwr_lp_mon_start_stop(struct gk20a *g, u32 lpwr_mon_index,
				bool start);
u32 nvgpu_pg_get_lpwr_lp_residency(struct gk20a *g);
#endif /*NVGPU_POWER_FEATURES_PG_H*/
