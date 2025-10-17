// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/vm.h>
#include <nvgpu/vm_remap.h>

u64 nvgpu_vm_remap_get_handle(struct nvgpu_vm_remap_os_buffer *remap_os_buf)
{
	return 0;
}

int nvgpu_vm_remap_os_buf_get(struct vm_gk20a *vm, u32 mem_handle,
			struct nvgpu_vm_remap_os_buffer *remap_os_buf)
{
	return 0;
}

void nvgpu_vm_remap_os_buf_put(struct vm_gk20a *vm,
			struct nvgpu_vm_remap_os_buffer *remap_os_buf)
{
	return;
}
