// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/error_notifier.h>
#include <nvgpu/channel.h>

#include <nvgpu/posix/posix-channel.h>

void nvgpu_set_err_notifier_locked(struct nvgpu_channel *ch, u32 error)
{
	struct nvgpu_posix_channel *cp = ch->os_priv;
	if (cp != NULL) {
		cp->err_notifier.error = error;
		cp->err_notifier.status = 0xffff;
	}
}

void nvgpu_set_err_notifier(struct nvgpu_channel *ch, u32 error)
{
	nvgpu_set_err_notifier_locked(ch, error);
}

void nvgpu_set_err_notifier_if_empty(struct nvgpu_channel *ch, u32 error)
{
	struct nvgpu_posix_channel *cp = ch->os_priv;
	if (cp != NULL && cp->err_notifier.status == 0) {
		nvgpu_set_err_notifier_locked(ch, error);
	}
}

bool nvgpu_is_err_notifier_set(struct nvgpu_channel *ch, u32 error_notifier)
{
	struct nvgpu_posix_channel *cp = ch->os_priv;
	if ((cp != NULL) && (cp->err_notifier.status != 0)) {
		return cp->err_notifier.error == error_notifier;
	}
	return false;
}
