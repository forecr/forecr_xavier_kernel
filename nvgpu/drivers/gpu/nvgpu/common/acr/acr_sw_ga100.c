// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/falcon.h>

#include "common/acr/acr_priv.h"
#include "common/acr/acr_sw_tu104.h"
#include "acr_sw_ga100.h"

static u32* ga100_get_versioned_sig(struct gk20a *g, struct nvgpu_acr *acr,
	u32 *sig, u32 *sig_size)
{
	u32 ucode_version = 0U;
	u32 sig_size_words = 0U;
	u32 sig_idx = 0;

	nvgpu_log_fn(g, " ");

	g->ops.fuse.read_ucode_version(g, FALCON_ID_SEC2, &ucode_version);

	*sig_size = *sig_size/acr->num_of_sig;

	sig_idx = (!ucode_version) ? 1U : 0U;

	sig_size_words = *sig_size/4U;

	sig = sig + nvgpu_safe_mult_u32(sig_idx, sig_size_words);

	return sig;
}

void nvgpu_ga100_acr_sw_init(struct gk20a *g, struct nvgpu_acr *acr)
{
	nvgpu_log_fn(g, " ");

	acr->num_of_sig = 2U;
	nvgpu_tu104_acr_sw_init(g, acr);
	acr->get_versioned_sig = ga100_get_versioned_sig;
}
