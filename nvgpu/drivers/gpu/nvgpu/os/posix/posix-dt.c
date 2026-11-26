// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/bug.h>
#include <nvgpu/dt.h>
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
#include <nvgpu/posix/posix-fault-injection.h>
#endif

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_dt_read_get_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
		nvgpu_posix_fault_injection_get_container();
	return &c->nvgpu_dt_read_fi;
}
#endif

int nvgpu_dt_read_u32_index(struct gk20a *g, const char *name,
				u32 index, u32 *value)
{
	(void)g;
	(void)name;
	(void)index;
	(void)value;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(
                nvgpu_dt_read_get_fault_injection())) {
        return -1;
}
#endif
#else
	BUG();
#endif
	return 0;
}
