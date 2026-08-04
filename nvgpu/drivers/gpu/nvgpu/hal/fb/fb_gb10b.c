// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/fbp.h>

#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>
#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>

#include "hal/fb/fb_gm20b.h"
#include "intr/fb_intr_gb10b.h"
#include "fb_gb10b.h"
#ifdef CONFIG_NVGPU_MIG
#include <nvgpu/grmgr.h>

void gb10b_fb_set_smc_veid_table_mask(struct gk20a *g, u32 veid_enable_mask)
{
	u32 reg_val = 0U;

	reg_val = nvgpu_readl(g, fb_mmu_hypervisor_ctl_r());
	reg_val &= ~fb_mmu_hypervisor_ctl_use_smc_veid_tables_m();
	reg_val |= veid_enable_mask;
	nvgpu_writel(g, fb_mmu_hypervisor_ctl_r(), reg_val);
}

int gb10b_fb_config_veid_smc_map(struct gk20a *g, bool enable)
{
	u32 reg_val;
	u32 gpu_instance_id;
	struct nvgpu_gpu_instance *gpu_instance;
	struct nvgpu_gr_syspipe *gr_syspipe;
	u32 veid_enable_mask = fb_mmu_hypervisor_ctl_use_smc_veid_tables_f(
		fb_mmu_hypervisor_ctl_use_smc_veid_tables_disable_v());
	u32 default_remote_swizid = 0U;

	if (enable) {
		for (gpu_instance_id = 0U;
				gpu_instance_id < g->mig.num_gpu_instances;
				++gpu_instance_id) {

			if (!nvgpu_grmgr_is_mig_type_gpu_instance(
					&g->mig.gpu_instance[gpu_instance_id])) {

				nvgpu_log(g, gpu_dbg_mig, "skip physical instance[%u]",
					gpu_instance_id);
				continue;
			}

			gpu_instance =
				&g->mig.gpu_instance[gpu_instance_id];
			gr_syspipe = &gpu_instance->gr_syspipe;

			reg_val = nvgpu_readl(g,
				fb_mmu_smc_eng_cfg_0_r(gr_syspipe->gr_syspipe_id));

			if (gpu_instance->is_memory_partition_supported) {
				default_remote_swizid = gpu_instance->swizzle_id;
			}
			reg_val = set_field(reg_val,
				fb_mmu_smc_eng_cfg_0_remote_swizid_m(),
				fb_mmu_smc_eng_cfg_0_remote_swizid_f(
					default_remote_swizid));
			nvgpu_writel(g,
				fb_mmu_smc_eng_cfg_0_r(gr_syspipe->gr_syspipe_id),
				reg_val);

		        reg_val = nvgpu_readl(g,fb_mmu_gfid_gr_fault_id_cfg_0_r(gr_syspipe->gr_syspipe_id));
			reg_val = set_field(reg_val,
						fb_mmu_gfid_gr_fault_id_cfg_0_bar_remote_swizid_m(),
						fb_mmu_gfid_gr_fault_id_cfg_0_bar_remote_swizid_f(default_remote_swizid));
			nvgpu_writel(g, fb_mmu_gfid_gr_fault_id_cfg_0_r(gr_syspipe->gr_syspipe_id),
					reg_val);

			reg_val  = nvgpu_readl(g,fb_mmu_smc_eng_cfg_1_r(gr_syspipe->gr_syspipe_id));
			reg_val = set_field(reg_val,
				fb_mmu_smc_eng_cfg_1_veid_mask_m(),
				fb_mmu_smc_eng_cfg_1_veid_mask_f(gpu_instance->veid_mask));
                        nvgpu_writel(g, fb_mmu_smc_eng_cfg_1_r(gr_syspipe->gr_syspipe_id), reg_val);
			/* write 0xff to make gfid 0 to own all veids */
			reg_val = nvgpu_readl(g,fb_mmu_gfid_gr_fault_id_cfg_0_r(0));
			reg_val = set_field(reg_val, fb_mmu_gfid_gr_fault_id_cfg_0_guest_veid_mask_m(),
						fb_mmu_gfid_gr_fault_id_cfg_0_guest_veid_mask_f(0xFF));
			nvgpu_writel(g, fb_mmu_gfid_gr_fault_id_cfg_0_r(0),
					reg_val);
			nvgpu_log(g, gpu_dbg_mig,
				"[%d] gpu_instance_id[%u] default_remote_swizid[%u] "
					"gr_instance_id[%u] gr_syspipe_id[%u] "
					"veid_start_offset[%u] veid_end_offset[%u] "
					"reg_val[%x] ",
				gpu_instance_id,
				g->mig.gpu_instance[gpu_instance_id].gpu_instance_id,
				default_remote_swizid,
				gr_syspipe->gr_instance_id,
				gr_syspipe->gr_syspipe_id,
				gr_syspipe->veid_start_offset,
				nvgpu_safe_sub_u32(
					nvgpu_safe_add_u32(gr_syspipe->veid_start_offset,
						gr_syspipe->max_veid_count_per_tsg), 1U),
					reg_val);

				if ((gpu_instance->is_gfx_capable)) {
					g->ops.grmgr.set_syspipe_gfx_or_compute(g,
							gr_syspipe->gr_syspipe_id, true);
				}
		}
		veid_enable_mask = fb_mmu_hypervisor_ctl_use_smc_veid_tables_f(
			fb_mmu_hypervisor_ctl_use_smc_veid_tables_enable_v());
	}

	if (g->ops.fb.set_smc_veid_table_mask != NULL) {
		g->ops.fb.set_smc_veid_table_mask(g, veid_enable_mask);
	}
	return 0;
}

void gb10b_fb_set_swizzid(struct gk20a *g,
			u32 mmu_fault_id, u32 swizz_id)
{
	u32 reg_val;
	reg_val = nvgpu_readl(g,
		fb_mmu_mmu_eng_id_cfg_r(mmu_fault_id));
	reg_val &= ~fb_mmu_mmu_eng_id_cfg_remote_swizid_m();
	reg_val |= fb_mmu_mmu_eng_id_cfg_remote_swizid_f(swizz_id);
	nvgpu_writel(g,
		fb_mmu_mmu_eng_id_cfg_r(mmu_fault_id),
		reg_val);
}

void gb10b_fb_configure_vpr_mmu_mig(
	struct gk20a *g,
	bool swizzid_check_enable,
	u32 gfx_swizzid)
{
	u32 mmu_vpr_mode;
	u32 mmu_vpr_mode_post_write;
	u32 swizzid_mode;

	if (swizzid_check_enable) {
		swizzid_mode = fb_mmu_vpr_mode_swizzid_enable_true_v();
	} else {
		swizzid_mode = fb_mmu_vpr_mode_swizzid_enable_false_v();
	}
	mmu_vpr_mode = nvgpu_readl(g, fb_mmu_vpr_mode_r());
	nvgpu_info(g, "Pre-programming MMU_VPR_MODE=%08x", mmu_vpr_mode);
	mmu_vpr_mode = set_field(mmu_vpr_mode, fb_mmu_vpr_mode_swizzid_enable_m(), fb_mmu_vpr_mode_swizzid_enable_f(swizzid_mode));
	mmu_vpr_mode = set_field(mmu_vpr_mode, fb_mmu_vpr_mode_swizzid_m(), fb_mmu_vpr_mode_swizzid_f(gfx_swizzid));

	nvgpu_info(g, "Set MMU_VPR_MODE=%08x", mmu_vpr_mode);
	nvgpu_writel(g, fb_mmu_vpr_mode_r(), mmu_vpr_mode);

	mmu_vpr_mode_post_write = nvgpu_readl(g, fb_mmu_vpr_mode_r());
	if (mmu_vpr_mode_post_write != mmu_vpr_mode) {
		nvgpu_err(g, "Unexpected MMU VPR mode. New mode: %08x", mmu_vpr_mode_post_write);
	}
}
#endif

int gb10b_fb_set_fbp_mask(struct gk20a *g,
		u32 num_gpu_instances)
{
	u32 index;
	u32 first_gpu_instance_id = 0U;
	struct nvgpu_gpu_instance *gpu_instance;
	u32 max_fbp, fbp_en_mask;
	u32 tmp_fbp_en_mask, i;
	int err;

	fbp_en_mask = nvgpu_fbp_get_fbp_en_mask(g->fbp);
	max_fbp = g->ops.top.get_max_fbps_count(g);
	if (nvgpu_is_enabled(g, NVGPU_SUPPORT_MIG)) {
		first_gpu_instance_id = 1U;
	}
	for (index = first_gpu_instance_id; index < num_gpu_instances; index++) {
		gpu_instance = &g->mig.gpu_instance[0];
		if (gpu_instance[index].is_memory_partition_supported) {
			/**
			 * It is expected to program Non floorswept FBPs out of first three FBPs and
			 * Its respectives LTCes are assigned to swizzle id
			 * one.
			 */
			if (gpu_instance[index].swizzle_id == 0x1) {
				gpu_instance[index].fbp_en_mask = fbp_en_mask &
					~BIT32(nvgpu_safe_sub_u32(nvgpu_fls(fbp_en_mask), 1U));
			} else if (gpu_instance[index].swizzle_id == 0x2) {
				gpu_instance[index].fbp_en_mask =
					BIT32(nvgpu_safe_sub_u32(nvgpu_fls(fbp_en_mask), 1U));
			} else {
				gpu_instance[index].fbp_en_mask = fbp_en_mask;
			}
		}

		tmp_fbp_en_mask = gpu_instance[index].fbp_en_mask;
		for (i = 0; i < max_fbp; i++) {
			if (tmp_fbp_en_mask & 1) {
				gpu_instance[index].fbp_l2_en_mask[i] = 1;
			}
			tmp_fbp_en_mask = tmp_fbp_en_mask >> 1;
			gpu_instance[index].logical_ltc_mask |=
				gpu_instance[index].fbp_l2_en_mask[i] <<
						nvgpu_fbp_get_fbp_logical_index(g->fbp, i);
		}
		err = g->ops.ltc.compute_lts_mask(g, gpu_instance[index].logical_ltc_mask,
				&gpu_instance[index].logical_lts_mask);
		if (err != 0) {
			nvgpu_err(g, "failed to fetch lts mask");
			return err;
		}
	}
	return 0;
}

#if defined(CONFIG_NVGPU_COMPRESSION)

u64 gb10b_fb_compression_page_size(struct gk20a *g)
{
	/*
	 * Use 2MB as the compression page size, which is necessary for good GPU performance.
	 *
	 * The .compression_page_size() op is used for two things:
	 * - internal calculation of the comptag offset, which is not relevant for GPUs operating in CBC RAW mode.
	 * - reporting to userspace.
	 */
	(void)g;
	return SZ_2M;
}

u32 gb10b_fb_compressible_page_size(struct gk20a *g)
{
	(void)g;
	return SZ_4K;
}

u64 gb10b_fb_compression_align_mask(struct gk20a *g)
{
	(void)g;
	return SZ_64K - 1UL;
}

#endif
