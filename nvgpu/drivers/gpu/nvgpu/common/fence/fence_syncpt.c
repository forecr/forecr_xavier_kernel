// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/nvhost.h>
#include <nvgpu/fence.h>
#include <nvgpu/fence_syncpt.h>
#include "fence_priv.h"

static int nvgpu_fence_syncpt_wait(struct nvgpu_fence_type *f, u32 timeout)
{
	struct nvgpu_fence_type_priv *pf = &f->priv;

	return nvgpu_nvhost_syncpt_wait_timeout_ext(
			pf->nvhost_device, pf->syncpt_id, pf->syncpt_value,
			timeout, NVGPU_NVHOST_DEFAULT_WAITER);
}

static bool nvgpu_fence_syncpt_is_expired(struct nvgpu_fence_type *f)
{
	struct nvgpu_fence_type_priv *pf = &f->priv;

	/*
	 * In cases we don't register a notifier, we can't expect the
	 * syncpt value to be updated. For this case, we force a read
	 * of the value from HW, and then check for expiration.
	 */
	if (!nvgpu_nvhost_syncpt_is_expired_ext(pf->nvhost_device,
				pf->syncpt_id, pf->syncpt_value)) {
		int err;
		u32 val;

		err = nvgpu_nvhost_syncpt_read_ext_check(pf->nvhost_device,
				pf->syncpt_id, &val);
		WARN(err != 0, "syncpt read failed??");
		if (err == 0) {
			return nvgpu_nvhost_syncpt_is_expired_ext(
					pf->nvhost_device,
					pf->syncpt_id, pf->syncpt_value);
		} else {
			return false;
		}
	}

	return true;
}

static void nvgpu_fence_syncpt_release(struct nvgpu_fence_type *f)
{
	(void)f;
}

static const struct nvgpu_fence_ops nvgpu_fence_syncpt_ops = {
	.wait = nvgpu_fence_syncpt_wait,
	.is_expired = nvgpu_fence_syncpt_is_expired,
	.release = nvgpu_fence_syncpt_release,
};

/* This function takes the ownership of the os_fence */
void nvgpu_fence_from_syncpt(
		struct nvgpu_fence_type *f,
		struct nvgpu_nvhost_dev *nvhost_device,
		u32 id, u32 value, struct nvgpu_os_fence os_fence)
{
	struct nvgpu_fence_type_priv *pf = &f->priv;

	nvgpu_fence_init(f, &nvgpu_fence_syncpt_ops, os_fence);

	pf->nvhost_device = nvhost_device;
	pf->syncpt_id = id;
	pf->syncpt_value = value;
}
