/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_IPC_H
#define NVGPU_GSP_IPC_H

#include <nvgpu/types.h>

#define GSP_QUEUE_NUM	2U

/* GSP ipc info */
struct nvgpu_gsp_ipc {
	struct gk20a *g;
	struct gsp_sequences *sequences;
	struct nvgpu_engine_mem_queue *queues[GSP_QUEUE_NUM];
	/* Set to true once init received */
	bool gsp_ready;
	/* Mutex to lock cmd post when in use */
	struct nvgpu_mutex cmd_lock;

	u32 error_code;
	u32 error_info;
	u32 lib_nvriscv_error;
};

#endif /* NVGPU_GSP_IPC_H */
