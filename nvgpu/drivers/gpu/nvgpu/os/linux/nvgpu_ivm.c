// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 14, 0)
#include <linux/tegra-ivc.h>
#else
#include <soc/tegra/virt/hv-ivc.h>
#endif

#include "os/linux/os_linux.h"
#include <nvgpu/nvgpu_ivm.h>

int nvgpu_nvlink_prot_update(struct nvgpu_contig_cbcmempool *contig_pool)
{
	return 0;
}

struct tegra_hv_ivm_cookie *nvgpu_ivm_mempool_reserve(unsigned int id)
{
	return tegra_hv_mempool_reserve(id);
}

int nvgpu_ivm_mempool_unreserve(struct tegra_hv_ivm_cookie *cookie)
{
	return tegra_hv_mempool_unreserve(cookie);
}

u64 nvgpu_ivm_get_ipa(struct tegra_hv_ivm_cookie *cookie)
{
	return cookie->ipa;
}

u64 nvgpu_ivm_get_size(struct tegra_hv_ivm_cookie *cookie)
{
	return cookie->size;
}

void *nvgpu_ivm_mempool_map(struct tegra_hv_ivm_cookie *cookie)
{
	return ioremap_cache(nvgpu_ivm_get_ipa(cookie),
				nvgpu_ivm_get_size(cookie));
}

void nvgpu_ivm_mempool_unmap(struct tegra_hv_ivm_cookie *cookie,
		void *addr)
{
	iounmap(addr);
}
