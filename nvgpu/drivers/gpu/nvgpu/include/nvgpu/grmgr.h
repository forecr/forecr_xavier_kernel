/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GRMGR_H
#define NVGPU_GRMGR_H

#include <nvgpu/types.h>
#include <nvgpu/mig.h>
#include <nvgpu/enabled.h>
#include <nvgpu/gk20a.h>

#define EMULATE_MODE_MASK          0X000000FFU

#define EMULATE_MODE_DISABLE	0U
#define EMULATE_MODE_1_GPC		1U
#define EMULATE_MODE_2_GPC		2U
#define EMULATE_MODE_MAX_CONFIG		3U


struct gk20a;

void nvgpu_grmgr_init_fbp_mapping(struct gk20a *g,
				struct nvgpu_gpu_instance *gpu_instance,
				u32 num_gpu_instances);
int nvgpu_init_gr_manager(struct gk20a *g);

int nvgpu_grmgr_config_gr_remap_window(struct gk20a *g,
		u32 gr_syspipe_id, bool enable);
u32 nvgpu_grmgr_get_num_gr_instances(struct gk20a *g);
u32 nvgpu_grmgr_get_gpu_instance_id(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_syspipe_id(struct gk20a *g, u32 gr_instance_id);
bool nvgpu_grmgr_get_support_gfx(struct gk20a *g, u32 gr_instance_id);
bool nvgpu_grmgr_get_support_memory_partition(struct gk20a *g,
		u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_num_gpcs(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_num_fbps(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_gpc_phys_id(struct gk20a *g, u32 gr_instance_id,
		u32 gpc_local_id);
u32 nvgpu_grmgr_get_gr_gpc_logical_id(struct gk20a *g, u32 gr_instance_id,
		u32 gpc_local_id);
u32 nvgpu_grmgr_get_gr_instance_id(struct gk20a *g, u32 gpu_instance_id);
bool nvgpu_grmgr_is_valid_runlist_id(struct gk20a *g,
		u32 gpu_instance_id, u32 runlist_id);
u32 nvgpu_grmgr_get_gpu_instance_runlist_id(struct gk20a *g,
		u32 gpu_instance_id);
u32 nvgpu_grmgr_get_gr_instance_id_for_syspipe(struct gk20a *g,
		u32 gr_syspipe_id);
u32 nvgpu_grmgr_get_gpu_instance_max_veid_count(struct gk20a *g,
		u32 gpu_instance_id);
u32 nvgpu_grmgr_get_gr_max_veid_count(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_logical_gpc_mask(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_gr_physical_gpc_mask(struct gk20a *g, u32 gr_instance_id);
u32 nvgpu_grmgr_get_num_fbps(struct gk20a *g, u32 gpu_instance_id);
u32 nvgpu_grmgr_get_fbp_en_mask(struct gk20a *g, u32 gpu_instance_id);
u32 nvgpu_grmgr_get_fbp_logical_id(struct gk20a *g, u32 gr_instance_id,
	u32 fbp_local_id);
u32 *nvgpu_grmgr_get_fbp_l2_en_mask(struct gk20a *g, u32 gpu_instance_id);
bool nvgpu_grmgr_get_memory_partition_support_status(struct gk20a *g,
	u32 gr_instance_id);
bool nvgpu_support_gfx_with_numa(struct gk20a *g);
bool nvgpu_grmgr_is_cur_instance_support_gfx(struct gk20a *g);
long nvgpu_grmgr_get_l2_size(struct gk20a *g, u32 max_ltc,
	u32 fbp_en_mask, u64 L2_size_bytes, u64 *L2_cache_size);

static inline bool nvgpu_grmgr_is_mig_type_gpu_instance(
		struct nvgpu_gpu_instance *gpu_instance)
{
	return (gpu_instance->gpu_instance_type == NVGPU_MIG_TYPE_MIG);
}

static inline bool nvgpu_grmgr_is_multi_gr_enabled(struct gk20a *g)
{
	return ((nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) &&
		(g->mig.num_gpu_instances > 1U));
}

static inline u32 nvgpu_grmgr_get_max_gpc_count(struct gk20a *g)
{
	return g->mig.max_gpc_count;
}

static inline u32 nvgpu_grmgr_get_max_fbps_count(struct gk20a *g)
{
	return g->mig.max_fbps_count;
}

u32 nvgpu_grmgr_assign_ofa(struct gk20a *g, u32 num_ofa, u32 swizzle_id, u32 index,
				struct nvgpu_device **ofa_devs);
u32 nvgpu_grmgr_assign_nvjpg(struct gk20a *g, u32 num_nvjpg, u32 swizzle_id, u32 index,
				struct nvgpu_device **nvjpg_devs);
u32 nvgpu_grmgr_assign_nvdec(struct gk20a *g, u32 num_nvdec, u32 swizzle_id, u32 index,
				struct nvgpu_device **nvdec_devs);
u32 nvgpu_grmgr_assign_nvenc(struct gk20a *g, u32 num_nvenc, u32 swizzle_id, u32 index,
				struct nvgpu_device **nvenc_devs);
#endif /* NVGPU_GRMGR_H */
