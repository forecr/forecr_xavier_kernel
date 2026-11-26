// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/device.h>
#include <nvgpu/gin.h>

#include "init_vf.h"

static u32 vf_intr_nonstall_handler(struct gk20a *g, u64 cookie)
{
	(void)g;
	(void)cookie;
	return (NVGPU_CIC_NONSTALL_OPS_WAKEUP_SEMAPHORE |
			NVGPU_CIC_NONSTALL_OPS_POST_EVENTS);
}

static u32 get_gin_unit_for_dev_type(u32 dev_type)
{
	switch (dev_type) {
	case NVGPU_DEVTYPE_GRAPHICS:
		return NVGPU_GIN_INTR_UNIT_GR;
	case NVGPU_DEVTYPE_LCE:
		return NVGPU_GIN_INTR_UNIT_CE;
	case NVGPU_DEVTYPE_NVENC:
		return NVGPU_GIN_INTR_UNIT_NVENC;
	case NVGPU_DEVTYPE_NVDEC:
		return NVGPU_GIN_INTR_UNIT_NVDEC;
	case NVGPU_DEVTYPE_NVJPG:
		return NVGPU_GIN_INTR_UNIT_NVJPG;
	case NVGPU_DEVTYPE_OFA:
		return NVGPU_GIN_INTR_UNIT_OFA;
	default:
		nvgpu_assert(false);
	}

	return 0U;
}

int vf_enable_engine_nonstall_intrs(struct gk20a *g)
{
	u32 i = 0U;
	u32 dev_type = 0U;
	u32 unit = 0U;
	u32 inst_id = 0U;
	u32 vector = 0U;
	const struct nvgpu_device *dev = NULL;

	const u32 dev_types[] = {
		NVGPU_DEVTYPE_GRAPHICS,
		NVGPU_DEVTYPE_LCE,
		NVGPU_DEVTYPE_NVENC,
		NVGPU_DEVTYPE_NVDEC,
		NVGPU_DEVTYPE_NVJPG,
		NVGPU_DEVTYPE_OFA,
	};

	for (i = 0U; i < ARRAY_SIZE(dev_types); i++) {
		dev_type = dev_types[i];
		unit = get_gin_unit_for_dev_type(dev_type);

		nvgpu_device_for_each(g, dev, dev_type) {
			inst_id = dev->inst_id;

			vector = nvgpu_gin_get_unit_nonstall_vector(g, unit, inst_id);
			nvgpu_gin_set_nonstall_handler(g, vector, &vf_intr_nonstall_handler, inst_id);
			nvgpu_gin_nonstall_unit_config(g, unit, inst_id, NVGPU_GIN_INTR_ENABLE);
		}
	}

	return 0;
}
