// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/mm.h>

#include "mm_tu104.h"

u32 tu104_mm_get_flush_retries(struct gk20a *g, enum nvgpu_flush_op op)
{
	u32 retries;

	switch (op) {
	/* TU104 has a large FB so it needs larger timeouts */
	case NVGPU_FLUSH_FB:
		retries = 2500U;
		break;
	case NVGPU_FLUSH_L2_FLUSH:
		retries = 2000U;
		break;
	default:
		retries = 200U; /* Default retry timer */
		break;
	}

	return retries;
}
