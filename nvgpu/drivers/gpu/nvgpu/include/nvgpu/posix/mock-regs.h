/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_MOCK_REGS_H
#define NVGPU_POSIX_MOCK_REGS_H

#include <nvgpu/types.h>

struct gk20a;

struct nvgpu_mock_iospace {
	u32             base;
	size_t          size;
	const uint32_t *data;
};

#ifndef NVGPU_MON_PRESENT
#define MOCK_REGS_FB		0U
#define MOCK_REGS_GR		1U
#define MOCK_REGS_FUSE		2U
#define MOCK_REGS_MASTER	3U
#define MOCK_REGS_TOP		4U
#define MOCK_REGS_FIFO		5U
#define MOCK_REGS_PRI		6U
#define MOCK_REGS_PBDMA		7U
#define MOCK_REGS_CCSR		8U
#define MOCK_REGS_USERMODE	9U
#define MOCK_REGS_RUNLIST1	10U
#define MOCK_REGS_RUNLIST2	11U
#define MOCK_REGS_RUNLIST3	12U
#define MOCK_REGS_RUNLIST4	13U
#define MOCK_REGS_CHRAM1	14U
#define MOCK_REGS_CHRAM2	15U
#define MOCK_REGS_CHRAM3	16U
#define MOCK_REGS_CHRAM4	17U
#define MOCK_REGS_LTC		18U
#define MOCK_REGS_PGSP		19U
#define MOCK_REGS_FUNC		20U
#define MOCK_REGS_CE		21U
#define MOCK_REGS_PBUS		22U
#define MOCK_REGS_HSHUB		23U
#define MOCK_REGS_CTRL		24U
#define MOCK_REGS_LAST		25U
#else
#define MOCK_REGS_GR		0U
#define MOCK_REGS_MASTER	1U
#define MOCK_REGS_PRI		2U
#define MOCK_REGS_LTC		3U
#define MOCK_REGS_PGSP		4U
#define MOCK_REGS_FUNC		5U
#define MOCK_REGS_LAST		6U
#endif

/**
 * Load a mocked register list into the passed IO space description.
 */
int nvgpu_get_mock_reglist_gv11b(struct gk20a *g, u32 reg_space,
			   struct nvgpu_mock_iospace *iospace);
int nvgpu_get_mock_reglist_ga10b(struct gk20a *g, u32 reg_space,
			   struct nvgpu_mock_iospace *iospace);

#endif
