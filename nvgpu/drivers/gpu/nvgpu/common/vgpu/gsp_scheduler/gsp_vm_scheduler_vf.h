/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_VM_SCHEDULER_VF
#define GSP_VM_SCHEDULER_VF

struct gk20a;
struct tegra_vgpu_vf_gsp_vm_sched_config;
struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params;

bool vf_gsp_vm_sched_control_allowed(struct gk20a *g);
int vf_gsp_vm_sched_get_config(struct gk20a *g,
		struct tegra_vgpu_vf_gsp_vm_sched_config *vm_sched_config);
int vf_gsp_vm_sched_get_eng_timeslice(struct gk20a *g,
		struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params *params);
int vf_gsp_vm_sched_set_eng_timeslice(struct gk20a *g,
		struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params *params);
#endif
