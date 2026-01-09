/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_CMD_IF_H
#define NVGPU_SEC2_CMD_IF_H

#include <nvgpu/sec2/lsfm.h>
#include <nvgpu/flcnif_cmn.h>
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_sec2;

struct nv_flcn_cmd_sec2 {
	struct pmu_hdr hdr;
	union {
		union nv_sec2_acr_cmd acr;
	} cmd;
};

/* command handling methods*/
int nvgpu_sec2_cmd_post(struct gk20a *g, struct nv_flcn_cmd_sec2 *cmd,
	u32 queue_id, sec2_callback callback,
	void *cb_param, u32 timeout);

#endif /* NVGPU_SEC2_CMD_IF_H */
