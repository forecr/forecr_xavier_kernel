/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_NVGPU_CMDIF_H
#define NVGPU_PMUIF_NVGPU_CMDIF_H

#include "cmn.h"
#include "init.h"
#include "ap.h"
#include "seq.h"

#define PMU_UNIT_REWIND			U8(0x00)
#define PMU_UNIT_CMDMGMT		U8(0x0F)
#define PMU_UNIT_PG			U8(0x03)
#define PMU_UNIT_PG_LOADING		U8(0x06)
#define PMU_UNIT_INIT			U8(0x07)
#define PMU_UNIT_ACR			U8(0x0A)
#define PMU_UNIT_PERFMON_T18X		U8(0x11)
#define PMU_UNIT_PERFMON		U8(0x12)
#define PMU_UNIT_PERF			U8(0x13)
#define PMU_UNIT_RC			U8(0x1F)
#define PMU_UNIT_FECS_MEM_OVERRIDE	U8(0x1E)
#define PMU_UNIT_CLK			U8(0x0D)
#define PMU_UNIT_THERM			U8(0x14)
#define PMU_UNIT_PMGR			U8(0x18)
#define PMU_UNIT_VOLT			U8(0x0E)
#define PMU_UNIT_INIT_DGPU		U8(0x0F)
#define PMU_UNIT_LPWR_LP		U8(0x1D)
#define PMU_UNIT_END			U8(0x23)
#define PMU_UNIT_INVALID		U8(0xFF)

#define PMU_UNIT_TEST_START		U8(0xFE)
#define PMU_UNIT_END_SIM		U8(0xFF)
#define PMU_UNIT_TEST_END		U8(0xFF)

#define PMU_UNIT_ID_IS_VALID(id)		\
		(((id) < PMU_UNIT_END) || ((id) >= PMU_UNIT_TEST_START))

/*
 * PMU Command structures for FB queue
 */

/* Size of a single element in the CMD queue. */
#define NV_PMU_FBQ_CMD_ELEMENT_SIZE		2048U

/* Number of elements in each queue. */
#define NV_PMU_FBQ_CMD_NUM_ELEMENTS		16U

/* Total number of CMD queues. */
#define NV_PMU_FBQ_CMD_COUNT		2U

/* Size of a single element in the MSG queue. */
#define NV_PMU_FBQ_MSG_ELEMENT_SIZE		64U

#define RM_PMU_FBQ_MSG_DATA_SIZE \
	(NV_PMU_FBQ_MSG_ELEMENT_SIZE - \
	sizeof(struct nv_falcon_fbq_msgq_hdr))

/* Number of elements in each queue. */
#define NV_PMU_FBQ_MSG_NUM_ELEMENTS		16U

/* Single MSG (response) queue. */
#define NV_PMU_FBQ_MSG_COUNT		1U

/*!
 * @brief
 * Command requesting execution of the RPC (Remote Procedure Call).
 */
struct nvgpu_pmu_rpc_cmd {
	/*!
	 * Must be set to @ref RM_PMU_RPC_CMD_ID.
	 */
	u8 padding1;
	/*!
	 * RPC call flags (@see RM_PMU_RPC_FLAGS).
	 */
	u8 flags;
	u16 padding2;
	/*!
	 * DMEM pointer of RPC structure allocated within RM managed DMEM heap.
	 */
	u32 rpc_dmem_ptr;
};

/*!
  * Defines the structure that must be populated to send any arbitrary command
  * to the PMU's command queue.  Each command packet will contain a command
  * header ('hdr') describing various command attributes as well as the command
  * data itself ('cmd').
*/
struct nvgpu_cmd_pmu {
	struct pmu_hdr hdr;
	union {
		// RPC command is re-used by all (interested) tasks.
		struct nvgpu_pmu_rpc_cmd rpc;
	} cmd;
};

/*!
 * Header structure for a single PMU FBQ RPC element
 */
struct nvgpu_pmu_fbq_headers {
	struct nv_falcon_fbq_hdr fbq_hdr;
	struct nvgpu_cmd_pmu old_cmd;
	struct nv_pmu_rpc_header rpc_hdr;
};

/* structure for a single PMU FB CMD queue entry */
struct nv_pmu_fbq_cmd_q_element {
	struct nvgpu_pmu_fbq_headers hdrs;
	u8 rpc_data[NV_PMU_FBQ_CMD_ELEMENT_SIZE -
	sizeof(struct nvgpu_pmu_fbq_headers)];

};

/* structure for a single PMU FB MSG queue entry */
struct nv_pmu_fbq_msg_q_element {
	struct pmu_hdr old_hdr;
	u8 bytes[NV_PMU_FBQ_MSG_ELEMENT_SIZE -
	sizeof(struct pmu_hdr)];
};

/* structure for a single FB CMD queue */
struct nv_pmu_fbq_cmd_queue {
	struct nv_pmu_fbq_cmd_q_element element[NV_PMU_FBQ_CMD_NUM_ELEMENTS];
};

/* structure for a set of FB CMD queue */
struct nv_pmu_fbq_cmd_queues {
	struct nv_pmu_fbq_cmd_queue queue[NV_PMU_FBQ_CMD_COUNT];
};

/* structure for a single FB MSG queue */
struct nv_pmu_fbq_msg_queue {
	struct nv_pmu_fbq_msg_q_element element[NV_PMU_FBQ_MSG_NUM_ELEMENTS];
};

#endif /* NVGPU_PMUIF_NVGPU_CMDIF_H */
