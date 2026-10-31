// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/dma.h>
#include <nvgpu/bug.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/vm.h>
#include <nvgpu/vm_area.h>

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/vm_vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"

/*
 * This is called by the common VM init routine to handle vGPU specifics of
 * intializing a VM on a vGPU. This alone is not enough to init a VM. See
 * nvgpu_vm_init().
 */
int vgpu_vm_as_alloc_share(struct gk20a *g, struct vm_gk20a *vm)
{
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_as_share_params *p = &msg.params.as_share;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_AS_ALLOC_SHARE;
	msg.handle = vgpu_get_handle(g);
	p->va_start = vm->virtaddr_start;
	p->va_limit = vm->va_limit;
	p->big_page_size = vm->big_page_size;

	err = vgpu_comm_sendrecv(g, &msg);
	if (err || msg.ret) {
		return -ENOMEM;
	}

	vm->handle = p->handle;

	return 0;
}

/*
 * Similar to vgpu_vm_as_alloc_share() this is called as part of the cleanup
 * path for VMs. This alone is not enough to remove a VM -
 * see nvgpu_vm_remove().
 */
void vgpu_vm_as_free_share(struct vm_gk20a *vm)
{
	struct gk20a *g = gk20a_from_vm(vm);
	struct tegra_vgpu_cmd_msg msg;
	struct tegra_vgpu_as_share_params *p = &msg.params.as_share;
	int err;

	msg.cmd = TEGRA_VGPU_CMD_AS_FREE_SHARE;
	msg.handle = vgpu_get_handle(g);
	p->handle = vm->handle;
	err = vgpu_comm_sendrecv(g, &msg);
	WARN_ON(err || msg.ret);
}
