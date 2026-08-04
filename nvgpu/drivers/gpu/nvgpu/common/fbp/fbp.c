// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/fbp.h>
#include <nvgpu/static_analysis.h>

#include "fbp_priv.h"

int nvgpu_fbp_init_support(struct gk20a *g)
{
	struct nvgpu_fbp *fbp;
	u32 fbp_en_mask;
#ifdef CONFIG_NVGPU_NON_FUSA
	u32 max_ltc_per_fbp;
	u32 l2_all_en_mask;
	unsigned long i;
	unsigned long fbp_en_mask_tmp;
	u32 tmp;
#endif
	u32 j;

	if (g->fbp != NULL) {
		return 0;
	}

	fbp = nvgpu_kzalloc(g, sizeof(*fbp));
	if (fbp == NULL) {
		return -ENOMEM;
	}

	fbp->num_fbps = g->ops.priv_ring.get_fbp_count(g);
	nvgpu_log_info(g, "fbps: %d", fbp->num_fbps);

	fbp->max_fbps_count = g->ops.top.get_max_fbps_count(g);
	nvgpu_log_info(g, "max_fbps_count: %d", fbp->max_fbps_count);

	/*
	 * Read active fbp mask from fuse
	 * Note that 0:enable and 1:disable in value read from fuse so we've to
	 * flip the bits.
	 * Also set unused bits to zero
	 */
	fbp_en_mask = g->ops.fuse.fuse_status_opt_fbp(g);
	fbp_en_mask = ~fbp_en_mask;
	fbp_en_mask = fbp_en_mask &
		nvgpu_safe_sub_u32(BIT32(fbp->max_fbps_count), 1U);
	fbp->fbp_en_mask = fbp_en_mask;

#ifdef CONFIG_NVGPU_NON_FUSA
	fbp->fbp_l2_en_mask =
		nvgpu_kzalloc(g,
			nvgpu_safe_mult_u64(fbp->max_fbps_count, sizeof(u32)));
	if (fbp->fbp_l2_en_mask == NULL) {
		nvgpu_kfree(g, fbp);
		return -ENOMEM;
	}

	fbp_en_mask_tmp = fbp_en_mask;
	max_ltc_per_fbp = g->ops.top.get_max_ltc_per_fbp(g);
	l2_all_en_mask = nvgpu_safe_sub_u32(BIT32(max_ltc_per_fbp), 1U);

	/* get active L2 mask per FBP */
	for_each_set_bit(i, &fbp_en_mask_tmp, fbp->max_fbps_count) {
		tmp = g->ops.fuse.fuse_status_opt_l2_fbp(g, (u32)i);
		fbp->fbp_l2_en_mask[i] = l2_all_en_mask ^ tmp;
	}
#endif

	g->fbp = fbp;

	fbp->fbp_physical_id_map = nvgpu_kzalloc(g,
			nvgpu_safe_mult_u32(fbp->max_fbps_count, (u32)sizeof(u32)));
	if (fbp->fbp_physical_id_map == NULL) {
		nvgpu_err(g, "alloc fbp_physical_id_map failed");
		nvgpu_kfree(g, fbp->fbp_l2_en_mask);
		nvgpu_kfree(g, fbp);
		return -ENOMEM;
	}

	if (g->ops.priv_ring.init_fbp_physical_logical_id_map != NULL) {
		/* Initialize all logical index to invalid number. i.e U32_MAX */
		for (j = 0; j < g->fbp->max_fbps_count; j++) {
			g->fbp->fbp_physical_id_map[j] = U32_MAX;
		}
		g->ops.priv_ring.init_fbp_physical_logical_id_map(g,
						fbp->fbp_physical_id_map);
	} else {
		for (j = 0U; j < g->fbp->max_fbps_count; j++) {
			g->fbp->fbp_physical_id_map[j] = j;
		}
	}

	return 0;
}

void nvgpu_fbp_remove_support(struct gk20a *g)
{
	struct nvgpu_fbp *fbp = g->fbp;

	if (fbp != NULL) {
		nvgpu_kfree(g, fbp->fbp_physical_id_map);
		nvgpu_kfree(g, fbp->fbp_l2_en_mask);
		nvgpu_kfree(g, fbp);
	}

	g->fbp = NULL;
}

u32 nvgpu_fbp_get_max_fbps_count(struct nvgpu_fbp *fbp)
{
	return fbp->max_fbps_count;
}

u32 nvgpu_fbp_get_fbp_en_mask(struct nvgpu_fbp *fbp)
{
	return fbp->fbp_en_mask;
}

u32 nvgpu_fbp_get_num_fbps(struct nvgpu_fbp *fbp)
{
	return fbp->num_fbps;
}

u32 nvgpu_fbp_get_fbp_logical_index(struct nvgpu_fbp *fbp, u32 physical_index)
{
	nvgpu_assert(physical_index < fbp->max_fbps_count);

	return fbp->fbp_physical_id_map[physical_index];
}

u32 nvgpu_fbp_get_fbp_physical_index(struct nvgpu_fbp *fbp, u32 logical_index)
{
	u32 i;

	nvgpu_assert(logical_index < fbp->num_fbps);

	for (i = 0U; i < fbp->max_fbps_count; i++) {
		if (fbp->fbp_physical_id_map[i] == logical_index) {
			return i;
		}
	}

	return U32_MAX;
}

#ifdef CONFIG_NVGPU_NON_FUSA
u32 *nvgpu_fbp_get_l2_en_mask_base_addr(struct nvgpu_fbp *fbp)
{
	return fbp->fbp_l2_en_mask;
}

u32 nvgpu_fbp_get_l2_en_mask(struct nvgpu_fbp *fbp, u32 fbp_phy_id)
{
	return fbp->fbp_l2_en_mask[fbp_phy_id];
}
#endif

