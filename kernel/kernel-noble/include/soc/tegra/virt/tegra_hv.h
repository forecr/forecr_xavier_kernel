/*
 * SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2025-2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef TEGRA_HV_H
#define TEGRA_HV_H

/**
 * @defgroup hypervisor_ivc_framework Hypervisor IVC Framework
 * @{
 */

#include <linux/types.h>
#include <soc/tegra/virt/syscalls.h>

/** @brief Supports trap notification to peer end via MSI irqs */
#define SUPPORTS_TRAP_MSI_NOTIFICATION

/** @brief Page size of IVC meta data queried from hypervisor */
#define IVC_INFO_PAGE_SIZE 65536

/** @brief Maximum Guest VM count */
#define MAX_NUM_GUESTS		16U
/** @brief The maximum number of IVC queues supported by the PCT. */
#define PCT_MAX_NUM_IVC_QUEUES	800U
/** @brief The maximum number of mempools supported by the PCT. */
#define PCT_MAX_NUM_MEMPOOLS	120U

/** @brief structure representing mempool cookie in hypervisor driver */
struct tegra_hv_ivm_cookie {
	/** @brief mempool base ipa address */
	uint64_t ipa;
	/** @brief mempool size */
	uint64_t size;
	/** @brief vmid of the peer */
	unsigned int peer_vmid;
	/** @brief reserved */
	void *reserved;
};

/**
 * @brief          Checks whether platform supports virtualization or not
 *
 * @retval         true If platform supports virtualization else false.
 *
 * @pre
 *                 - Tegra hypervisor driver should have been initialized.
 *
 * @post
 *                 - Client can take runtime decision in driver if OS is running in native environment or in virtualized environment.
 *
 * @usage
 *                 - Call this function to check if OS is running in native environment or in virtualized environment.
 *                 - Allowed context for the API call
 *                   - Interrupt handler: Yes
 *                   - Signal handler: N/A
 *                   - Thread-safe: Yes
 *                   - Async/Sync: Sync
 *                   - Re-entrant: No
 *                 - API Group
 *                   - Init: No
 *                   - Runtime: Yes
 *                   - De-Init: No
 */
bool is_tegra_hypervisor_mode(void);

/**
 * @brief          Reserve a mempool for use
 * @param[in]      id Id of the requested mempool.
 *
 * @retval         ivck Returns a cookie representing the mempool on success, otherwise an ERR_PTR.
 *
 * @pre
 *                 - Tegra hypervisor driver should have been initialized.
 *                 - This API should be invoked on virtual/hypervisor environment only.
 *
 * @post
 *                 - Reserved mempool will be available for I/O operations.
 *
 * @usage
 *                 - Mempool will be available for data transfer with peer end.
 *                 - Allowed context for the API call
 *                   - Interrupt handler: Yes
 *                   - Signal handler: N/A
 *                   - Thread-safe: No
 *                   - Async/Sync: Sync
 *                   - Re-entrant: No
 *                 - API Group
 *                   - Init: No
 *                   - Runtime: Yes
 *                   - De-Init: No
 */
struct tegra_hv_ivm_cookie *tegra_hv_mempool_reserve(unsigned int id);

/**
 * @brief          Release a reserved mempool
 * @param[in]      ivck IVC cookie of the queue
 *
 * @retval         ret 0 On success or a negative error code otherwise.
 *
 * @pre
 *                 - Tegra hypervisor driver should have been initialized.
 *                 - This API should be invoked on virtual/hypervisor environment only.
 *
 * @post
 *                 - Unreserved mempool will not be available for I/O operations.
 *
 * @usage
 *                 - Mempool will not be available for data transfer available for data transfer.
 *                 - Allowed context for the API call
 *                   - Interrupt handler: Yes
 *                   - Signal handler: N/A
 *                   - Thread-safe: No
 *                   - Async/Sync: Sync
 *                   - Re-entrant: No
 *                 - API Group
 *                   - Init: No
 *                   - Runtime: Yes
 *                   - De-Init: No
 */
int tegra_hv_mempool_unreserve(struct tegra_hv_ivm_cookie *ivck);

/**
 * @brief          Query IVC meta data from hypervisor
 *
 * @retval         ptr To ivc_info_page structure having ivc meta data info
 *                 shared by hypervisor
 *
 * @pre
 *                 - Tegra hypervisor driver should have been initialized.
 *                 - This API should be invoked on virtual/hypervisor environment only.
 *
 * @post
 *                 - IVC meta data will be retuned to client
 *
 * @usage
 *                 - Allowed context for the API call
 *                   - Interrupt handler: Yes
 *                   - Signal handler: N/A
 *                   - Thread-safe: No
 *                   - Async/Sync: Sync
 *                   - Re-entrant: No
 *                 - API Group
 *                   - Init: No
 *                   - Runtime: Yes
 *                   - De-Init: No
 */
const struct ivc_info_page *tegra_hv_get_ivc_info(void);

/**
 * @brief          Query vmid of the GOS VM from hypervisor
 *
 * @retval         vmid On success else -ve value
 *
 * @pre
 *                 - Tegra hypervisor driver should have been initialized.
 *                 - This API should be invoked on virtual/hypervisor environment only.
 *
 * @post
 *                 - Guest VM ID will be returned.
 *
 * @usage
 *                 - Allowed context for the API call
 *                   - Interrupt handler: Yes
 *                   - Signal handler: N/A
 *                   - Thread-safe: No
 *                   - Async/Sync: Sync
 *                   - Re-entrant: No
 *                 - API Group
 *                   - Init: No
 *                   - Runtime: Yes
 *                   - De-Init: No
 */
int tegra_hv_get_vmid(void);

/** @} */

#endif /* TEGRA_HV_H */
