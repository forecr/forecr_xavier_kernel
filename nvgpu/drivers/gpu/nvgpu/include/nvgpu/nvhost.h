/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVHOST_H
#define NVGPU_NVHOST_H

#include <nvgpu/types.h>
#include <nvgpu/enabled.h>

struct gk20a;
#define NVGPU_INVALID_SYNCPT_ID	(~U32(0U))

#ifdef CONFIG_TEGRA_GK20A_NVHOST

struct nvgpu_nvhost_dev;

/**
 * @file Functions that initialize the sync points
 *  and describe other functionalities.
 */

/**
 * @brief Initializes the nvhost device for the GPU.
 *
 * This function allocates and configures the nvhost device based on the GPU's
 * chip ID. It sets up the number of hardware sync points and initializes the
 * nvhost device with the appropriate configuration. If any step fails, it
 * deallocates any allocated resources and returns an error.
 *
 * This is required for sync point shim read operations.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for the nvhost device structure.
 * -# If the allocation fails, return an out-of-memory error code.
 * -# Set the GPU pointer in the nvhost device structure.
 * -# Based on the chip ID, configure the number of hardware sync points via
 *    'nvgpu_nvhost_syncpt_nb_hw_pts()' and the nvhost device's base address and size via
 *    'nvgpu_nvhost_set_aperture()'.
 * -# If the chip ID is invalid or configuration fails, deallocate the nvhost
 *    device structure and return an error code.
 * -# Initialize the mutex for sync point access synchronization.
 * -# Allocate internal resources for the nvhost device via 'nvgpu_nvhost_device_alloc_internal()'.
 * -# If the internal resource allocation fails, deallocate the nvhost device
 *    and any other allocated resources, then return an out-of-memory error code.
 *
 * @param [in]  g  The GPU super structure.
 *
 * @return 0 if the nvhost device was successfully initialized.
 * @return -ENOMEM if memory allocation failed.
 * @return -EINVAL if an invalid chip ID was encountered.
 */
int nvgpu_get_nvhost_dev(struct gk20a *g);

/**
 * @brief Frees the nvhost device and associated resources.
 *
 * This function is responsible for releasing all resources associated with the
 * nvhost device within the GPU super structure. It performs cleanup of various
 * allocated memory blocks and handles, and resets the nvhost device pointer to NULL.
 *
 * Frees the different fields of nvhost device initialized by nvgpu_get_nvhost_dev().
 *
 * The steps performed by the function are as follows:
 * -# If the host1x handle is not zero, close the host1x handle using the
 *    NvRmHost1xClose() function.
 * -# Free the memory allocated for the channel ID using nvgpu_kfree().
 * -# Free the memory allocated for the sync point names using nvgpu_kfree().
 * -# Free the memory allocated for the sync point pointer using nvgpu_kfree().
 * -# If kernel mode submit is configured, free the memory allocated for the waiter
 *    pointer using nvgpu_kfree().
 * -# Free the shim mapping using nvgpu_shimmap_free().
 * -# Destroy the sync point mutex using nvgpu_mutex_destroy().
 * -# Free the memory allocated for the nvhost device using nvgpu_kfree().
 * -# Set the nvhost device pointer in the GPU super structure to NULL.
 *
 * @param [in] g  The GPU super structure.
 *
 * @return None.
 */
void nvgpu_free_nvhost_dev(struct gk20a *g);

/**
 * @brief Check if the gpu has access to syncpoints.
 *
 * @param g [in]	The GPU super structure.
 *
 * @return		whether syncpt access is available.
 *
 * @retval		TRUE For syncpoint support.
 *
 */
static inline bool nvgpu_has_syncpoints(struct gk20a *g)
{
	return nvgpu_is_enabled(g, NVGPU_HAS_SYNCPOINTS);
}

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
/**
 * Available waiter index is used for sync point wait path.
 * Maximum supported waiter count is 2U.
 */
#define NVGPU_NVHOST_DEFAULT_WAITER		0U
#define NVGPU_NVHOST_WAITER_1			1U
#define NVGPU_NVHOST_MAX_WAITER_COUNT		2U

int nvgpu_nvhost_module_busy_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev);
void nvgpu_nvhost_module_idle_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev);

void nvgpu_nvhost_debug_dump_device(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev);

int nvgpu_nvhost_intr_register_notifier(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 thresh, void (*callback)(void *priv, int nr_completed),
	void *private_data);

bool nvgpu_nvhost_syncpt_is_expired_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 thresh);
int nvgpu_nvhost_syncpt_wait_timeout_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 thresh, u32 timeout, u32 waiter_index);

int nvgpu_nvhost_create_symlink(struct gk20a *g);
void nvgpu_nvhost_remove_symlink(struct gk20a *g);

#endif

/**
 * @brief External interface to read the current value of a given sync point.
 *
 * This function attempts to read the current value of a sync point specified by
 * its ID and stores the value in the provided output parameter. The function
 * is also designed to be used in a testing environment with fault injection enabled.
 * The steps performed by the function are as follows:
 * -# Check if unit testing and fault injection are enabled. If a fault is
 *    injected, return an error code.
 * -# Read the sync point value using the sync point ID and store the result in
 *    the provided output parameter by calling nvgpu_nvhost_syncpt_read().
 * -# Return success.
 *
 * @param [in]  nvgpu_syncpt_dev  The sync point device structure.
 * @param [in]  id                The ID of the sync point to read.
 *                                Range: [0, nvgpu_nvhost_syncpt_nb_hw_pts() - 1]
 * @param [out] val               Pointer to store the read sync point value.
 *                                Range: Pointer to a valid memory location.
 *
 * @return 0  On success, negative error code on failure.
 *
 * @retval 0  Success.
 * @retval -1 Fault injection triggered an error (only in unit testing).
 */
int nvgpu_nvhost_syncpt_read_ext_check(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 *val);

/**
 * @brief Retrieves the name of a sync point based on its ID.
 *
 * The function checks if the sync point ID is valid and if so, returns the name
 * associated with that sync point ID. The name is retrieved from an array of
 * sync point names nvgpu_nvhost_dev.syncpt_names indexed by the sync point ID.
 *
 * The steps performed by the function are as follows:
 * -# Check if the sync point ID is valid by calling the function
 *    'nvgpu_nvhost_syncpt_is_valid_pt_ext()'.
 * -# If the sync point ID is valid, retrieve the sync point name from the array
 *    of sync point names using the sync point ID as the index.
 * -# Return the retrieved sync point name.
 *
 * @param [in] nvgpu_syncpt_dev The device structure for the sync point.
 * @param [in] id               The ID of the sync point.
 *                              Range: [0, nvgpu_nvhost_syncpt_nb_hw_pts() - 1]
 *
 * @return The name of the sync point if the ID is valid, otherwise NULL.
 */
const char *nvgpu_nvhost_syncpt_get_name(
	struct nvgpu_nvhost_dev *nvgpu_syncpt_dev, int id);

#ifdef CONFIG_NVGPU_KERNEL_MODE_SUBMIT
/**
 * @brief Increment the value of given sync point to the maximum value.
 * The function is needed to initialise the maximum value to the allocated
 * sync point.
 *
 * @param nvgpu_syncpt_dev [in]	Sync point device.
 * @param id [in]		Sync point id.
 * @param val [in]		Final desired syncpt value.
 *
 * - Read the current value of the given sync point by calling
 *   #NvRmHost1xSyncpointRead().
 * - If val is less than current, increment the syncpoint
 *   by the difference(val - current) by calling
 *   #nvgpu_nvhost_syncptshim_map_increment().
 *
 * Note that this can race and cause the syncpt value to go over the desired
 * value if some other entity (such as the gpu hardware) is incrementing the
 * syncpoint concurrently.
 *
 * @return			None.
 */
void nvgpu_nvhost_syncpt_set_minval(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id, u32 val);
#endif

/**
 * @brief Set the syncpoint to a safe state.
 *
 * This function increments the value of a given syncpoint by a large number to
 * ensure that all waiters on this syncpoint are released. It is intended to be
 * used in situations where an application unexpectedly crashes, to prevent
 * indefinite waiting on the syncpoint. The function behaves differently based
 * on the build configuration. For non-safety builds, it performs the increment;
 * for safety builds, this function does nothing to avoid potential safety issues.
 *
 * For Non-Safety build configuration, the steps performed by the function are as follows:
 * -# Acquire the mutex lock for syncpoint operations.
 * -# Check if the syncpoint name for the given ID is valid by calling
 *    'nvgpu_nvhost_syncpt_get_name()'. If not valid, release the mutex and
 *    return early.
 * -# Increment the syncpoint value by a predefined large number (256U) to
 *    release any waiters. The actual increment operation depends on whether
 *    the shim map is supported. If supported, call
 *    'nvgpu_nvhost_syncptshim_map_increment()' with the GPU and the virtual
 *    address of the syncpoint. Otherwise, call 'NvRmHost1xSyncpointIncrement()'
 *    with the syncpoint pointer and the increment value.
 * -# Release the mutex lock for syncpoint operations.
 *
 * For Safety build configuration, the steps performed by the function are as follows:
 * -# None (no operation in Safety).
 *
 * @param [in] nvgpu_syncpt_dev The syncpoint device structure.
 * @param [in] id               The ID of the syncpoint to set to a safe state.
 *                              Range: [0, nvgpu_nvhost_syncpt_nb_hw_pts() - 1]
 *
 * @return None.
 */
void nvgpu_nvhost_syncpt_set_safe_state(
	struct nvgpu_nvhost_dev *nvgpu_syncpt_dev, u32 id);

/**
 * @brief Check if the given sync point id is a valid hardware sync point.
 *
 * This function delegates the check to another function that specifically
 * verifies the validity of a hardware sync point id.
 *
 * The steps performed by the function are as follows:
 * -# Call the function 'nvgpu_nvhost_syncpt_is_valid_hw_pt()' with the sync
 *    point device and sync point id to determine if the id is a valid hardware
 *    sync point.
 *
 * @param [in] nvgpu_syncpt_dev  The sync point device.
 * @param [in] id                The sync point id to be checked.
 *                               Range: [0, nvgpu_nvhost_syncpt_nb_hw_pts() - 1]
 *
 * @return True if the sync point id is valid, False otherwise.
 */
bool nvgpu_nvhost_syncpt_is_valid_pt_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id);

/**
 * @brief Release a reference to a sync point.
 *
 * This function is responsible for releasing a previously acquired reference to a sync point
 * in the Host1x device. It is a wrapper for the nvhost_free_syncpt() function, which is used
 * to free the sync point.
 *
 * The steps performed by the function are as follows:
 * -# Call nvhost_free_syncpt() with the Host1x device and sync point ID to release the sync point.
 *
 * @param [in] nvgpu_syncpt_dev  The Host1x device structure.
 * @param [in] id                The sync point ID to be released.
 *
 * @note There is no return value for this function.
 */
void nvgpu_nvhost_syncpt_put_ref_ext(struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
	u32 id);

/**
 * @brief Allocate a client-managed sync point.
 *
 * This function allocates a sync point that is managed by the client. It is
 * used to obtain a unique sync point ID that can be used for synchronization
 * purposes between different hardware blocks or between hardware and software.
 *
 * The steps performed by the function are as follows:
 * -# Call the function 'nvgpu_nvhost_allocate_syncpoint()' with the provided
 *    sync point device and sync point name, and a flag value of 0.
 *
 * @param [in]  nvgpu_syncpt_dev  The sync point device for which a sync point is to be allocated.
 * @param [in]  syncpt_name       The name of the sync point to be allocated.
 *
 * @return The allocated sync point ID, or 0 if allocation fails.
 */
u32 nvgpu_nvhost_get_syncpt_client_managed(struct nvgpu_nvhost_dev
	*nvgpu_syncpt_dev,
	const char *syncpt_name);

/**
 * @brief Retrieves the base address and size of the memory-mapped sync point unit region
 *        (MSS).
 *
 * The function retrieves the base address and size of the memory-mapped sync point unit
 * region from the provided sync point device structure. It checks for valid input
 * pointers and then assigns the base address and size to the provided output parameters.
 *
 * This function is needed for sync point manipulation.
 *
 * The steps performed by the function are as follows:
 * -# Assign the base address of the sync point unit region to the output parameter.
 * -# Assign the size of the sync point unit region to the output parameter.
 * -# Return success code.
 *
 * @param [in]  nvgpu_syncpt_dev  The sync point device structure containing the base
 *                                address and size of the sync point unit region.
 *                                Range: Valid pointer to nvgpu_nvhost_dev structure.
 * @param [out] base              Pointer to store the base address of the sync point unit
 *                                region.
 *                                Range: Valid pointer to u64.
 * @param [out] size              Pointer to store the size of the sync point unit region.
 *                                Range: Valid pointer to size_t.
 *
 * @return 0 on success.
 * @return -ENOSYS if any input pointer is NULL.
 */
int nvgpu_nvhost_get_syncpt_aperture(
		struct nvgpu_nvhost_dev *nvgpu_syncpt_dev,
		u64 *base, size_t *size, bool is_second_die);

/**
 * @brief Calculates the byte offset of a sync point based on its ID.
 *
 * This function determines the byte offset of a sync point within the sync point
 * unit interface memory region. The offset is calculated by multiplying the sync
 * point ID with a stride size that is dependent on the chip ID.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the operating system specific structure from the GPU super structure.
 * -# Obtain the module structure from the operating system specific structure.
 * -# Determine the stride size based on the chip ID:
 * -# Calculate the byte offset by multiplying the sync point ID with the determined
 *    stride size using the function 'nvgpu_safe_mult_u32()'.
 *
 * @param [in] g          The GPU super structure.
 * @param [in] syncpt_id  The sync point ID for which to calculate the byte offset.
 *                        Range: [0, nvgpu_nvhost_syncpt_nb_hw_pts() - 1]
 *
 * @return The byte offset of the sync point within the sync point unit interface
 *         memory region.
 */
u32 nvgpu_nvhost_syncpt_unit_interface_get_byte_offset(struct gk20a *g,
		u32 syncpt_id);

u64 nvgpu_nvhost_compute_os_specific_syncpt_addr(struct gk20a *g,
		u32 syncpt_id);

#ifdef __KERNEL__
int nvgpu_nvhost_syncpt_init(struct gk20a *g);
#endif

#else /* CONFIG_TEGRA_GK20A_NVHOST */
/**
 * @brief Check if the gpu has access to syncpoints.
 *
 * @param g [in]	The GPU super structure.
 *
 * @return		whether syncpt access is available
 */
static inline bool nvgpu_has_syncpoints(struct gk20a *g)
{
	return false;
}

#endif /* CONFIG_TEGRA_GK20A_NVHOST */
#endif /* NVGPU_NVHOST_H */
