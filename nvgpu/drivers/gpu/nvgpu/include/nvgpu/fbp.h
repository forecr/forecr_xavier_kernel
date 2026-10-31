/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_FBP_H
#define NVGPU_FBP_H

/**
 * @file
 *
 * Declares the Public APIs exposed by fbp unit.
 *
 * @page fbp fbp
 *
 * Overview
 * ========
 * The |nvgpu-rm| fbp unit is an interface to the Frame Buffer Partition(FBP)
 * on the GPU. It provides APIs to initialize, query and remove FBP related
 * information. The APIs exposed by fbp unit are used by DEVCTL unit
 * to help nvrm_gpu know about configuration of FBP on GPU device. The FBP
 * initialization API is used by init during GPU initialization.
 *
 * Static Design
 * =============
 *
 * Initialization
 * --------------
 * Initialization involves reading the FBP configuration information like maximum
 * number of FBPs, active FBP mask, mask for Rop_L2 from HW and storing it in
 * unit's private structure.
 *
 * Teardown
 * --------
 * Resources allocated during initialization are freed when device is deinitialized.
 *
 * External APIs
 * -------------
 *   + nvgpu_fbp_init_support()
 *   + nvgpu_fbp_remove_support()
 *   + nvgpu_fbp_get_max_fbps_count()
 *   + nvgpu_fbp_get_fbp_en_mask()
 */
#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_fbp;

/**
 * @brief Read and initialize FBP configuration information in struct nvgpu_fbp.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This API reads various FBP related configuration information like:
 * 1. Maximum number of FBPs from PTOP_SCAL_NUM_FBPS
 * 2. Active FBP mask from the fuse(accessed from GPU MMIO register space)
 *
 * All the above configuration information is stored in a struct nvgpu_fbp
 * and exposed to other units through APIs.
 *
 * @return 0 for success and <0 value for failure.
 * @retval -ENOMEM in case there is insufficient memory to allocate struct
 *         nvgpu_fbp
 */
int nvgpu_fbp_init_support(struct gk20a *g);

/**
 * @brief Removes all the FBP related stored configuration information.
 *
 * @param g [in]	Pointer to GPU driver struct.
 *
 * This API frees up all the memory used to store FBP configuration information
 * and sets the pointer to FBP structure in \a g to NULL.
 */
void nvgpu_fbp_remove_support(struct gk20a *g);


/**
 * @brief Get the maximum number of FBPs as stored in \a fbp.
 *
 * @param fbp [in]	Pointer to FBP configuration structure.
 *
 * This API helps retrieve the FBP data namely maximum number of FBPs from the
 * information stored in \a fbp.
 * During initialization, \a fbp is populated with all the configuration
 * information like max_fbps_count and APIs like this are used to get data from
 * FBP's private structure (struct nvgpu_fbp).
 *
 * @return Maximum number of FBPs as stored in \a fbp.
 */
u32 nvgpu_fbp_get_max_fbps_count(struct nvgpu_fbp *fbp);

/**
 * @brief Gets the active FBP mask as stored in \a fbp.
 *
 * @param fbp [in]	Pointer to FBP configuration structure.
 *
 * This API helps retrieve the FBP data namely active FBP mask from the
 * information stored in \a fbp.
 * During initialization, \a fbp is populated with all the configuration
 * information like fbp_en_mask and APIs like this are used to get data from
 * FBP's private structure (struct nvgpu_fbp).
 *
 * @return Mask corresponding to active FBPs as stored in \a fbp.
 */
u32 nvgpu_fbp_get_fbp_en_mask(struct nvgpu_fbp *fbp);

/**
 * @brief Gets fbp logical index for given physical index.
 *
 * @param fbp [in]		Pointer to FBP configuration structure.
 * @param physical_index [in]	Physical index of the fbp for which we need logical index.
 *
 * This API helps retrieve the FBP logical index for a physcial index from the
 * information stored in \a fbp.
 * During initialization, \a fbp is populated with all the configuration
 * information like fbp_physical_id_map and APIs like this are used to get data from
 * FBP's private structure (struct nvgpu_fbp).
 *
 * @return logical index mapped for the physical index in \a fbp->fbp_physical_id_map.
 */
u32 nvgpu_fbp_get_fbp_logical_index(struct nvgpu_fbp *fbp, u32 physical_index);

/**
 * @brief Gets fbp physical index for given logical index.
 *
 * @param fbp [in]		Pointer to FBP configuration structure.
 * @param logical_index [in]	Logical index of the fbp for which we need physical index.
 *
 * This API helps retrieve the FBP physical index for a logical index from the
 * information stored in \a fbp.
 * During initialization, \a fbp is populated with all the configuration
 * information like fbp_physical_id_map and APIs like this are used to get data from
 * FBP's private structure (struct nvgpu_fbp).
 *
 * @return physcial index mapped for the logical index in \a fbp->fbp_physical_id_map.
 */
u32 nvgpu_fbp_get_fbp_physical_index(struct nvgpu_fbp *fbp, u32 logical_index);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */

u32 nvgpu_fbp_get_num_fbps(struct nvgpu_fbp *fbp);
#ifdef CONFIG_NVGPU_NON_FUSA
u32 *nvgpu_fbp_get_l2_en_mask_base_addr(struct nvgpu_fbp *fbp);
u32 nvgpu_fbp_get_l2_en_mask(struct nvgpu_fbp *fbp, u32 fbp_phy_id);
#endif

/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_FBP_H */
