/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_SEQ_H
#define NVGPU_PMUIF_SEQ_H

#include <nvgpu/flcnif_cmn.h>

#define PMU_UNIT_SEQ            (0x02)

/*
 * @file   seq.h
 * @brief  PMU Command/Message Interfaces - Sequencer
 */

/*
 * Defines the identifiers various high-level types of sequencer commands.
 *
 * _RUN_SCRIPT @ref NV_PMU_SEQ_CMD_RUN_SCRIPT
 */
enum {
	NV_PMU_SEQ_CMD_ID_RUN_SCRIPT = 0,
};

struct nv_pmu_seq_cmd_run_script {
	u8 cmd_type;
	u8 pad[3];
	struct pmu_allocation_v3 script_alloc;
	struct pmu_allocation_v3 reg_alloc;
};

#define NV_PMU_SEQ_CMD_ALLOC_OFFSET              4

#define NV_PMU_SEQ_MSG_ALLOC_OFFSET                                         \
	(NV_PMU_SEQ_CMD_ALLOC_OFFSET + NV_PMU_CMD_ALLOC_SIZE)

struct nv_pmu_seq_cmd {
	struct pmu_hdr hdr;
	union {
		u8 cmd_type;
		struct nv_pmu_seq_cmd_run_script run_script;
	};
};

enum {
	NV_PMU_SEQ_MSG_ID_RUN_SCRIPT = 0,
};

struct nv_pmu_seq_msg_run_script {
	u8 msg_type;
	u8 error_code;
	u16 error_pc;
	u32 timeout_stat;
};

struct nv_pmu_seq_msg {
	struct pmu_hdr hdr;
	union {
		u8 msg_type;
		struct nv_pmu_seq_msg_run_script run_script;
	};
};

#endif /* NVGPU_PMUIF_SEQ_H */
