/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_RPC_H
#define NVGPU_PMUIF_RPC_H

/*
 * Command requesting execution of the RPC (Remote Procedure Call)
 */
struct nv_pmu_rpc_cmd {
	/* Must be set to @ref NV_PMU_RPC_CMD_ID */
	u8 cmd_type;
	/* RPC call flags (@see PMU_RPC_FLAGS) */
	u8 flags;
	/* Size of RPC structure allocated
	 *  within NV managed DMEM heap
	 */
	u16 rpc_dmem_size;
	/*
	 * DMEM pointer of RPC structure allocated
	 * within RM managed DMEM heap.
	 */
	u32 rpc_dmem_ptr;
};

#define NV_PMU_RPC_CMD_ID 0x80U
#define NV_PMU_TASK_ID__END	0x19U

/* Message carrying the result of the RPC execution */
struct nv_pmu_rpc_msg {
	/* Must be set to @ref NV_PMU_RPC_MSG_ID */
	u8 msg_type;
	/* RPC call flags (@see PMU_RPC_FLAGS)*/
	u8 flags;
	/*
	 * Size of RPC structure allocated
	 *  within NV managed DMEM heap.
	 */
	u16 rpc_dmem_size;
	/*
	 * DMEM pointer of RPC structure allocated
	 * within NV managed DMEM heap.
	 */
	u32 rpc_dmem_ptr;
};

#define NV_PMU_RPC_MSG_ID 0x80U

#endif /* NVGPU_PMUIF_RPC_H */
