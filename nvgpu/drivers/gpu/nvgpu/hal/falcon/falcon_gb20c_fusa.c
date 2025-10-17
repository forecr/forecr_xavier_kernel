// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "falcon_gb20c.h"

u32 gb20c_video_boot_done_value(void)
{
	return FALCON_DEBUGINFO_BOOT_SUCCESS;
}

void gb20c_get_bootplugin_fw_name(struct gk20a *g, const char **name, u32 falcon_id)
{
	(void) g;
	if (falcon_id == FALCON_ID_FECS_RISCV_EB || falcon_id == FALCON_ID_FECS) {
		*name = GB20C_FECS_RISCV_BOOTPLUGIN;
	} else if (falcon_id == FALCON_ID_GPCCS_RISCV_EB || falcon_id == FALCON_ID_GPCCS) {
		*name = GB20C_GPCCS_RISCV_BOOTPLUGIN;
	}
}
