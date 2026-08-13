/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SGT_OS_H
#define NVGPU_SGT_OS_H

/**
 * @file
 *
 * Abstract interface for exposing the OS interface for creating an nvgpu_sgt
 * from an nvgpu_mem. The reason this is handled by the OS is because the
 * nvgpu_mem - especially the part that defines the underlying SGT - is
 * intrinsically tied to the OS.
 */

struct gk20a;
struct nvgpu_sgt;
struct nvgpu_mem;

/**
 * @brief Create a scatter gather table (SGT) from a memory descriptor.
 *
 * This function creates a scatter gather table (SGT) from a given memory
 * descriptor (mdesc) within an nvgpu_mem structure. It handles the case where
 * the memory is video memory (vidmem) and when the memory has been created as
 * a shadow copy from another memory allocation. The function sets the SGT
 * operations to the os-specific specific SGT operations.
 *
 * Since a DMA allocation may well be discontiguous nvgpu requires that a
 * table describe the chunks of memory that make up the DMA allocation. This
 * scatter gather table, SGT, must be created from an nvgpu_mem. Since the
 * representation of a DMA allocation varies wildly from OS to OS the OS is
 * tasked with creating an implementation of the nvgpu_sgt op interface.
 *
 * This function should be defined by each OS. It should create an nvgpu_sgt
 * object from the passed nvgpu_mem. The nvgpu_mem and the nvgpu_sgt together
 * abstract the DMA allocation in such a way that the GMMU can map any buffer
 * from any OS.
 *
 * The nvgpu_sgt object returned by this function must be freed by the
 * nvgpu_sgt_free() function.
 *
 * The steps performed by the function are as follows:
 * -# Check if the memory is video memory and if so, return the SGT obtained
 *    from the video memory using nvgpu_mem_get_sgt_from_vidmem().
 * -# If the memory has the #NVGPU_MEM_FLAG_SHADOW_COPY flag set, retrieve the
 *    offset from the memory descriptor.
 * -# Create the SGT from the memory descriptor using #nvgpu_mem_sgt_create_from_mdesc()
 *    with the obtained offset.
 * -# If the SGT creation is successful, set the SGT operations to os-specific
 *    operations.
 * -# Return the created SGT.
 *
 * @param [in]  g    The GPU for which the SGT is being created.
 * @param [in]  mem  The memory descriptor from which to create the SGT.
 *
 * @return Pointer to the created nvgpu_sgt object on success.
 * @return NULL if the creation fails.
 */
struct nvgpu_sgt *nvgpu_sgt_os_create_from_mem(struct gk20a *g,
					       struct nvgpu_mem *mem);

int nvgpu_sgt_os_init_mem_from_iova(struct gk20a *g, struct nvgpu_mem *mem,
				u64 syncpt_addr, u64 nr_pages);

#endif /* NVGPU_SGT_OS_H */
