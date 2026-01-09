/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef LTC_TU104_H
#define LTC_TU104_H

#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_DEBUGGER
/*
 * These macros are based on the TSTG registers present in the refmanual.
 * The first address of register of the TSTG block is:
 * NV_PLTS_TSTG_CFG_0 (0x90) and final register is: NV_PLTS_TSTG_REDUCE_REPLAY
 * (0x114).
 */
#define LTS_TSTG_BASE		(0x90U)
#define LTS_TSTG_EXTENT		(0x114U)
#endif

void ltc_tu104_init_fs_state(struct gk20a *g);
#ifdef CONFIG_NVGPU_DEBUGGER
u32 tu104_ltc_pri_is_lts_tstg_addr(struct gk20a *g, u32 addr);
int tu104_set_l2_sector_promotion(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 policy);
#endif

#endif /* LTC_TU104_H */
