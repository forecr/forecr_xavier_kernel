// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/io.h>
#include <nvgpu/io.h>

#include "os_linux.h"

u32 nvgpu_os_readl(uintptr_t addr)
{
	return readl((void __iomem *)addr);
}

void nvgpu_os_writel(u32 v, uintptr_t addr)
{
	writel(v, (void __iomem *)addr);
}

void nvgpu_os_writel_relaxed(u32 v, uintptr_t addr)
{
	writel_relaxed(v, (void __iomem *)addr);
}

uintptr_t nvgpu_io_map(struct gk20a *g, uintptr_t addr, size_t size)
{
	return (uintptr_t)devm_ioremap(dev_from_gk20a(g), addr, size);
}

void nvgpu_io_unmap(struct gk20a *g, uintptr_t addr, size_t size)
{
	devm_iounmap(dev_from_gk20a(g), (void __iomem *)addr);
}
