/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef ACR_BLOB_CONSTRUCT_V2_H
#define ACR_BLOB_CONSTRUCT_V2_H

#include <nvgpu/falcon.h>
#include <nvgpu/flcnif_cmn.h>

#include "nvgpu_acr_interface_v2.h"
#include "acr_blob_construct.h"

s32 nvgpu_get_acr_lsf_fw(struct gk20a *g, u32 falcon_id,
		struct flcn_ucode_img *p_img);
int nvgpu_acr_prepare_ucode_blob_v2(struct gk20a *g);

#endif /* ACR_BLOB_CONSTRUCT_V2_H */
