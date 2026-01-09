// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/errata.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/io.h>
#include <nvgpu/ltc.h>
#include <nvgpu/regops.h>

#include <nvgpu/hw/gb10b/hw_ltc_gb10b.h>
#include <nvgpu/hw/gb10b/hw_fb_gb10b.h>

#include "hal/gr/gr/gr_gk20a.h"
#include "ltc_gb10b.h"

/* Minimum value of cacheline_size */
#define CACHELINE_SIZE_BASE		512U

void gb10b_ltc_init_fs_state(struct gk20a *g)
{
	u32 reg;

	g->ltc->max_ltc_count = g->ops.top.get_num_ltcs(g);
	g->ltc->ltc_count = g->ops.priv_ring.enum_ltc(g);
	nvgpu_log_info(g, "%u ltcs present out of %u total supported ltcs",
		g->ltc->ltc_count, g->ltc->max_ltc_count);

	reg = nvgpu_readl(g, ltc_ltcs_ltss_cbc_param2_r());
	g->ltc->slices_per_ltc =
		ltc_ltcs_ltss_cbc_param2_slices_per_ltc_v(reg);
	g->ltc->cacheline_size = CACHELINE_SIZE_BASE <<
		ltc_ltcs_ltss_cbc_param2_cache_line_size_v(reg);

	nvgpu_log_info(g, "slices_per_ltc %u", g->ltc->slices_per_ltc);
	nvgpu_log_info(g, "cacheline_size %u", g->ltc->cacheline_size);
}

#ifndef CONFIG_NVGPU_NON_FUSA
void gb10b_set_default_l2_max_ways_evict_last(struct gk20a *g,
		struct nvgpu_gr_ctx *gr_ctx)
{
	u32 reg_val;
	nvgpu_gr_ctx_patch_write_begin(g, gr_ctx, true);

	reg_val = nvgpu_readl(g, ltc_ltcs_ltss_tstg_set_mgmt0_r());
	reg_val = set_field(reg_val,
			ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_m(),
			ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_f(16));
	nvgpu_gr_ctx_patch_write(g, gr_ctx, ltc_ltcs_ltss_tstg_set_mgmt0_r(),
			reg_val, true);

	nvgpu_gr_ctx_patch_write_end(g, gr_ctx, true);
}

#endif

#ifdef CONFIG_NVGPU_DEBUGGER
int gb10b_set_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 num_ways)
{
	struct nvgpu_dbg_reg_op ops = {
		.op = REGOP(READ_32),
		.type = REGOP(TYPE_GR_CTX),
		.offset = ltc_ltcs_ltss_tstg_set_mgmt0_r(),
		.and_n_mask_lo = 0xffffffff
	};
	int err = -EINVAL;
	u32 flags = NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE;
	const u32 num_ops = 1U;

	/*
	 * MAX_WAYS_EVICT_LAST ways should not exceed the number of ways in a
	 * L2 set.
	 */
	if (num_ways > g->ops.get_litter_value(g, GPU_LIT_NUM_LTC_LTS_WAYS)) {
		nvgpu_err(g, "error: num_ways(%d) > max_ways(%d)", num_ways,
				g->ops.get_litter_value(g, GPU_LIT_NUM_LTC_LTS_WAYS));
		return err;
	}

	/*
	 * Readback the current TSTG setting.
	 */
	err = gr_gk20a_exec_ctx_ops(tsg, &ops, num_ops, 0, num_ops, &flags);
	if (err != 0) {
		nvgpu_err(g, "regops_rd failed for LTCS_LTSS_TSTG_MGMT_0");
		return err;
	}
	nvgpu_log_info(g, "current max_ways_l2_evict_last value=0x%x",
		ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_v(ops.value_lo));

	ops.value_lo = set_field(ops.value_lo,
			ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_m(),
			ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_f(num_ways));
	nvgpu_log_info(g, "writing 0x%x to change l2 max_ways_evict_last to 0x%x",
			ops.value_lo, num_ways);

	/*
	 * Write out the new value for L2_MAX_EVICT_LAST.
	 */
	ops.op = REGOP(WRITE_32);
	err = gr_gk20a_exec_ctx_ops(tsg, &ops, num_ops, num_ops, 0, &flags);
	if (err != 0) {
		nvgpu_err(g, "regops_wr failed for LTCS_LTSS_TSTG_MGMT_0");
		return err;
	}

	/*
	 * Readback and verify L2_MAX_EVICT_LAST.
	 */
	ops.op = REGOP(READ_32);
	ops.value_lo = 0U;
	err = gr_gk20a_exec_ctx_ops(tsg, &ops, num_ops, 0, num_ops, &flags);
	if (err != 0) {
		nvgpu_err(g, "regops_rd failed for LTCS_LTSS_TSTG_MGMT_0");
	}
	if (ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_v(ops.value_lo) !=
			num_ways) {
		nvgpu_err(g, "mismatch, expected(%d) != readback(%d)", num_ways,
			ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_v(ops.value_lo));
		return -EINVAL;
	}

	return err;
}

int gb10b_get_l2_max_ways_evict_last(struct gk20a *g, struct nvgpu_tsg *tsg,
		u32 *num_ways)
{
	struct nvgpu_dbg_reg_op ops = {
		.op = REGOP(READ_32),
		.type = REGOP(TYPE_GR_CTX),
		.offset = ltc_ltcs_ltss_tstg_set_mgmt0_r(),
		.and_n_mask_lo = 0xffffffff
	};
	int err;
	u32 flags = NVGPU_REG_OP_FLAG_MODE_ALL_OR_NONE;
	u32 num_ops = 1U;

	if (num_ways == NULL) {
		return -EINVAL;
	}

	/*
	 * Readback the current TSTG setting.
	 */
	err = gr_gk20a_exec_ctx_ops(tsg, &ops, num_ops, 0, num_ops, &flags);
	if (err != 0) {
		nvgpu_err(g, "regops_rd failed for LTCS_LTSS_TSTG_MGMT_0");
		return err;
	}
	*num_ways = ltc_ltcs_ltss_tstg_set_mgmt0_max_evict_last_v(ops.value_lo);
	nvgpu_log_info(g, "current max_ways_l2_evict_last value=0x%x", *num_ways);

	return err;
}
#endif /* CONFIG_NVGPU_DEBUGGER */

#if defined(CONFIG_NVGPU_NON_FUSA)
u32 gb10b_ltcs_ltss_tstg_cfg1_active_sets(u32 reg_val)
{
	return ltc_ltcs_ltss_tstg_cfg1_active_sets_v(reg_val);
}
#endif

u64 gb10b_determine_L2_size_bytes(struct gk20a *g)
{
	u32 reg_val;
	u32 slice_size;
	u32 slices_per_l2;
	u64 size = 0ULL;
#if defined(CONFIG_NVGPU_NON_FUSA)
	u32 active_sets = 0U;
#endif

	nvgpu_log_fn(g, " ");

	reg_val = nvgpu_readl(g, ltc_ltc0_lts0_tstg_info_1_r());
	slice_size = ltc_ltc0_lts0_tstg_info_1_slice_size_in_kb_v(reg_val);
	slices_per_l2 = ltc_ltc0_lts0_tstg_info_1_slices_per_l2_v(reg_val);

	/* L2 size = ltc_count * slice_size in KB * 1024 * slices_per_l2 */
	size = nvgpu_safe_mult_u64(U64(nvgpu_ltc_get_ltc_count(g)),
			nvgpu_safe_mult_u64(
				nvgpu_safe_mult_u64(U64(slice_size), 1024ULL),
				U64(slices_per_l2)));

#if defined(CONFIG_NVGPU_NON_FUSA)
	reg_val = nvgpu_readl(g, ltc_ltcs_ltss_tstg_cfg1_r());
	active_sets = g->ops.ltc.get_ltcs_ltss_tstg_cfg1_active_sets(reg_val);
	if (active_sets == ltc_ltcs_ltss_tstg_cfg1_active_sets_all_v()) {
		nvgpu_log(g, gpu_dbg_info, "L2 active sets ALL");
	} else if (active_sets ==
				ltc_ltcs_ltss_tstg_cfg1_active_sets_half_v()) {
		nvgpu_log(g, gpu_dbg_info, "L2 active sets HALF");
	} else {
		nvgpu_err(g, "Invalid L2 Active sets %d", active_sets);
		return 0ULL;
	}
	/* Update size to reflect active sets */
	size = size >> active_sets;
#endif
	nvgpu_log(g, gpu_dbg_info, "L2 size: %llu\n", size);
	nvgpu_log_fn(g, "done");

	return size;
}

#if defined(CONFIG_NVGPU_NON_FUSA) || defined(CONFIG_NVGPU_KERNEL_MODE_SUBMIT)
void gb10b_ltc_set_enabled(struct gk20a *g, bool enabled)
{
	u32 reg_f = ltc_ltcs_ltss_tstg_set_mgmt_2_l2_bypass_mode_enabled_f();
	u32 reg = gk20a_readl(g, ltc_ltcs_ltss_tstg_set_mgmt_2_r());

	if (enabled) {
		/* bypass disabled (normal caching ops) */
		reg &= ~reg_f;
	} else {
		/* bypass enabled (no caching) */
		reg |= reg_f;
	}

	nvgpu_writel(g, ltc_ltcs_ltss_tstg_set_mgmt_2_r(), reg);
}
#endif

static int gb10b_ltc_wait_for_clean(struct gk20a *g)
{
	struct nvgpu_timeout timeout;
	u32 ltc;
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	bool is_clean_pending_set = false;
	int err = 0;

	/* Wait on each LTC individually. */
	for (ltc = 0; ltc < g->ltc->ltc_count; ltc++) {
		u32 op_pending;

		/*
		 * Use 5ms - this should be sufficient time to flush the cache.
		 * On tegra, rough EMC BW available can be estimated as follows:
		 *
		 * Lowest reasonable EMC clock speed will be around 204MHz on
		 * t234 for display enabled boards and generally fixed to max
		 * for non-display boards (since they are generally plugged in).
		 *
		 * Thus, the available BW is 128B * 2 * 204MHz = ~52GB/s. Of that
		 * BW the GPU will likely get about half (display and overhead/
		 * utilization inefficiency eating the rest) so 26GB/s at
		 * worst. Assuming at most 1MB of GPU L2 cache (less for most
		 * chips) worst case is we take 1MB/26GB/s = 38us.
		 *
		 * So 5ms timeout here should be more than sufficient.
		 */
		nvgpu_timeout_init_cpu_timer(g, &timeout, 5);

		do {
			u32 cmgmt1 = nvgpu_safe_add_u32(
					ltc_ltc0_ltss_tstg_cmgmt1_r(),
					nvgpu_safe_mult_u32(ltc, ltc_stride));
			op_pending = gk20a_readl(g, cmgmt1);
			is_clean_pending_set = (op_pending &
				ltc_ltc0_ltss_tstg_cmgmt1_clean_pending_f()) != 0U;
			if (nvgpu_timeout_expired_msg(&timeout,
					"L2 flush timeout!") != 0) {
				err = -ETIMEDOUT;
			}
		} while (is_clean_pending_set && (err == 0));
	}
	return err;
}

static int gb10b_ltc_wait_for_invalidate(struct gk20a *g)
{
	u32 ltc;
	u32 ltc_stride = nvgpu_get_litter_value(g, GPU_LIT_LTC_STRIDE);
	struct nvgpu_timeout timeout;
	bool is_invalidate_pending_set = false;
	int err = 0;

	for (ltc = 0; ltc < g->ltc->ltc_count; ltc++) {
		u32 op_pending;

		/* Again, 5ms. */
		nvgpu_timeout_init_cpu_timer(g, &timeout, 5);

		do {
			u32 cmgmt0 = nvgpu_safe_add_u32(
					ltc_ltc0_ltss_tstg_cmgmt0_r(),
					nvgpu_safe_mult_u32(ltc, ltc_stride));
			op_pending = gk20a_readl(g, cmgmt0);
			is_invalidate_pending_set = (op_pending &
				ltc_ltc0_ltss_tstg_cmgmt0_invalidate_pending_f()) != 0U;
			if (nvgpu_timeout_expired_msg(&timeout,
					"L2 flush timeout!") != 0) {
				err = -ETIMEDOUT;
			}
		} while (is_invalidate_pending_set && (err == 0));
	}
	return err;
}


/*
 * Performs a full flush of the L2 cache.
 */
void gb10b_flush_ltc(struct gk20a *g)
{
	int err;

	/* Clean... */
	nvgpu_writel(g, ltc_ltcs_ltss_tstg_cmgmt1_r(),
		ltc_ltcs_ltss_tstg_cmgmt1_clean_pending_f() |
		ltc_ltcs_ltss_tstg_cmgmt1_max_cycles_between_cleans_3_f() |
		ltc_ltcs_ltss_tstg_cmgmt1_clean_wait_for_fb_to_pull_true_f() |
		ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_last_class_true_f() |
		ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_normal_class_true_f() |
		ltc_ltcs_ltss_tstg_cmgmt1_clean_evict_first_class_true_f());

	/* Wait on each LTC individually. */
	err = gb10b_ltc_wait_for_clean(g);
	if (err != 0) {
		nvgpu_err(g, "gb10b_ltc_wait_for_clean failed");
	}

	/* And invalidate. */
	nvgpu_writel(g, ltc_ltcs_ltss_tstg_cmgmt0_r(),
	     ltc_ltcs_ltss_tstg_cmgmt0_invalidate_pending_f() |
	     ltc_ltcs_ltss_tstg_cmgmt0_max_cycles_between_invalidates_3_f() |
	     ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_last_class_true_f() |
	     ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_normal_class_true_f() |
	     ltc_ltcs_ltss_tstg_cmgmt0_invalidate_evict_first_class_true_f());

	/* Wait on each LTC individually. */
	err = gb10b_ltc_wait_for_invalidate(g);
	if (err != 0) {
		nvgpu_err(g, "gb10b_ltc_wait_for_invalidate failed");
	}
}

/**
 * logical_ltc_mask is calculated from number LTCs present in the gpu_instance.
 * This id is fixed in the static config.
 * for example instance 0 has 1 LTC and other has 3 LTCs.
 * so masks are 1 and 14.
 */
int  gb10b_ltc_compute_lts_mask(struct gk20a *g, u64 logical_ltc_mask, u64 *logical_lts_mask)
{
	u64 lts_mask;
	u32 bit_index;
	u32 tmp_result;

	if (nvgpu_safe_sub_u64_return(BIT64(g->ltc->slices_per_ltc), 1, &lts_mask) == false) {
		nvgpu_err(g, "buffer underflow");
		return -EOVERFLOW;
	}
	*logical_lts_mask = 0;
	for (bit_index = 0; logical_ltc_mask != 0; logical_ltc_mask = logical_ltc_mask >> 1) {
		if( (logical_ltc_mask & BIT64(0)) != 0U){
			if (nvgpu_safe_mult_u32_return(bit_index, g->ltc->slices_per_ltc, &tmp_result) == false) {
				nvgpu_err(g, "buffer overflow");
				return -EOVERFLOW;
			}
			*logical_lts_mask = *logical_lts_mask | (lts_mask << tmp_result % BITS_PER_LONG);
		}

		if (nvgpu_safe_add_u32_return(bit_index, 1, &bit_index) == false) {
			nvgpu_err(g, "buffer overflow");
			return -EOVERFLOW;
		}
	}

	return 0;
}


#ifdef CONFIG_NVGPU_MIG
void gb10b_ltc_set_remote_swizzle_id(struct gk20a *g, int gridx, u32 swizzle_id)
{
	u32 reg_val = 0U;

	/*set smc config */
	reg_val = nvgpu_readl(g,
			fb_mmu_smc_eng_cfg_0_r(gridx));
	reg_val &= ~fb_mmu_smc_eng_cfg_0_remote_swizid_m();
	reg_val |= fb_mmu_smc_eng_cfg_0_remote_swizid_f(
			swizzle_id);
	nvgpu_writel(g, fb_mmu_smc_eng_cfg_0_r(gridx),
			reg_val);

	/*set mmu_eng_id_cfg */
	reg_val = nvgpu_readl(g,
			fb_mmu_gfid_gr_fault_id_cfg_0_r(gridx));
	reg_val &= ~fb_mmu_gfid_gr_fault_id_cfg_0_bar_remote_swizid_m();
	reg_val |= fb_mmu_gfid_gr_fault_id_cfg_0_bar_remote_swizid_f(
			swizzle_id);
	nvgpu_writel(g, fb_mmu_gfid_gr_fault_id_cfg_0_r(gridx),
			reg_val);

}
#endif
