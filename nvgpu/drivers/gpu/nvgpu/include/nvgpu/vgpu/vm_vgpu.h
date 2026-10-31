/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VGPU_VM_H
#define NVGPU_VGPU_VM_H

#ifdef CONFIG_NVGPU_GR_VIRTUALIZATION
int  vgpu_vm_as_alloc_share(struct gk20a *g, struct vm_gk20a *vm);
void vgpu_vm_as_free_share(struct vm_gk20a *vm);
#endif

#endif /* NVGPU_VGPU_VM_H */
