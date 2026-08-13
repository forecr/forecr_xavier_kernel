// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bitops.h>

#include "mm_gm20b.h"

bool gm20b_mm_is_bar1_supported(struct gk20a *g)
{
	(void)g;
	return true;
}

void gm20b_mm_get_default_va_sizes(u64 *aperture_size,
			u64 *user_size, u64 *kernel_size)
{
	/*
	 * The maximum GPU VA range supported.
	 * Max VA Bits = 40, refer dev_mmu.ref.
	 */
	if (aperture_size != NULL) {
		*aperture_size = BIT64(40);
	}

	/*
	 * The default userspace-visible GPU VA size.
	 */
	if (user_size != NULL) {
		*user_size = BIT64(37);
	}

	/*
	 * The default kernel-reserved GPU VA size.
	 */
	if (kernel_size != NULL) {
		*kernel_size = BIT64(32);
	}
}
