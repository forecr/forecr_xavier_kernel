/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_GOPS_RAMIN_H
#define NVGPU_GOPS_RAMIN_H

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;

/**
 * @file
 *
 * RAMIN HAL interface.
 */

/**
 * RAMIN HAL operations.
 *
 * @see gpu_ops
 */
struct gops_ramin {
	/**
	 * @brief Sets GR context in Channel Instance Block.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_block [in]	Memory descriptor of Instance Block.
	 * @param gpu_va [in]		GPU VA of GR context.
	 *
	 * This HAL programs GR engine context state address in channel
	 * Instance Block.
	 *
	 * \a gpu_va is the GPU VA of the block of memory that will be
	 * used for storing GR engine context state.
	 */
	void (*set_gr_ptr)(struct gk20a *g,
			struct nvgpu_mem *inst_block, u64 gpu_va);
	/**
	 * @brief Sets size of big pages.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param mem [in]		Memory descriptor of Instance Block.
	 * @param size [in]		Big page size in bytes (e.g #SZ_64K
	 * 				or #SZ_128K).
	 *
	 * Programs the \a size of big pages in a given Instance Block.
	 */
	void (*set_big_page_size)(struct gk20a *g,
			struct nvgpu_mem *mem, u32 size);

	/**
	 * @brief Init Instance Block's PDB.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_block [in]	Memory descriptor of Instance Block.
	 * @param pdb_addr [in]		Page Directory Base (physical address).
	 * @param pdb_mem [in]		Memory descriptor of PDB.
	 *
	 * Initializes Page Directory Base in Instance Block:
	 * - Set aperture for PDB memory, as per pdb_mem descriptor
	 *   (i.e. sysmem/vidmem coherent/non-coherent).
	 * - Set big page size to default (64K).
	 * - Set lo and hi 32-bits of \a pdb_addr.
	 * - Set PT format.
	 * - Set volatile attribute default.
	 *
	 * \a pdb_mem is the DMA memory describing the PTEs or PDEs.
	 *
	 * @see nvgpu_pd_gpu_addr
	 */
	void (*init_pdb)(struct gk20a *g, struct nvgpu_mem *inst_block,
			u64 pdb_addr, struct nvgpu_mem *pdb_mem);

	/**
	 * @brief Init subcontext pdb map for a TSG.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param subctx_pdb_map [in]	Memory pointing to pdb map for a TSG.
	 *
	 * This HAL configures PDB for all subcontexts of an instance block.
	 * It sets all PDBs invalid.
	 */
	void (*init_subctx_pdb_map)(struct gk20a *g,
			u32 *subctx_pdb_map);

	/**
	 * @brief Update subcontext pdb map for subcontext addition/removal.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param subctx_id [in]	Subcontext ID.
	 * @param pdb_mem [in]		Memory descriptor of PDB.
	 * @param replayable [in]	Indicates if errors are replayable
	 *				for this Subcontext.
	 * @param add [in]		Indicate if subcontext PDB is to be
	 *				added or removed.
	 * @param subctx_pdb_map [in]	Memory pointing to pdb map for a TSG.
	 *
	 * This HAL configures PDB for sub-context of Instance Block:
	 * If adding a subcontext PDB:
	 * - Get aperture mask from \a pdb_mem.
	 * - Get physical address of \a pdb_mem.
	 * - Build PDB entry with defaults for PT version, big page size,
	 *   volatile attribute, and above aperture.
	 * - If \a replayable is true, set replayable attribute for TEX
	 *   and GCC faults.
	 * - Set lo and hi 32-bits to point to \a pdb_mem.
	 * - Program related entry in \a subctx_pdb_map.
	 * If removing a subcontext PDB:
	 * - Set aperture as ram_in_sc_page_dir_base_target_invalid_v().
	 * - Program related entry in \a subctx_pdb_map.
	 */
	void (*set_subctx_pdb_info)(struct gk20a *g,
		u32 subctx_id, struct nvgpu_mem *pdb_mem,
		bool replayable, bool add, u32 *subctx_pdb_map);

	/**
	 * @brief Init PDB for sub-contexts.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 * @param inst_block [in]	Memory descriptor of Instance Block.
	 * @param subctx_pdb_map [in]	Memory pointing to pdb map for a TSG.
	 *
	 * This HAL configures PDB for all sub-contexts of Instance Block.
	 * It copies \a subctx_pdb_map to the offset
	 * ram_in_sc_page_dir_base_vol_w(0) * 4U in
	 * the instance block.
	 */
	void (*init_subctx_pdb)(struct gk20a *g,
		struct nvgpu_mem *inst_block, u32 *subctx_pdb_map);

	/**
	 * @brief Set valid subcontexts masks.
	 *
	 * @param g [in]			Pointer to GPU driver struct.
	 * @param inst_block [in]		Memory descriptor of Instance
	 *					Block.
	 * @param valid_subctx_mask [in]	Max number of sub context.
	 *
	 * This HAL configures mask for all sub-contexts of Instance Block:
	 * - Get max number of sub-contexts from HW.
	 * - For each set of 32 subcontexts, set the mask from
	 *   \a valid_subctx_mask in ram_in_sc_pdb_valid_long_w().
	 */
	void (*init_subctx_mask)(struct gk20a *g,
		struct nvgpu_mem *inst_block, unsigned long *valid_subctx_mask);

	/**
	 * @brief Instance Block shift.
	 *
	 * Returns Instance Block shift in bits, as defined in hardware manuals.
	 * This is the amount of bits that should be 0 in the physical address
	 * of the Instance Block. Therefore it defines the expected alignement
	 * for the Instance Block address.
	 *
	 * @return Instance block shift in bits.
	 */
	u32 (*base_shift)(void);

	/**
	 * @brief Instance Block size.
	 *
	 * Returns Instance Block size, as defined in hardware manuals.
	 *
	 * @returns Instance Block size in bytes.
	 */
	u32 (*alloc_size)(void);

	/** @cond DOXYGEN_SHOULD_SKIP_THIS */

#ifdef CONFIG_NVGPU_DGPU
	/**
	 * @brief Init fix for PDB cache.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This HAL allows implementing chip specific initialization
	 * related to PDB cache.
	 *
	 * @return 0 in case of success, < 0 in case of failure.
	 */
	int (*init_pdb_cache_errata)(struct gk20a *g);

	/**
	 * @brief Deinit fix for PDB cache.
	 *
	 * @param g [in]		Pointer to GPU driver struct.
	 *
	 * This HAL allows implementing chip specific de-initialization
	 * related to PDB cache.
	 */
	void (*deinit_pdb_cache_errata)(struct gk20a *g);
#endif

	void (*set_adr_limit)(struct gk20a *g,
			struct nvgpu_mem *inst_block, u64 va_limit);
	/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

	/**
	 * @brief Set the engine method buffer address in the RAMIN area.
	 *
	 * This function programs the address of the engine method buffer into the
	 * instance block of a channel. The steps are as follows:
	 * -# Extract the lower 32 bits of the GPU virtual address of the engine method
	 *    buffer and write it to the lower address field of the engine method buffer
	 *    in the instance block using nvgpu_mem_wr32().
	 * -# Extract the upper 32 bits of the GPU virtual address of the engine method
	 *    buffer and write it to the higher address field of the engine method buffer
	 *    in the instance block using nvgpu_mem_wr32().
	 *
	 * @param [in] g          Pointer to the GPU driver structure.
	 * @param [in] inst_block Pointer to the instance block memory descriptor.
	 * @param [in] gpu_va     GPU virtual address of the engine method buffer.
	 */
	void (*set_eng_method_buffer)(struct gk20a	 *g,
			struct nvgpu_mem *inst_block, u64 gpu_va);

	/**
	 * @brief Set a magic value in the RAMIN area for engine firmware.
	 *
	 * This function programs a predefined magic value into the instance block of a
	 * channel to indicate the presence of engine firmware. The steps are as follows:
	 * -# Write the predefined magic value #ram_in_engine_fw_magic_value_v() to the
	 *    magic value field #ram_in_engine_fw_magic_value_w() in the instance block using
	 *    nvgpu_mem_wr32().
	 *
	 * @param [in] g          Pointer to the GPU driver structure.
	 * @param [in] inst_block Pointer to the instance block memory descriptor.
	 */
	void (*set_magic_value)(struct gk20a *g,
			struct nvgpu_mem *inst_block);
};

#endif
