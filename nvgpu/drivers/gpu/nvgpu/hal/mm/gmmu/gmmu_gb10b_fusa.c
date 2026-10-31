// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/gmmu.h>
#include <nvgpu/static_analysis.h>

#include <nvgpu/hw/gb10b/hw_gmmu_gb10b.h>

#include "hal/mm/gmmu/gmmu_gk20a.h"
#include "gmmu_gb10b.h"

/*
 * From page table structure, PDE0 entry uses (VA[28:21]) 8 bits i.e 256 entries
 * Size of page directory 0 is 4KB.
 * So, each entry is 4KB/256 = 16 bytes.
 */
#define GB10B_PDE0_ENTRY_SIZE		16U

/*
 * From page table structure, PDE1 entry uses (VA[37:29]) 9 bits i.e 512 entries
 * Size of page directory 1 is 4KB.
 * So, each entry is 4KB/512 = 8 bytes.
 * Similarly, PDE2 entry uses (VA[46:38]) 9 bits i.e 512 entries
 * Size of page directory 2 is 4KB.
 * So, each entry is 4KB/512 = 8 bytes.
 *
 * Ideally, PDE3 can have more entries. However, PDE3 entries are limited to 8.
 */
#define GB10B_PDE_DEFAULT_ENTRY_SIZE	8U

/*
 * From page table structure,
 * for small page,
 *        PTE uses (VA[20:12]) 9 bits i.e 512 entries with 4KB size
 *        So, each entry is 4KB/512 = 8 bytes
 * for big page,
 *        PTE uses (VA[20:16]) 5 bits i.e 32 entries with 256B size
 *        So, each entry is 256B/32 = 8 bytes
 */
#define GB10B_PTE_ENTRY_SIZE		8U

/*
 * Blackwell follows ver3 Page Table format of 6 levels with 5 page directory levels
 * and a page table
 */
#define GB10B_MAX_PAGE_TABLE_LEVELS	6U

/* Bit position in PTE PCF field for cacheability control*/
#define GB10B_PTE_PCF_VOL_BIT	BIT(0)
/* Bit position in PTE PCF field for privilege control */
#define GB10B_PTE_PCF_PRIV_BIT	BIT(1)
/* Bit position in PTE PCF field for Read Only control */
#define GB10B_PTE_PCF_RO_BIT	BIT(2)
/* Bit position in PTE PCF field for Atomics control */
#define GB10B_PTE_PCF_AD_BIT	BIT(3)
/* Bit position in PTE PCF field for Access counting control */
#define GB10B_PTE_PCF_ACD_BIT	BIT(4)

#define GB10B_PTE_PCF_SPARSE_BIT    BIT(0)

/* Bit position in PDE PCF field for cacheability control */
#define GB10B_PDE_PCF_VOL_BIT	BIT(0)
/* Bit position in PDE PCF field for ATS lookup control */
#define GB10B_PDE_PCF_ATSD_BIT	BIT(1)

static void gb10b_update_gmmu_pde4_locked(struct vm_gk20a *vm,
					  const struct gk20a_mmu_level *l,
					  struct nvgpu_gmmu_pd *pd,
					  u32 pd_idx,
					  u64 virt_addr,
					  u64 phys_addr,
					  struct nvgpu_gmmu_attrs *attrs)
{
	struct gk20a *g = gk20a_from_vm(vm);
	struct nvgpu_gmmu_pd *next_pd = &pd->entries[pd_idx];
	u32 pd_offset = nvgpu_pd_offset_from_index(l, pd_idx);
	u32 pde_v[2] = {0, 0};

	phys_addr >>= gmmu_ver3_pde_address_shift_v();

	pde_v[0] |= nvgpu_aperture_mask(g, next_pd->mem,
					gmmu_ver3_pde_aperture_sys_mem_ncoh_f(),
					gmmu_ver3_pde_aperture_sys_mem_coh_f(),
					gmmu_ver3_pde_aperture_video_memory_f());
	pde_v[0] |= gmmu_ver3_pde_address_f(u64_lo32(phys_addr));
	pde_v[0] |= gmmu_ver3_pde_pcf_f(GB10B_PDE_PCF_VOL_BIT);
	pde_v[1] |= nvgpu_safe_cast_u64_to_u32(phys_addr >> 20);

	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 0U),
		       pde_v[0]);
	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 1U),
		       pde_v[1]);

	pte_dbg(g, attrs,
		"PDE: i=%-4u size=%-2u offs=%-4u pgsz: -- | "
		"GPU %#-12llx  phys %#-12llx "
		"[0x%08x, 0x%08x]",
		pd_idx, l->entry_size, pd_offset,
		virt_addr, phys_addr,
		pde_v[1], pde_v[0]);
}

static void gb10b_update_gmmu_pde0_locked(struct vm_gk20a *vm,
					  const struct gk20a_mmu_level *l,
					  struct nvgpu_gmmu_pd *pd,
					  u32 pd_idx,
					  u64 virt_addr,
					  u64 phys_addr,
					  struct nvgpu_gmmu_attrs *attrs)
{
	struct gk20a *g = gk20a_from_vm(vm);
	struct nvgpu_gmmu_pd *next_pd = &pd->entries[pd_idx];
	u32 small_addr = 0, big_addr_lo = 0, big_addr_hi;
	bool small_valid, big_valid;
	u32 pd_offset = nvgpu_pd_offset_from_index(l, pd_idx);
	u32 pde_v[4] = {0, 0, 0, 0};
	u64 tmp_addr;

	small_valid = attrs->pgsz == GMMU_PAGE_SIZE_SMALL;
	big_valid   = attrs->pgsz == GMMU_PAGE_SIZE_BIG;

	if (small_valid) {
		tmp_addr = phys_addr >> gmmu_ver3_dual_pde_address_shift_v();
		nvgpu_assert(u64_hi32(tmp_addr) == 0U);
		small_addr = (u32)tmp_addr;

		pde_v[2] |=
			gmmu_ver3_dual_pde_address_small_f(small_addr);
		pde_v[2] |= nvgpu_aperture_mask(g, next_pd->mem,
			gmmu_ver3_dual_pde_aperture_small_sys_mem_ncoh_f(),
			gmmu_ver3_dual_pde_aperture_small_sys_mem_coh_f(),
			gmmu_ver3_dual_pde_aperture_small_video_memory_f());
		pde_v[2] |= gmmu_ver3_pde_pcf_f(GB10B_PDE_PCF_VOL_BIT);
		pde_v[3] |= small_addr >> 20;
	}

	if (big_valid) {
		tmp_addr = phys_addr >> gmmu_ver3_dual_pde_address_big_shift_v();
		big_addr_lo = (u32)tmp_addr;
		big_addr_hi = u64_hi32(tmp_addr);

		pde_v[0] |= gmmu_ver3_dual_pde_address_big_f(big_addr_lo);
		pde_v[0] |= nvgpu_aperture_mask(g, next_pd->mem,
			gmmu_ver3_dual_pde_aperture_big_sys_mem_ncoh_f(),
			gmmu_ver3_dual_pde_aperture_big_sys_mem_coh_f(),
			gmmu_ver3_dual_pde_aperture_big_video_memory_f());
		pde_v[0] |= gmmu_ver3_pde_pcf_f(GB10B_PDE_PCF_VOL_BIT);
		pde_v[1] |= (big_addr_lo >> 24) | (big_addr_hi << 8);
	}

	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 0U),
		       pde_v[0]);
	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 1U),
		       pde_v[1]);
	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 2U),
		       pde_v[2]);
	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 3U),
		       pde_v[3]);

	pte_dbg(g, attrs,
		"PDE: i=%-4u size=%-2u offs=%-4u pgsz: %c%c | "
		"GPU %#-12llx  phys %#-12llx "
		"[0x%08x, 0x%08x, 0x%08x, 0x%08x]",
		pd_idx, l->entry_size, pd_offset,
		small_valid ? 'S' : '-',
		big_valid ?   'B' : '-',
		virt_addr, phys_addr,
		pde_v[3], pde_v[2], pde_v[1], pde_v[0]);
}

static u32 pte_pcf_encode(struct nvgpu_gmmu_attrs *attrs)
{
	u32 encode = 0;

	if (attrs->priv) {
		encode |= GB10B_PTE_PCF_PRIV_BIT;
	}

	if (attrs->rw_flag == gk20a_mem_flag_read_only) {
		encode |= GB10B_PTE_PCF_RO_BIT;
	}

	if (!attrs->cacheable) {
		encode |= GB10B_PTE_PCF_VOL_BIT;
	}

	return encode;
}

static void gb10b_update_pte_sparse(u32 *pte_w)
{
	pte_w[0] = gmmu_ver3_pte_pcf_f(GB10B_PTE_PCF_SPARSE_BIT);
	pte_w[1] = 0;
}

static void gb10b_update_pte(struct vm_gk20a *vm,
			     u32 *pte_w,
			     u64 phys_addr,
			     struct nvgpu_gmmu_attrs *attrs)
{
	struct gk20a *g = gk20a_from_vm(vm);
	u32 pte_valid = attrs->valid ?
		gmmu_ver3_pte_valid_true_f() :
		gmmu_ver3_pte_valid_false_f();
	u64 phys_shifted = phys_addr >> gmmu_ver3_pte_address_shift_v();
	u32 pte_addr = nvgpu_aperture_is_sysmem(attrs->aperture) ?
		gmmu_ver3_pte_address_sys_f(u64_lo32(phys_shifted)) :
		gmmu_ver3_pte_address_vid_f(u64_lo32(phys_shifted));
	u32 pte_tgt = nvgpu_gmmu_aperture_mask(g,
					attrs->aperture,
					gmmu_ver3_pte_aperture_sys_mem_ncoh_f(),
					gmmu_ver3_pte_aperture_sys_mem_coh_f(),
					gmmu_ver3_pte_aperture_video_memory_f());
	u64 tmp_addr;

	pte_w[0] = pte_valid | pte_addr | pte_tgt | gmmu_ver3_pte_kind_f(attrs->kind_v);
	pte_w[0] |= gmmu_ver3_pte_pcf_f(pte_pcf_encode(attrs));

	tmp_addr = phys_addr >> (20U + gmmu_ver3_pte_address_shift_v());
	nvgpu_assert(u64_hi32(tmp_addr) == 0U);
	pte_w[1] = (u32)tmp_addr;
}

static void gb10b_update_gmmu_pte_locked(struct vm_gk20a *vm,
					 const struct gk20a_mmu_level *l,
					 struct nvgpu_gmmu_pd *pd,
					 u32 pd_idx,
					 u64 virt_addr,
					 u64 phys_addr,
					 struct nvgpu_gmmu_attrs *attrs)
{
	struct gk20a *g = vm->mm->g;
	u32 page_size  = vm->gmmu_page_sizes[attrs->pgsz];
	u32 pd_offset = nvgpu_pd_offset_from_index(l, pd_idx);
	u32 pte_w[2] = {0, 0};

	if (phys_addr) {
		gb10b_update_pte(vm, pte_w, phys_addr, attrs);
	} else {
		if (attrs->sparse) {
			gb10b_update_pte_sparse(pte_w);
		}
	}

	nvgpu_pte_dbg_print(g, attrs, vm->name, pd_idx, l->entry_size,
			    virt_addr, phys_addr, page_size, pte_w);

	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 0U),
		       pte_w[0]);
	nvgpu_pd_write(g, pd, (size_t)nvgpu_safe_add_u32(pd_offset, 1U),
		       pte_w[1]);
}

/*
 * Calculate the pgsz of the pde level
 * Blackwell implements a 6 level page table structure with only the last
 * level having a different number of entries depending on whether it holds
 * big pages or small pages.
 */
static u32 gb10b_get_pde0_pgsz(struct gk20a *g, const struct gk20a_mmu_level *l,
			       struct nvgpu_gmmu_pd *pd, u32 pd_idx)
{
	u32 pde_base = pd->mem_offs / (u32)sizeof(u32);
	u32 pde_offset = nvgpu_safe_add_u32(pde_base,
					nvgpu_pd_offset_from_index(l, pd_idx));
	u32 pde_v[GB10B_PDE0_ENTRY_SIZE >> 2];
	u32 idx;
	u32 pgsz = GMMU_NR_PAGE_SIZES;

	if (!pd->mem) {
		return pgsz;
	}

	for (idx = 0; idx < (GB10B_PDE0_ENTRY_SIZE >> 2); idx++) {
		pde_v[idx] =
			nvgpu_mem_rd32(g, pd->mem, (u64)pde_offset + (u64)idx);
	}

	/*
	 * When a PDE aperture field is not zero, then the corresponding page
	 * table is valid.
	 */
	if ((pde_v[2] &
	     (gmmu_ver3_dual_pde_aperture_small_sys_mem_ncoh_f() |
	      gmmu_ver3_dual_pde_aperture_small_sys_mem_coh_f() |
	      gmmu_ver3_dual_pde_aperture_small_video_memory_f())) != 0U) {
		pgsz = GMMU_PAGE_SIZE_SMALL;
	}

	if ((pde_v[0] &
		(gmmu_ver3_dual_pde_aperture_big_sys_mem_ncoh_f() |
		 gmmu_ver3_dual_pde_aperture_big_sys_mem_coh_f() |
		 gmmu_ver3_dual_pde_aperture_big_video_memory_f())) != 0U) {
		/*
		 * If small is set that means that somehow MM allowed
		 * both small and big to be set, the PDE is not valid
		 * and may be corrupted
		 */
		if (pgsz == GMMU_PAGE_SIZE_SMALL) {
			nvgpu_err(g, "both small and big apertures enabled");
			return GMMU_NR_PAGE_SIZES;
		}
		pgsz = GMMU_PAGE_SIZE_BIG;
	}

	return pgsz;
}

static const struct gk20a_mmu_level gb10b_mm_levels[] = {
	{.hi_bit = {56, 56},
	 .lo_bit = {56, 56},
	 .update_entry = gb10b_update_gmmu_pde4_locked,
	 .entry_size = GB10B_PDE_DEFAULT_ENTRY_SIZE,
	 .get_pgsz = gk20a_get_pde_pgsz},
	{.hi_bit = {55, 55},
	 .lo_bit = {47, 47},
	 .update_entry = gb10b_update_gmmu_pde4_locked,
	 .entry_size = GB10B_PDE_DEFAULT_ENTRY_SIZE,
	 .get_pgsz = gk20a_get_pde_pgsz},
	{.hi_bit = {46, 46},
	 .lo_bit = {38, 38},
	 .update_entry = gb10b_update_gmmu_pde4_locked,
	 .entry_size = GB10B_PDE_DEFAULT_ENTRY_SIZE,
	 .get_pgsz = gk20a_get_pde_pgsz},
	{.hi_bit = {37, 37},
	 .lo_bit = {29, 29},
	 .update_entry = gb10b_update_gmmu_pde4_locked,
	 .entry_size = GB10B_PDE_DEFAULT_ENTRY_SIZE,
	 .get_pgsz = gk20a_get_pde_pgsz},
	{.hi_bit = {28, 28},
	 .lo_bit = {21, 21},
	 .update_entry = gb10b_update_gmmu_pde0_locked,
	 .update_pde_entry_as_pte = gb10b_update_gmmu_pte_locked,
	 .entry_size = GB10B_PDE0_ENTRY_SIZE,
	 .get_pgsz = gb10b_get_pde0_pgsz},
	{.hi_bit = {20, 20},
	 .lo_bit = {12, 16},
	 .update_entry = gb10b_update_gmmu_pte_locked,
	 .entry_size = GB10B_PTE_ENTRY_SIZE,
	 .get_pgsz = gk20a_get_pte_pgsz},
	{.update_entry = NULL}
};

const struct gk20a_mmu_level *gb10b_mm_get_mmu_levels(struct gk20a *g,
						      u64 big_page_size)
{
	(void)g;
	(void)big_page_size;
	return gb10b_mm_levels;
}

u32 gb10b_get_max_page_table_levels(struct gk20a *g)
{
	(void)g;
	return GB10B_MAX_PAGE_TABLE_LEVELS;
}

u32 gb10b_mm_default_huge_page_size(void)
{
	return nvgpu_safe_cast_u64_to_u32(SZ_2M);
}
