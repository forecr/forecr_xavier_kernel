// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/tsg.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>

#include "gsp_vm_scheduler_vf.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vf_gsp_vm_sched_set_eng_timeslice(struct gk20a *g,
				struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params *params)
{
	struct tegra_vgpu_cmd_msg msg = {};
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_SET_ENG_TIMESLICE;
	msg.handle = vgpu_get_handle(g);
	msg.params.gsp_vm_sched_timeslice = *params;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
	}

	return err;
}

int vf_gsp_vm_sched_get_eng_timeslice(struct gk20a *g,
				struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params *params)
{
	struct tegra_vgpu_cmd_msg msg = {};
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_GET_ENG_TIMESLICE;
	msg.handle = vgpu_get_handle(g);
	msg.params.gsp_vm_sched_timeslice = *params;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err == 0 ? msg.ret : err;
	if (err != 0) {
		nvgpu_err(g, "cmd failed err=%d", err);
	} else {
		*params = msg.params.gsp_vm_sched_timeslice;
	}
	return err;
}

int vf_gsp_vm_sched_get_config(struct gk20a *g,
				struct tegra_vgpu_vf_gsp_vm_sched_config *vm_sched_config)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_vf_gsp_vm_sched_config_params *p =
					&msg.params.gsp_vm_sched_config_params;
	int ivc = -1;
	void *oob;
	size_t oob_size, data_size;
	int err;

	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&oob, &oob_size);
	if (ivc < 0) {
		return ivc;
	}

	msg.cmd = TEGRA_VGPU_CMD_VF_GSP_VM_SCHED_GET_ENG_TIMESLICE_CFG;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err ? err : msg.ret;
	if (err != 0) {
		nvgpu_err(g, "%s failed, err=%d", __func__, err);
		goto free_ivc;
	}

	vm_sched_config->min_timeslice = p->min_timeslice;
	vm_sched_config->max_timeslice = p->max_timeslice;
	vm_sched_config->count = p->timeslice_params_count;

	data_size = vm_sched_config->count *
			sizeof(struct tegra_vgpu_vf_gsp_vm_sched_timeslice_params);
	if (oob_size < data_size) {
		err = -ENOMEM;
		goto free_ivc;
	}

	memcpy(vm_sched_config->eng_params, oob, data_size);

free_ivc:
	vgpu_comm_free_ivc(g, ivc);
	return err;
}

bool vf_gsp_vm_sched_control_allowed(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	return priv->constants.can_control_gsp_vm_sched;
}
