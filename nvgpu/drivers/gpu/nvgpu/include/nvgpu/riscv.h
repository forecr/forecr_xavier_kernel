/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_RISCV_H
#define NVGPU_RISCV_H

#include <nvgpu/types.h>
#include <nvgpu/falcon.h>

struct gk20a;
struct nvgpu_falcon;
struct nvgpu_firmware;

/*
 * Structure for nvriscv firmware descriptor information.
 */
struct nvgpu_nvriscv_fw_desc {
	/*
	 * Descriptor structure version
	 * Version 1
	 * Version 2
	 * Version 3 = for Partition boot
	 * Version 4 = for eb riscv boot
	 * Version 5 = Support signing entire RISC-V image as "code"
	 *             in code section.
	 */
	u32 version;
	/* Bootloader offset */
	u32 bootloaderOffset;
	/* Bootloader size */
	u32 bootloaderSize;
	/* Bootloader parameter offset */
	u32 bootloaderParamOffset;
	/* Bootloader parameter size */
	u32 bootloaderParamSize;
	/* riscv elf offset */
	u32 riscvElfOffset;
	/* riscv elf size */
	u32 riscvElfSize;
	/* Changelist number associated with the image */
	u32  appVersion;
	/*
	 * Manifest contains information about Monitor and it is input to BR.
	 * Manifest offset
	 */
	u32  manifestOffset;
	/* Manifest size */
	u32  manifestSize;
	/* Monitor Data offset within RISCV image */
	u32  monitorDataOffset;
	/* Monitor Data size within RISCV image */
	u32  monitorDataSize;
	/* Monitor Code offset within RISCV image */
	u32  monitorCodeOffset;
	/* Monitor Code size within RISCV image */
	u32  monitorCodeSize;
	/* Indicates whether monitor is enabled */
	u32  bIsMonitorEnabled;
	/* Swbrom Code offset within RISCV image */
	u32  swbromCodeOffset;
	/* Swbrom Code size within RISCV image */
	u32  swbromCodeSize;
	/* Swbrom Data offset within RISCV image */
	u32  swbromDataOffset;
	/* Swbrom Data size within RISCV image */
	u32  swbromDataSize;
	/* Total size of FB carveout (image and reserved space) */
	u32  fbReservedSize;
	/* Indicates whether the entire RISC-V image is signed as "code" in code section */
	u32 bSignedAsCode;
};

/**
 * Indexes for nvriscv descriptor info needed for BCR config.
 */
enum {
	/** code offset */
	NVGPU_NVRISCV_CODE_OFFSET = 0U,
	/** code size */
	NVGPU_NVRISCV_CODE_SIZE = 1U,
	/** data offset */
	NVGPU_NVRISCV_DATA_OFFSET = 2U,
	/** data size */
	NVGPU_NVRISCV_DATA_SIZE = 3U,
	/** manifest offset */
	NVGPU_NVRISCV_MANIFEST_OFFSET = 4U,
	/** manifest size */
	NVGPU_NVRISCV_MANIFEST_SIZE = 5U,
	/** max index */
	NVGPU_NVRISCV_INDEX_MAX = 6U
};

/**
 * @brief Read the riscv register.
 *
 * @param flcn [in] The falcon.
 * @param offset [in] offset of the register.
 *
 * This function is called to read a register with common flcn offset.
 *
 * Steps:
 * - Read and return data from register at \a offset from
 *   the base of riscv core of \a flcn.
 *
 * @return register data.
 */
u32 nvgpu_riscv_readl(struct nvgpu_falcon *flcn, u32 offset);

/**
 * @brief Write to the riscv register.
 *
 * @param flcn [in] The falcon.
 * @param offset [in] Index of the register.
 * @param data [in] Data to be written to the register.
 *
 * This function is called to write to a register with common flcn offset.
 *
 * Steps:
 * - Write \a data to register at \a offet from base of
 *   riscv core of \a flcn.
 */
void nvgpu_riscv_writel(struct nvgpu_falcon *flcn,
                                       u32 offset, u32 val);

/**
 * @brief Dump RISCV BootROM status.
 *
 * @param flcn [in] The falcon.
 *
 * This function is called to get the status of RISCV BootROM.
 *
 * Steps:
 * - Print the flcn's RISCV BCR control configuratation.
 * - Print the flcn's RISCV BR priv lockdown status.
 * - Print the flcn's BR retcode value.
 */
void nvgpu_riscv_dump_brom_stats(struct nvgpu_falcon *flcn);

/**
 * @brief Get the size of falcon's memory.
 *
 * @param flcn [in] The falcon.
 * @param type [in] Falcon memory type (IMEM, DMEM).
 *		    - Supported types: MEM_DMEM (0), MEM_IMEM (1)
 * @param size [out] Size of the falcon memory type.
 *
 * This function is called to get the size of falcon's memory for validation
 * while copying to IMEM/DMEM.
 *
 * Steps:
 * - Validate that the passed in falcon struct is not NULL and is for supported
 *   falcon. If not valid, return -EINVAL.
 * - Read the size of the falcon memory of \a type in bytes from the HW config
 *   register in output parameter \a size.
 *
 * @return 0 in case of success, < 0 in case of failure.
 */
int nvgpu_riscv_hs_ucode_load_bootstrap(struct nvgpu_falcon *flcn,
						struct nvgpu_firmware *manifest_fw,
						struct nvgpu_firmware *code_fw,
						struct nvgpu_firmware *data_fw,
						u64 ucode_sysmem_desc_addr);

/**
 * @brief Boot nvriscv based engine.
 *
 * @param flcn    [in] The falcon.
 * @param fw_mem  [in] Pointer to nvgpu_mem where firmware is present.
 * @param fw_desc [in] Offset and size information of the firmware.
 *
 * This function is called to boot the nvriscv based engine.
 *
 * Steps:
 * - Reset the nvriscv core.
 * - Program interrupt mask and routing registers.
 * - Setup boot-config.
 * - Write debuginfo register with dummy value.
 * - Dma the firmware for nvriscv-eb based engines.
 * - Program interface enable register.
 * - Program bcr registers.
 * - Kick-start nvriscv core.
 * - Wait for engine idle for nvriscv-eb based engine or
 *   wait for BROM completion for nvriscv with BR.
 * - Check if it has reached a proper initialized state,
 *   by polling for debuginfo clearance.
 *
 * @return 0 in case of success, < 0 in case of failure.
 * @retval -ETIMEDOUT in case of wait timeout.
 */
s32 nvgpu_nvriscv_ns_boot(struct nvgpu_falcon *flcn, struct nvgpu_mem *fw_mem,
			u32 *fw_desc);
#endif /* NVGPU_RISCV_H */
