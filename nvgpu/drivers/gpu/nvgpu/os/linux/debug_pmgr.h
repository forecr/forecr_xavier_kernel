/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * SPDX-FileCopyrightText: Copyright (c) 2018-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef __LINUX_DEBUG_PMGR_H
#define __LINUX_DEBUG_PMGR_H

#include "os_linux.h"

#ifdef CONFIG_DEBUG_FS
int nvgpu_pmgr_init_debugfs_linux(struct nvgpu_os_linux *l);
#else
static inline int nvgpu_pmgr_init_debugfs_linux(struct nvgpu_os_linux *l)
{
	return 0;
}
#endif
#endif
