/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_PMUIF_ACR_H
#define NVGPU_PMUIF_ACR_H

#include <nvgpu/types.h>
#include "cmn.h"

/* ACR Commands/Message structures */

enum {
	PMU_ACR_CMD_ID_INIT_WPR_REGION = 0x0,
	PMU_ACR_CMD_ID_BOOTSTRAP_FALCON,
	PMU_ACR_CMD_ID_RESERVED,
	PMU_ACR_CMD_ID_BOOTSTRAP_MULTIPLE_FALCONS,
};

/*
 * Initializes the WPR region details
 */
struct pmu_acr_cmd_init_wpr_details {
	u8  cmd_type;
	u32 regionid;
	u32 wproffset;

};

/*
 * falcon ID to bootstrap
 */
struct pmu_acr_cmd_bootstrap_falcon {
	u8 cmd_type;
	u32 flags;
	u32 falconid;
};

/*
 * falcon ID to bootstrap
 */
struct pmu_acr_cmd_bootstrap_multiple_falcons {
	u8 cmd_type;
	u32 flags;
	u32 falconidmask;
	u32 usevamask;
	struct falc_u64 wprvirtualbase;
};

#define PMU_ACR_CMD_BOOTSTRAP_FALCON_FLAGS_RESET_NO  1U
#define PMU_ACR_CMD_BOOTSTRAP_FALCON_FLAGS_RESET_YES 0U


struct pmu_acr_cmd {
	union {
		u8 cmd_type;
		struct pmu_acr_cmd_bootstrap_falcon bootstrap_falcon;
		struct pmu_acr_cmd_init_wpr_details init_wpr;
		struct pmu_acr_cmd_bootstrap_multiple_falcons boot_falcons;
	};
};

/* acr messages */

/*
 * returns the WPR region init information
 */
#define PMU_ACR_MSG_ID_INIT_WPR_REGION   0U

/*
 * Returns the Bootstrapped falcon ID to RM
 */
#define PMU_ACR_MSG_ID_BOOTSTRAP_FALCON  1U

/*
 * Returns the WPR init status
 */
#define PMU_ACR_SUCCESS                  0U
#define PMU_ACR_ERROR                    1U

/*
 * PMU notifies about bootstrap status of falcon
 */
struct pmu_acr_msg_bootstrap_falcon {
	u8 msg_type;
	union {
		u32 errorcode;
		u32 falconid;
	};
};

struct pmu_acr_msg {
	union {
		u8 msg_type;
		struct pmu_acr_msg_bootstrap_falcon acrmsg;
	};
};

/* ACR RPC */
#define NV_PMU_RPC_ID_ACR_INIT_WPR_REGION      0x00U
#define NV_PMU_RPC_ID_ACR_WRITE_CBC_BASE       0x01U
#define NV_PMU_RPC_ID_ACR_BOOTSTRAP_FALCON     0x02U
#define NV_PMU_RPC_ID_ACR_BOOTSTRAP_GR_FALCONS 0x03U
#define NV_PMU_RPC_ID_ACR__COUNT               0x04U

/*
 * structure that holds data used
 * to execute INIT_WPR_REGION RPC.
 */
struct nv_pmu_rpc_struct_acr_init_wpr_region {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	/*[IN] ACR region ID of WPR region */
	u32 wpr_regionId;
	/* [IN] WPR offset from startAddress */
	u32 wpr_offset;
	u32 scratch[1];
};

/*
 * structure that holds data used to
 * execute BOOTSTRAP_GR_FALCONS RPC.
 */
struct nv_pmu_rpc_struct_acr_bootstrap_gr_falcons {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;
	/* [IN] Mask of falcon IDs @ref FALCON_ID_<XYZ> */
	u32  falcon_id_mask;
	/*
	 * [IN] Boostrapping flags @ref
	 * PMU_ACR_CMD_BOOTSTRAP_FALCON_FLAGS_<XYZ>
	 */
	u32 flags;
	/* [IN] Indicate whether the particular falon uses VA */
	u32  falcon_va_mask;
	/*
	 * [IN] WPR Base Address in VA. The Inst Block containing
	 * this VA should be bound to both PMU and GR falcons
	 * during the falcon boot
	 */
	struct falc_u64  wpr_base_virtual;
	u32  scratch[1];
};

/*
 * structure that holds data used to
 * execute BOOTSTRAP_FALCON RPC.
 */
struct nv_pmu_rpc_struct_acr_bootstrap_falcon {
	/*[IN/OUT] Must be first field in RPC structure */
	struct nv_pmu_rpc_header hdr;

	/* [IN] Falcon ID */
	u32 falcon_id;

	/* [IN] Engine Instance to be bootstrapped by ACR */
	u32 engine_instance;

	/* [IN] Mask of indexes of same engine to be multi bootstrapped by ACR */
	u32 engine_index_mask;

	/*
	 * [IN] Boostrapping flags
	 */
	u32 flags;

	u32  scratch[1];
};

#endif /* NVGPU_PMUIF_ACR_H */
