/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_GB10B_H
#define NVGPU_GR_FALCON_GB10B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_fecs_method_op;
struct nvgpu_fecs_host_intr_status;
struct nvgpu_gr_falcon_query_sizes;

void gb10b_gr_falcon_dump_stats(struct gk20a *g);
void gb10b_gr_falcon_start_gpccs(struct gk20a *g);
void gb10b_gr_falcon_start_fecs(struct gk20a *g);
int gb10b_gr_falcon_wait_mem_scrubbing(struct gk20a *g);
u32 gb10b_gr_falcon_get_gpccs_start_reg_offset(void);
u32 gb10b_gr_falcon_fecs_base_addr(void);
u32 gb10b_gr_falcon_fecs2_base_addr(void);
u32 gb10b_gr_falcon_gpccs_base_addr(void);
u32 gb10b_gr_falcon_gpccs2_base_addr(void);
void gb10b_gr_falcon_reset_ctxsw_mailbox_scratch(struct gk20a *g);
u32 gb10b_gr_falcon_get_compute_preemption_mode(struct gk20a *g);

#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void gb10b_gr_falcon_gpccs_dmemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void gb10b_gr_falcon_fecs_dmemc_write(struct gk20a *g, u32 reg_offset,
		u32 port, u32 offs, u32 blk, u32 ainc);
void gb10b_gr_falcon_gpccs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void gb10b_gr_falcon_load_gpccs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gb10b_gr_falcon_load_gpccs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gb10b_gr_falcon_load_ctxsw_ucode_header(struct gk20a *g,
	u32 reg_offset, u32 boot_signature, u32 addr_code32,
	u32 addr_data32, u32 code_size, u32 data_size);
void gb10b_gr_falcon_load_ctxsw_ucode_boot(struct gk20a *g,
	u32 reg_offset, u32 boot_entry, u32 addr_load32, u32 blocks,
	u32 dst);
void gb10b_gr_falcon_load_fecs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gb10b_gr_falcon_load_fecs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gb10b_gr_falcon_start_ucode(struct gk20a *g);
void gb10b_gr_falcon_fecs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
#endif
int gb10b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
	u32 data, u32 *ret_val);
#ifdef CONFIG_NVGPU_SIM
void gb10b_gr_falcon_configure_fmodel(struct gk20a *g);
#endif
u32 gb10b_gr_falcon_get_fecs_current_ctx_hi_data(struct gk20a *g,
		struct nvgpu_mem *inst_block);
void gb10b_gr_falcon_set_null_fecs_method_data(struct gk20a *g,
				struct nvgpu_fecs_method_op *op,
				u32 fecs_method);
u32 gb10b_gr_falcon_fecs_host_intr_status(struct gk20a *g,
			struct nvgpu_fecs_host_intr_status *fecs_host_intr);
int gb10b_gr_intr_handle_fecs_error(struct gk20a *g,
		struct nvgpu_channel *ch_ptr,
		struct nvgpu_gr_isr_data *isr_data);
void gb10b_gr_falcon_fecs_host_int_enable(struct gk20a *g);
#endif /* NVGPU_GR_FALCON_GB10B_H */
