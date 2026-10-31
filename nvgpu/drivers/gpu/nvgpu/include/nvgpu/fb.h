/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FB_H
#define NVGPU_FB_H

#if defined(CONFIG_NVGPU_NON_FUSA)
/* VAB track all accesses (read and write) */
#define NVGPU_VAB_MODE_ACCESS	BIT32(0U)
/* VAB track only writes (writes and read-modify-writes) */
#define NVGPU_VAB_MODE_DIRTY	BIT32(1U)

/* No change to VAB logging with VPR setting requested */
#define NVGPU_VAB_LOGGING_VPR_NONE  0U
/* VAB logging disabled if vpr IN_USE=1, regardless of PROTECTED_MODE */
#define NVGPU_VAB_LOGGING_VPR_IN_USE_DISABLED  BIT32(0U)
/* VAB logging disabled if vpr PROTECTED_MODE=1, regardless of IN_USE */
#define NVGPU_VAB_LOGGING_VPR_PROTECTED_DISABLED  BIT32(1U)
/* VAB logging enabled regardless of IN_USE and PROTECTED_MODE */
#define NVGPU_VAB_LOGGING_VPR_ENABLED  BIT32(2U)
/* VAB logging disabled regardless of IN_USE and PROTECTED_MODE */
#define NVGPU_VAB_LOGGING_VPR_DISABLED  BIT32(3U)

struct nvgpu_vab_range_checker {

	/*
	 * in: starting physical address. Must be aligned by
         *     1 << (granularity_shift + bitmask_size_shift) where
	 *     bitmask_size_shift is a HW specific constant.
	 */
	u64 start_phys_addr;

	/* in: log2 of coverage granularity per bit */
	u8  granularity_shift;

	u8  reserved[7];
};

struct nvgpu_vab {
	u32 user_num_range_checkers;
	u32 num_entries;
	unsigned long entry_size;
	struct nvgpu_mem buffer;

	/*
	 * Evaluates to true if a VAB_ERROR mmu fault has happened since
	 * dump has started
	 */
	nvgpu_atomic_t mmu_vab_error_flag;
};

int nvgpu_fb_vab_init_hal(struct gk20a *g);
int nvgpu_fb_vab_teardown_hal(struct gk20a *g);
#endif

/**
 * @brief Initializes the FB unit.
 *
 * @param g	[in]	The GPU.
 *
 * - Request common/power_features/cg to load the prod values for
 *   slcg and blcg.
 * - Initializes the fbhub mmu.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_init_fb_support(struct gk20a *g);

#endif /* NVGPU_FB_H */
