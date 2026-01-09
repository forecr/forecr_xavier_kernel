/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GR_FALCON_GM20B_H
#define NVGPU_GR_FALCON_GM20B_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_fecs_method_op;
struct nvgpu_fecs_host_intr_status;
struct nvgpu_gr_falcon_query_sizes;

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gm20b_gr_falcon_get_fw_name(struct gk20a *g, const char **ucode_name, u32 falcon_id);
void gm20b_gr_falcon_fecs_ctxsw_clear_mailbox(struct gk20a *g,
					u32 reg_index, u32 clear_val);
#endif
u32 gm20b_gr_falcon_read_mailbox_fecs_ctxsw(struct gk20a *g, u32 reg_index);
void gm20b_gr_falcon_fecs_host_clear_intr(struct gk20a *g, u32 fecs_intr);
u32 gm20b_gr_falcon_fecs_host_intr_status(struct gk20a *g,
			struct nvgpu_fecs_host_intr_status *fecs_host_intr);
u32 gm20b_gr_falcon_fecs_base_addr(void);
u32 gm20b_gr_falcon_gpccs_base_addr(void);
#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void gm20b_gr_falcon_dump_stats(struct gk20a *g);
#endif
u32 gm20b_gr_falcon_get_fecs_ctx_state_store_major_rev_id(struct gk20a *g);
u32 gm20b_gr_falcon_get_fecs_ctxsw_mailbox_size(void);
void gm20b_gr_falcon_start_gpccs(struct gk20a *g);
void gm20b_gr_falcon_start_fecs(struct gk20a *g);
u32 gm20b_gr_falcon_get_gpccs_start_reg_offset(void);
void gm20b_gr_falcon_bind_instblk(struct gk20a *g,
				struct nvgpu_mem *mem, u64 inst_ptr);
int gm20b_gr_falcon_wait_mem_scrubbing(struct gk20a *g);
int gm20b_gr_falcon_wait_ctxsw_ready(struct gk20a *g);
int gm20b_gr_falcon_submit_fecs_method_op(struct gk20a *g,
	struct nvgpu_fecs_method_op op, u32 flags, u32 fecs_method);
int gm20b_gr_falcon_ctrl_ctxsw(struct gk20a *g, u32 fecs_method,
						u32 data, u32 *ret_val);
int gm20b_gr_falcon_ctrl_ctxsw_internal(struct gk20a *g, u32 fecs_method,
						u32 data, u32 *ret_val);
void gm20b_gr_falcon_set_current_ctx_invalid(struct gk20a *g);
u32 gm20b_gr_falcon_get_current_ctx(struct gk20a *g);
u32 gm20b_gr_falcon_get_ctx_ptr(u32 ctx);
u32 gm20b_gr_falcon_get_fecs_current_ctx_data(struct gk20a *g,
						struct nvgpu_mem *inst_block);
int gm20b_gr_falcon_init_ctx_state(struct gk20a *g,
		struct nvgpu_gr_falcon_query_sizes *sizes);
#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
int gm20b_gr_falcon_get_zcull_image_size(struct gk20a *g,
                struct nvgpu_gr_falcon_query_sizes *sizes);
#endif
u32 gm20b_gr_falcon_read_status0_fecs_ctxsw(struct gk20a *g);
u32 gm20b_gr_falcon_read_status1_fecs_ctxsw(struct gk20a *g);
#ifdef CONFIG_NVGPU_GRAPHICS
#ifdef CONFIG_NVGPU_CILP
int gm20b_gr_falcon_submit_fecs_sideband_method_op(struct gk20a *g,
				struct nvgpu_fecs_method_op op);
#endif
#endif
#ifdef CONFIG_NVGPU_GR_FALCON_NON_SECURE_BOOT
void gm20b_gr_falcon_load_ctxsw_ucode_header(struct gk20a *g,
	u32 reg_offset, u32 boot_signature, u32 addr_code32,
	u32 addr_data32, u32 code_size, u32 data_size);
void gm20b_gr_falcon_load_ctxsw_ucode_boot(struct gk20a *g,
	u32 reg_offset, u32 boot_entry, u32 addr_load32, u32 blocks,
	u32 dst);
void gm20b_gr_falcon_load_gpccs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gm20b_gr_falcon_gpccs_dmemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void gm20b_gr_falcon_load_fecs_dmem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gm20b_gr_falcon_fecs_dmemc_write(struct gk20a *g, u32 reg_offset, u32 port,
	u32 offs, u32 blk, u32 ainc);
void gm20b_gr_falcon_load_gpccs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gm20b_gr_falcon_gpccs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void gm20b_gr_falcon_load_fecs_imem(struct gk20a *g,
			const u32 *ucode_u32_data, u32 ucode_u32_size);
void gm20b_gr_falcon_fecs_imemc_write(struct gk20a *g, u32 port, u32 offs,
	u32 blk, u32 ainc);
void gm20b_gr_falcon_start_ucode(struct gk20a *g);
void gm20b_gr_falcon_fecs_host_int_enable(struct gk20a *g);
#endif
#ifdef CONFIG_NVGPU_SIM
void gm20b_gr_falcon_configure_fmodel(struct gk20a *g);
#endif
#endif /* NVGPU_GR_FALCON_GM20B_H */
