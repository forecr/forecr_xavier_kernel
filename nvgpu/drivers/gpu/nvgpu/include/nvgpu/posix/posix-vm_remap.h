/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_VM__REMAP_H
#define NVGPU_POSIX_VM_REMAP_H

#include <nvgpu/types.h>

struct nvgpu_vm_remap_os_buffer {
	struct nvgpu_os_buffer os_buf;
	struct nvgpu_mapped_buf_priv os_priv;

	struct nvgpu_sgt *nv_sgt;
	u64 aperture;
};

u64 nvgpu_vm_remap_get_handle(struct nvgpu_vm_remap_os_buffer *remap_os_buf);
int nvgpu_vm_remap_os_buf_get(struct vm_gk20a *vm, u32 mem_handle,
			struct nvgpu_vm_remap_os_buffer *remap_os_buf);
void nvgpu_vm_remap_os_buf_put(struct vm_gk20a *vm,
			struct nvgpu_vm_remap_os_buffer *remap_os_buf);

#endif /* NVGPU_POSIX_VM_REMAP_H */
