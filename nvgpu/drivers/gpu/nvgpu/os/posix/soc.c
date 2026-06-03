// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/timers.h>
#include <nvgpu/soc.h>
#include <os/posix/os_posix.h>

bool nvgpu_platform_is_silicon(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_silicon;
}

bool nvgpu_platform_is_simulation(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_simulation;
}

bool nvgpu_platform_is_fpga(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_fpga;
}

bool nvgpu_is_hypervisor_mode(struct gk20a *g)
{
	(void)g;
	if (g->ops.cbc.use_contig_pool != NULL) {
		return true;
	}
	return false;
}

bool nvgpu_is_soc_t194_a01(struct gk20a *g)
{
	struct nvgpu_os_posix *p = nvgpu_os_posix_from_gk20a(g);

	return p->is_soc_t194_a01;
}

int nvgpu_init_soc_vars(struct gk20a *g)
{
	(void)g;
	return 0;
}

u64 nvgpu_get_pa_from_ipa(struct gk20a *g, u64 ipa)
{
	(void)g;
	return ipa;
}
