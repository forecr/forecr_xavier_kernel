/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _ECC_VGPU_H_
#define _ECC_VGPU_H_

#include <nvgpu/types.h>
#include <nvgpu/ecc.h>	/* For NVGPU_ECC_STAT_NAME_MAX_SIZE */

struct gk20a;

struct vgpu_ecc_stat {
	u32 ecc_id;
	char name[NVGPU_ECC_STAT_NAME_MAX_SIZE + 1];
};

int vgpu_ecc_get_info(struct gk20a *g);
void vgpu_ecc_remove_info(struct gk20a *g);

#endif
