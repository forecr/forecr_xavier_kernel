// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/types.h>
#include <nvgpu/os_fence.h>
#include <nvgpu/linux/os_fence_android.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/channel.h>
#include <nvgpu/nvhost.h>

#include "os_fence_priv.h"

#include "../drivers/staging/android/sync.h"

inline struct sync_fence *nvgpu_get_sync_fence(struct nvgpu_os_fence *s)
{
	struct sync_fence *fence = (struct sync_fence *)s->priv;
	return fence;
}

void nvgpu_os_fence_android_drop_ref(struct nvgpu_os_fence *s)
{
	struct sync_fence *fence = nvgpu_get_sync_fence(s);

	sync_fence_put(fence);

	nvgpu_os_fence_clear(s);
}

int nvgpu_os_fence_android_install_fd(struct nvgpu_os_fence *s, int fd)
{
	struct sync_fence *fence = nvgpu_get_sync_fence(s);

	sync_fence_get(fence);
	sync_fence_install(fence, fd);

	return 0;
}

void nvgpu_os_fence_android_dup(struct nvgpu_os_fence *s)
{
	struct sync_fence *fence = nvgpu_get_sync_fence(s);

	sync_fence_get(fence);
}

int nvgpu_os_fence_fdget(struct nvgpu_os_fence *fence_out,
	struct nvgpu_channel *c, int fd)
{
	int err = -ENOSYS;

#ifdef CONFIG_TEGRA_GK20A_NVHOST
	if (nvgpu_has_syncpoints(c->g)) {
		err = nvgpu_os_fence_syncpt_fdget(fence_out, c, fd);
	}
#endif

	if (err)
		err = nvgpu_os_fence_sema_fdget(fence_out, c, fd);

	if (err)
		nvgpu_err(c->g, "error obtaining fence from fd %d", fd);

	return err;
}
