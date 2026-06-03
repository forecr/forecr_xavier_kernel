// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/kmem.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvgpu_sgt.h>
#include <nvgpu/dma.h>
#include <nvgpu/vidmem.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/pramin.h>
#include <nvgpu/string.h>

/*
 * Make sure to use the right coherency aperture if you use this function! This
 * will not add any checks. If you want to simply use the default coherency then
 * use nvgpu_aperture_mask().
 */
u32 nvgpu_aperture_mask_raw(struct gk20a *g, enum nvgpu_aperture aperture,
			    u32 sysmem_mask, u32 sysmem_coh_mask,
			    u32 vidmem_mask)
{
	u32 ret_mask = 0;

	switch (aperture) {
	case APERTURE_SYSMEM_COH:
		ret_mask = sysmem_coh_mask;
		break;
	case APERTURE_SYSMEM:
		ret_mask = sysmem_mask;
		break;
	case APERTURE_VIDMEM:
		ret_mask = vidmem_mask;
		break;
	default:
		nvgpu_do_assert_print(g, "Bad aperture");
		ret_mask = 0;
		break;
	}

	/*
	 * Some iGPUs treat sysmem (i.e SoC DRAM) as vidmem. In these cases the
	 * "sysmem" aperture should really be translated to VIDMEM.
	 */
	if (!nvgpu_is_enabled(g, NVGPU_MM_HONORS_APERTURE) && ret_mask != 0U) {
		ret_mask = vidmem_mask;
	}

	return ret_mask;
}

u32 nvgpu_aperture_mask(struct gk20a *g, struct nvgpu_mem *mem,
			u32 sysmem_mask, u32 sysmem_coh_mask, u32 vidmem_mask)
{
	enum nvgpu_aperture ap = mem->aperture;

	return nvgpu_aperture_mask_raw(g, ap,
				       sysmem_mask,
				       sysmem_coh_mask,
				       vidmem_mask);
}

bool nvgpu_aperture_is_sysmem(enum nvgpu_aperture ap)
{
	return (ap == APERTURE_SYSMEM_COH) || (ap == APERTURE_SYSMEM);
}

bool nvgpu_mem_is_sysmem(struct nvgpu_mem *mem)
{
	return nvgpu_aperture_is_sysmem(mem->aperture);
}

u64 nvgpu_mem_iommu_translate(struct gk20a *g, u64 phys)
{
	/* ensure it is not vidmem allocation */
#ifdef CONFIG_NVGPU_DGPU
	WARN_ON(nvgpu_addr_is_vidmem_page_alloc(phys));
#endif

	if (nvgpu_iommuable(g) && (g->ops.mm.gmmu.get_iommu_bit != NULL)) {
		return phys | (1ULL << g->ops.mm.gmmu.get_iommu_bit(g));
	}

	return phys;
}

u32 nvgpu_mem_rd32(struct gk20a *g, struct nvgpu_mem *mem, u64 w)
{
	u32 data = 0;

	if (nvgpu_aperture_is_sysmem(mem->aperture)) {
		u32 *ptr = mem->cpu_va;

		WARN_ON(ptr == NULL);
		data = ptr[w];
	}
#ifdef CONFIG_NVGPU_DGPU
	else if (mem->aperture == APERTURE_VIDMEM) {
		nvgpu_pramin_rd_n(g, mem, w * (u64)sizeof(u32),
				(u64)sizeof(u32), &data);
	}
#endif
	else {
		nvgpu_do_assert_print(g, "Accessing unallocated nvgpu_mem");
	}

	return data;
}

u64 nvgpu_mem_rd32_pair(struct gk20a *g, struct nvgpu_mem *mem, u32 lo, u32 hi)
{
	u64 lo_data = U64(nvgpu_mem_rd32(g, mem, lo));
	u64 hi_data = U64(nvgpu_mem_rd32(g, mem, hi));

	return lo_data | (hi_data << 32ULL);
}

u32 nvgpu_mem_rd(struct gk20a *g, struct nvgpu_mem *mem, u64 offset)
{
	WARN_ON((offset & 3ULL) != 0ULL);
	return nvgpu_mem_rd32(g, mem, offset / (u64)sizeof(u32));
}

void nvgpu_mem_rd_n(struct gk20a *g, struct nvgpu_mem *mem,
		u64 offset, void *dest, u64 size)
{
	WARN_ON((offset & 3ULL) != 0ULL);
	WARN_ON((size & 3ULL) != 0ULL);

	if (nvgpu_aperture_is_sysmem(mem->aperture)) {
		u8 *src = (u8 *)mem->cpu_va + offset;

		WARN_ON(mem->cpu_va == NULL);
		nvgpu_memcpy((u8 *)dest, src, size);
	}
#ifdef CONFIG_NVGPU_DGPU
	else if (mem->aperture == APERTURE_VIDMEM) {
		nvgpu_pramin_rd_n(g, mem, offset, size, dest);
	}
#endif
	else {
		nvgpu_do_assert_print(g, "Accessing unallocated nvgpu_mem");
	}
}

void nvgpu_mem_wr32(struct gk20a *g, struct nvgpu_mem *mem, u64 w, u32 data)
{
	if (nvgpu_aperture_is_sysmem(mem->aperture)) {
		u32 *ptr = mem->cpu_va;

		WARN_ON(ptr == NULL);
		ptr[w] = data;
	}
#ifdef CONFIG_NVGPU_DGPU
	else if (mem->aperture == APERTURE_VIDMEM) {
		nvgpu_pramin_wr_n(g, mem, w * (u64)sizeof(u32),
				  (u64)sizeof(u32), &data);

		if (!mem->skip_wmb) {
			nvgpu_wmb();
		}
	}
#endif
	else {
		nvgpu_do_assert_print(g, "Accessing unallocated nvgpu_mem");
	}
}

void nvgpu_mem_wr(struct gk20a *g, struct nvgpu_mem *mem, u64 offset, u32 data)
{
	WARN_ON((offset & 3ULL) != 0ULL);
	nvgpu_mem_wr32(g, mem, offset / (u64)sizeof(u32), data);
}

void nvgpu_mem_wr_n(struct gk20a *g, struct nvgpu_mem *mem, u64 offset,
		void *src, u64 size)
{
	WARN_ON((offset & 3ULL) != 0ULL);
	WARN_ON((size & 3ULL) != 0ULL);

	if (nvgpu_aperture_is_sysmem(mem->aperture)) {
		u8 *dest = (u8 *)mem->cpu_va + offset;

		WARN_ON(mem->cpu_va == NULL);
		nvgpu_memcpy(dest, (u8 *)src, size);
	}
#ifdef CONFIG_NVGPU_DGPU
	else if (mem->aperture == APERTURE_VIDMEM) {
		nvgpu_pramin_wr_n(g, mem, offset, size, src);
		if (!mem->skip_wmb) {
			nvgpu_wmb();
		}
	}
#endif
	else {
		nvgpu_do_assert_print(g, "Accessing unallocated nvgpu_mem");
	}
}

void nvgpu_memset(struct gk20a *g, struct nvgpu_mem *mem, u64 offset,
		u32 c, u64 size)
{
	WARN_ON((offset & 3ULL) != 0ULL);
	WARN_ON((size & 3ULL) != 0ULL);
	WARN_ON((c & ~0xffU) != 0U);

	c &= 0xffU;

	if (nvgpu_aperture_is_sysmem(mem->aperture)) {
		u8 *dest = (u8 *)mem->cpu_va + offset;

		WARN_ON(mem->cpu_va == NULL);
		(void) memset(dest, (int)c, size);
	}
#ifdef CONFIG_NVGPU_DGPU
	else if (mem->aperture == APERTURE_VIDMEM) {
		u32 repeat_value = c | (c << 8) | (c << 16) | (c << 24);

		nvgpu_pramin_memset(g, mem, offset, size, repeat_value);
		if (!mem->skip_wmb) {
			nvgpu_wmb();
		}
	}
#endif
	else {
		nvgpu_do_assert_print(g, "Accessing unallocated nvgpu_mem");
	}
}

/**
 * @brief Retrieve the next scatter gather list entry.
 *
 * This function returns the next entry in a scatter gather list by accessing
 * the 'next' pointer of the current scatter gather list entry.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input void pointer to a pointer of type 'struct nvgpu_mem_sgl'.
 * -# Access the 'next' field of the casted scatter gather list entry.
 * -# Return the 'next' field as a void pointer.
 *
 * @param [in] sgl  Pointer to the current scatter gather list entry.
 *                  Range: Valid pointer to 'struct nvgpu_mem_sgl'.
 *
 * @return Pointer to the next scatter gather list entry.
 *         Range: NULL if there is no next entry, otherwise a valid pointer to
 *         'struct nvgpu_mem_sgl'.
 */
static void *nvgpu_mem_phys_sgl_next(void *sgl)
{
	struct nvgpu_mem_sgl *sgl_impl = (struct nvgpu_mem_sgl *)sgl;

	return (void *)(void *)sgl_impl->next;
}

/**
 * @brief Returns the physical address of the scatter-gather list entry.
 *
 * This function retrieves the physical address stored within a scatter-gather
 * list entry. The scatter-gather list entry is expected to be of type
 * `struct nvgpu_mem_sgl`, which contains the physical address of the memory
 * chunk it represents.
 *
 * Provided for compatibility - the DMA address is the same as the phys address
 * for these nvgpu_mem's.
 *
 * The steps performed by the function are as follows:
 * -# Return the physical address field from the scatter-gather list entry.
 *
 * @param [in] sgl  Pointer to the scatter-gather list entry. The pointer is
 *                  expected to be a valid non-NULL pointer to a
 *                  `struct nvgpu_mem_sgl` object.
 *
 * @return The physical address of the scatter-gather list entry.
 */
static u64 nvgpu_mem_phys_sgl_dma(void *sgl)
{
	struct nvgpu_mem_sgl *sgl_impl = (struct nvgpu_mem_sgl *)sgl;

	return sgl_impl->phys;
}

/**
 * @brief Retrieve the physical address from a scatter-gather list entry.
 *
 * This function extracts the physical address field from a given scatter-gather
 * list entry which is represented by a struct nvgpu_mem_sgl. The function
 * assumes that the scatter-gather list entry is of type struct nvgpu_mem_sgl
 * and directly accesses the 'phys' member to obtain the physical address.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input void pointer to a struct nvgpu_mem_sgl pointer.
 * -# Return the 'phys' member of the struct nvgpu_mem_sgl, which contains the
 *    physical address.
 *
 * @param [in]  g    The GPU driver struct (unused in this function).
 * @param [in]  sgl  Pointer to the scatter-gather list entry.
 *                   Range: Valid pointer to a struct nvgpu_mem_sgl.
 *
 * @return The physical address contained in the scatter-gather list entry.
 */
static u64 nvgpu_mem_phys_sgl_phys(struct gk20a *g, void *sgl)
{
	struct nvgpu_mem_sgl *sgl_impl = (struct nvgpu_mem_sgl *)sgl;

	(void)g;
	return sgl_impl->phys;
}

/**
 * @brief Converts an intermediate physical address (IPA) to a physical address (PA).
 *
 * This function is a stub that simply returns the provided IPA as the PA. It does not
 * perform any actual conversion or lookup. The parameters related to the GPU state and
 * scatter-gather list (SGL) are unused in this implementation.
 *
 * The steps performed by the function are as follows:
 * -# The function accepts the GPU state, SGL, IPA, and a pointer to store the length
 *    of the physical memory, but these parameters are not used.
 * -# The function directly returns the provided IPA as the PA.
 *
 * @param [in]  g         Pointer to the GPU state structure.
 * @param [in]  sgl       Pointer to the scatter-gather list.
 * @param [in]  ipa       Intermediate physical address to be converted.
 * @param [out] pa_len    Pointer to store the length of the physical memory.
 *
 * @return The physical address which is the same as the input intermediate physical address.
 */
static u64 nvgpu_mem_phys_sgl_ipa_to_pa(struct gk20a *g,
		void *sgl, u64 ipa, u64 *pa_len)
{
	(void)g;
	(void)sgl;
	(void)pa_len;
	return ipa;
}

/**
 * @brief Retrieves the length of a scatter-gather list entry.
 *
 * This function returns the length of the memory region described by a single
 * scatter-gather list entry. The length represents the size of the memory
 * region in bytes.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input void pointer to a struct nvgpu_mem_sgl pointer.
 * -# Return the length field of the struct nvgpu_mem_sgl.
 *
 * @param [in] sgl  Pointer to the scatter-gather list entry.
 *
 * @return Length of the memory region described by the scatter-gather list entry.
 */
static u64 nvgpu_mem_phys_sgl_length(void *sgl)
{
	struct nvgpu_mem_sgl *sgl_impl = (struct nvgpu_mem_sgl *)sgl;

	return sgl_impl->length;
}

/**
 * @brief Returns the physical address associated with a scatter gather list entry.
 *
 * This function retrieves the GPU accessible address from the provided scatter gather
 * list entry structure.
 *
 * The steps performed by the function are as follows:
 * -# Cast the input scatter gather list pointer to a struct nvgpu_mem_sgl pointer.
 * -# Return the physical address field of the struct nvgpu_mem_sgl.
 *
 * @param [in]  g       The GPU driver struct (unused in this function).
 * @param [in]  sgl     Pointer to the scatter gather list entry.
 * @param [in]  attrs   Attributes for the GMMU (unused in this function).
 *
 * @return The GPU accessible address associated with the scatter gather list entry.
 */
static u64 nvgpu_mem_phys_sgl_gpu_addr(struct gk20a *g, void *sgl,
					 struct nvgpu_gmmu_attrs *attrs)
{
	struct nvgpu_mem_sgl *sgl_impl = (struct nvgpu_mem_sgl *)sgl;

	(void)g;
	(void)attrs;
	return sgl_impl->phys;
}

/**
 * @brief Frees the scatter gather table associated with nvgpu_mem.
 *
 * This function is a no-operation function. It is intended to be used in
 * contexts where the scatter gather table does not need to be explicitly freed
 * because its memory management is handled elsewhere, specifically by the
 * freeing of the nvgpu_mem structure itself.
 *
 * The steps performed by the function are as follows:
 * -# The function accepts two parameters but does not perform any operations
 *    on them.
 * -# The function serves as a placeholder to match the signature expected by
 *    other parts of the system that may require a function to free scatter
 *    gather tables.
 *
 * @param [in] g         The GPU driver structure.
 * @param [in] sgt       The scatter gather table to be freed.
 *
 * There are no return values for this function as it is a void function.
 */
static void nvgpu_mem_phys_sgt_free(struct gk20a *g, struct nvgpu_sgt *sgt)
{
	(void)g;
	(void)sgt;
	/*
	 * No-op here. The free is handled by freeing the nvgpu_mem itself.
	 */
}

static const struct nvgpu_sgt_ops nvgpu_mem_phys_ops = {
	.sgl_next      = nvgpu_mem_phys_sgl_next,
	.sgl_dma       = nvgpu_mem_phys_sgl_dma,
	.sgl_phys      = nvgpu_mem_phys_sgl_phys,
	.sgl_ipa       = nvgpu_mem_phys_sgl_phys,
	.sgl_ipa_to_pa = nvgpu_mem_phys_sgl_ipa_to_pa,
	.sgl_length    = nvgpu_mem_phys_sgl_length,
	.sgl_gpu_addr  = nvgpu_mem_phys_sgl_gpu_addr,
	.sgt_free      = nvgpu_mem_phys_sgt_free,

	/*
	 * The physical nvgpu_mems are never IOMMU'able by definition.
	 */
	.sgt_iommuable = NULL
};

int nvgpu_mem_create_from_phys(struct gk20a *g, struct nvgpu_mem *dest,
			       u64 src_phys, u64 nr_pages)
{
	int ret = 0;
	struct nvgpu_sgt *sgt;
	struct nvgpu_mem_sgl *sgl;

	/*
	 * Do the two operations that can fail before touching *dest.
	 */
	sgt = nvgpu_kzalloc(g, sizeof(*sgt));
	sgl = nvgpu_kzalloc(g, sizeof(*sgl));
	if ((sgt == NULL) || (sgl == NULL)) {
		nvgpu_kfree(g, sgt);
		nvgpu_kfree(g, sgl);
		return -ENOMEM;
	}

	(void) memset(dest, 0, sizeof(*dest));

	dest->aperture     = APERTURE_SYSMEM;
	dest->size         = nvgpu_safe_mult_u64(nr_pages,
			(u64)NVGPU_CPU_PAGE_SIZE);
	dest->aligned_size = dest->size;
	dest->mem_flags    = NVGPU_MEM_FLAG_NO_DMA;
	dest->phys_sgt     = sgt;

	sgl->next   = NULL;
	sgl->phys   = src_phys;
	sgl->length = dest->size;
	sgt->sgl    = (void *)sgl;
	sgt->ops    = &nvgpu_mem_phys_ops;

	return ret;
}

u64 nvgpu_mem_phys_get_addr(struct gk20a *g, struct nvgpu_mem *mem)
{
	struct nvgpu_mem_sgl *sgl_impl;

	(void)g;
	sgl_impl = (struct nvgpu_mem_sgl *)(mem->phys_sgt->sgl);
	return sgl_impl->phys;
}
