// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/rwsem.h>

void nvgpu_rwsem_init(struct nvgpu_rwsem *rwsem)
{
	init_rwsem(&rwsem->rwsem);
}

void nvgpu_rwsem_up_read(struct nvgpu_rwsem *rwsem)
{
	up_read(&rwsem->rwsem);
}

void nvgpu_rwsem_down_read(struct nvgpu_rwsem *rwsem)
{
	down_read(&rwsem->rwsem);
}

void nvgpu_rwsem_up_write(struct nvgpu_rwsem *rwsem)
{
	up_write(&rwsem->rwsem);
}

void nvgpu_rwsem_down_write(struct nvgpu_rwsem *rwsem)
{
	down_write(&rwsem->rwsem);
}
