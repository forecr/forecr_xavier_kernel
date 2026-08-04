// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/device.h>
#include <nvgpu/mc.h>

int nvgpu_mc_reset_units(struct gk20a *g, u32 units)
{
	int err;

	err = g->ops.mc.enable_units(g, units, false);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info, "Unit disable failed");
		return err;
	}

	err = g->ops.mc.enable_units(g, units, true);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info, "Unit disable failed");
		return err;
	}
	return 0;
}

int nvgpu_mc_reset_dev(struct gk20a *g, const struct nvgpu_device *dev)
{
	int err = 0;

	if (g->ops.mc.enable_dev == NULL) {
		goto fail;
	}

	err = g->ops.mc.enable_dev(g, dev, false);
	if (err != 0) {
		nvgpu_device_dump_dev(g, dev);
		goto fail;
	}

	err = g->ops.mc.enable_dev(g, dev, true);
	if (err != 0) {
		nvgpu_device_dump_dev(g, dev);
		goto fail;
	}

fail:
	return err;
}

int nvgpu_mc_reset_devtype(struct gk20a *g, u32 devtype)
{
	int err;

	err = g->ops.mc.enable_devtype(g, devtype, false);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info, "Devtype:%u disable failed",
			devtype);
		return err;
	}

	err = g->ops.mc.enable_devtype(g, devtype, true);
	if (err != 0) {
		nvgpu_log(g, gpu_dbg_info, "Devtype:%u enable failed",
			devtype);
		return err;
	}
	return 0;
}

