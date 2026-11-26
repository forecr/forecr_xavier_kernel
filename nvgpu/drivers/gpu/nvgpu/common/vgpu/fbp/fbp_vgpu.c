// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/tegra_vgpu.h>
#include <nvgpu/gk20a.h>

#include "fbp_vgpu.h"
#include "common/fbp/fbp_priv.h"

int vgpu_fbp_init_support(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct nvgpu_gpu_instance *gpu_instance = &g->mig.gpu_instance[0];
	struct nvgpu_fbp *fbp;
	u32 i;
	u32 max;

	if (g->fbp != NULL) {
		return 0;
	}

	fbp = nvgpu_kzalloc(g, sizeof(*fbp));
	if (fbp == NULL) {
		return -ENOMEM;
	}

	fbp->num_fbps = priv->constants.num_fbps;
	fbp->max_fbps_count = priv->constants.num_fbps;
	fbp->fbp_en_mask = priv->constants.fbp_en_mask;

	fbp->fbp_l2_en_mask =
		nvgpu_kzalloc(g, fbp->max_fbps_count * sizeof(u32));
	if (fbp->fbp_l2_en_mask == NULL) {
		nvgpu_kfree(g, fbp);
		return -ENOMEM;
	}

	for (i = 0U; i < fbp->max_fbps_count; i++) {
		fbp->fbp_l2_en_mask[i] = priv->constants.l2_en_mask[i];
	}

	fbp->fbp_physical_id_map = nvgpu_kzalloc(g,
			nvgpu_safe_mult_u32(fbp->max_fbps_count, (u32)sizeof(u32)));
	if (fbp->fbp_physical_id_map == NULL) {
		nvgpu_err(g, "alloc fbp_physical_id_map failed");
		nvgpu_kfree(g, fbp->fbp_l2_en_mask);
		nvgpu_kfree(g, fbp);
		return -ENOMEM;
	}
	memset(fbp->fbp_physical_id_map, 0xff,
		nvgpu_safe_mult_u32(fbp->max_fbps_count, (u32)sizeof(u32)));
	max = 0U;
	for (i = 0U; i < TEGRA_VGPU_MAX_FBP_COUNT; i++) {
		if (priv->constants.fbp_physical_id_map[i] != U32_MAX) {
			max++;
		}
	}
	fbp->num_fbps = max;
	nvgpu_assert(fbp->num_fbps <= fbp->max_fbps_count);
	memcpy(fbp->fbp_physical_id_map, priv->constants.fbp_physical_id_map,
		nvgpu_safe_mult_u32(fbp->max_fbps_count, (u32)sizeof(u32)));

	memset(gpu_instance->fbp_mappings, 0xff, NVGPU_MIG_MAX_FBPS * sizeof(u32));
	max = 0U;
	for (i = 0U; i < TEGRA_VGPU_MAX_FBP_COUNT; i++) {
		if (priv->constants.fbp_mappings[i] != U32_MAX) {
			max++;
		}
	}
	gpu_instance->num_fbp = max;
	memcpy(gpu_instance->fbp_mappings, priv->constants.fbp_mappings,
		nvgpu_safe_mult_u32(fbp->max_fbps_count, (u32)sizeof(u32)));

	g->fbp = fbp;

	return 0;
}
