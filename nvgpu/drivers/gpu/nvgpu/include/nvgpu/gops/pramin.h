/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_PRAMIN_H
#define NVGPU_GOPS_PRAMIN_H

struct gops_pramin {
	u32 (*data032_r)(u32 i);
};

#endif /* NVGPU_GOPS_PRAMIN_H */
