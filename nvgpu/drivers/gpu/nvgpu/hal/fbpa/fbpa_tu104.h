/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FBPA_TU104_H
#define NVGPU_FBPA_TU104_H

struct gk20a;

int tu104_fbpa_init(struct gk20a *g);
void tu104_fbpa_handle_intr(struct gk20a *g, u32 fbpa_id);

/**
 * @brief Allocate and initialize error counters for all fbpa instances.
 *
 * @param g [in] The GPU driver struct.
 * @param stat [out] Pointer to array of tpc error counters.
 * @param name [in] Unique name for error counter.
 *
 * Calculates the total number of fbpa instances, allocates memory for each
 * instance of error counter, initializes the counter with 0 and the specified
 * string identifier. Finally the counter is added to the stats_list of
 * struct nvgpu_ecc.
 *
 * @return 0 in case of success, less than 0 for failure.
 */
int nvgpu_ecc_counter_init_per_fbpa(struct gk20a *g,
		struct nvgpu_ecc_stat **stat, const char *name);
#define NVGPU_ECC_COUNTER_INIT_PER_FBPA(stat) \
	nvgpu_ecc_counter_init_per_fbpa(g, &g->ecc.fbpa.stat, #stat)

int tu104_fbpa_ecc_init(struct gk20a *g);
void tu104_fbpa_ecc_free(struct gk20a *g);

#endif /* NVGPU_FBPA_TU104_H */
