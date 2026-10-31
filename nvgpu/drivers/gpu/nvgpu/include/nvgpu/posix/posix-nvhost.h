/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_NVHOST_H
#define NVGPU_POSIX_NVHOST_H

#include <nvgpu/types.h>

#define NUM_HW_PTS 		704U
#define SYNCPT_SAFE_STATE_INCR  256U

struct gk20a;
struct nvgpu_nvhost_dev {
	u32 host1x_sp_base;
	u32 host1x_sp_size;
	u32 nb_hw_pts;
	u32 syncpt_id;
	u32 syncpt_value;
};

int nvgpu_get_nvhost_dev(struct gk20a *g);

void nvgpu_free_nvhost_dev(struct gk20a *g);

bool nvgpu_has_syncpoints(struct gk20a *g);

int nvgpu_nvhost_get_syncpt_aperture(
		struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
		u64 *base, size_t *size);

u32 nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(struct gk20a *g,
	u32 syncpt_id);

void nvgpu_nvhost_syncpt_set_minval(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 val);
void nvgpu_nvhost_syncpt_put_ref_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id);

u32 nvgpu_nvhost_get_syncpt_client_managed(
	struct nvgpu_nvhost_dev *nvhost_dev,
	const char *syncpt_name);

const char *nvgpu_nvhost_syncpt_get_name(
	struct nvgpu_nvhost_dev *nvgpu_syncpt_dev, int id);

void nvgpu_nvhost_syncpt_set_safe_state(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id);

bool nvgpu_nvhost_syncpt_is_expired_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 thresh);

bool nvgpu_nvhost_syncpt_is_valid_pt_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id);

int nvgpu_nvhost_intr_register_notifier(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 thresh,
	void (*callback)(void *, int), void *private_data);

int nvgpu_nvhost_syncpt_wait_timeout_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id,
	u32 thresh, u32 timeout, u32 waiter_index);

int nvgpu_nvhost_syncpt_read_ext_check(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 *val);

u64 nvgpu_nvhost_compute_os_specific_syncpt_addr(struct gk20a *g,
			u32 syncpt_id);

int nvgpu_tegra_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi);

#endif
