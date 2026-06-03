/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_GV11B_H
#define CBC_GV11B_H

#ifdef CONFIG_NVGPU_COMPRESSION
/* forward declarations. */
struct gk20a;
struct nvgpu_cbc;
/**
 * @brief This function is used to do HAL based initialization required
 * for comptag memory allocation.
 *
 * Call required HAL APIs for cbc_configure(#ga10b_fb_cbc_configure)
 *  and ctrl(#tu104_cbc_ctrl).
 * The cbc_op_invalidate command marks all CBC lines as invalid, this
 * causes all comptag lines to be fetched from the backing store.
 * Whereas, the cbc_op_clear goes a step further and clears the contents
 * of the backing store as well, because of this, cbc_op_clear should
 * only be called during the first power-on and not on suspend/resume
 * cycle, as the backing store might contain valid compression metadata
 * for already allocated surfaces and clearing it will corrupt those
 * surfaces.
 *
 * @param g         [in]    Pointer to the gk20a structure.
 * @param cbc       [in]    Pointer to the #nvgpu_cbc structure.
 * @param is_resume [in]    Flag to indicate the GPU drive state.
 *
 * @return None.
 */
void gv11b_cbc_init(struct gk20a *g, struct nvgpu_cbc *cbc, bool is_resume);

#endif
#endif
