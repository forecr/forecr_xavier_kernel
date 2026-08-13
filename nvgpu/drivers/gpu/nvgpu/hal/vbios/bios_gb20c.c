// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include "bios_gb20c.h"

// SSMPU SR register index for VBIOS SR Start address
#define VBIOS_SR_INDEX	58ULL

// SSMPU register addresses
#define SSMPU_BASE	0x1182A000ULL
#define SSMPU_SR_REG_BASE(srIdx) (SSMPU_BASE + (0x80ULL * ((srIdx  - 1ULL) & 0x03FULL)))
#define SSMPU_SR_START_ADDRESS_REG(srIdx) (SSMPU_SR_REG_BASE(srIdx))
#define SSMPU_SR_END_ADDRESS_REG(srIdx) (SSMPU_SR_REG_BASE(srIdx) + 0x04ULL)
#define SSMPU_SR_ADDRESS_MASK (0x03FFFFFFULL)
#define SR_APCPU_TRANSLATE 0x80000000ULL


void *check_ssmpu_sr_index(struct gk20a *g, u32 sr_index)
{
	uintptr_t ssmpu_address = 0ULL;
	uintptr_t ssmpu_mapped = 0ULL;
	uintptr_t bios_mapped = 0ULL;
	u32 ssmpu_value = 0;
	u32 ssmpu_masked = 0;
	uintptr_t bios_address = 0ULL;

	// Get pointer to VBIOS in overlapped SR
	ssmpu_address = SSMPU_SR_START_ADDRESS_REG(sr_index);

	// Map the SSMPU register to virtual memory before accessing it
	ssmpu_mapped = nvgpu_io_map(g, ssmpu_address, sizeof(u32));
	if (ssmpu_mapped == 0ULL) {
		nvgpu_err(g, "Failed to map SSMPU register at 0x%lx", ssmpu_address);
		return NULL;
	}

	// Read the SSMPU register value
	ssmpu_value = nvgpu_os_readl(ssmpu_mapped);
	nvgpu_io_unmap(g, ssmpu_mapped, sizeof(u32));

	// Validate SSMPU value
	if (ssmpu_value == 0 || ssmpu_value == 0xFFFFFFFF) {
		nvgpu_err(g, "Invalid SSMPU value: 0x%x", ssmpu_value);
		return NULL;
	}

	// Calculate BIOS address
	ssmpu_masked = ssmpu_value & SSMPU_SR_ADDRESS_MASK;
	bios_address = (uintptr_t)ssmpu_masked;

	if (bios_address != 0ULL) {
		bios_address <<= 12;	// SSMPU address unit is 4KB
		bios_address += SR_APCPU_TRANSLATE;
		bios_address += BIOS_OFFSET;

		// Map the full BIOS region
		bios_mapped = nvgpu_io_map(g, bios_address, GB20C_BIOS_SIZE);
		if (bios_mapped == 0ULL) {
			nvgpu_err(g, "Failed to map BIOS region at 0x%lx", bios_address);
			return NULL;
		}

		return (void *)bios_mapped;
	}

	return NULL;
}
