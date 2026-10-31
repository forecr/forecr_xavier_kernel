/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2015-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _NVHOST_GP10B_CDE
#define _NVHOST_GP10B_CDE

#include "os_linux.h"

void gp10b_cde_get_program_numbers(struct gk20a *g,
				   u32 block_height_log2,
				   u32 shader_parameter,
				   int *hprog_out, int *vprog_out);
bool gp10b_need_scatter_buffer(struct gk20a *g);
int gp10b_populate_scatter_buffer(struct gk20a *g,
				  struct sg_table *sgt,
				  size_t surface_size,
				  void *scatter_buffer_ptr,
				  size_t scatter_buffer_size);
#endif
