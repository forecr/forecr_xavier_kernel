/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef GSP_SCHEDULER_H
#define GSP_SCHEDULER_H

#define GSP_SCHED_DEBUG_BUFFER_QUEUE	3U
#define GSP_SCHED_DMESG_BUFFER_SIZE		0x1000U

#define SAFETY_SCHED_RISCV_FW_MANIFEST	"safety-scheduler.manifest.encrypt.bin.out.bin"
#define SAFETY_SCHED_RISCV_FW_CODE		"safety-scheduler.text.encrypt.bin"
#define SAFETY_SCHED_RISCV_FW_DATA		"safety-scheduler.data.encrypt.bin"

#define SAFETY_SCHED_RISCV_FW_MANIFEST_PROD	"safety-scheduler.manifest.encrypt.bin.out.bin.prod"
#define SAFETY_SCHED_RISCV_FW_CODE_PROD		"safety-scheduler.text.encrypt.bin.prod"
#define SAFETY_SCHED_RISCV_FW_DATA_PROD		"safety-scheduler.data.encrypt.bin.prod"

/* GSP descriptor's */
struct nvgpu_gsp_sched {
	struct nvgpu_gsp *gsp;

	/* Log info of Domains */
	u32 no_of_domains;
	u32 active_domain;
};

struct nvgpu_gsp_inst_bind_info {
	u32 inst_ptr;

	u32 aperture;
};
#endif /* GSP_SCHEDULER_H */
