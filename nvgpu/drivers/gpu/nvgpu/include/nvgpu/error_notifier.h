/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_ERROR_NOTIFIER_H
#define NVGPU_ERROR_NOTIFIER_H

#include <nvgpu/types.h>

struct nvgpu_channel;

#define NVGPU_ERR_NOTIFIER_FIFO_ERROR_IDLE_TIMEOUT		0U
#define NVGPU_ERR_NOTIFIER_GR_ERROR_SW_METHOD			1U
#define NVGPU_ERR_NOTIFIER_GR_ERROR_SW_NOTIFY			2U
#define NVGPU_ERR_NOTIFIER_GR_EXCEPTION				3U
#define NVGPU_ERR_NOTIFIER_GR_SEMAPHORE_TIMEOUT			4U
#define NVGPU_ERR_NOTIFIER_GR_ILLEGAL_NOTIFY			5U
#define NVGPU_ERR_NOTIFIER_FIFO_ERROR_MMU_ERR_FLT		6U
#define NVGPU_ERR_NOTIFIER_PBDMA_ERROR				7U
#define NVGPU_ERR_NOTIFIER_FECS_ERR_UNIMP_FIRMWARE_METHOD	8U
#define NVGPU_ERR_NOTIFIER_RESETCHANNEL_VERIF_ERROR		9U
#define NVGPU_ERR_NOTIFIER_PBDMA_PUSHBUFFER_CRC_MISMATCH	10U
#define NVGPU_ERR_NOTIFIER_CE_ERROR				11U
#define NVGPU_ERR_NOTIFIER_INVAL				12U

/**
 * @brief Set the error notifier for a channel while holding the channel's error notifier mutex.
 *
 * This function sets the error notifier for a channel when the channel's error notifier mutex
 * is already acquired. The steps are as follows:
 * -# Retrieve the private data structure for the channel's error notifier.
 * -# Convert the common error code to a channel-specific error code using
 *    nvgpu_error_notifier_to_channel_notifier().
 * -# Check if the error notifier nvmap handle is non-zero, indicating that an error notifier
 *    has been previously allocated.
 * -# If an error notifier exists, retrieve the current system time using clock_gettime().
 * -# Convert the system time to nanoseconds and store it in the error notifier structure.
 * -# Set the error code in the error notifier structure.
 * -# Perform a write memory barrier to ensure all writes are visible to other observers.
 * -# Set the error notifier status to indicate that an error has occurred.
 * -# Log an error message indicating the error notifier has been set for the channel.
 *
 * @param [in]  ch     Pointer to the channel structure.
 * @param [in]  error  Common error code to be set in the error notifier.
 */
void nvgpu_set_err_notifier_locked(struct nvgpu_channel *ch, u32 error);

/**
 * @brief Set the error notifier for a channel.
 *
 * This function sets the error notifier for a channel by performing the following steps:
 * -# Retrieve the private data structure for the channel's error notifier.
 * -# Acquire the mutex lock to protect concurrent access to the error notifier data.
 * -# Call nvgpu_set_err_notifier_locked() to set the error notifier while the mutex is held.
 * -# Release the mutex lock.
 *
 * @param [in]  ch     Pointer to the channel structure.
 * @param [in]  error  Common error code to be set in the error notifier.
 */
void nvgpu_set_err_notifier(struct nvgpu_channel *ch, u32 error);

/**
 * @brief Conditionally set the error notifier for a channel if it is not already set.
 *
 * This function sets the error notifier for a channel only if the error notifier status
 * is not already indicating an error. The steps are as follows:
 * -# Retrieve the private data structure for the channel's error notifier.
 * -# Acquire the mutex lock to protect concurrent access to the error notifier data.
 * -# Check if the error notifier nvmap handle is non-zero, indicating that an error notifier
 *    has been previously allocated.
 * -# If an error notifier exists, check if the error notifier status is not already set to
 *    indicate an error.
 * -# If the error notifier status does not indicate an error, call nvgpu_set_err_notifier_locked()
 *    to set the error notifier while the mutex is held.
 * -# Release the mutex lock.
 *
 * @param [in]  ch     Pointer to the channel structure.
 * @param [in]  error  Common error code to be set in the error notifier.
 */
void nvgpu_set_err_notifier_if_empty(struct nvgpu_channel *ch, u32 error);

/**
 * @brief Check if a specific error notifier is set for a channel.
 *
 * This function checks if a given error notifier is set for a channel by performing the following steps:
 * -# Retrieve the private data structure for the channel's error notifier.
 * -# Convert the common error code to a channel-specific error code using
 *    nvgpu_error_notifier_to_channel_notifier().
 * -# Acquire the mutex lock to protect concurrent access to the error notifier data.
 * -# Check if the error notifier nvmap handle is non-zero, indicating that an error notifier
 *    has been previously allocated.
 * -# If an error notifier exists, retrieve the current error code from the error notifier structure.
 * -# Compare the retrieved error code with the channel-specific error code.
 * -# If they match, set the notifier_set flag to true, indicating that the specific error notifier
 *    is set.
 * -# Release the mutex lock.
 * -# Return the notifier_set flag indicating whether the specific error notifier is set.
 *
 * @param [in]  ch             Pointer to the channel structure.
 * @param [in]  error_notifier Common error notifier ID to check.
 *
 * @return True if the specific error notifier is set, false otherwise.
 */
bool nvgpu_is_err_notifier_set(struct nvgpu_channel *ch, u32 error_notifier);

#endif /* NVGPU_ERROR_NOTIFIER_H */
