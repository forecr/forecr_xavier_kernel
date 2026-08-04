// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include "ctxsw_prog_gb10b.h"

#include <nvgpu/hw/gb10b/hw_ctxsw_prog_gb10b.h>

#ifdef CONFIG_NVGPU_DEBUGGER
u32 gb10b_ctxsw_prog_hw_get_gpccs_header_stride(void)
{
	return ctxsw_prog_gpccs_header_size_in_bytes_v();
}
#endif
