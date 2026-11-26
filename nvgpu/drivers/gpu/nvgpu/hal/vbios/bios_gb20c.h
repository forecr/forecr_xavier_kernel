/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GB20C_VBIOS_H
#define NVGPU_GB20C_VBIOS_H

// VBIOS ROM starts in the SR at a fixed offset at 512KB.
#define BIOS_OFFSET (512ULL * 1024ULL)
#define GB20C_BIOS_SIZE 0x101894	// Todo: adjust to final VBIOS size.
#define VBIOS_SR_INDEX	58ULL

struct gk20a;
/**
 * Get VBIOS base address for GB20C architecture
 *
 * GB20C stores VBIOS ROM in a secure memory region that requires special
 * SSMPU register access to locate and map. This function provides the
 * OS-specific implementation to retrieve the VBIOS base address.
 *
 * @param g - GPU device structure
 * @return mapped VBIOS base address or NULL on failure
 */
void *nvgpu_gb20c_get_vbios_addr(struct gk20a *g);

/**
 * GB20C SSMPU secure region accessor for VBIOS ROM
 *
 * Reads the SSMPU register for the specified secure region index to obtain
 * the VBIOS ROM base address and maps it to virtual memory for access.
 *
 * @param g - GPU device structure
 * @param sr_index - SSMPU secure region index (typically 58 for VBIOS)
 * @return mapped VBIOS base address on success, NULL on failure
 */
void *check_ssmpu_sr_index(struct gk20a *g, u32 sr_index);

#endif /* NVGPU_GB20C_VBIOS_H */
