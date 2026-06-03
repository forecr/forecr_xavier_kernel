/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_CTC_CHI_H
#define NVGPU_GOPS_CTC_CHI_H

#include <nvgpu/types.h>

struct gk20a;

/**
 * This structure stores common.ctcchi unit hal pointers.
 */
struct gops_ctc_chi {
	int (*init_hw)(struct gk20a *g);
};

#endif /* NVGPU_GOPS_CTC_CHI_H */
