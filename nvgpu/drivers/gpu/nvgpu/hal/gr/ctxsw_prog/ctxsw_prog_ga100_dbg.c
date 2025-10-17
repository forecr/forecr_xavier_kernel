// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/utils.h>

#include "ctxsw_prog_ga100.h"

#include <nvgpu/hw/ga100/hw_ctxsw_prog_ga100.h>

#ifdef CONFIG_NVGPU_DEBUGGER
u32 ga100_ctxsw_prog_hw_get_gpccs_header_size(void)
{
	return ctxsw_prog_gpccs_header_stride_v();
}

bool ga100_ctxsw_prog_check_main_image_header_magic(u32 *context)
{
	u32 magic_1, magic_2, magic_3;

	magic_1 = *(context + (ctxsw_prog_main_image_magic_value_1_o() >> 2));
	magic_2 = *(context + (ctxsw_prog_main_image_magic_value_2_o() >> 2));
	magic_3 = *(context + (ctxsw_prog_main_image_magic_value_3_o() >> 2));

	if (magic_1 != ctxsw_prog_main_image_magic_value_1_v_value_v()) {
		return false;
	}
	if (magic_2 != ctxsw_prog_main_image_magic_value_2_v_value_v()) {
		return false;
	}
	if (magic_3 != ctxsw_prog_main_image_magic_value_3_v_value_v()) {
		return false;
	}
	return true;
}

bool ga100_ctxsw_prog_check_local_header_magic(u32 *context)
{
	u32 magic_1, magic_2, magic_3;

	magic_1 = *(context + (ctxsw_prog_local_magic_value_1_o() >> 2));
	magic_2 = *(context + (ctxsw_prog_local_magic_value_2_o() >> 2));
	magic_3 = *(context + (ctxsw_prog_local_magic_value_3_o() >> 2));

	if (magic_1 != ctxsw_prog_local_magic_value_1_v_value_v()) {
		return false;
	}
	if (magic_2 != ctxsw_prog_local_magic_value_2_v_value_v()) {
		return false;
	}
	if (magic_3 != ctxsw_prog_local_magic_value_3_v_value_v()) {
		return false;
	}
	return true;
}

#endif /* CONFIG_NVGPU_DEBUGGER */
