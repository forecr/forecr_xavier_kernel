/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PMUIF_CMN_H
#define NVGPU_PMUIF_CMN_H

#include <nvgpu/types.h>
#include <nvgpu/flcnif_cmn.h>

/*
 * Defines the logical queue IDs that must be used when submitting
 * commands to the PMU
 */
/* write by sw, read by pmu, protected by sw mutex lock */
#define PMU_COMMAND_QUEUE_HPQ		0U
/* write by sw, read by pmu, protected by sw mutex lock */
#define PMU_COMMAND_QUEUE_LPQ		1U
/* write by pmu, read by sw, accessed by interrupt handler, no lock */
#define PMU_MESSAGE_QUEUE		4U
#define PMU_QUEUE_COUNT			5U

#define PMU_IS_COMMAND_QUEUE(id)	\
		((id)  < PMU_MESSAGE_QUEUE)

#define PMU_IS_SW_COMMAND_QUEUE(id)	\
		(((id) == PMU_COMMAND_QUEUE_HPQ) || \
		 ((id) == PMU_COMMAND_QUEUE_LPQ))

#define  PMU_IS_MESSAGE_QUEUE(id)	\
		((id) == PMU_MESSAGE_QUEUE)

/*
 * The PMU's frame-buffer interface block has several slots/indices
 * which can be bound to support DMA to various surfaces in memory
 */
enum {
	PMU_DMAIDX_UCODE         = 0,
	PMU_DMAIDX_VIRT          = 1,
	PMU_DMAIDX_PHYS_VID      = 2,
	PMU_DMAIDX_PHYS_SYS_COH  = 3,
	PMU_DMAIDX_PHYS_SYS_NCOH = 4,
	PMU_DMAIDX_RSVD          = 5,
	PMU_DMAIDX_PELPG         = 6,
	PMU_DMAIDX_END           = 7
};

/*
 * Falcon PMU DMA's minimum size in bytes.
 */
#define PMU_DMA_MIN_READ_SIZE_BYTES		16U
#define PMU_DMA_MIN_WRITE_SIZE_BYTES	4U

#define PMU_FB_COPY_RW_ALIGNMENT	\
	((PMU_DMA_MIN_READ_SIZE_BYTES > PMU_DMA_MIN_WRITE_SIZE_BYTES) ?	\
	PMU_DMA_MIN_READ_SIZE_BYTES : PMU_DMA_MIN_WRITE_SIZE_BYTES)

/*
 * Macros to make aligned versions of RM_PMU_XXX structures. PMU needs aligned
 * data structures to issue DMA read/write operations.
 */
#define NV_PMU_MAKE_ALIGNED_STRUCT(name, size)           \
union name##_aligned {		                         \
		struct name data;                        \
		u8 pad[ALIGN_UP(sizeof(struct name),     \
		(PMU_FB_COPY_RW_ALIGNMENT))];            \
}

#define NV_PMU_MAKE_ALIGNED_UNION(name, size)	         \
union name##_aligned {		                         \
		union name data;                         \
		u8 pad[ALIGN_UP(sizeof(union name),      \
		(PMU_FB_COPY_RW_ALIGNMENT))];            \
}

/* RPC (Remote Procedure Call) header structure */
#define NV_PMU_RPC_FLAGS_TYPE_SYNC 0x00000000U

struct nv_pmu_rpc_header {
	/*
	 * RPC call flags (@see PMU_RPC_FLAGS)
	 *   PMU may modify this field.
	 */
	u8  flags;
	/*
	 * Identifies the unit servicing requested RPC
	 * PMU must not modify this field.
	 */
	u8  unit_id;
	/*
	 * Identifies the requested RPC (within the unit
	 * PMU must not modify this field.
	 */
	u8  function;
	/*
	 *  Carries the RM's index of seq struct
	 *  Used only in responses to the NvGPU issued requests.
	 *  PMU must not modify this field.
	 */
	u8 seq_num_id;
	/* ID of task in PMU that handled this RPC */
	u8 task_id;
	/*
	 * Falcon's status code to describe failures
	 * Used only in responses to the NvGPU issued requests.
	 */
	falcon_status  flcn_status;
	/* Size of the RPC structure.*/
	u16 rpc_size;
	/* queue level seq number */
	u16 seq_number;
	/* negative checksum of entire queue entry */
	u16 check_sum;
	union {
		/*
		 *  RPC's actual exec. time (measured on PMU side).
		 *  Used only in responses to the nvgpu issued requests.
		 */
		u32 exec_pmu_ns;
		/*
		 * PTimer [ns] when the RPC was triggered by the PMU.
		 * Used only in PMU triggered RPCs
		 */
		u32 rpc_triggered;
	} time;
};

/* PMU to NVGPU RPC header structure.*/
struct pmu_nvgpu_rpc_header {
	/* Identifies the unit servicing requested RPC */
	u8 unit_id;
	/* Identifies the requested RPC(within the unit)*/
	u8 function;
	/* Time of RPC to transfer from PMU, to dispatch in the nvgpu */
	struct falc_u64 rpc_transfer_time;
};

#endif /* NVGPU_PMUIF_CMN_H */
