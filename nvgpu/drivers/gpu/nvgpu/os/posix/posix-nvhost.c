// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/posix/posix-nvhost.h>

void nvgpu_free_nvhost_dev(struct gk20a *g) {

	if (g->nvhost != NULL) {
		nvgpu_kfree(g, g->nvhost);
		g->nvhost = NULL;
	}
}

bool nvgpu_has_syncpoints(struct gk20a *g)
{
	return nvgpu_is_enabled(g, NVGPU_HAS_SYNCPOINTS);
}

u64 nvgpu_nvhost_compute_os_specific_syncpt_addr(struct gk20a *g,
			u32 syncpt_id)
{
	// u64 syncpt_addr = 0U;

	// if (nvgpu_iommuable(g)) {
	// 	syncpt_addr = g->nvhost->iova_syncp[syncpt_id];
	// } else {
	// 	syncpt_addr = nvgpu_safe_add_u64(g->syncpt_unit_base[0],
	// 			nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(g,
	// 			syncpt_id));
	// }

	// return syncpt_addr;

	(void)g;
	(void)syncpt_id;

	return 0;
}

int nvgpu_tegra_fuse_read_per_device_identifier(struct gk20a *g, u64 *pdi)
{
	// u32 lo = 0U;
	// u32 hi = 0U;
	// int err;

	// err = nvgpu_rmos_fuse_read(g, &lo, FUSE_BEGIN + FUSE_PDI0);
	// if (err != 0) {
	// 	return err;
	// }
	// err = nvgpu_rmos_fuse_read(g, &hi, FUSE_BEGIN + FUSE_PDI1);
	// if (err != 0) {
	// 	return err;
	// }
	// *pdi = ((u64)lo) | (((u64)hi) << 32);

	(void)g;
	(void)pdi;
	return 0;
}

static void allocate_new_syncpt(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev)
{
	u32 syncpt_id, syncpt_val;

	srand((unsigned int)time(NULL));

	/* Limit the range between {1, NUM_HW_PTS} */
	syncpt_id = ((unsigned int)rand() % NUM_HW_PTS) + 1U;
	/* Limit the range between {1, UINT_MAX - SYNCPT_SAFE_STATE_INCR - 1} */
	syncpt_val = ((unsigned int)rand() % (UINT_MAX - SYNCPT_SAFE_STATE_INCR - 1));

	nvgpu_syncpt_dev->syncpt_id = syncpt_id;
	nvgpu_syncpt_dev->syncpt_value = syncpt_val;
}

int nvgpu_get_nvhost_dev(struct gk20a *g)
{
	int ret = 0;
	g->nvhost = nvgpu_kzalloc(g, sizeof(struct nvgpu_nvhost_dev));
	if (g->nvhost == NULL) {
		return -ENOMEM;
	}

	g->nvhost->host1x_sp_base = 0x60000000;
	g->nvhost->host1x_sp_size = 0x4000;
	g->nvhost->nb_hw_pts = 704U;
	ret = nvgpu_nvhost_get_syncpt_aperture(
				g->nvhost, &g->syncpt_unit_base[0],
				&g->syncpt_unit_size);
	if (ret != 0) {
		nvgpu_err(g, "Failed to get syncpt interface");
		goto fail_nvgpu_get_nvhost_dev;
	}
	g->syncpt_size =
		nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(g, 1);

	return 0;

fail_nvgpu_get_nvhost_dev:
	nvgpu_free_nvhost_dev(g);

	return ret;
}

int nvgpu_nvhost_get_syncpt_aperture(
		struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
		u64 *base, size_t *size)
{
	if (nvgpu_syncpt_dev == NULL || base == NULL || size == NULL) {
		return -ENOSYS;
	}

	*base = (u64)nvgpu_syncpt_dev->host1x_sp_base;
	*size = nvgpu_syncpt_dev->host1x_sp_size;

	return 0;

}

const char *nvgpu_nvhost_syncpt_get_name(
	struct nvgpu_nvhost_dev *nvgpu_syncpt_dev, int id)
{
	(void)nvgpu_syncpt_dev;
	(void)id;
	return NULL;
}

u32 nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(struct gk20a *g,
	u32 syncpt_id)
{
	(void)g;
	(void)syncpt_id;
	return nvgpu_safe_mult_u32(syncpt_id, 0x1000U);
}

void nvgpu_nvhost_syncpt_set_minval(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 val)
{
	(void)nvgpu_syncpt_dev;
	(void)id;
	(void)val;
}

void nvgpu_nvhost_syncpt_put_ref_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id)
{
	(void)id;
	nvhost_dev->syncpt_id = 0U;
	nvhost_dev->syncpt_value = 0U;
}

u32 nvgpu_nvhost_get_syncpt_client_managed(
	struct nvgpu_nvhost_dev *nvhost_dev,
	const char *syncpt_name)
{
	(void)syncpt_name;

	/* Only allocate new syncpt if nothing exists already */
	if (nvhost_dev->syncpt_id == 0U) {
		allocate_new_syncpt(nvhost_dev);
	} else {
		nvhost_dev->syncpt_id = 0U;
	}

	return nvhost_dev->syncpt_id;
}

void nvgpu_nvhost_syncpt_set_safe_state(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id)
{
	u32 syncpt_value_cur;

	if (nvhost_dev->syncpt_id == id) {
		syncpt_value_cur = nvhost_dev->syncpt_value;
		nvhost_dev->syncpt_value =
			nvgpu_safe_add_u32(syncpt_value_cur,
				SYNCPT_SAFE_STATE_INCR);
	}
}

bool nvgpu_nvhost_syncpt_is_expired_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 thresh)
{
	(void)nvhost_dev;
	(void)id;
	(void)thresh;
	return true;
}

bool nvgpu_nvhost_syncpt_is_valid_pt_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id)
{
	(void)nvhost_dev;
	(void)id;
	return true;
}

int nvgpu_nvhost_intr_register_notifier(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 thresh,
	void (*callback)(void *, int), void *private_data)
{
	(void)nvhost_dev;
	(void)id;
	(void)thresh;
	(void)callback;
	(void)private_data;
	return -ENOSYS;
}

int nvgpu_nvhost_syncpt_read_ext_check(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id, u32 *val)
{
	(void)nvhost_dev;
	(void)id;
	(void)val;
	return -ENOSYS;
}

int nvgpu_nvhost_syncpt_wait_timeout_ext(
	struct nvgpu_nvhost_dev *nvhost_dev, u32 id,
	u32 thresh, u32 timeout, u32 waiter_index)
{
	(void)nvhost_dev;
	(void)id;
	(void)thresh;
	(void)timeout;
	(void)waiter_index;
	return -ENOSYS;
}
