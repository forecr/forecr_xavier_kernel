/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef CBC_GA10B_H
#define CBC_GA10B_H

#ifdef CONFIG_NVGPU_COMPRESSION

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_cbc;
/**
 * @brief This function is used to allocate comptag memory and configure
 *   bit map allocator to the clients requesting compression.
 *
 * - Compbit backing store is a memory buffer to store compressed data
 *   corresponding to total compressible memory.
 * - In GA10B, 1 ROP tile = 256B data is compressed to 1B compression
 *   bits. i.e. 1 GOB = 512B data is compressed to 2B compbits.
 * - A comptagline is a collection of compbits corresponding to a
 *   compressible page size. In GA10B, compressible page size is 64KB.
 *
 * - GA10B has 2 LTCs with 4 slices each. A 64KB page is distributed
 *   into 8 slices having 8KB (16 GOBs) data each.
 * - Thus, each comptagline per slice contains compression status bits
 *   corresponding to 16 GOBs.
 *
 * Address calculation for CBC applies swizzle to the lower 16 bits
 * of physical address. So, CBC start and end address should be 64KB
 * aligned.
 * Memory allocated is aligned corresponding to PAGE_SIZE and can be
 * seen as:
 *
 * ------------------------ Allocated physical memory end address
 * ^     -------------- 64KB aligned CBC end address
 * |             ^
 * | allocated   |
 * | physical    |
 * | address     | CBC occupied
 * | space       | address space
 * |             |
 * |             v
 * v     -------------- 64KB aligned CBC start address
 * ------------------------ Allocated physical memory start address
 *
 * With PAGE_SIZE other than 64KB, the physical memory start address
 * may not be 64KB aligned. So, choose CBC start address to be the
 * lower 64KB multiple within the allocated memory.
 * This function is used to allocate comptag memory and configure
 * bit map allocator to the clients requesting compression
 *
 * Compute required compatags by querying total system memory and reading
 * required registers.
 * Initialize and allocate required comptags memory by calling #nvgpu_cbc_alloc.
 * Initialize the bit map allocator for the clients(consuming the compags)
 *  by calling #gk20a_comptag_allocator_init
 *
 * @param g	[in]	Pointer to the gk20a structure.
 * @param cbc	[in]	Pointer to the #nvgpu_cbc structure.
 *
 * @return 0 on success, < 0 in case of failure.
 */
int ga10b_cbc_alloc_comptags(struct gk20a *g, struct nvgpu_cbc *cbc);

/**
 * @brief This function is used to check the contig_pool supported by
 *  device tree or not.
 *
 * This function is expected to return true always.
 *
 * @param g     [in]    Pointer to the gk20a structure.
 *
 * @return always true.
 */
bool ga10b_cbc_use_contig_pool(struct gk20a *g);
#endif
#endif /* CBC_GA10B_H */
