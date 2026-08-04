/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_IVM_H
#define NVGPU_IVM_H

#include <nvgpu/types.h>

/* forward declarations */
struct tegra_hv_ivm_cookie;
struct nvgpu_contig_cbcmempool;

/**
 *
 * @brief This function is used to set protection table maintained by
 *  hypervisor for physically contiguous memory partition.
 *
 * NVGPU compression needs physically contiguous memory for
 * compgtags allocation. The physically contiguous memory partition
 * configured via device tree entry.
 * Call #nvgpu_ivm_get_ipa get the intermediate physical
 *  address for the contig pool.
 * Get the contig pool size by calling #nvgpu_ivm_get_size.
 * Update and synchronise the protection table by calling
 *  NvHvNvlinkUpdateProt() and NvHvNvlinkSync().
 *
 * @param contig_pool [in]      Pointer to the #nvgpu_contig_cbcmempool.
 *
 * @return 0 on success, < 0 in case of failure.
 * @retval -EIO for NvHvNvlinkUpdateProt failures.
 */
int nvgpu_nvlink_prot_update(struct nvgpu_contig_cbcmempool *contig_pool);

/**
 * @brief This function is used to reserve the pool for GPU use case.
 *
 * Call the hypervisor API tegra_hv_mempool_reserve() to
 *  reserve the pool. This is a wrapper over tegra_hv_mempool_reserve()
 *  API.

 * @param id [in]	id of the IVM pool.
 *
 * @return pointer to the pool on success, NULL in case of failure.
 */
struct tegra_hv_ivm_cookie *nvgpu_ivm_mempool_reserve(unsigned int id);

/**
 * @brief This function is used to unreserve the pool from GPU use case.
 *
 * Call the hypervisor API tegra_hv_mempool_unreserve() to
 *  remove the pool reservation. This is a wrapper over tegra_hv_mempool_unreserve()
 *  API.

 * @param cookie [in]       pointer to the IVM pool.
 *
 * @return 0 on success, errors in case of failure.
 */
int nvgpu_ivm_mempool_unreserve(struct tegra_hv_ivm_cookie *cookie);

/**
 * @brief This function is used to get the Intermediate Physical Address
 *  of the  the IVM pool.
 *
 * @param cookie [in]       Pointer to the tegra_hv_ivm_cookie structure.
 *
 * @return IPA of the pool.
 */
u64 nvgpu_ivm_get_ipa(struct tegra_hv_ivm_cookie *cookie);

/**
 * @brief This function is used to get the Intermediate Physical Address
 *  of the  the IVM pool.
 *
 * @param cookie [in]       Pointer to the tegra_hv_ivm_cookie structure.
 *
 * @return size of the pool.
 */
u64 nvgpu_ivm_get_size(struct tegra_hv_ivm_cookie *cookie);

/**
 * @brief This function is used to map the pool for GPU use case.
 *
 * Call the hypervisor API tegra_hv_mempool_map() to
 *  map the pool. This is a wrapper over tegra_hv_mempool_map()
 *  API.

 * @param cookie [in]       Pointer to the tegra_hv_ivm_cookie structure.
 *
 * @return None.
 */
void *nvgpu_ivm_mempool_map(struct tegra_hv_ivm_cookie *cookie);

/**
 * @brief This function is used to unmap the pool from GPU use case.
 *
 * Call the hypervisor API tegra_hv_mempool_unmap() to
 *  map the pool. This is a wrapper over tegra_hv_mempool_unmap()
 *  API.

 * @param cookie [in]       Pointer to the tegra_hv_ivm_cookie structure.
 *
 * @return None.
 */
void nvgpu_ivm_mempool_unmap(struct tegra_hv_ivm_cookie *cookie,
		void *addr);

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_ivm_reserve_get_fault_injection(void);
struct nvgpu_posix_fault_inj *nvgpu_ivm_unreserve_get_fault_injection(void);
struct nvgpu_posix_fault_inj *nvgpu_ivm_map_get_fault_injection(void);
struct nvgpu_posix_fault_inj *nvgpu_nvlink_prot_get_fault_injection(void);
struct nvgpu_posix_fault_inj *nvgpu_ivm_unmap_get_fault_injection(void);
#endif

#endif /* NVGPU_VGPU_IVM_H */
