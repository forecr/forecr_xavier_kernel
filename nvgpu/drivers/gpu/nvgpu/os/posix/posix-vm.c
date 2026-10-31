// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <stdlib.h>

#include <nvgpu/vm.h>
#include <nvgpu/bug.h>

#include <nvgpu/posix/vm.h>

u64 nvgpu_os_buf_get_size(struct nvgpu_os_buffer *os_buf)
{
	return os_buf->size;
}

struct nvgpu_mapped_buf *nvgpu_vm_find_mapping(struct vm_gk20a *vm,
					       struct nvgpu_os_buffer *os_buf,
					       u64 map_addr,
					       u32 flags,
					       s16 kind)
{
	struct nvgpu_mapped_buf *mapped_buffer = NULL;

	(void)os_buf;
	(void)kind;

	mapped_buffer = nvgpu_vm_find_mapped_buf(vm, map_addr);
	if (mapped_buffer == NULL) {
		return NULL;
	}

	if (mapped_buffer->flags != flags) {
		return NULL;
	}

	return mapped_buffer;
}

void nvgpu_vm_unmap_system(struct nvgpu_mapped_buf *mapped_buffer)
{
	free(mapped_buffer->os_priv.buf);
}
