// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

/**
 * Here lie OS stubs that do not have an implementation yet nor has any plans
 * for an implementation.
 */

#include <nvgpu/ecc.h>
#include <nvgpu/debugger.h>
#include <nvgpu/cic_mon.h>
#include <nvgpu/types.h>

struct gk20a;

#ifdef CONFIG_NVGPU_DEBUGGER
void nvgpu_dbg_session_post_event(struct dbg_session_gk20a *dbg_s)
{
}
#endif

#ifdef CONFIG_NVGPU_SYSFS
int nvgpu_ecc_sysfs_init(struct gk20a *g)
{
	(void)g;
	return 0;
}

void nvgpu_ecc_sysfs_remove(struct gk20a *g)
{
	(void)g;
}
#endif

int nvgpu_cic_mon_report_err_safety_services(struct gk20a *g,
		u32 err_id)
{
	(void)g;
	(void)err_id;
	return 0;
}

#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
int nvgpu_cic_mon_reg_errinj_cb(struct gk20a *g)
{
	(void)g;
	return 0;
}

int nvgpu_cic_mon_dereg_errinj_cb(void)
{
	return 0;
}
#endif
