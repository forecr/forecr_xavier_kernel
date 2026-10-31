/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __NVGPU_MODULE_T19X_H__
#define __NVGPU_MODULE_T19X_H__

struct gk20a;

void nvgpu_init_usermode_support(struct gk20a *g);
void nvgpu_remove_usermode_support(struct gk20a *g);
void nvgpu_lockout_usermode_registers(struct gk20a *g);
void nvgpu_restore_usermode_registers(struct gk20a *g);

#endif
