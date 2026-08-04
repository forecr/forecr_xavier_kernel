/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FIFO_INTR_GB20C_H
#define NVGPU_FIFO_INTR_GB20C_H

#include <nvgpu/types.h>

struct gk20a;

u32 gb20c_runlist_intr_ctrl(u32 intr_tree);
u32 gb20c_runlist_intr_0(void);
u32 gb20c_runlist_intr_0_mask(void);
u32 gb20c_runlist_intr_0_recover_mask(void);
u32 gb20c_runlist_intr_0_recover_unmask(void);
u32 gb20c_runlist_intr_0_ctxsw_timeout_mask(void);
u32 gb20c_runlist_intr_0_en_clear_tree(u32 intr_tree);
u32 gb20c_runlist_intr_0_en_set_tree(u32 intr_tree);
u32 gb20c_runlist_intr_bad_tsg(void);
u32 gb20c_runlist_intr_retrigger_reg_off(u32 intr_tree);

#endif /* NVGPU_FIFO_INTR_GB20C_H */
