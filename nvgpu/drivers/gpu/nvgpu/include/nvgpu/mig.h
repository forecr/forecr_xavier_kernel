/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MIG_H
#define NVGPU_MIG_H

#include <nvgpu/types.h>
#include <nvgpu/lock.h>

/** Maximum GPC group supported by HW. */
#define NVGPU_MIG_MAX_GPCGRP				2U

/** Maximum gpu instances count (1 Physical + 8 MIGs). */
#define NVGPU_MIG_MAX_GPU_INSTANCES			9U

/** Maximum mig config count. */
#define NVGPU_MIG_MAX_MIG_CONFIG_COUNT			16U

/** INVALID sys pipe id. */
#define NVGPU_MIG_INVALID_GR_SYSPIPE_ID			(~U32(0U))

/** Maximum engine slot count. */
#define NVGPU_MIG_MAX_ENGINES				32U

/** Maximum config name size. */
#define NVGPU_MIG_MAX_CONFIG_NAME_SIZE			256U

/** Maximum number of GPC count. */
#define NVGPU_MIG_MAX_GPCS				32U

/** Maximum number of FBP count. */
#define NVGPU_MIG_MAX_FBPS				12U

/** Enumerated type used to identify various gpu instance types */
enum nvgpu_mig_gpu_instance_type {
	NVGPU_MIG_TYPE_PHYSICAL = 0,
	NVGPU_MIG_TYPE_MIG
};

/**
 * @brief GPC Id information.
 * This struct describes the logical, physical and gpcgrp id of each GPC.
 */
struct nvgpu_gpc {
	/** Logical GPC Id which is used to access GPC registers. */
	u32 logical_id;
	/** Physical GPC Id. */
	u32 physical_id;
	/** GPC group Id. */
	u32 gpcgrp_id;
};

/**
 * @brief GR syspipe information.
 * This struct describes the number of gpc, physical_gpc_mask, veid, etc
 * associated to a particualr gr syspipe.
 */
struct nvgpu_gr_syspipe {
	/** GR sys pipe instance Id */
	u32 gr_instance_id;
	/** GR syspipe id which is used to set gr remap window */
	u32 gr_syspipe_id;

	/**
	 * GR device that belongs to this syspipe.
	 */
	const struct nvgpu_device *gr_dev;

	u32 num_gpc;

	/** GPC Id information (logical, physical and gpcgrp Ids). */
	struct nvgpu_gpc gpcs[NVGPU_MIG_MAX_GPCS];

	/**
	 * Mask of local GPCs belongs to this syspipe. A set bit indicates
	 * GPC is available, otherwise it is not available.
	 */
	u32 gpc_mask;
	/** Maximum veid allocated to this gr syspipe. */
	u32 max_veid_count_per_tsg;
	/** VEID start offset. */
	u32 veid_start_offset;
	/** veid_mask to be programmed in the smc_veid_config */
	u32 veid_mask;
};

/**
 * @brief GPU instance information.
 * This struct describes the gr_syspipe, LCEs, etc associated
 * to a particualr gpu instance.
 */
struct nvgpu_gpu_instance {
	/** GPU instance Id */
	u32 gpu_instance_id;
	/** GR syspipe information. */
	struct nvgpu_gr_syspipe gr_syspipe;
	/** Number of Logical CE engine associated to this gpu instances. */
	u32 num_lce;
	/** Number of Logical FBP associated to this gpu instances. */
	u32 num_fbp;
	/**
	 * Mask of FBPs. A set bit indicates FBP is available, otherwise
	 * it is not available.
	 * For Legacy and MIG, it currently represents physical FBP mask.
	 * [TODO]: When SMC memory partition will be enabled, a mapping should
	 * be created for local to physical.
	 */
	u32 fbp_en_mask;
	/**
	 * Array to hold physical masks of LTCs per FBP.
	 * For Legacy and MIG, array is currently indexed by FBP physical index.
	 * [TODO]: When SMC memory partition will be enabled, a mapping should
	 * be created for local to {logical, physical}.
	 */
	u32 *fbp_l2_en_mask;

	/* Array to hold the logical Ids of the fbp corresponding to the local Ids
	 */
	u32 fbp_mappings[NVGPU_MIG_MAX_FBPS];

	/** Memory area to store h/w CE engine ids. */
	const struct nvgpu_device *lce_devs[NVGPU_MIG_MAX_ENGINES];
	/** Memory area to store h/w MM engine ids. */
	const struct nvgpu_device *ofa_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvjpg_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvenc_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvdec_devs[NVGPU_MIG_MAX_ENGINES];
	u32 num_ofa;
	u32 num_nvjpg;
	u32 num_nvenc;
	u32 num_nvdec;
	/** Memory area to store h/w GSP information. */
	const struct nvgpu_device *nvgsp_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvgsplite_devs[NVGPU_MIG_MAX_ENGINES];
	u32 num_nvgsp;
	u32 num_nvgsplite;

	/* Flag to indicate whether memory partition is supported or not. */
	bool is_memory_partition_supported;
	/** Enumerated type used to identify various gpu instance types */
	enum nvgpu_mig_gpu_instance_type gpu_instance_type;
	/** The mig instance supports GFX or not */
	bool is_gfx_capable;
	/** VEID mask for the mig instance */
	u32 veid_mask;
	/** LTS mask to be programmed for the mig instance */
	u32 lts_mask;
	/** LTC enabled for the instance. */
	u32 logical_ltc_mask;
	/** L2 slides enbled for the given enabled LTCs. */
	u64 logical_lts_mask;
	/** Swizzle id for the instance */
	u32 swizzle_id;
};

/**
 * @brief GPU instance static configuration information.
 * This struct describes the gpu_instance_id, number of gpc, gr_syspipe_id,
 * veid, etc associated to a particualr static congig.
 */
struct nvgpu_gpu_instance_static_config {
	/** GPU instance Id */
	u32 gpu_instance_id;
	/** GR syspipe id which is used to set gr remap window */
	u32 gr_syspipe_id;
	/** Number of GPC assigned to this config. */
	u32 num_gpc;
	/** Gfx capable */
	bool is_gfx_capable;
	/** Swizzle id for memory partition support */
	u32 swizzle_id;
	/** The mig instance supports memory_partition or not */
	bool is_memory_partition_supported;
	/** lts_mask for the corresponding swizzle id. */
	u32 lts_mask;
	/** veid_mask to be programmed in the smc_veid_config */
	u32 veid_mask;
	/** num_fbps associated with the instance. */
	u32 num_fbps;
	u32 fbp_en_mask;

	/** Memory area to store h/w MM engine ids. */
	const struct nvgpu_device *ofa_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvjpg_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvenc_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvdec_devs[NVGPU_MIG_MAX_ENGINES];
	u32 num_ofa;
	u32 num_nvjpg;
	u32 num_nvenc;
	u32 num_nvdec;
	/** Memory area to store h/w GSP information. */
	const struct nvgpu_device *nvgsp_devs[NVGPU_MIG_MAX_ENGINES];
	const struct nvgpu_device *nvgsplite_devs[NVGPU_MIG_MAX_ENGINES];
	u32 num_nvgsp;
	u32 num_nvgsplite;
};

/**
 * @brief GPU instance configuration information.
 * This struct describes the number of gpu instances, gr_syspipe, LCEs, etc
 * associated to a particualr mig config.
 */
struct nvgpu_gpu_instance_config {
	/** Name of the gpu instance config. */
	char config_name[NVGPU_MIG_MAX_CONFIG_NAME_SIZE];
	/** Number of gpu instance associated to this config. */
	u32 num_gpu_instances;
	/**
	 * Array of gpu instance static config information associated
	 * to this config (gpu_instance_id, gr_syspipe_id, num_gpc, etc).
	 */
	struct nvgpu_gpu_instance_static_config
		gpu_instance_static_config[NVGPU_MIG_MAX_GPU_INSTANCES];
};

/**
 * @brief MIG configuration options.
 * This struct describes the various number of mig gpu instance configuration
 * supported by a particual GPU.
 */
struct nvgpu_mig_gpu_instance_config {
	/** Total Number of GR syspipe is supported by HW after floor swept. */
	u32 usable_gr_syspipe_count;
	/** Usable GR sys pipe mask. */
	u32 usable_gr_syspipe_mask;
	/** Number of gpu instance configurations. */
	u32 num_config_supported;
	/** Total Number of GPCs (priv_ring enumerated (floor swept) value). */
	u32 gpc_count;
	/** GPC count associated to each GPC group. */
	u32 gpcgrp_gpc_count[NVGPU_MIG_MAX_GPCGRP];
	/** Array of gpu instance configuration information. */
	struct nvgpu_gpu_instance_config
		gpu_instance_config[NVGPU_MIG_MAX_MIG_CONFIG_COUNT];
};

/**
 * @brief Multi Instance GPU information.
 * This struct describes the mig top level information supported
 * by a particual GPU.
 */
struct nvgpu_mig {
	/** Total Number of GR syspipe is supported by HW after floor swept. */
	u32 usable_gr_syspipe_count;
	/** Usable GR sys pipe mask. */
	u32 usable_gr_syspipe_mask;
	/** Array of usable GR sys pipe instance id. */
	u32 usable_gr_syspipe_instance_id[NVGPU_MIG_MAX_ENGINES];
	/**
	 * Max possible number of GPCs in GR engines.
	 */
	u32 max_gpc_count;
	/** Total Number of GPCs (priv_ring enumerated (floor swept) value). */
	u32 gpc_count;
	/** GPC count associated to each GPC group. */
	u32 gpcgrp_gpc_count[NVGPU_MIG_MAX_GPCGRP];
	/** Enabled gpu instances count. */
	u32 num_gpu_instances;
	/** Maximum gr sys pipes are supported by HW. */
	u32 max_gr_sys_pipes_supported;
	/** Maximum fbps are supported by HW. */
	u32 max_fbps_count;
	/** Total number of enabled gr syspipes count. */
	u32 num_gr_sys_pipes_enabled;
	/** GR sys pipe enabled mask. */
	u32 gr_syspipe_en_mask;
	/**
	 * Current gr syspipe id.
	 * It is valid if num_gr_sys_pipes_enabled > 1.
	 */
	u32 current_gr_syspipe_id;
	/*
	 * Current GR instance being programmed.
	 * Defaults to zero for non-MIG cases. Respective GR instance id in
	 * case MIG support is enabled.
	 */
	u32 cur_gr_instance;
	/*
	 * Current thread Id which holds gr_syspipe_lock.
	 */
	int cur_tid;
	/*
	 * Recursive gr_syspipe_lock count.
	 */
	u32 recursive_ref_count;
	/**
	 * GR syspipe acquire lock.
	 * It is valid lock if num_gr_sys_pipes_enabled > 1.
	 */
	struct nvgpu_mutex gr_syspipe_lock;
	/** Gpu instance configuration id. */
	u32 current_gpu_instance_config_id;
	/**
	 * Flag to indicate whether nonGR(CE) engine is sharable
	 * between gr syspipes or not.
	 */
	bool is_nongr_engine_sharable;
	/** Array of enabled gpu instance information. */
	struct nvgpu_gpu_instance
		gpu_instance[NVGPU_MIG_MAX_GPU_INSTANCES];
};

#endif /* NVGPU_MIG_H */
