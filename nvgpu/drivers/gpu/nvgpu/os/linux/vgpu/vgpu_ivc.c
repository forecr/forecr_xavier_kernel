// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/types.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <linux/tegra_gr_comm.h>

#include "os/linux/os_linux.h"
#include "os/linux/module.h"

int vgpu_ivc_init(struct gk20a *g, u32 elems,
		const size_t *queue_sizes, u32 queue_start, u32 num_queues)
{
	struct device *dev = dev_from_gk20a(g);

	return tegra_gr_comm_init(dev, nvgpu_get_node(g), elems,
				queue_sizes, queue_start,
				num_queues);
}

void vgpu_ivc_deinit(u32 queue_start, u32 num_queues)
{
	tegra_gr_comm_deinit(queue_start, num_queues);
}

void vgpu_ivc_release(void *handle)
{
	tegra_gr_comm_release(handle);
}

u32 vgpu_ivc_get_server_vmid(void)
{
	return tegra_gr_comm_get_server_vmid();
}

int vgpu_ivc_recv(u32 index, void **handle, void **data,
				size_t *size, u32 *sender)
{
	return tegra_gr_comm_recv(index, handle, data, size, sender);
}

int vgpu_ivc_send(u32 peer, u32 index, void *data, size_t size)
{
	return tegra_gr_comm_send(peer, index, data, size);
}

int vgpu_ivc_sendrecv(u32 peer, u32 index, void **handle,
				void **data, size_t *size)
{
	return tegra_gr_comm_sendrecv(peer, index, handle, data, size);
}

u32 vgpu_ivc_get_peer_self(void)
{
	return TEGRA_GR_COMM_ID_SELF;
}

void *vgpu_ivc_oob_get_ptr(u32 peer, u32 index, void **ptr,
					size_t *size)
{
	return tegra_gr_comm_oob_get_ptr(peer, index, ptr, size);
}

void vgpu_ivc_oob_put_ptr(void *handle)
{
	tegra_gr_comm_oob_put_ptr(handle);
}
