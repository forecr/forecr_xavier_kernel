/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_SCHED_H
#define GSP_SCHED_H

/**
 * @file
 * @page igpu_safety_scheduler gsp_sched
 *
 * Overview
 * ========
 * Safety scheduler firmware implements GPU domain switching. During init time,
 * nvgpu-rm creates user domains upon the request from the third party system
 * scheduler through dev-ctls and sends the domain info to safety scheduler firmware
 * through command RPCs. safety scheduler firmware also responsible for runlist submits
 * during init time. During runtime, the system scheduler requests
 * safety scheduler firmware to schedule a domain using API which uses a shared buffer.
 * safety scheduler firmware schedules specific domains on the hardware by programming
 * runlist hardware registers by using runlist device info and domain info which was
 * sent during init time. nvgpu-rm loads safety scheduler firmware firmware onto the GSP
 * microcontroller.
 * Refer detail design at GIZA:NvGPU/SWAD/gpu-domain-scheduling and GSP design doc.
 *
 * nvgpu-rm interacts with GSP at below software stages:
 * - Init
 *   - bootstrap : Bootstrap the GSP engine.
 *   - @ref gsp_hal
 *   - @ref gsp_communication
 * - runlist updates : Send RL info to GSP for all RL updates.
 * - domain management : Send domain info to GSP for all domain updates.
 * - domain control : Establish control fifo communication between nvrm_gpu_sc and GSP.
 *
 * External APIs
 * ============================
 * gsp_sched_bootstrap
 * =====================
 * - @ref nvgpu_gsp_sched_bootstrap_hs
 * - @ref nvgpu_gsp_sched_sw_init
 * - @ref nvgpu_gsp_sched_sw_deinit
 * - @ref nvgpu_gsp_sched_suspend
 * - @ref nvgpu_gsp_sched_isr
 * - @ref nvgpu_gsp_sched_bind_ctx_reg
 * - @ref nvgpu_gsp_is_ready
 * gsp_runlist
 * =====================
 * - @ref nvgpu_gsp_sched_send_devices_info
 * - @ref nvgpu_gsp_sched_domain_submit
 * - @ref nvgpu_gsp_sched_domain_add
 * - @ref nvgpu_gsp_sched_domain_update
 * - @ref nvgpu_gsp_sched_runlist_update
 * - @ref nvgpu_gsp_sched_domain_delete
 * - @ref nvgpu_gsp_sched_query_active_domain
 * - @ref nvgpu_gsp_sched_query_no_of_domains
 * - @ref nvgpu_gsp_sched_start
 * - @ref nvgpu_gsp_sched_stop
 * gsp_domain management
 * =====================
 * - @ref nvgpu_gsp_nvs_add_domain
 * - @ref nvgpu_gsp_nvs_delete_domain
 * - @ref nvgpu_gps_sched_update_runlist
 * gsp_domain control
 * =====================
 * - @ref nvgpu_gsp_sched_send_queue_info
 * - @ref nvgpu_gsp_sched_erase_ctrl_fifo
 *
 * Internal APIs
 * ============================
 * gsp_sched_bootstrap
 * =====================
 * - @ref gsp_sched_get_file_names
 * - @ref gsp_sched_deinit
 * - @ref gsp_sched_wait_for_init
 * gsp_runlist
 * =====================
 * - @ref gsp_handle_cmd_ack
 * - @ref gsp_get_device_info
 * - @ref gsp_sched_send_devices_info
 * - @ref gsp_sched_send_grs_dev_info
 * - @ref gsp_sched_send_ces_dev_info
 * gsp_domain management
 * =====================
 * - @ref gsp_nvs_update_runlist_info
 * - @ref gsp_nvs_get_runlist_info
 * - @ref gsp_nvs_get_domain_info
 * gsp_domain control
 * =====================
 * - @ref gsp_ctrl_fifo_get_queue_info
 *
 */

#include <nvgpu/nvs.h>

struct gk20a;
struct nvgpu_gsp_sched;
struct nvgpu_runlist;
/*
 * Scheduler shall support only two engines with two runlists per domain.
 * 1. GR0
 * 2. Async CE0
 */
#define TOTAL_NO_OF_RUNLISTS 4U

struct nvgpu_gsp_runlist_info {
	/*
	 * Is this runlist valid, this field will be updated by NVGPU which tell GSP
	 * to submit this runlist or ignore for that domain.
	 */
	bool is_runlist_valid;
	/*
	 * This is don't care for KMD.
	 */
	bool is_runlist_updated;
	/*
	 * Device id to which this runlist belongs to.
	 */
	u8 device_id;
	/*
	 * Domain Id to which this runlist belongs to.
	 */
	u32 domain_id;
	/*
	 * ID contains the identifier of the runlist.
	 */
	u32 runlist_id;
	/*
	 * Indicates how many runlist entries are in the runlist.
	 */
	u32 num_entries;
	/*
	 * Indicates runlist memory aperture.
	 */
	u32 aperture;
	/*
	 *NV_RUNLIST_SUBMIT_BASE_L0 in-memory location of runlist.
	 */
	u32 runlist_base_lo;
	/*
	 *NV_RUNLIST_SUBMIT_BASE_Hi in-memory location of runlist.
	 */
	u32 runlist_base_hi;
};

struct nvgpu_gsp_domain_info {
	/*
	 * Is the current Domain Active. This is don't care for KMD.
	 */
	bool is_domain_active;
	/*
	 * Is the current Domain Valid. This is don't care for KMD.
	 */
	bool is_domain_valid;
	/*
	 * Domain Id
	 */
	u32 domain_id;
	/*
	 * Priority of the Domain for priority driven scheduling.
	 */
	u32 priority;
	/*
	 * Time-slicing of the domain for which scheduler will schedule it for.
	 */
	u32 time_slicing;
	/*
	 * Runlist info
	 */
	struct nvgpu_gsp_runlist_info runlist_info[TOTAL_NO_OF_RUNLISTS];
};

/**
 * @brief Bootstraps the GSP scheduler for high-speed (HS) operation.
 *
 * The steps performed by the function are as follows:
 * -# If falcon debug is enabled, initialize the debug buffer for the GSP scheduler
 *    using nvgpu_gsp_debug_buf_init() and check for errors. If an error occurs, log
 *    it and proceed to de-initialization.
 * -# Retrieve the firmware file names for the GSP scheduler using gsp_sched_get_file_names().
 * -# Bootstrap the GSP in non-secure mode using nvgpu_gsp_bootstrap_ns() and check for
 *    errors. If an error occurs, log it and proceed to de-initialization.
 * -# Wait for the GSP to release the private lockdown using nvgpu_gsp_wait_for_priv_lockdown_release()
 *    and check for errors. If an error occurs, log it and proceed to de-initialization.
 * -# If a falcon setup boot configuration function is provided, call it using
 *    gops_gsp.falcon_setup_boot_config() to set up the GSP context instance.
 * -# Wait for the GSP to complete its basic initialization using gsp_sched_wait_for_init()
 *    and check for errors. If an error occurs, log it and proceed to de-initialization.
 * -# Bind the GSP context register using nvgpu_gsp_sched_bind_ctx_reg() and check for errors.
 *    If an error occurs, log it and proceed to de-initialization.
 * -# Send device information to the GSP using nvgpu_gsp_sched_send_devices_info() and check
 *    for errors. If an error occurs, log it and proceed to de-initialization.
 * -# Log that the GSP scheduler has been successfully bootstrapped.
 * -# If any step in the bootstrap process fails, de-initialize the GSP scheduler
 *    software state by calling nvgpu_gsp_sched_sw_deinit().
 * -# Return the status code indicating the success or failure of the bootstrap process.
 *
 * @param [in] g  The GPU structure for which the GSP scheduler is to be bootstrapped.
 *
 * @return 0 if the GSP scheduler is successfully bootstrapped.
 * @return less than 0 if the bootstrap process fails at any step.
 */
int nvgpu_gsp_sched_bootstrap_hs(struct gk20a *g);

/**
 * @brief Initializes the software state for the GSP scheduler.
 *
 * The steps performed by the function are as follows:
 * -# Check if the GSP scheduler software state is already initialized. If it is,
 *    return early to reuse the existing state for a faster boot in the case of
 *    recovery or unrailgate.
 * -# Allocate memory for the GSP scheduler software state structure using nvgpu_kzalloc().
 * -# If the memory allocation fails, set the error code to -ENOMEM and proceed to
 *    de-initialization.
 * -# Allocate memory for the GSP software state structure using nvgpu_kzalloc().
 * -# If the memory allocation fails, set the error code to -ENOMEM and proceed to
 *    de-initialization.
 * -# Initialize the GSP falcon software state by assigning the GSP falcon and GPU
 *    structure pointers.
 * -# Initialize the ISR (Interrupt Service Routine) mutex for the GSP using nvgpu_mutex_init().
 * -# Initialize the GSP sequences by calling nvgpu_gsp_sequences_init().
 * -# If the GSP sequences initialization fails, log an error and proceed to
 *    de-initialization.
 * -# If all initializations are successful, log that the initialization is done and
 *    return the error code.
 * -# If de-initialization is needed due to an error, call nvgpu_gsp_sched_sw_deinit()
 *    to clean up and return the error code.
 *
 * @param [in] g  The GPU structure for which the GSP scheduler software state is to be initialized.
 *
 * @return 0 if the GSP scheduler software state is successfully initialized.
 * @return -ENOMEM if memory allocation for the GSP scheduler or GSP software state fails.
 */
int nvgpu_gsp_sched_sw_init(struct gk20a *g);

/**
 * @brief Deinitializes the software state of the GSP scheduler.
 *
 * This function cleans up the software state of the GSP scheduler by deinitializing
 * the GSP, freeing any allocated sequences, freeing the queues, and finally
 * deinitializing the GSP scheduler structure itself.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the GSP scheduler structure from the GPU driver
 *    structure.
 * -# If the GSP scheduler structure pointer is NULL, return from the function
 *    as there is nothing to deinitialize.
 * -# If the GSP within the GSP scheduler structure is not NULL, call
 *    nvgpu_gsp_sw_deinit() to deinitialize the GSP.
 * -# If the sequences within the GSP scheduler structure are not NULL, call
 *    nvgpu_gsp_sequences_free() to free the allocated sequences.
 * -# Call nvgpu_gsp_queues_free() to free the GSP queues.
 * -# Call gsp_sched_deinit() to deinitialize the GSP scheduler structure and
 *    free its memory.
 *
 * @param [in] g  Pointer to the GPU driver structure.
 */
void nvgpu_gsp_sched_sw_deinit(struct gk20a *g);

/**
 * @brief Suspends the GSP scheduler.
 *
 * This function is responsible for suspending the GPU Scheduler Processor (GSP)
 * scheduler by marking it as not ready, freeing associated queues, and invoking
 * the suspend routine for the GSP.
 *
 * The steps performed by the function are as follows:
 * -# Check if the GSP instance within the GSP scheduler structure is NULL. If it
 *    is NULL, log an error message indicating that the GSP is not initialized
 *    and return from the function.
 * -# Set the GSP scheduler's 'gsp_ready' flag to false to indicate that the GSP
 *    scheduler is no longer ready to schedule tasks.
 * -# Call nvgpu_gsp_queues_free() to free the memory resources associated with
 *    the GSP queues.
 * -# Call nvgpu_gsp_suspend() to suspend the GSP.
 *
 * @param [in] g          Pointer to the GPU driver structure.
 * @param [in] gsp_sched  Pointer to the GSP scheduler structure.
 */
void nvgpu_gsp_sched_suspend(struct gk20a *g, struct nvgpu_gsp_sched *gsp_sched);

/**
 * @brief Sends a command to bind the context register in the GSP scheduler.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a GSP command structure.
 * -# Send the command to bind the context register to the GSP and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails, log an error.
 * -# Return the error code, which will be 0 if the command is successfully sent
 *    and acknowledged.
 *
 * @param [in] g  The GPU structure for which the GSP context register is to be bound.
 *
 * @return 0 if the command is successfully sent and acknowledged by the GSP.
 * @return Non-zero error code if sending the command fails.
 */
int nvgpu_gsp_sched_bind_ctx_reg(struct gk20a *g);

//runlist
/**
 * @brief Sends device information for graphics and copy engines to the GSP scheduler.
 *
 * The function sequentially sends device information for graphics engines and
 * asynchronous copy engines to the GSP scheduler. If an error occurs while
 * sending either of the device information, it reports the error.
 *
 * The steps performed by the function are as follows:
 * -# Send the graphics engines device information to the GSP scheduler by
 *    calling gsp_sched_send_grs_dev_info().
 * -# If an error occurs while sending graphics engines device information,
 *    report the error and exit the function.
 * -# Send the asynchronous copy engines device information to the GSP scheduler
 *    by calling gsp_sched_send_ces_dev_info().
 * -# If an error occurs while sending asynchronous copy engines device
 *    information, report the error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 *
 * @return 0 if device information for both graphics and copy engines is
 *         successfully sent.
 * @return Error code from gsp_sched_send_grs_dev_info() if sending graphics
 *         engines device information fails.
 * @return Error code from gsp_sched_send_ces_dev_info() if sending asynchronous
 *         copy engines device information fails.
 */
int nvgpu_gsp_sched_send_devices_info(struct gk20a *g);

/**
 * @brief Submits a domain to the GSP scheduler.
 *
 * The function prepares a command to submit a domain by its identifier to the
 * GSP scheduler and sends it. If the command fails to send or is not
 * acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Copy the domain identifier into the command buffer.
 * -# Send the command to submit the domain to the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [in] domain_id - Identifier of the domain to be submitted.
 *
 * @return 0 if the domain is successfully submitted to the GSP scheduler.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_domain_submit(struct gk20a *g, u32 domain_id);

/**
 * @brief Adds a domain to the GSP scheduler.
 *
 * The function prepares a command to add a domain with its information to the
 * GSP scheduler and sends it. If the command fails to send or is not
 * acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Copy the domain information into the command buffer.
 * -# Send the command to add the domain to the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [in] gsp_dom - Pointer to the domain information structure to be added.
 *
 * @return 0 if the domain is successfully added to the GSP scheduler.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_domain_add(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom);

/**
 * @brief Updates a domain in the GSP scheduler.
 *
 * The function prepares a command to update a domain with its information in the
 * GSP scheduler and sends it. If the command fails to send or is not
 * acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Copy the domain information into the command buffer.
 * -# Send the command to update the domain in the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [in] gsp_dom - Pointer to the domain information structure to be updated.
 *
 * @return 0 if the domain is successfully updated in the GSP scheduler.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_domain_update(struct gk20a *g,
		struct nvgpu_gsp_domain_info *gsp_dom);

/**
 * @brief Updates a runlist in the GSP scheduler.
 *
 * The function prepares a command to update a runlist with its information in the
 * GSP scheduler and sends it. If the command fails to send or is not
 * acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Copy the runlist information into the command buffer.
 * -# Send the command to update the runlist in the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [in] gsp_rl - Pointer to the runlist information structure to be updated.
 *
 * @return 0 if the runlist is successfully updated in the GSP scheduler.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_runlist_update(struct gk20a *g,
		struct nvgpu_gsp_runlist_info *gsp_rl);

/**
 * @brief Deletes a domain from the GSP scheduler.
 *
 * The function prepares a command to delete a domain by its identifier from the
 * GSP scheduler and sends it. If the command fails to send or is not
 * acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Copy the domain identifier into the command buffer.
 * -# Send the command to delete the domain from the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [in] domain_id - Identifier of the domain to be deleted.
 *
 * @return 0 if the domain is successfully deleted from the GSP scheduler.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_domain_delete(struct gk20a *g, u32 domain_id);

/**
 * @brief Queries the active domain managed by the GSP scheduler.
 *
 * The function sends a command to the GSP scheduler to retrieve the identifier
 * of the active domain and stores the result in the provided output parameter.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Send the command to query the active domain from the GSP scheduler and
 *    wait for an acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails, report the error and exit the function.
 * -# Retrieve the identifier of the active domain from the GSP scheduler's
 *    internal data and store it in the output parameter.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [out] active_domain - Pointer to store the identifier of the active
 *              domain managed by the GSP scheduler.
 *
 * @return 0 if the query is successful and the active domain is retrieved.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_query_active_domain(struct gk20a *g, u32 *active_domain);

/**
 * @brief Queries the number of domains managed by the GSP scheduler.
 *
 * The function sends a command to the GSP scheduler to retrieve the number of
 * domains it manages and stores the result in the provided output parameter.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Send the command to query the number of domains from the GSP scheduler and
 *    wait for an acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails, report the error and exit the function.
 * -# Retrieve the number of domains from the GSP scheduler's internal data and
 *    store it in the output parameter.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 * @Param [out] no_of_domains - Pointer to store the number of domains managed by
 *              the GSP scheduler.
 *
 * @return 0 if the query is successful and the number of domains is retrieved.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_query_no_of_domains(struct gk20a *g, u32 *no_of_domains);

/**
 * @brief Starts the GSP scheduler.
 *
 * The function sends a command to the GSP to start the scheduler. If the
 * command fails to send or is not acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Send the command to start the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 *
 * @return 0 if the GSP scheduler is successfully started.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_start(struct gk20a *g);

/**
 * @brief Stops the GSP scheduler.
 *
 * The function sends a command to the GSP to stop the scheduler. If the
 * command fails to send or is not acknowledged, it reports an error.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for communication with the GSP.
 * -# Send the command to stop the GSP scheduler and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails or no acknowledgment is received, report the
 *    error.
 *
 * @Param [in] g - Pointer to the GPU driver struct.
 *
 * @return 0 if the GSP scheduler is successfully stopped.
 * @return Error code from gsp_send_cmd_and_wait_for_ack() if sending the
 *         command fails or no acknowledgment is received.
 */
int nvgpu_gsp_sched_stop(struct gk20a *g);

/**
 * @brief Adds a domain to the GSP scheduler.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a GSP domain information structure.
 * -# Retrieve the domain information for the specified domain ID using gsp_nvs_get_domain_info().
 * -# If retrieving the domain information fails, log an error and exit the function.
 * -# Add the domain to the GSP scheduler using nvgpu_gsp_sched_domain_add().
 * -# If adding the domain to the GSP scheduler fails, log an error and exit the function.
 * -# Return the error code, which will be 0 if the domain is successfully added to the GSP scheduler.
 *
 * @param [in] g               The GPU structure.
 * @param [in] nvgpu_domain_id The domain ID to be added to the GSP scheduler.
 *
 * @return 0 if the domain is successfully added to the GSP scheduler.
 * @return Non-zero error code if retrieving the domain information or adding the domain fails.
 */
int nvgpu_gsp_nvs_add_domain(struct gk20a *g, u64 nvgpu_domain_id);

/**
 * @brief Requests the deletion of a domain from the GSP scheduler.
 *
 * The steps performed by the function are as follows:
 * -# Log the start of the domain deletion process.
 * -# Request the GSP scheduler to delete the domain with the specified domain ID
 *    by calling nvgpu_gsp_sched_domain_delete() with the lower 32 bits of the domain ID.
 * -# If the domain deletion request fails, log an error.
 * -# Return the error code, which will be 0 if the domain is successfully deleted.
 *
 * @param [in] g               The GPU structure.
 * @param [in] nvgpu_domain_id The domain ID to be deleted from the GSP scheduler.
 *
 * @return 0 if the domain is successfully deleted from the GSP scheduler.
 * @return Non-zero error code if the domain deletion request fails.
 */
int nvgpu_gsp_nvs_delete_domain(struct gk20a *g, u64 nvgpu_domain_id);

/**
 * @brief Updates the GSP scheduler with the latest runlist information.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a GSP runlist information structure.
 * -# Copy the runlist data to the command buffer by calling gsp_nvs_update_runlist_info().
 * -# If copying the runlist data to the command buffer fails, log an error and exit the function.
 * -# Send the updated runlist information to the GSP scheduler by calling nvgpu_gsp_sched_runlist_update().
 * -# If sending the runlist information to the GSP scheduler fails, log an error and exit the function.
 * -# Return the error code, which will be 0 if the runlist information is successfully updated.
 *
 * @param [in] g        The GPU structure.
 * @param [in] domain   The runlist domain containing the runlist data.
 * @param [in] rl       The runlist structure associated with the runlist domain.
 *
 * @return 0 if the runlist information is successfully updated in the GSP scheduler.
 * @return Non-zero error code if updating the runlist information fails.
 */
int nvgpu_gps_sched_update_runlist(struct gk20a *g,
    struct nvgpu_runlist_domain *domain, struct nvgpu_runlist *rl);
#ifdef CONFIG_NVS_PRESENT
/**
 * @brief Sends control FIFO queue information to the GSP (Graphics System Processor).
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for sending to the GSP.
 * -# Call gsp_ctrl_fifo_get_queue_info() to retrieve the control FIFO queue information
 *    based on the provided queue and queue direction.
 * -# If retrieving the queue information fails, log an error and go to the exit label.
 * -# Update the command structure with the retrieved control FIFO information,
 *    including the lower and higher 32 bits of the queue's GPU virtual address,
 *    the queue size, queue type, and the number of queue entries.
 * -# Send the command to the GSP with the control FIFO information and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command to the GSP fails, log an error.
 * -# Return the error code, which will be 0 if no errors occurred during the process.
 *
 * @param [in] g                  The GPU structure.
 * @param [in] queue              The NVS control queue whose information is to be sent to the GSP.
 * @param [in] queue_direction    The direction of the queue, indicating whether it is for
 *                                client to scheduler or scheduler to client communication.
 *
 * @return 0 if the control FIFO queue information is successfully sent to the GSP.
 * @return Non-zero error code if retrieving the queue information or sending the command fails.
 */
int nvgpu_gsp_sched_send_queue_info(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction);

/**
 * @brief Erases the GSP control FIFO based on the specified queue direction and
 *        whether the closure is abrupt.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a command structure for the GSP.
 * -# Set the abrupt closure flag in the command structure based on the provided
 *    is_abrupt_close parameter.
 * -# Determine the queue type based on the provided queue_direction parameter.
 *    If the direction is from client to scheduler, set the queue type to #CONTROL_QUEUE.
 *    If the direction is from scheduler to client, set it to #RESPONSE_QUEUE.
 *    If the direction is neither, log an error and return early.
 * -# Populate the command structure with the determined queue type.
 * -# Send the command to the GSP to erase the control FIFO and wait for an
 *    acknowledgment using gsp_send_cmd_and_wait_for_ack().
 * -# If sending the command fails, log an error.
 *
 * @param [in] g                  The GPU structure.
 * @param [in] queue_direction    The direction of the queue, indicating whether it is for
 *                                client to scheduler or scheduler to client communication.
 * @param [in] is_abrupt_close    Flag indicating whether the closure of the control FIFO
 *                                should be abrupt.
 */
void nvgpu_gsp_sched_erase_ctrl_fifo(struct gk20a *g,
	enum nvgpu_nvs_ctrl_queue_direction queue_direction, bool is_abrupt_close);
#endif
#endif /* GSP_SCHED_H */
