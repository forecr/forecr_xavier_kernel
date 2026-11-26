// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2025-2026, NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bios.h>
#include <nvgpu/nvgpu_common.h>
#include <nvgpu/timers.h>
#include <nvgpu/io.h>
#include <nvgpu/gk20a.h>
#include "bios_sw_gb20c.h"
#include <nvgpu/log.h>
#include <nvgpu/nvgpu_mem.h>

#include <hal/vbios/bios_gb20c.h>

/* The minimum VBIOS version supported */
#define NVGPU_GB20C_MIN_VBIOS      0x9a0c0000U
#define FFL_FIRMWARE_INSTANCE_MAGIC_DWORD_VALUE 0x5746564e // "NVFW"
#define NV_ROM_DIRECTORY_IDENTIFIER     0x44524652  // "RFRD"
#define NV_ROM_DIRECTORY_VER_OFFSET             4U
#define NV_ROM_DIRECTORY_VER_04             0x0004
#define NV_ROM_DIRECTORY_OPTROM_OFFSET          8U

/* Secure region boundaries for GB20C */
#define SECURE_REGION_32_START	0x80000000ULL
#define SECURE_REGION_32_END	0x8FFFFFFFULL
/**
 * Safe memcpy for potentially unaligned BIOS data to avoid alignment faults
 * This function copies data byte-by-byte to avoid alignment issues that can
 * occur when BIOS data pointers are adjusted and no longer aligned properly.
 */
static void safe_bios_memcpy(void *dest, const void *src, size_t n)
{
	u8 *d = (u8 *)dest;
	const u8 *s = (const u8 *)src;
	size_t i;

	for (i = 0; i < n; i++)
		d[i] = s[i];
}

/**
 * Find the 32 bit signature in the blob of memory.
 * Search in step size of 2 bytes.
 * fsig - byte pattern to search for
 * blob - pointer to start address of memory to search
 * bsz  - how many bytes to search (must be equal or less than size of valid blob[] memory area.)
 * start_offset - offset from start of memory area to search.
 */
static size_t findSig_u32(struct gk20a *g, uint32_t fsig, uint8_t *blob,
			size_t bsz, size_t start_offset)
{
	size_t offset;
	// Validate input parameters
	if (blob == NULL || bsz < 4 || start_offset >= bsz) {
		nvgpu_err(g, "%s: Invalid parameters", __func__);
		return bsz;
	}
	for (offset = start_offset; offset <= (bsz-4); offset += 2) {
		uint32_t d32;

		// Use byte-by-byte copy to avoid any alignment issues
		u8 *src = &blob[offset];
		u8 *dst = (u8 *)&d32;

		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];

		if (d32 == fsig)
			return offset;
	}
	return bsz;
}

/**
 * Find the 16 bit signature in the blob of memory.
 * Search in step size of 2 bytes.
 * fsig - byte pattern to search for
 * blob - pointer to start address of memory to search
 * bsz  - how many bytes to search (must be equal or less than valid blob[] memory area.)
 * start_offset - offset from start of memory area to search.
 */
static size_t findSig_u16(struct gk20a *g, uint16_t fsig, uint8_t *blob,
			size_t bsz, size_t start_offset)
{
	size_t offset;

	// Validate input parameters
	if (blob == NULL || bsz < 2 || start_offset >= bsz) {
		nvgpu_err(g, "%s: Invalid parameters", __func__);
		return bsz;
	}

	for (offset = start_offset; offset <= (bsz-2); offset += 2) {
		uint16_t d16;

		// Use byte-by-byte copy to avoid any alignment issues
		u8 *src = &blob[offset];
		u8 *dst = (u8 *)&d16;

		dst[0] = src[0];
		dst[1] = src[1];

		if (d16 == fsig)
			return offset;
	}
	return bsz;
}

s32 gb20c_bios_init(struct gk20a *g)
{
	size_t loc = 0U;
	size_t mgcLoc = 0;
	u16 romDirVersion = 0;
	u32 biosImageOffset = 0;
	s32 err = 0;
	u8 *original_mapped_base = NULL;
	u8 *rom_ver_src, *rom_ver_dst;
	u8 *bios_offset_src, *bios_offset_dst;
	size_t aligned_size;
	u8 *aligned_bios_data;
	u8 *original_mapped_data;

	if (g == NULL)
		return -EINVAL;

	if (g->bios_is_init)
		return 0;

	if (g->bios == NULL)
		return -EINVAL;

	g->bios->data = nvgpu_gb20c_get_vbios_addr(g);
	original_mapped_base = g->bios->data;

	if (g->bios->data == NULL)
		return -ENOMEM;

	g->bios->size = GB20C_BIOS_SIZE;

	// Find NVFW signature first
	mgcLoc = findSig_u32(g, FFL_FIRMWARE_INSTANCE_MAGIC_DWORD_VALUE,
			g->bios->data, g->bios->size, 0UL);
	if (mgcLoc == g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	// ROM directory's offset is relative to NVFW signature location
	g->bios->data = &g->bios->data[mgcLoc];
	g->bios->size = nvgpu_safe_sub_u64(g->bios->size, mgcLoc);

	// Find ROM directory which has offset pointer to PCI_EXP_ROM_SIG.
	loc = findSig_u32(g, NV_ROM_DIRECTORY_IDENTIFIER, g->bios->data, g->bios->size, 0UL);

	if (loc == g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	if (loc < g->bios->size) {
		u32 rom_dir_sig;

		safe_bios_memcpy(&rom_dir_sig, &g->bios->data[loc], sizeof(u32));

		if (rom_dir_sig != NV_ROM_DIRECTORY_IDENTIFIER) {
			err = -EINVAL;
			goto err_exit;
		}
	}

	// Validate we have enough space to read ROM directory fields
	if (loc + NV_ROM_DIRECTORY_OPTROM_OFFSET + sizeof(u32) > g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	// Use byte-by-byte copy to avoid alignment faults!
	rom_ver_src = &g->bios->data[loc + NV_ROM_DIRECTORY_VER_OFFSET];
	rom_ver_dst = (u8 *)&romDirVersion;

	rom_ver_dst[0] = rom_ver_src[0];
	rom_ver_dst[1] = rom_ver_src[1];

	if (romDirVersion != NV_ROM_DIRECTORY_VER_04) {
		err = -EINVAL;
		goto err_exit;
	}

	// Use byte-by-byte copy to avoid alignment faults
	bios_offset_src = &g->bios->data[loc + NV_ROM_DIRECTORY_OPTROM_OFFSET];
	bios_offset_dst = (u8 *)&biosImageOffset;

	bios_offset_dst[0] = bios_offset_src[0];
	bios_offset_dst[1] = bios_offset_src[1];
	bios_offset_dst[2] = bios_offset_src[2];
	bios_offset_dst[3] = bios_offset_src[3];

	// Validate biosImageOffset before using it
	if (biosImageOffset >= g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	g->bios->data = &g->bios->data[biosImageOffset];
	g->bios->size = nvgpu_safe_sub_u64(g->bios->size, biosImageOffset);

	// Find pci-exp-rom signature
	loc = findSig_u16(g, PCI_EXP_ROM_SIG, g->bios->data, g->bios->size, 0U);
	if (loc == g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	// g->bios-data must point to the pci-exp-rom signature.
	g->bios->data = &g->bios->data[loc];
	g->bios->size = nvgpu_safe_sub_u64(g->bios->size, loc);

	// Verify we can find the VBIOS BIT signatures
	loc = findSig_u16(g, BIT_HEADER_ID, g->bios->data, g->bios->size, 0U);
	if (loc == g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	loc = findSig_u32(g, BIT_HEADER_SIGNATURE, g->bios->data, g->bios->size, loc);
	if (loc == g->bios->size) {
		err = -EINVAL;
		goto err_exit;
	}

	// Allocate aligned memory for BIOS data copy
	aligned_size = (g->bios->size + 7) & ~7; // Round up to 8-byte boundary
	aligned_bios_data = (u8 *)nvgpu_kzalloc(g, aligned_size);

	if (aligned_bios_data == NULL) {
		err = -ENOMEM;
		goto err_exit;
	}

	original_mapped_data = g->bios->data;

	g->bios->data = aligned_bios_data;

	// Copy BIOS data using safe byte-by-byte copy
	safe_bios_memcpy(aligned_bios_data, original_mapped_data, g->bios->size);

	// parse the VBIOS tables into internal structures
	err = nvgpu_bios_parse_rom(g);
	if (err != 0)
		goto err_exit_aligned;

	if (g->bios->verify_version != NULL) {
		if (g->bios->verify_version(g) < 0) {
			err = -EINVAL;
			goto err_exit_aligned;
		}
	}

	g->bios_is_init = true;
	return 0;

err_exit_aligned:
	if (aligned_bios_data != NULL)
		nvgpu_kfree(g, aligned_bios_data);

	g->bios->data = original_mapped_data; // Restore for cleanup

err_exit:
	if (original_mapped_base != NULL) {
		nvgpu_io_unmap(g, (uintptr_t)original_mapped_base, GB20C_BIOS_SIZE);
		g->bios->data = NULL;
		g->bios->size = 0U;
	}
	return err;
}

static s32 gb20c_bios_verify_version(struct gk20a *g)
{
	if (g->bios->vbios_version < NVGPU_GB20C_MIN_VBIOS) {
		nvgpu_err(g, "unsupported VBIOS version: %08x",
			g->bios->vbios_version);
		return -EINVAL;
	}
	nvgpu_pmu_dbg(g, "VBIOS version: %08x",
		g->bios->vbios_version);
	return 0;
}
void nvgpu_gb20c_bios_sw_init(struct gk20a *g,
		struct nvgpu_bios *bios)
{
	if (g == NULL || bios == NULL)
		return;

	g->bios_is_init = false;
	bios->init = gb20c_bios_init;
	bios->verify_version = gb20c_bios_verify_version;
	bios->preos_wait_for_halt = NULL;
	bios->preos_reload_check = NULL;
	bios->preos_bios = NULL;
	bios->devinit_bios = NULL;
	bios->verify_devinit = NULL;
}
