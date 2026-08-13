/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RUNLIST_RAM_GB20C_H
#define NVGPU_RUNLIST_RAM_GB20C_H

#include <nvgpu/types.h>

u32 gb20c_runlist_get_ch_entry_veid(u32 value);
u32 gb20c_chan_userd_ptr_hi(u32 addr_hi);

#endif /* NVGPU_RUNLIST_RAM_GB20C_H */
