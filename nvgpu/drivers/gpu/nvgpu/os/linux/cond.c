// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/wait.h>
#include <linux/sched.h>

#include <nvgpu/cond.h>

int nvgpu_cond_init(struct nvgpu_cond *cond)
{
	init_waitqueue_head(&cond->wq);
	cond->initialized = true;

	return 0;
}

void nvgpu_cond_destroy(struct nvgpu_cond *cond)
{
	cond->initialized = false;
}

void nvgpu_cond_signal(struct nvgpu_cond *cond)
{
	BUG_ON(!cond->initialized);

	wake_up(&cond->wq);
}

void nvgpu_cond_signal_interruptible(struct nvgpu_cond *cond)
{
	BUG_ON(!cond->initialized);

	wake_up_interruptible(&cond->wq);
}

int nvgpu_cond_broadcast(struct nvgpu_cond *cond)
{
	if (!cond->initialized)
		return -EINVAL;

	wake_up_all(&cond->wq);

	return 0;
}

int nvgpu_cond_broadcast_interruptible(struct nvgpu_cond *cond)
{
	if (!cond->initialized)
		return -EINVAL;

	wake_up_interruptible_all(&cond->wq);

	return 0;
}
