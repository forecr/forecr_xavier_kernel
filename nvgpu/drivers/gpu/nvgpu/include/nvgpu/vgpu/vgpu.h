/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2014-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_VGPU_H
#define NVGPU_VGPU_H

#include <nvgpu/types.h>
#include <nvgpu/utils.h>
#include <nvgpu/thread.h>
#include <nvgpu/log.h>
#include <nvgpu/lock.h>
#include <nvgpu/vgpu/tegra_vgpu.h>

#include "common/vgpu/ivc/comm_vgpu.h"

struct gk20a;
struct vgpu_ecc_stat;

struct vgpu_priv_data {
	u64 virt_handle;
	struct vgpu_ivc_cmd_chs ivc_cmd_chs;
	struct vgpu_ivc ivc_intr;
	struct nvgpu_thread intr_handler;
	struct tegra_vgpu_constants_params constants;
	struct vgpu_ecc_stat *ecc_stats;
	int ecc_stats_count;
	u32 num_freqs;
	unsigned long *freqs;
	struct nvgpu_mutex vgpu_clk_get_freq_lock;
	struct tegra_hv_ivm_cookie *css_cookie;
};

struct vgpu_priv_data *vgpu_get_priv_data(struct gk20a *g);

static inline u64 vgpu_get_handle(struct gk20a *g)
{
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);

	if (unlikely(!priv)) {
		nvgpu_err(g, "invalid vgpu_priv_data in %s", __func__);
		return INT_MAX;
	}

	return priv->virt_handle;
}

#endif /* NVGPU_VGPU_H */
