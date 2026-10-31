/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2011-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PBDMA_COMMON_H
#define NVGPU_PBDMA_COMMON_H

#include <nvgpu/types.h>

/**
 * @file
 *
 * Push-Buffer DMA
 *
 * PBDMA unit fetches pushbuffer data from memory, generates
 * commands, called "methods", from the fetched data, executes some of the
 * generated methods itself, and sends the remainder of the methods to engines.
 */
struct gk20a;

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#define PBDMA_PER_RUNLIST_SIZE		2U
#define NVGPU_INVALID_PBDMA_PRI_BASE	U32_MAX
#define NVGPU_INVALID_PBDMA_ID		U32_MAX

struct nvgpu_pbdma_info {
	/** The pri offset of the i'th PBDMA for runlist_pri_base */
	u32 pbdma_pri_base[PBDMA_PER_RUNLIST_SIZE];
	/** The ID of the i'th PBDMA that runs channels on this runlist */
	u32 pbdma_id[PBDMA_PER_RUNLIST_SIZE];
};
/** @endcond */

/**
 * @brief Set up the PBDMA software context.
 *
 * This function initializes the PBDMA software context by setting up interrupt
 * descriptors. The steps involved are:
 * -# Call the function nvgpu_pbdma_init_intr_descs() to initialize PBDMA interrupt
 *    descriptors.
 * -# Return 0 indicating success.
 *
 * @param [in] g  The GPU driver struct for which to set up the PBDMA software context.
 *
 * @return 0 indicating success.
 */
int nvgpu_pbdma_setup_sw(struct gk20a *g);

/**
 * @brief the function does nothing
 */
void nvgpu_pbdma_cleanup_sw(struct gk20a *g);

#endif /* NVGPU_PBDMA_COMMON_H */
