/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_FALCON_GB20C_H
#define NVGPU_FALCON_GB20C_H

#include <nvgpu/gk20a.h>

#define FALCON_DEBUGINFO_BOOT_SUCCESS	0xFC10ADEDU
#define GB20C_FECS_RISCV_BOOTPLUGIN                "g_rv_boot_plugin_fecs_gb202.text.bin"
#define GB20C_GPCCS_RISCV_BOOTPLUGIN               "g_rv_boot_plugin_gpccs_gb10b.text.bin"

u32 gb20c_video_boot_done_value(void);
void gb20c_get_bootplugin_fw_name(struct gk20a *g, const char **name, u32 falcon_id);

#endif /* NVGPU_FALCON_GB20C_H */
