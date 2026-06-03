/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_GA10B_H
#define GSP_GA10B_H

/**
 * @file
 * @page gsp_hal gsp_hal
 *
 * Overview
 * ========
 * This unit has API related to gsp ga10b hal.
 *
 * External APIs
 * ============================
 * - @ref ga10b_gsp_falcon2_base_addr
 * - @ref ga10b_gsp_falcon_base_addr
 * - @ref ga10b_gsp_engine_reset
 * - @ref ga10b_gsp_validate_mem_integrity
 * - @ref ga10b_gsp_is_debug_mode_en
 * - @ref ga10b_gsp_get_emem_boundaries
 * - @ref ga10b_gsp_queue_head_r
 * - @ref ga10b_gsp_queue_head__size_1_v
 * - @ref ga10b_gsp_queue_tail_r
 * - @ref ga10b_gsp_queue_tail__size_1_v
 * - @ref ga10b_gsp_isr
 * - @ref ga10b_gsp_enable_irq
 * - @ref ga10b_gsp_flcn_copy_to_emem
 * - @ref ga10b_gsp_flcn_copy_from_emem
 * - @ref ga10b_gsp_flcn_setup_boot_config
 * - @ref ga10b_gsp_queue_head
 * - @ref ga10b_gsp_queue_tail
 * - @ref ga10b_gsp_msgq_tail
 * - @ref ga10b_gsp_set_msg_intr
 *
 * Internal APIs
 * ============================
 * - @ref ga10b_gsp_handle_ecc
 * - @ref ga10b_gsp_get_irqmask
 * - @ref ga10b_gsp_is_interrupted
 * - @ref ga10b_gsp_handle_swgen1_irq
 * - @ref ga10b_gsp_handle_halt_irq
 * - @ref ga10b_gsp_clr_intr
 * - @ref ga10b_gsp_handle_interrupts
 * - @ref gsp_memcpy_params_check
 * - @ref ga10b_gsp_emem_transfer
 *
 */

u32 ga10b_gsp_falcon_base_addr(void);
u32 ga10b_gsp_falcon2_base_addr(void);
int ga10b_gsp_engine_reset(struct gk20a *g, u32 inst_id);
bool ga10b_gsp_validate_mem_integrity(struct gk20a *g);
void ga10b_gsp_enable_irq(struct gk20a *g, bool enable);
bool ga10b_gsp_is_interrupted(struct gk20a *g, u32 *intr);
void ga10b_gsp_isr(struct gk20a *g);
bool ga10b_gsp_is_debug_mode_en(struct gk20a *g);
/**
 * @brief Calculates the start and end boundaries of EMEM.
 *
 * This function determines the boundaries of the External Memory (EMEM) used by the
 * GSP Falcon by calculating the start and end addresses based on the DMEM tag width
 * and EMEM size configuration. The start of EMEM is calculated as 2 raised to the
 * power of (DMEM tag width + 8), and the end of EMEM is the start address plus the
 * size of EMEM.
 *
 * The steps performed by the function are as follows:
 * -# Check if the pointer to the start EMEM address is NULL. If it is, set the status
 *    to '-EINVAL' and go to 'exit'.
 * -# Read the DMEM tag width from the hardware configuration register using 'gk20a_readl()'
 *    and 'pgsp_falcon_hwcfg1_dmem_tag_width_v()'. Add 8 to the tag width to calculate
 *    the tag width shift value.
 * -# If the tag width shift value is greater than 31, log an error message using
 *    'nvgpu_err()' and set the status to '-EINVAL', then go to 'exit'.
 * -# Calculate the start EMEM address using 'BIT32()' macro with the tag width shift value.
 * -# If the pointer to the end EMEM address is NULL, go to 'exit'.
 * -# Calculate the end EMEM address by adding the start EMEM address to the product of
 *    the EMEM size configuration value and 256. The EMEM size configuration value is
 *    read from the hardware configuration register using 'gk20a_readl()' and
 *    'pgsp_hwcfg_emem_size_f()'. The product is calculated using 'nvgpu_safe_mult_u32()'
 *    and the addition using 'nvgpu_safe_add_u32()'.
 * -# Label 'exit' for exiting the function with the current status value.
 *
 * @param [in]  g           Pointer to the GPU driver struct.
 * @param [out] start_emem  Pointer to store the calculated start address of EMEM.
 * @param [out] end_emem    Pointer to store the calculated end address of EMEM.
 *
 * @return Zero if the boundaries are successfully calculated, otherwise '-EINVAL'.
 */
s32 ga10b_gsp_get_emem_boundaries(struct gk20a *g,
	u32 *start_emem, u32 *end_emem);
int ga10b_gsp_handle_ecc(struct gk20a *g, u32 err_module);

void ga10b_gsp_flcn_setup_boot_config(struct gk20a *g);

/* queue */
u32 ga10b_gsp_queue_head_r(u32 i);
u32 ga10b_gsp_queue_head__size_1_v(void);
u32 ga10b_gsp_queue_tail_r(u32 i);
u32 ga10b_gsp_queue_tail__size_1_v(void);
int ga10b_gsp_queue_head(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *head, bool set);
int ga10b_gsp_queue_tail(struct gk20a *g, u32 queue_id, u32 queue_index,
	u32 *tail, bool set);
void ga10b_gsp_msgq_tail(struct gk20a *g, struct nvgpu_gsp *gsp,
	u32 *tail, bool set);
int ga10b_gsp_flcn_copy_to_emem(struct gk20a *g,
	u32 dst, u8 *src, u32 size, u8 port);
int ga10b_gsp_flcn_copy_from_emem(struct gk20a *g,
	u32 src, u8 *dst, u32 size, u8 port);

/* interrupt */
void ga10b_gsp_set_msg_intr(struct gk20a *g);
#endif /* GSP_GA10B_H */
