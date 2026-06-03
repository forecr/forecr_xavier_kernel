/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_TEMPERATURE_VGPU_H
#define NVGPU_TEMPERATURE_VGPU_H

/**
 * @file pmu_vgpu.h
 *
 * VGPU PMU interface
 */

struct gk20a;
struct nvgpu_clk_vf_points_status;
/**
 * Gets GPU temperature through virtualization interface.
 * The raw temperature value is returned in the same format used by PMU firmware
 * (temp_celsius = raw_value / 256).
 *
 * @param g GPU context
 * @param type Temperature type (TEGRA_VGPU_GPU_TEMPERATURE_TYPE_*)
 * @param temp_raw Pointer to store raw temperature result (signed 32-bit)
 * @return 0 on success, negative error code on failure
 */
int vgpu_get_gpu_temperature(struct gk20a *g, u32 type, s32 *temp_raw);

/**
 * @brief Get GPU voltage via VGPU IVC communication
 *
 * Communicates with the GPU server to retrieve voltage based on the
 * requested type (GPM, GPC). Voltage is returned in microvolts.
 *
 * @param g GPU context
 * @param type Voltage type (TEGRA_VGPU_GPU_VOLTAGE_TYPE_*)
 * @param voltage Pointer to store voltage result
 * @return 0 on success, negative error code on failure
 */
int vgpu_get_gpu_voltage(struct gk20a *g, u32 type, u32 *voltage);

/**
 * @brief Get GPU voltage rail limits via VGPU IVC communication
 *
 * @param g GPU context
 * @param type Voltage type (TEGRA_VGPU_GPU_VOLTAGE_TYPE_*)
 * @param v_min Pointer to store minimum voltage result
 * @param v_max Pointer to store maximum voltage result
 * @return 0 on success, negative error code on failure
 */
int vgpu_get_gpu_volt_rail_limits(struct gk20a *g, u8 type, u32 *v_min, u32 *v_max);

/**
 * @brief Get VF points status via VGPU IVC communication
 *
 * @param g GPU context
 * @param type Domain type (CLK_GPC, CLK_SYS, CLK_NVD, CLK_UPROC)
 * @param vf_points_status Pointer to store VF points status
 * @param num_vf_points Pointer to store number of VF points
 * @return 0 on success, negative error code on failure
 */
int vgpu_get_vf_points_status(struct gk20a *g, struct nvgpu_clk_vf_points_status *vf_points_status,
	u32 *num_vf_points, u8 type);
#endif /* NVGPU_TEMPERATURE_VGPU_H */
