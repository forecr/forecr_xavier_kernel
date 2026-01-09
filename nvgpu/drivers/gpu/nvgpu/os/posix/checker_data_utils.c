// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/checker_common_data_utils.h>
#include <nvgpu/posix/posix-fault-injection.h>
#include <nvgpu/types.h>

#include <nvgpu/gk20a.h>

struct nvgpu_posix_fault_inj *nvgpu_runlist_store_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c = nvgpu_posix_fault_injection_get_container();
	return &c->nvgpu_runlist_store_fi;
}

int nvgpu_runlist_store_shadow_rl_domain(struct gk20a *g)
{
	(void)g;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(
				nvgpu_runlist_store_fault_injection())) {
		nvgpu_err(NULL, "fault  :  nvgpu_runlist_store failed");
		return -EINVAL;
	}
#endif
#endif
	return 0;
}

int nvgpu_checker_nvs_domains_store_rlmem_count(u64 domain_id,
		u32 runlist_id, struct nvgpu_mem *mem, u32 count)
{
	(void)domain_id;
	(void)runlist_id;
	(void)mem;
	(void)count;

#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(
				nvgpu_runlist_store_fault_injection())) {
		nvgpu_err(NULL, "fault  :  nvgpu_runlist_store failed");
		return -EINVAL;
	}
#endif
#endif
	return 0;
}

void nvgpu_checker_nvs_domains_remove_domain(u64 domain_id)
{
	(void)domain_id;
}

void nvgpu_checker_tsg_deinit(u32 tsgid)
{
	(void)tsgid;
}
