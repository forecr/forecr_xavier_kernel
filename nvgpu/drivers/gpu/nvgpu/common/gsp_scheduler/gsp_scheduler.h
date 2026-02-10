/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GSP_SCHEDULER_H
#define GSP_SCHEDULER_H

#define GSP_SCHED_DEBUG_BUFFER_QUEUE	3U
#define GSP_SCHED_DMESG_BUFFER_SIZE		0x1000U

#define GSP_QUEUE_NUM	2U

#define SAFETY_SCHED_RISCV_FW_MANIFEST	"safety-scheduler.manifest.encrypt.bin.out.bin"
#define SAFETY_SCHED_RISCV_FW_CODE		"safety-scheduler.text.encrypt.bin"
#define SAFETY_SCHED_RISCV_FW_DATA		"safety-scheduler.data.encrypt.bin"

#define SAFETY_SCHED_RISCV_FW_MANIFEST_PROD	"safety-scheduler.manifest.encrypt.bin.out.bin.prod"
#define SAFETY_SCHED_RISCV_FW_CODE_PROD		"safety-scheduler.text.encrypt.bin.prod"
#define SAFETY_SCHED_RISCV_FW_DATA_PROD		"safety-scheduler.data.encrypt.bin.prod"

/* GSP descriptor's */
struct nvgpu_gsp_sched {
	struct nvgpu_gsp *gsp;

	struct gsp_sequences *sequences;

	struct nvgpu_engine_mem_queue *queues[GSP_QUEUE_NUM];

	u32 command_ack;

	/* set to true once init received */
	bool gsp_ready;

	/* Log info of Domains */
	u32 no_of_domains;
	u32 active_domain;
};

#endif /* GSP_SCHEDULER_H */
