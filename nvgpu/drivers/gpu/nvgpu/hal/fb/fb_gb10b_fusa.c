// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/dma.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>

#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>

#include "hal/fb/fb_gm20b.h"
#include "intr/fb_intr_gb10b.h"
#include "fb_gb10b.h"


void gb10b_fb_read_wpr_info(struct gk20a *g, u64 *wpr_base, u64 *wpr_size)
{
	u32 val = 0U;
	u64 wpr_start = 0U;
	u64 wpr_end = 0U;

	/*
	 * For GB10B, WPR1 is shared by SEC2 and GSP.
	 * SEC2 uses starting few bytes of WPR1 for DRM wrapper
	 * key usecase and updates the gsp_cfga(0) of the end addr
	 * of the DRM keys, for GSP to not overwrite the keys, the
	 * WPR1 start is read from gsp_cfga(0 instead of wpr_addr_lo)
	 */
	val = fb_mmu_falcon_gsp_cfga_addr_lo_v(
			nvgpu_readl(g, fb_mmu_falcon_gsp_cfga_r(0)));
	wpr_start = hi32_lo32_to_u64(
		(val >> ALIGN_HI32(fb_mmu_wpr1_addr_lo_val_alignment_v())),
		(val << fb_mmu_wpr1_addr_lo_val_alignment_v()));

	val = fb_mmu_wpr1_addr_hi_val_v(
			nvgpu_readl(g, fb_mmu_wpr1_addr_hi_r()));
	wpr_end = hi32_lo32_to_u64(
		(val >> ALIGN_HI32(fb_mmu_wpr1_addr_hi_val_alignment_v())),
		(val << fb_mmu_wpr1_addr_hi_val_alignment_v()));

	*wpr_base = wpr_start;
	*wpr_size = nvgpu_safe_sub_u64(wpr_end, wpr_start);
}

int gb10b_fb_serial_ats_ctrl(struct gk20a *g, bool enable)
{
	u32 reg_val;

	if (!nvgpu_iommuable(g) && enable) {
		nvgpu_err(g, "Failed to enable serial ats");
		return -ENOSYS;
	}


	reg_val = nvgpu_readl(g, fb_mmu_hypervisor_ctl_r());

	if (enable) {
		nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn, "Enabling serial ats");
		reg_val = set_field(reg_val, fb_mmu_hypervisor_ctl_non_pasid_ats_m(),
				    fb_mmu_hypervisor_ctl_non_pasid_ats_f(fb_mmu_hypervisor_ctl_non_pasid_ats_enabled_v()));
	} else {
		nvgpu_log(g, gpu_dbg_gpu_dbg | gpu_dbg_fn, "Disabling serial ats");
		reg_val = set_field(reg_val, fb_mmu_hypervisor_ctl_non_pasid_ats_m(),
				    fb_mmu_hypervisor_ctl_non_pasid_ats_f(fb_mmu_hypervisor_ctl_non_pasid_ats_disabled_v()));
	}

	nvgpu_writel(g, fb_mmu_hypervisor_ctl_r(), reg_val);

	return 0;
}

int gb10b_fb_set_atomic_mode(struct gk20a *g)
{
	u32 reg_val;

	/*
	 * NV_PFB_PRI_MMU_CTRL_ATOMIC_CAPABILITY_MODE to RMW MODE
	 * NV_PFB_PRI_MMU_CTRL_ATOMIC_CAPABILITY_SYS_NCOH_MODE to L2
	 */
	reg_val = nvgpu_readl(g, fb_mmu_ctrl_r());
	reg_val = set_field(reg_val, fb_mmu_ctrl_atomic_capability_mode_m(),
			    fb_mmu_ctrl_atomic_capability_mode_rmw_f());
	reg_val = set_field(reg_val,
			    fb_mmu_ctrl_atomic_capability_sys_ncoh_mode_m(),
			    fb_mmu_ctrl_atomic_capability_sys_ncoh_mode_l2_f());
	nvgpu_writel(g, fb_mmu_ctrl_r(), reg_val);

	return 0;
}

static void gb10b_fb_check_ltcs_count(struct gk20a *g)
{
	u32 ltcs_count;

	/*
	 * Number of active ltcs should be same in below registers
	 * - pri_ringmaster_enum_ltc_r
	 * - fb_mmu_num_active_ltcs_r
	 *
	 * top_num_ltcs_r gives max number of ltcs. If chip is floorswept
	 * then max ltcs count may not match active ltcs count.
	 */
	ltcs_count = g->ops.priv_ring.enum_ltc(g);

	if (fb_mmu_num_active_ltcs_count_v(nvgpu_readl(g,
						       fb_mmu_num_active_ltcs_r())) != ltcs_count) {
		nvgpu_err(g,
			  "mmu_num_active_ltcs = %u not equal to enum_ltc() = %u",
			  fb_mmu_num_active_ltcs_count_v(nvgpu_readl(g,
								     fb_mmu_num_active_ltcs_r())),
			  ltcs_count);
	} else {
		nvgpu_log(g, gpu_dbg_info, "mmu active ltcs %u",
			  fb_mmu_num_active_ltcs_count_v(nvgpu_readl(g,
								     fb_mmu_num_active_ltcs_r())));
	}
}

void gb10b_fb_init_fs_state(struct gk20a *g)
{
	nvgpu_log(g, gpu_dbg_fn, "initialize gb10b fb");

	gb10b_fb_check_ltcs_count(g);

	if (!nvgpu_is_enabled(g, NVGPU_SEC_PRIVSECURITY)) {
		/* Bypass MMU check for non-secure boot. For
		 * secure-boot,this register write has no-effect
		 */
		nvgpu_writel(g, fb_priv_mmu_phy_secure_r(), U32_MAX);
	}
}

static void gb10b_fb_set_sysmem_flush_buffer(struct gk20a *g)
{
	u64 sysmem_flush_addr;
	u32 lo;
	u32 hi;

	sysmem_flush_addr = nvgpu_mem_get_addr(g, &g->mm.sysmem_flush);
	nvgpu_assert(sysmem_flush_addr != 0);

	lo = u64_lo32(sysmem_flush_addr);
	/* Assert when  sysmem flush buffer address is not aligned to 256
	 * bytes
	 */
	nvgpu_assert((lo & ~fb_pcie_flush_sysmem_addr_lo_adr_mask_v()) == 0);

	/* Assert when sysmem flush buffer has more than 52-bit address */
	hi = u64_hi32(sysmem_flush_addr);
	nvgpu_assert((hi & ~fb_pcie_flush_sysmem_addr_hi_adr_mask_v()) == 0);

	nvgpu_writel(g, fb_pcie_flush_sysmem_addr_lo_r(), lo);
	nvgpu_writel(g, fb_pcie_flush_sysmem_addr_hi_r(), hi);
}

static void gb10b_fb_buffer_init(struct gk20a *g)
{
	u64 addr;

	gb10b_fb_set_sysmem_flush_buffer(g);

	/* init mmu debug buffer */
	addr = nvgpu_mem_get_addr(g, &g->mm.mmu_wr_mem);
	addr >>= fb_mmu_debug_wr_addr_alignment_v();

	nvgpu_assert(u64_hi32(addr) == 0U);
	nvgpu_writel(g, fb_mmu_debug_wr_r(),
		     nvgpu_aperture_mask(g, &g->mm.mmu_wr_mem,
				fb_mmu_debug_wr_aperture_sys_mem_ncoh_f(),
				fb_mmu_debug_wr_aperture_sys_mem_coh_f(),
				fb_mmu_debug_wr_aperture_vid_mem_f()) |
		     fb_mmu_debug_wr_vol_false_f() |
		     fb_mmu_debug_wr_addr_f(U32(addr)));

	addr = nvgpu_mem_get_addr(g, &g->mm.mmu_rd_mem);
	addr >>= fb_mmu_debug_rd_addr_alignment_v();

	nvgpu_assert(u64_hi32(addr) == 0U);
	nvgpu_writel(g, fb_mmu_debug_rd_r(),
		     nvgpu_aperture_mask(g, &g->mm.mmu_rd_mem,
				fb_mmu_debug_wr_aperture_sys_mem_ncoh_f(),
				fb_mmu_debug_wr_aperture_sys_mem_coh_f(),
				fb_mmu_debug_rd_aperture_vid_mem_f()) |
		     fb_mmu_debug_rd_vol_false_f() |
		     fb_mmu_debug_rd_addr_f(U32(addr)));
}

void gb10b_fb_init_hw(struct gk20a *g)
{
	gb10b_fb_buffer_init(g);

	gb10b_fb_intr_vectorid_init(g);

	if (g->ops.fb.intr.enable != NULL) {
		g->ops.fb.intr.enable(g);
	}
}

u32 gb10b_fb_get_num_active_ltcs(struct gk20a *g)
{
	return fb_mmu_num_active_ltcs_count_v(nvgpu_readl(g,
							  fb_mmu_num_active_ltcs_r()));
}

u32 gb10b_fb_mmu_ctrl(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_ctrl_r());
}

u32 gb10b_fb_mmu_debug_ctrl(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_debug_ctrl_r());
}

u32 gb10b_fb_mmu_debug_wr(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_debug_wr_r());
}

u32 gb10b_fb_mmu_debug_rd(struct gk20a *g)
{
	return nvgpu_readl(g, fb_mmu_debug_rd_r());
}

#ifdef CONFIG_NVGPU_DEBUGGER
bool gb10b_fb_debug_mode_enabled(struct gk20a *g)
{
	u32 debug_ctrl = nvgpu_readl(g, fb_mmu_debug_ctrl_r());

	return get_field(debug_ctrl, fb_mmu_debug_ctrl_debug_m()) ==
			fb_mmu_debug_ctrl_debug_enabled_f();
}

void gb10b_fb_set_mmu_debug_mode(struct gk20a *g, bool enable)
{
	u32 reg_val, fb_debug_ctrl;

	if (enable) {
		fb_debug_ctrl = fb_mmu_debug_ctrl_debug_enabled_f();
		g->mmu_debug_ctrl = true;
	} else {
		fb_debug_ctrl = fb_mmu_debug_ctrl_debug_disabled_f();
		g->mmu_debug_ctrl = false;
	}

	reg_val = nvgpu_readl(g, fb_mmu_debug_ctrl_r());
	reg_val = set_field(reg_val,
			fb_mmu_debug_ctrl_debug_m(), fb_debug_ctrl);
	nvgpu_writel(g, fb_mmu_debug_ctrl_r(), reg_val);
}
#endif
