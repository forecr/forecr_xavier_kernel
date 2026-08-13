// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/kmem.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/errno.h>

#include "common/vgpu/ecc_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"

int vgpu_ecc_get_info(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_ecc_info_params *p = &msg.params.ecc_info;
	struct tegra_vgpu_ecc_info_entry *entry;
	struct vgpu_ecc_stat *stats;
	int err, i, count, ivc;
	size_t oob_size;

	msg.cmd = TEGRA_VGPU_CMD_GET_ECC_INFO;
	msg.handle = vgpu_get_handle(g);
	ivc = vgpu_comm_alloc_ivc_oob(g, (void **)&entry, &oob_size);
	if (ivc < 0) {
		return ivc;
	}
	err = vgpu_comm_sendrecv_locked(g, ivc, &msg);
	err = err ? err : msg.ret;
	if (unlikely(err)) {
		nvgpu_err(g, "vgpu get_ecc_info failed, err=%d", err);
		goto out;
	}

	count = p->ecc_stats_count;

	if (unlikely(oob_size < count * sizeof(*entry))) {
		err = -E2BIG;
		goto out;
	}

	stats = nvgpu_kzalloc(g, count * sizeof(*stats));
	if (unlikely(!stats)) {
		err = -ENOMEM;
		goto out;
	}

	for (i = 0; i < count; i++) {
		stats[i].ecc_id = entry[i].ecc_id;
		(void) strncpy(stats[i].name, entry[i].name,
			NVGPU_ECC_STAT_NAME_MAX_SIZE);
	}

	priv->ecc_stats = stats;
	priv->ecc_stats_count = count;
out:
	vgpu_comm_free_ivc(g, ivc);
	return err;
}

void vgpu_ecc_remove_info(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	priv->ecc_stats_count = 0;

	if (priv->ecc_stats) {
		nvgpu_kfree(g, priv->ecc_stats);
		priv->ecc_stats = NULL;
	}
}
