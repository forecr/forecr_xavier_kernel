// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/engines.h>
#include <nvgpu/timers.h>
#include <nvgpu/mc.h>
#include <nvgpu/soc.h>
#include <nvgpu/device.h>

#include "mc_gb10b.h"

#include <nvgpu/hw/gb10b/hw_mc_gb10b.h>

int gb10b_mc_poll_device_enable(struct gk20a *g, u32 reg_idx,
	u32 poll_val)
{
	u32 reg_val;
	u32 delay = POLL_DELAY_MIN_US;
	struct nvgpu_timeout timeout;

	nvgpu_timeout_init_cpu_timer(g, &timeout, MC_ENGINE_RESET_DELAY_US);

	reg_val = nvgpu_readl(g, mc_device_enable_r(reg_idx));

	/*
	 * Engine disable/enable status can also be checked by using
	 * status field of mc_device_enable_r().
	 */

	while ((poll_val != reg_val) &&
		(nvgpu_timeout_expired(&timeout) == 0)) {

		nvgpu_log(g, gpu_dbg_info,
			"poll device_enable_r(%u) to be set to 0x%08x",
			reg_idx, poll_val);

		nvgpu_usleep_range(delay, delay * 2U);
		delay = min_t(u32, delay << 1, POLL_DELAY_MAX_US);
		reg_val = nvgpu_readl(g, mc_device_enable_r(reg_idx));
	}

	if (reg_val != poll_val) {
		nvgpu_err(g, "Failed to set device_enable_r(%u) to 0x%08x",
			reg_idx, poll_val);
		return -ETIMEDOUT;
	}
	return 0;
}

static int gb10b_mc_enable_engine(struct gk20a *g, u32 *device_enable_val,
		bool enable)
{
	u32 reg_val;
	u32 i;
	int err = 0;

	nvgpu_spinlock_acquire(&g->mc.enable_lock);

	for (i = 0U; i < mc_device_enable__size_1_v(); i++) {
		nvgpu_log(g, gpu_dbg_info, "%s device_enable_r[%u]: 0x%08x",
			(enable ? "enable" : "disable"), i,
			device_enable_val[i]);

		reg_val = nvgpu_readl(g, mc_device_enable_r(i));

		if (enable) {
			reg_val |= device_enable_val[i];
		} else {
			reg_val &= ~device_enable_val[i];
		}

		nvgpu_writel(g, mc_device_enable_r(i), reg_val);
		err = gb10b_mc_poll_device_enable(g, i, reg_val);

		if (err != 0) {
			nvgpu_err(g, "Couldn't %s device_enable_reg[%u]: 0x%x]",
				(enable ? "enable" : "disable"), i, reg_val);
		}
	}

	nvgpu_spinlock_release(&g->mc.enable_lock);
	return err;
}

int gb10b_mc_enable_dev(struct gk20a *g, const struct nvgpu_device *dev,
			bool enable)
{
	int err = 0;
	u32 device_enable_val[mc_device_enable__size_1_v()] = {0};
	u32 reg_index = RESET_ID_TO_REG_IDX(dev->reset_id);

	device_enable_val[reg_index] |= RESET_ID_TO_REG_MASK(dev->reset_id);

	err = gb10b_mc_enable_engine(g, device_enable_val, enable);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info,
			"Engine [id: %u] reset failed", dev->engine_id);
	}
	return 0;
}

static void gb10b_mc_get_devtype_reset_mask(struct gk20a *g, u32 devtype,
	u32 *device_enable_reg)
{
	u32 reg_index = 0U;
	const struct nvgpu_device *dev = NULL;

	nvgpu_device_for_each(g, dev, devtype) {
		reg_index = RESET_ID_TO_REG_IDX(dev->reset_id);
		device_enable_reg[reg_index] |=
			RESET_ID_TO_REG_MASK(dev->reset_id);
	}
}

int gb10b_mc_enable_devtype(struct gk20a *g, u32 devtype, bool enable)
{
	int err = 0;
	u32 device_enable_val[mc_device_enable__size_1_v()] = {0};

	gb10b_mc_get_devtype_reset_mask(g, devtype, device_enable_val);

	err = gb10b_mc_enable_engine(g, device_enable_val, enable);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info, "Devtype: %u reset failed", devtype);
	}
	return 0;
}

#ifdef CONFIG_NVGPU_LS_PMU
bool gb10b_mc_is_enabled(struct gk20a *g, u32 unit)
{
	u32 mask = 0U;

	switch (unit) {
	case NVGPU_UNIT_PERFMON:
		mask = mc_enable_perfmon_enabled_f();
		break;
	default:
		nvgpu_err(g, "unknown reset unit %d", unit);
		break;
	}

	return (nvgpu_readl(g, mc_enable_r()) & mask) != 0U;
}
#endif
