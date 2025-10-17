/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP
#define NVGPU_GSP
#include <nvgpu/lock.h>
#include <nvgpu/nvgpu_mem.h>

/* macro is used to log debug messages when the gpu_dbg_gsp is enabled */
#define nvgpu_gsp_dbg(g, fmt, arg...)  nvgpu_log(g, gpu_dbg_gsp, fmt, ##arg)

#define GSP_NV_CMDQ_LOG_ID              0U
#define GSP_NV_CMDQ_LOG_ID__LAST        0U

struct gk20a;
struct nvgpu_gsp;
struct nvgpu_runlist;
struct nvgpu_gsp_ipc;

struct gsp_fw {
	/* gsp ucode name */
	const char *code_name;
	const char *data_name;
	const char *manifest_name;

	/* gsp ucode */
	struct nvgpu_firmware *code;
	struct nvgpu_firmware *data;
	struct nvgpu_firmware *manifest;
};

/* GSP descriptor's */
struct nvgpu_gsp {
	struct gk20a *g;

	struct gsp_fw gsp_ucode;
	struct nvgpu_falcon *gsp_flcn;

	struct nvgpu_gsp_ipc *gsp_ipc;
};

int nvgpu_gsp_debug_buf_init(struct gk20a *g, u32 queue_no, u32 buffer_size);
void nvgpu_gsp_suspend(struct gk20a *g);
void gsp_release_firmware(struct gk20a *g, struct nvgpu_gsp *gsp);
int nvgpu_gsp_sw_init(struct gk20a *g);
void nvgpu_gsp_sw_deinit(struct gk20a *g);
void nvgpu_gsp_isr_mutex_acquire(struct gk20a *g, struct nvgpu_falcon *flcn);
void nvgpu_gsp_isr_mutex_release(struct gk20a *g, struct nvgpu_falcon *flcn);
bool nvgpu_gsp_is_isr_enable(struct gk20a *g, struct nvgpu_falcon *flcn);

/**
 * @brief Retrieves the last command ID used by the GSP.
 *
 * The steps performed by the function are as follows:
 * -# Cast the GPU structure to void to explicitly state that it is unused.
 * -# Return the constant #GSP_NV_CMDQ_LOG_ID__LAST which represents the last command ID.
 *
 * @param [in] g  The GPU structure (unused in this function).
 *
 * @return The last command ID used by the GSP.
 */
u32 nvgpu_gsp_get_last_cmd_id(struct gk20a *g);

struct nvgpu_falcon *nvgpu_gsp_falcon_instance(struct gk20a *g);

/**
 * @brief Processes messages from the GSP message queue and handles initialization messages.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GSP scheduler structure from the GPU structure.
 * -# Initialize a variable to store the message from the GSP.
 * -# Check if the GSP is ready to process messages. If not, process the initialization
 *    message using gsp_process_init_msg(). If processing the initialization message fails,
 *    log the failure and exit.
 * -# Enter a loop to read and process messages from the GSP message queue.
 *    -# Attempt to read a message from the GSP message queue using gsp_read_message().
 *    -# If a message is successfully read, log the message header details.
 *    -# Handle the response for the read message using gsp_response_handle().
 *    -# If handling the response fails, return the error status.
 *    -# Check if the GSP message queue is empty by calling nvgpu_gsp_queue_is_empty().
 *       If it is not, set the message interrupt using gops_gsp.set_msg_intr().
 *    -# Continue reading messages until no more messages are available.
 * -# Return the status code indicating the success or failure of message processing.
 *
 * @param [in] g  The GPU structure.
 *
 * @return 0 if all messages are processed successfully.
 * @return Non-zero error code if processing the initialization message or any subsequent
 *         message fails.
 */
int nvgpu_gsp_process_message(struct gk20a *g);
int nvgpu_gsp_wait_for_mailbox_update(struct nvgpu_gsp *gsp,
		u32 mailbox_index, u32 exp_value, signed int timeoutms);
int nvgpu_gsp_bootstrap_ns(struct gk20a *g, struct nvgpu_gsp *gsp);
void nvgpu_gsp_isr(struct gk20a *g);
void nvgpu_gsp_isr_support(struct gk20a *g, bool enable);
int nvgpu_gsp_wait_for_priv_lockdown_release(struct nvgpu_gsp *gsp,
		signed int timeoutms);
/**
 * @brief Checks if the GSP is ready.
 *
 * The steps performed by the function are as follows:
 * -# Return the status of the GSP readiness by accessing the 'gsp_ready' flag
 *    within the GSP scheduler structure of the GPU.
 *
 * @param [in] g  The GPU structure to check the GSP readiness status.
 *
 * @return true if the GSP is ready.
 * @return false if the GSP is not ready.
 */
bool nvgpu_gsp_is_ready(struct gk20a *g);
#endif /* NVGPU_GSP */
