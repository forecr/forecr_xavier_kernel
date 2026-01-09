// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvlink.h>
#include <nvgpu/bug.h>
#include <nvgpu/nvlink_probe.h>

struct gk20a;
struct nvgpu_firmware;

int nvgpu_nvlink_enumerate(struct gk20a *g)
{
	return -ENOSYS;
}

int nvgpu_nvlink_register_device(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_unregister_device(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_register_link(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_unregister_link(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_setup_ndev(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_init_ops(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_read_dt_props(struct gk20a *g)
{
	BUG();
	return 0;
}

int nvgpu_nvlink_minion_load_ucode(struct gk20a *g,
					struct nvgpu_firmware *nvgpu_minion_fw)
{
	BUG();
	return 0;
}

void nvgpu_nvlink_free_minion_used_mem(struct gk20a *g,
					struct nvgpu_firmware *nvgpu_minion_fw)
{
	BUG();
}
