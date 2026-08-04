/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_MSG_H
#define NVGPU_GSP_MSG_H

#include <nvgpu/types.h>
#include <nvgpu/gsp.h>

#include "gsp_cmd.h"

struct nvgpu_gsp;

#define GSP_CMD_FLAGS_MASK		U8(0xF0U)
#define GSP_CMD_FLAGS_STATUS		BIT8(0U)
#define GSP_CMD_FLAGS_INTR		BIT8(1U)
#define GSP_CMD_FLAGS_EVENT		BIT8(2U)
#define GSP_CMD_FLAGS_RPC_EVENT		BIT8(3U)

#define GSP_DMEM_ALLOC_ALIGNMENT	32U
#define GSP_DMEM_ALIGNMENT		4U

#define NV_GSP_INIT_MSG_ID_GSP_INIT		0U


struct gsp_init_msg_gsp_init {
	u8 msg_type;
	u8 num_queues;

	u16 os_debug_entry_point;
	struct {
		u32 queue_offset;
		u16 queue_size;
		u8  queue_phy_id;
		u8  queue_log_id;
	} q_info[GSP_QUEUE_NUM];

	u32 rsvd1;
	u8 rsvd2;
	u8 flcn_status;
};

union nv_flcn_msg_gsp_init {
	u8 msg_type;
	struct gsp_init_msg_gsp_init gsp_init;
};

struct nvgpu_gsp_domain_msg_no_of_domains {
	u8 msg_type;
	u32 no_of_domains;
};

struct nvgpu_gsp_domain_msg_active_domain {
	u8 msg_type;
	u32 active_domain;
};

struct nvgpu_gsp_acr_msg_status {
	u8 msg_type;
	u32 error_code;
	u32 error_info;
	u32 lib_nvriscv_error;
};

struct nvgpu_gsp_acr_msg_bootstrap_engine {
	struct nvgpu_gsp_acr_msg_status acr_status;
	u32 engine_id;
	u32 engine_instance;
};

union nvgpu_gsp_acr_msg {
	u8 msg_type;
	struct nvgpu_gsp_acr_msg_status acr_status;
	struct nvgpu_gsp_acr_msg_bootstrap_engine msg_engine;
};

struct nv_flcn_msg_gsp {
	struct gsp_hdr hdr;
	union {
		union nv_flcn_msg_gsp_init init;
		struct nvgpu_gsp_domain_msg_no_of_domains no_of_domains;
		struct nvgpu_gsp_domain_msg_active_domain active_domain;
		union nvgpu_gsp_acr_msg acr_msg;
	} msg;
};

/**
 * @brief Waits for a variable to reach a specified value or until a timeout occurs.
 *
 * The steps performed by the function are as follows:
 * -# Initialize a timeout structure with the specified timeout in milliseconds using
 *    nvgpu_timeout_init_cpu_timer().
 * -# Enter a loop that continues until the timeout expires.
 *    -# Check if the variable has reached the specified value.
 *       - If it has, return 0 indicating success.
 *    -# Sleep for a delay period using nvgpu_usleep_range(), initially set to #POLL_DELAY_MIN_US,
 *       and then double the delay each iteration without exceeding #POLL_DELAY_MAX_US.
 * -# If the timeout expires before the variable reaches the specified value,
 *    return -ETIMEDOUT to indicate a timeout has occurred.
 *
 * @param [in] g          The GPU structure.
 * @param [in] timeout_ms The timeout in milliseconds to wait for the variable to reach the value.
 * @param [in] var        The variable to check.
 * @param [in] val        The value to wait for the variable to reach.
 *
 * @return 0 if the variable reaches the specified value within the timeout period.
 * @return -ETIMEDOUT if the timeout expires before the variable reaches the value.
 */
int nvgpu_gsp_wait_message_cond(struct gk20a *g, u32 timeout_ms,
	void *var, u8 val);

int nvgpu_gsp_process_ipc_message(struct gk20a *g);
#endif /* NVGPU_GSP_MSG_H */
