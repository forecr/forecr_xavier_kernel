/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_CE_GV11B_H
#define NVGPU_CE_GV11B_H

struct gk20a;
struct nvgpu_device;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gv11b_ce_mthd_buffer_fault_in_bar2_fault(struct gk20a *g);
#endif
u32 gv11b_ce_get_num_pce(struct gk20a *g);
void gv11b_ce_stall_isr(struct gk20a *g, u32 inst_id, u32 pri_base,
				bool *needs_rc, bool *needs_quiesce);
void gv11b_ce_init_prod_values(struct gk20a *g);
void gv11b_ce_halt_engine(struct gk20a *g, const struct nvgpu_device *dev);
u64 gv11b_ce_get_inst_ptr_from_lce(struct gk20a *g, u32 inst_id);
#endif /* NVGPU_CE_GV11B_H */
