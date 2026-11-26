/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_CHECKER_COMMON_DATA_UTILS_H
#define NVGPU_CHECKER_COMMON_DATA_UTILS_H

#include <nvgpu/types.h>

struct nvgpu_mem;
struct gk20a;

struct nvgpu_posix_fault_inj *nvgpu_runlist_store_fault_injection(void);

/**
 * @brief Function to store runlist memory buffer handles for shadow domain.
 *
 * This function stores the runlist mem handles in the nvgpu_checker_data's
 * nvs_domains array.
 *
 * Steps:
 * - Verify that num_runlists in g->fifo is less than or equal to
 *   NVGPU_CHECKER_RUNLISTS_NUM_MAX.
 * - For each runlist store the memory handle for two runlist buffers.
 * - Store the memory handles in the checker domain data using
 *   \ref struct nvgpu_checker_nvs_domain_data corresponding.
 *
 * @param g     The GPU super struct.
 *
 *  @return 0, if runlist memory handles are successfully stored in the
 *  @ref nvgpu_checker_data.
 *  @return -EINVAL, if max number of runlists >= @ref NVGPU_CHECKER_RUNLISTS_NUM_MAX or
 *  if the domain info can't be stored.
 */
int nvgpu_runlist_store_shadow_rl_domain(struct gk20a *g);

/**
 * @brief Function to store runlist mem count to checker struct.
 *
 * This function stores the runlist mem count in the nvgpu_checker_data's
 * nvs_domains array.
 *
 * Steps:
 * - Verify that domain_id and runlist_id are valid.
 * - Store the field count in the checker domain data using
 *   \ref struct nvgpu_checker_nvs_domain_data corresponding to fields
 *   runlist_id and rl_mem_handle.
 *
 * @param domain_id     The domain id.
 *                      Range: [0, U64_MAX]
 * @param runlist_id    The runlist id to store the counts.
 *                      Range: [0, NVGPU_CHECKER_RUNLISTS_NUM_MAX - 1U]
 * @param mem           The runlist mem buffer.
 * @param count         The count of runlist entries.
 *
 *  @return 0, if runlist memory handle and count is successfully stored in the
 *  @ref nvgpu_checker_data.
 *  @return -EINVAL, if the runlist id >= @ref NVGPU_CHECKER_RUNLISTS_NUM_MAX or
 *  if the domain info can't be stored.
 */
int nvgpu_checker_nvs_domains_store_rlmem_count(u64 domain_id,
		u32 runlist_id, struct nvgpu_mem *mem, u32 count);

/**
 * @brief This function removes domain data from the checker struct.
 *
 * This function clears all the domain info of the domain whose domain id
 * is domain_id. To ASIL-D harden our struct, the function after deleting the
 * channel info, updates the CRC of the struct.
 *
 * Steps:
 * - Verify that domain_id is valid.
 * - Mark the domain corresponding to domain_id in the domain data
 *   \ref struct nvgpu_checker_nvs_domain_data as not valid/allocated.
 *
 * @param domain_id The domain id.
 *                  Range: [0, U64_MAX]
 */
void nvgpu_checker_nvs_domains_remove_domain(u64 domain_id);

/**
 * @brief This function removes TSG data from the checker struct.
 *
 * This function clears all the TSG info of the TSG whose TSG id
 * is tsgid.
 *
 * Steps:
 * - Verify that tsgid is valid.
 * - Verify that all channels are unbound.
 * - Mark the tsg corresponding to tsgid in the tsg data
 *   \ref struct nvgpu_checker_tsg_data as not valid/allocated.
 *
 * @param tsgid The TSG id.
 *              Range: [0, NVGPU_CHECKER_CHANNEL_NUM_MAX - 1U]
 */
void nvgpu_checker_tsg_deinit(u32 tsgid);

#endif /* NVGPU_CHECKER_COMMON_DATA_UTILS_H */
