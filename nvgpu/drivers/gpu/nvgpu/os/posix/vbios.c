// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#ifdef CONFIG_NVGPU_GB20C
#include <hal/vbios/bios_gb20c.h>
#endif

#ifdef CONFIG_NVGPU_GB20C
void *nvgpu_gb20c_get_vbios_addr(struct gk20a *g)
{
	u8 *bios_data = NULL;

	bios_data = check_ssmpu_sr_index(g, VBIOS_SR_INDEX);

	if (bios_data != NULL)
		return bios_data;

	return NULL;
}
#endif
