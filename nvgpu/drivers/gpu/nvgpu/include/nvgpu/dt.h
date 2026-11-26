/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#include <nvgpu/types.h>

struct gk20a;

int nvgpu_dt_read_u32_index(struct gk20a *g, const char *name,
				u32 index, u32 *value);

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_dt_read_get_fault_injection(void);
#endif