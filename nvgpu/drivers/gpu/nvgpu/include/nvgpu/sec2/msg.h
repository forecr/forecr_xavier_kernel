/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_MSG_H
#define NVGPU_SEC2_MSG_H

#include <nvgpu/sec2/lsfm.h>
#include <nvgpu/sec2/sec2_cmn.h>
#include <nvgpu/flcnif_cmn.h>
#include <nvgpu/types.h>

struct nvgpu_sec2;

/*
 * SEC2 Message Interfaces - SEC2 Management
 */

/*
 * Defines the identifiers various high-level types of sequencer commands and
 * messages.
 * _SEC2_INIT - sec2_init_msg_sec2_init
 */
enum {
	NV_SEC2_INIT_MSG_ID_SEC2_INIT = 0U,
};

struct sec2_init_msg_sec2_init {
	u8  msg_type;
	u8  num_queues;

	u16 os_debug_entry_point;

	struct {
		u32 queue_offset;
		u16 queue_size;
		u8  queue_phy_id;
		u8  queue_log_id;
	} q_info[SEC2_QUEUE_NUM];

	u32 nv_managed_area_offset;
	u16 nv_managed_area_size;
	/* Unused, kept for the binary compatibility */
	u8 rsvd_1[16];
	u8 rsvd_2[16];
};

union nv_flcn_msg_sec2_init {
	u8 msg_type;
	struct sec2_init_msg_sec2_init sec2_init;
};


struct nv_flcn_msg_sec2 {
	struct pmu_hdr hdr;

	union {
		union nv_flcn_msg_sec2_init init;
		union nv_sec2_acr_msg acr;
	} msg;
};

int nvgpu_sec2_process_message(struct nvgpu_sec2 *sec2);
int nvgpu_sec2_wait_message_cond(struct nvgpu_sec2 *sec2, u32 timeout_ms,
	void *var, u8 val);

#endif /* NVGPU_SEC2_MSG_H */
