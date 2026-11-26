/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_H
#define NVGPU_SEC2_H

#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/allocator.h>
#include <nvgpu/lock.h>
#include <nvgpu/falcon.h>
#include <nvgpu/sec2/seq.h>
#include <nvgpu/sec2/sec2_cmn.h>

struct gk20a;
struct nv_flcn_msg_sec2;
struct nvgpu_engine_mem_queue;

#define nvgpu_sec2_dbg(g, fmt, args...) \
	nvgpu_log(g, gpu_dbg_pmu, fmt, ##args)

#define NVGPU_SEC2_TRACE_BUFSIZE	(32U * 1024U)

struct sec2_fw {
	struct nvgpu_firmware *fw_desc;
	struct nvgpu_firmware *fw_image;
	struct nvgpu_firmware *fw_sig;
};

struct nvgpu_sec2 {
	struct gk20a *g;
	struct nvgpu_falcon flcn;
	u32 falcon_id;

	struct nvgpu_engine_mem_queue *queues[SEC2_QUEUE_NUM];

	struct sec2_sequences sequences;

	bool isr_enabled;
	struct nvgpu_mutex isr_mutex;

	struct nvgpu_allocator dmem;

	/* set to true once init received */
	bool sec2_ready;

	struct nvgpu_mem trace_buf;

	void (*remove_support)(struct nvgpu_sec2 *sec2);

	u32 command_ack;

	struct sec2_fw fw;
};

/* sec2 init */
int nvgpu_init_sec2_setup_sw(struct gk20a *g);
int nvgpu_init_sec2_support(struct gk20a *g);
int nvgpu_sec2_destroy(struct gk20a *g);

#endif /* NVGPU_SEC2_H */
