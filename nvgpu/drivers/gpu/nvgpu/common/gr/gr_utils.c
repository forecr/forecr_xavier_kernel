// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/types.h>
#include <nvgpu/gr/gr_utils.h>
#include <nvgpu/gr/gr_instances.h>

#include <nvgpu/gr/config.h>

#include "gr_priv.h"

u32 nvgpu_gr_checksum_u32(u32 a, u32 b)
{
	return nvgpu_safe_cast_u64_to_u32(((u64)a + (u64)b) & (U32_MAX));
}

struct nvgpu_gr_falcon *nvgpu_gr_get_falcon_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->falcon;
}

struct nvgpu_gr_config *nvgpu_gr_get_config_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->config;
}

#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_MIG)
struct nvgpu_gr_config *nvgpu_gr_get_gr_instance_config_ptr(struct gk20a *g,
		u32 gr_instance_id)
{
	return g->gr[gr_instance_id].config;
}
#endif

struct nvgpu_gr_intr *nvgpu_gr_get_intr_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->intr;
}

#ifdef CONFIG_NVGPU_NON_FUSA
u32 nvgpu_gr_get_override_ecc_val(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);
	return gr->fecs_feature_override_ecc_val;
}

void nvgpu_gr_override_ecc_val(struct nvgpu_gr *gr, u32 ecc_val)
{
	gr->fecs_feature_override_ecc_val = ecc_val;
}
#endif

#if defined(CONFIG_NVGPU_GRAPHICS) && defined(CONFIG_NVGPU_ZCULL)
struct nvgpu_gr_zcull *nvgpu_gr_get_zcull_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->zcull;
}

struct nvgpu_gr_zbc *nvgpu_gr_get_zbc_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->zbc;
}
#endif

struct nvgpu_gr_global_ctx_buffer_desc *nvgpu_gr_get_global_ctx_buffer_ptr(
							struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));
	return gr->global_ctx_buffer;
}

int nvgpu_gr_get_dump_gr_hw_state_on_ctxsw_timeout(struct gk20a *g, bool *state)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	if (gr == NULL) {
		nvgpu_err(g, "gr not initialized");
		return -ENODEV;
	}

	*state = gr->dump_gr_hw_state_on_ctxsw_timeout;

	return 0;
}

int nvgpu_gr_set_dump_gr_hw_state_on_ctxsw_timeout(struct gk20a *g, bool enable)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	if (gr == NULL) {
		nvgpu_err(g, "gr not initialized");
		return -ENODEV;
	}

	gr->dump_gr_hw_state_on_ctxsw_timeout = enable;

	return 0;
}

#ifdef CONFIG_NVGPU_CILP
bool nvgpu_gr_get_cilp_preempt_pending(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	return gr->cilp_preempt_pending;
}

void nvgpu_gr_set_cilp_preempt_pending(struct gk20a *g,
	bool cilp_preempt_pending)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	gr->cilp_preempt_pending = cilp_preempt_pending;
}

u32 nvgpu_gr_get_cilp_preempt_pending_tsgid(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	return gr->cilp_preempt_pending_tsgid;
}

void nvgpu_gr_clear_cilp_preempt_pending_tsgid(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	gr->cilp_preempt_pending_tsgid =
				NVGPU_INVALID_TSG_ID;
}
#endif

struct nvgpu_gr_obj_ctx_golden_image *nvgpu_gr_get_golden_image_ptr(
		struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr((g));

	return gr->golden_image;
}

#ifdef CONFIG_NVGPU_DEBUGGER
struct nvgpu_gr_hwpm_map *nvgpu_gr_get_hwpm_map_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	return gr->hwpm_map;
}

void nvgpu_gr_reset_falcon_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	gr->falcon = NULL;
}

void nvgpu_gr_reset_golden_image_ptr(struct gk20a *g)
{
	struct nvgpu_gr *gr = nvgpu_gr_get_cur_instance_ptr(g);

	gr->golden_image = NULL;
}
#endif
