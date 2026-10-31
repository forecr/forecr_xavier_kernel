/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_TEST
#define NVGPU_GSP_TEST

#include <nvgpu/types.h>
struct gk20a;

u32 nvgpu_gsp_get_current_iteration(struct gk20a *g);
u32 nvgpu_gsp_get_current_test(struct gk20a *g);
bool nvgpu_gsp_get_test_fail_status(struct gk20a *g);
void nvgpu_gsp_set_test_fail_status(struct gk20a *g, bool val);
bool nvgpu_gsp_get_stress_test_start(struct gk20a *g);
int nvgpu_gsp_set_stress_test_start(struct gk20a *g, bool flag);
bool nvgpu_gsp_get_stress_test_load(struct gk20a *g);
int nvgpu_gsp_set_stress_test_load(struct gk20a *g, bool flag);
int nvgpu_gsp_stress_test_bootstrap(struct gk20a *g, bool start);
int nvgpu_gsp_stress_test_halt(struct gk20a *g, bool restart);
bool nvgpu_gsp_is_stress_test(struct gk20a *g);
int nvgpu_gsp_stress_test_sw_init(struct gk20a *g);
void nvgpu_gsp_test_sw_deinit(struct gk20a *g);
void nvgpu_gsp_write_test_sysmem_addr(struct gk20a *g);
#endif /* NVGPU_GSP_TEST */
