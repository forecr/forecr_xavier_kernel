// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/pmu/clk/clk.h>

#include "pmu_vgpu.h"

int vgpu_get_gpu_temperature(struct gk20a *g, u32 type, s32 *temp_raw)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_gpu_temperature_params *p = &msg.params.gpu_temperature;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (temp_raw == NULL)
		return -EINVAL;

	msg.cmd = TEGRA_VGPU_CMD_GET_GPU_TEMPERATURE;
	msg.handle = vgpu_get_handle(g);
	p->type = type;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(g, "%s failed - %d", __func__, err);
		return err;
	}

	*temp_raw = p->temp_raw;

	nvgpu_log(g, gpu_dbg_pmu,
		"Temperature type %u: raw value %d", type, *temp_raw);

	return 0;
}

int vgpu_get_gpu_voltage(struct gk20a *g, u32 type, u32 *voltage)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_gpu_voltage_params *p = &msg.params.gpu_voltage;
	int err = 0;

	nvgpu_log_fn(g, " ");

	if (voltage == NULL)
		return -EINVAL;

	msg.cmd = TEGRA_VGPU_CMD_GET_GPU_VOLTAGE;
	msg.handle = vgpu_get_handle(g);
	p->type = type;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(g, "%s failed - %d", __func__, err);
		return err;
	}

	*voltage = p->voltage;

	nvgpu_log(g, gpu_dbg_pmu,
		"Voltage type %u: raw value %u", type, *voltage);

	return 0;
}

int vgpu_get_gpu_volt_rail_limits(struct gk20a *g, u8 type, u32 *v_min, u32 *v_max)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct tegra_vgpu_gpu_volt_rail_limits_params *p = &msg.params.gpu_volt_rail_limits;
	int err = 0;

	msg.cmd = TEGRA_VGPU_CMD_GET_GPU_VOLT_RAIL_LIMITS;
	msg.handle = vgpu_get_handle(g);
	p->type = type;

	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(g, "%s failed - %d", __func__, err);
		return err;
	}

	*v_min = p->v_min;
	*v_max = p->v_max;

	nvgpu_log(g, gpu_dbg_pmu,
		"Voltage rail limits: vmin=%u uV, vmax=%u uV", *v_min, *v_max);

	return 0;
}


int vgpu_get_vf_points_status(struct gk20a *g, struct nvgpu_clk_vf_points_status *vf_points_status,
			 u32 *num_vf_points, u8 type)
{
	struct tegra_vgpu_cmd_msg msg = {};
	struct nvgpu_clk_vf_points_status *vf_points_status_priv;
	struct tegra_vgpu_get_vf_points_status_params *p = &msg.params.get_vf_points_status;
	int err = 0;
	size_t oob_size;
	int ivc = -1;

	if (vf_points_status == NULL || num_vf_points == NULL)
		return -EINVAL;

	msg.cmd = TEGRA_VGPU_CMD_GET_VF_POINTS_STATUS;
	msg.handle = vgpu_get_handle(g);
	p->type = type;

	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&vf_points_status_priv, &oob_size);
	if (ivc < 0) {
		err = ivc;
		goto done;
	}

	if (oob_size < sizeof(struct nvgpu_clk_vf_points_status)) {
		nvgpu_err(g, "OOB size is too small");
		err = -ENOMEM;
		goto done;
	}

	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err ? err : msg.ret;
	if (err) {
		nvgpu_err(g, "%s failed - %d", __func__, err);
		goto done;
	}

	*num_vf_points = p->num_vf_points;
	for (int i = 0; i < *num_vf_points; i++)
		vf_points_status->vf_points[i] = vf_points_status_priv->vf_points[i];

done:
	if (ivc >= 0)
		vgpu_comm_free_ivc(g, ivc);
	return err;
}

