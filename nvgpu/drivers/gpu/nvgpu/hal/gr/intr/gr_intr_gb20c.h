/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_INTR_GB20C_H
#define NVGPU_GR_INTR_GB20C_H

u32 gb20c_gpc0_rop0_crop_hww_esr_offset(void);
u32 gb20c_gpc0_rop0_rrh_status_offset(void);
void gb20c_gr_intr_enable_gpc_crop_hww(struct gk20a *g);
int gb20c_gr_intr_handle_sw_method(struct gk20a *g, u32 addr,
			u32 class_num, u32 offset, u32 data);

#endif /* NVGPU_GR_INTR_GB20C_H */
