// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/atomic.h>
#include <nvgpu/bug.h>
#include <nvgpu/list.h>
#include <nvgpu/nvhost.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/os_fence.h>
#include <nvgpu/os_fence_syncpts.h>
#include <nvgpu/os_fence_semas.h>
#include <nvgpu/channel.h>
#include <nvgpu/channel_sync.h>
#include <nvgpu/channel_sync_syncpt.h>
#include <nvgpu/channel_sync_semaphore.h>
#include <nvgpu/fence.h>

#include "channel_sync_priv.h"

struct nvgpu_channel_sync *nvgpu_channel_sync_create(struct nvgpu_channel *c)
{
	if (nvgpu_has_syncpoints(c->g)) {
		return nvgpu_channel_sync_syncpt_create(c);
	} else {
#ifdef CONFIG_NVGPU_SW_SEMAPHORE
		return nvgpu_channel_sync_semaphore_create(c);
#else
		return NULL;
#endif
	}
}

bool nvgpu_channel_sync_needs_os_fence_framework(struct gk20a *g)
{
	return !nvgpu_has_syncpoints(g);
}

int nvgpu_channel_sync_wait_fence_fd(struct nvgpu_channel_sync *s, int fd,
	struct priv_cmd_entry **entry, u32 max_wait_cmds)
{
	return s->ops->wait_fence_fd(s, fd, entry, max_wait_cmds);
}

int nvgpu_channel_sync_incr(struct nvgpu_channel_sync *s,
	struct priv_cmd_entry **entry, struct nvgpu_fence_type *fence,
	bool need_sync_fence)
{
	return s->ops->incr(s, entry, fence, need_sync_fence);
}

int nvgpu_channel_sync_incr_user(struct nvgpu_channel_sync *s,
	struct priv_cmd_entry **entry, struct nvgpu_fence_type *fence,
	bool wfi, bool need_sync_fence)
{
	return s->ops->incr_user(s, entry, fence, wfi, need_sync_fence);
}

void nvgpu_channel_sync_mark_progress(struct nvgpu_channel_sync *s,
	bool register_irq)
{
	s->ops->mark_progress(s, register_irq);
}

void nvgpu_channel_sync_set_min_eq_max(struct nvgpu_channel_sync *s)
{
	s->ops->set_min_eq_max(s);
}

void nvgpu_channel_sync_get_ref(struct nvgpu_channel_sync *s)
{
	nvgpu_atomic_inc(&s->refcount);
}

bool nvgpu_channel_sync_put_ref_and_check(struct nvgpu_channel_sync *s)
{
	return nvgpu_atomic_dec_and_test(&s->refcount);
}

void nvgpu_channel_sync_destroy(struct nvgpu_channel_sync *sync)
{
	sync->ops->destroy(sync);
}
