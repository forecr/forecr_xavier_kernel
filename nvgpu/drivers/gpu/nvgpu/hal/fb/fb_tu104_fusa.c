// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/trace.h>
#include <nvgpu/log.h>
#include <nvgpu/types.h>
#include <nvgpu/timers.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/io.h>
#include <nvgpu/utils.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/nvgpu_init.h>

#include "hal/fb/fb_gv11b.h"
#include "hal/fb/fb_gv100.h"
#include "hal/mc/mc_tu104.h"

#include "fb_tu104.h"

#include "nvgpu/hw/tu104/hw_fb_tu104.h"
#include "nvgpu/hw/tu104/hw_func_tu104.h"

#ifdef CONFIG_NVGPU_COMPRESSION
void tu104_fb_cbc_get_alignment(struct gk20a *g,
		u64 *base_divisor, u64 *top_divisor)
{
	u64 ltc_count = (u64)nvgpu_ltc_get_ltc_count(g);

	if (base_divisor != NULL) {
		*base_divisor =
			ltc_count << fb_mmu_cbc_base_alignment_shift_v();
	}

	if (top_divisor != NULL) {
		*top_divisor =
			ltc_count << fb_mmu_cbc_top_alignment_shift_v();
	}
}

#ifdef CONFIG_NVGPU_HAL_NON_FUSA
void tu104_fb_cbc_configure(struct gk20a *g, struct nvgpu_cbc *cbc)
{
	u64 base_divisor;
	u64 top_divisor;
	u64 compbit_store_base;
	u64 compbit_store_pa;
	u64 cbc_start_addr, cbc_end_addr;
	u64 cbc_top;
	u64 cbc_top_size;
	u32 cbc_max;

	g->ops.fb.cbc_get_alignment(g, &base_divisor, &top_divisor);
	compbit_store_pa = nvgpu_mem_get_addr(g, &cbc->compbit_store.mem);
	compbit_store_base = DIV_ROUND_UP(compbit_store_pa, base_divisor);

	cbc_start_addr = nvgpu_safe_mult_u64(compbit_store_base, base_divisor);
	cbc_end_addr = nvgpu_safe_add_u64(cbc_start_addr, cbc->compbit_backing_size);

	cbc_top = (cbc_end_addr / top_divisor);
	cbc_top_size = u64_lo32(cbc_top) - compbit_store_base;

	nvgpu_assert(cbc_top_size < U64(U32_MAX));
	nvgpu_writel(g, fb_mmu_cbc_top_r(),
			fb_mmu_cbc_top_size_f(U32(cbc_top_size)));

	cbc_max = nvgpu_readl(g, fb_mmu_cbc_max_r());
	cbc_max = set_field(cbc_max,
		  fb_mmu_cbc_max_comptagline_m(),
		  fb_mmu_cbc_max_comptagline_f(cbc->max_comptag_lines));
	nvgpu_writel(g, fb_mmu_cbc_max_r(), cbc_max);

	nvgpu_assert(compbit_store_base < U64(U32_MAX));
	nvgpu_writel(g, fb_mmu_cbc_base_r(),
		fb_mmu_cbc_base_address_f(U32(compbit_store_base)));

	nvgpu_log(g, gpu_dbg_info | gpu_dbg_map_v | gpu_dbg_pte,
		"compbit base.pa: 0x%x,%08x cbc_base:0x%llx\n",
		(u32)(compbit_store_pa >> 32),
		(u32)(compbit_store_pa & 0xffffffffU),
		compbit_store_base);

	cbc->compbit_store.base_hw = compbit_store_base;
}
#endif
#endif
