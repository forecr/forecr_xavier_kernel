// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gr/gr_ecc.h>
#include <nvgpu/ltc.h>
#include <nvgpu/string.h>

void nvgpu_ecc_stat_add(struct gk20a *g, struct nvgpu_ecc_stat *stat)
{
	struct nvgpu_ecc *ecc = &g->ecc;

	nvgpu_init_list_node(&stat->node);

	nvgpu_mutex_acquire(&ecc->stats_lock);

	nvgpu_list_add_tail(&stat->node, &ecc->stats_list);
	ecc->stats_count = nvgpu_safe_add_s32(ecc->stats_count, 1);

	nvgpu_mutex_release(&ecc->stats_lock);
}

void nvgpu_ecc_stat_del(struct gk20a *g, struct nvgpu_ecc_stat *stat)
{
	struct nvgpu_ecc *ecc = &g->ecc;

	nvgpu_mutex_acquire(&ecc->stats_lock);

	nvgpu_list_del(&stat->node);
	ecc->stats_count = nvgpu_safe_sub_s32(ecc->stats_count, 1);

	nvgpu_mutex_release(&ecc->stats_lock);
}

int nvgpu_ecc_counter_init(struct gk20a *g,
		struct nvgpu_ecc_stat **statp, const char *name)
{
	struct nvgpu_ecc_stat *stat;

	stat = nvgpu_kzalloc(g, sizeof(*stat));
	if (stat == NULL) {
		nvgpu_err(g, "ecc counter alloc failed");
		return -ENOMEM;
	}

	(void)strncpy(stat->name, name, NVGPU_ECC_STAT_NAME_MAX_SIZE - 1U);
	nvgpu_ecc_stat_add(g, stat);
	*statp = stat;
	return 0;
}

void nvgpu_ecc_counter_deinit(struct gk20a *g, struct nvgpu_ecc_stat **statp)
{
	struct nvgpu_ecc_stat *stat;

	if (*statp == NULL) {
		return;
	}

	stat = *statp;

	nvgpu_ecc_stat_del(g, stat);
	nvgpu_kfree(g, stat);
	*statp = NULL;
}

/* release all ecc_stat */
void nvgpu_ecc_free(struct gk20a *g)
{
	struct nvgpu_ecc *ecc = &g->ecc;

	nvgpu_gr_ecc_free(g);
	nvgpu_ltc_ecc_free(g);

	if (g->ops.fb.ecc.ecc_counter_free != NULL) {
		g->ops.fb.ecc.ecc_counter_free(g);
	}

#ifdef CONFIG_NVGPU_DGPU
	if (g->ops.fb.fbpa_ecc_free != NULL) {
		g->ops.fb.fbpa_ecc_free(g);
	}
#endif

	if (g->ops.pmu.ecc_free != NULL) {
		g->ops.pmu.ecc_free(g);
	}

	nvgpu_mutex_acquire(&ecc->stats_lock);
	WARN_ON(!nvgpu_list_empty(&ecc->stats_list));
	nvgpu_mutex_release(&ecc->stats_lock);

	(void)memset(ecc, 0, sizeof(*ecc));
}

int nvgpu_ecc_init_support(struct gk20a *g)
{
	struct nvgpu_ecc *ecc = &g->ecc;

	if (ecc->initialized) {
		return 0;
	}

	nvgpu_mutex_init(&ecc->stats_lock);
	nvgpu_init_list_node(&ecc->stats_list);

	return 0;
}

/**
 * Note that this function is to be called after all units requiring ecc stats
 * have added entries to ecc->stats_list.
 */
int nvgpu_ecc_finalize_support(struct gk20a *g)
{
#ifdef CONFIG_NVGPU_SYSFS
	int err;
#endif

	if (g->ecc.initialized) {
		return 0;
	}

#ifdef CONFIG_NVGPU_SYSFS
	if (!nvgpu_is_enabled(g, NVGPU_DISABLE_ECC_STATS)) {
		err = nvgpu_ecc_sysfs_init(g);
		if (err != 0) {
			nvgpu_ecc_free(g);
			return err;
		}
	}
#endif

	g->ecc.initialized = true;

	return 0;
}

void nvgpu_ecc_remove_support(struct gk20a *g)
{
	if (!g->ecc.initialized) {
		return;
	}

#ifdef CONFIG_NVGPU_SYSFS
	if (!nvgpu_is_enabled(g, NVGPU_DISABLE_ECC_STATS)) {
		nvgpu_ecc_sysfs_remove(g);
	}
#endif
	nvgpu_ecc_free(g);

	nvgpu_mutex_destroy(&g->ecc.stats_lock);
}
