// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/comptags.h>
#include "posix-comptags.h"
#include <nvgpu/posix/vm.h>
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
#include <nvgpu/posix/posix-fault-injection.h>
#endif


#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *gk20a_get_comptags_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
		nvgpu_posix_fault_injection_get_container();

	return &c->gk20a_get_comptags_fi;
}

struct nvgpu_posix_fault_inj *gk20a_get_comptags_2_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
		nvgpu_posix_fault_injection_get_container();

	return &c->gk20a_get_comptags_2_fi;
}

struct nvgpu_posix_fault_inj *gk20a_get_comptags_3_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
		nvgpu_posix_fault_injection_get_container();

	return &c->gk20a_get_comptags_3_fi;
}

struct nvgpu_posix_fault_inj *gk20a_comptags_start_clear_fault_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
		nvgpu_posix_fault_injection_get_container();

	return &c->gk20a_comptags_start_clear_fi;
}

#endif

void gk20a_get_comptags(struct nvgpu_os_buffer *buf,
			struct gk20a_comptags *comptags)
{
	(void)buf;
	(void)comptags;
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(gk20a_get_comptags_fault_injection())) {
		comptags->needs_clear = false;
		comptags->enabled = false;
		comptags->allocated = true;
		comptags->offset = 0;
		comptags->lines = 0;
	}
	if (nvgpu_posix_fault_injection_handle_call(gk20a_get_comptags_2_fault_injection())) {
		comptags->needs_clear = false;
		comptags->enabled = true;
		comptags->allocated = true;
		comptags->offset = 1;
		comptags->lines = 0;
	}
	if (nvgpu_posix_fault_injection_handle_call(gk20a_get_comptags_3_fault_injection())) {
		comptags->needs_clear = true;
		comptags->enabled = true;
		comptags->allocated = false;
		comptags->offset = 0;
		comptags->lines = 0;
	}
#endif
}

int gk20a_alloc_comptags(struct gk20a *g, struct nvgpu_os_buffer *buf,
			 struct gk20a_comptag_allocator *allocator)
{
	(void)g;
	(void)buf;
	(void)allocator;
	return -ENODEV;
}

void gk20a_alloc_or_get_comptags(struct gk20a *g,
				 struct nvgpu_os_buffer *buf,
				 struct gk20a_comptag_allocator *allocator,
				 struct gk20a_comptags *comptags)
{
	(void)g;
	(void)buf;
	(void)allocator;
	(void)comptags;
}

bool gk20a_comptags_start_clear(struct nvgpu_os_buffer *buf)
{
	(void)buf;
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(gk20a_comptags_start_clear_fault_injection())) {
		return true;
	}
#endif
	return false;
}

void gk20a_comptags_finish_clear(struct nvgpu_os_buffer *buf,
				 bool clear_successful)
{
	(void)buf;
	(void)clear_successful;
}
