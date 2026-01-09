/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_GA10B_H
#define NVGPU_GR_FALCON_GA10B_H

#include <nvgpu/types.h>

struct gk20a;

void ga10b_gr_falcon_get_fw_name(struct gk20a *g, const char **ucode_name, u32 falcon_id);
u32 ga10b_gr_falcon_get_fecs_ctxsw_mailbox_size(void);
void ga10b_gr_falcon_fecs_ctxsw_clear_mailbox(struct gk20a *g,
						u32 reg_index, u32 clear_val);
void ga10b_gr_falcon_dump_stats(struct gk20a *g);
#ifndef CONFIG_NVGPU_HAL_NON_FUSA
void ga10b_gr_falcon_set_null_fecs_method_data(struct gk20a *g,
				struct nvgpu_fecs_method_op *op,
		                u32 fecs_method);
#endif
#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void ga10b_gr_falcon_gpccs_dmemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void ga10b_gr_falcon_fecs_dmemc_write(struct gk20a *g, u32 reg_offset, u32 port,
	u32 offs, u32 blk, u32 ainc);
void ga10b_gr_falcon_gpccs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void ga10b_gr_falcon_load_fecs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void ga10b_gr_falcon_fecs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
#endif
#endif /* NVGPU_GR_FALCON_GA10B_H */
