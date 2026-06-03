/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SGT_H
#define NVGPU_SGT_H

/**
 * @file
 *
 * Abstract interface for interacting with the scatter gather list entries.
 */

#include <nvgpu/types.h>

struct gk20a;
struct nvgpu_mem;
struct nvgpu_gmmu_attrs;

struct nvgpu_sgt;

/**
 * This structure holds the necessary operations required for
 * interacting with the underlying scatter gather list entries.
 */
struct nvgpu_sgt_ops {
	/**
	 * Used to get the next scatter gather list entry in the
	 * scatter gather list entries.
	 */
	void *(*sgl_next)(void *sgl);
	/**
	 * Used to get the physical address associated with the
	 * scatter gather list entry.
	 */
	u64   (*sgl_phys)(struct gk20a *g, void *sgl);
	/**
	 * Used to get the intermediate physical address associated with the
	 * scatter gather list entry.
	 */
	u64   (*sgl_ipa)(struct gk20a *g, void *sgl);
	/**
	 * Used to get the physical address from the intermediate
	 * physical address.
	 */
	u64   (*sgl_ipa_to_pa)(struct gk20a *g, void *sgl,
				u64 ipa, u64 *pa_len);
	/**
	 * Used to get the iommuable virtual address associated with the
	 * scatter gather list entry.
	 */
	u64   (*sgl_dma)(void *sgl);
	/**
	 * Used to get the length associated with the scatter gather list
	 * entry.
	 */
	u64   (*sgl_length)(void *sgl);
	/**
	 * Used to get the gpu understandable physical address from the
	 * soc physical address.
	 */
	u64   (*sgl_gpu_addr)(struct gk20a *g, void *sgl,
			      struct nvgpu_gmmu_attrs *attrs);
	/**
	 * Used to get the iommu on/off status.
	 * If left NULL then iommuable is assumed to be false.
	 */
	bool  (*sgt_iommuable)(struct gk20a *g, struct nvgpu_sgt *sgt);
	/**
	 * Function ptr to free the entire scatter gather table.
	 * Note: this operates on the whole scatter gather table not a
	 * specific scatter gather list entry.
	 */
	void  (*sgt_free)(struct gk20a *g, struct nvgpu_sgt *sgt);
};

/**
 * Scatter gather table: this is a list of scatter list entries and the ops for
 * interacting with those entries.
 */
struct nvgpu_sgt {
	/**
	 * Ops for interacting with the underlying scatter gather list entries.
	 */
	const struct nvgpu_sgt_ops *ops;

	/**
	 * The first node in the scatter gather list.
	 */
	void *sgl;
};

/**
 * This struct holds the necessary information for describing a struct
 * nvgpu_mem's scatter gather list.
 *
 * This is one underlying implementation for nvgpu_sgl. Not all nvgpu_sgt's use
 * this particular implementation. Nor is a given OS required to use this at
 * all.
 */
struct nvgpu_mem_sgl {
	/**
	 * Pointer to a next scatter gather list entry in this scatter
	 * gather list entries. Internally this is implemented as a singly
	 * linked list.
	 */
	struct nvgpu_mem_sgl	*next;
	/**
	 * Physical address associated with this scatter gather list entry.
	 */
	u64			 phys;
	/**
	 * Iommuable virtual address associated with this scatter gather
	 * list entry.
	 */
	u64			 dma;
	/**
	 * Length associated with this scatter gather list entry.
	 */
	u64			 length;
};

/**
 * Iterate over the SGL entries in an SGT.
 */
#define nvgpu_sgt_for_each_sgl(sgl, sgt)		\
	for ((sgl) = (sgt)->sgl;			\
	     (sgl) != NULL;				\
	     (sgl) = nvgpu_sgt_get_next(sgt, sgl))

/**
 * @brief Create a scatter-gather table (SGT) from a memory descriptor.
 *
 * This function is responsible for creating a scatter-gather table (SGT) from
 * a given memory descriptor. The SGT is used to describe the physical memory
 * layout of the buffer represented by the memory descriptor. If the memory
 * descriptor has the #NVGPU_MEM_FLAG_NO_DMA flag set, indicating that it does
 * not use DMA API for memory allocation, the function returns the pre-existing
 * physical SGT associated with the memory descriptor. Otherwise, it delegates
 * the creation of the SGT to the OS-specific implementation.
 *
 * This list lets the calling code iterate across each chunk of a DMA allocation for when
 * that DMA allocation is not completely contiguous. Since a DMA allocation may well be
 * discontiguous nvgpu requires that a table describe the chunks of memory that make up
 * the DMA allocation.
 *
 * The steps performed by the function are as follows:
 * -# Check if the memory descriptor has the #NVGPU_MEM_FLAG_NO_DMA flag set.
 * -# If the flag is set, return the pre-existing physical SGT associated with
 *    the memory descriptor.
 * -# If the flag is not set, call the OS-specific function to create an SGT
 *    from the memory descriptor.
 *
 * @param [in] g          The GPU driver structure.
 * @param [in] mem        The memory descriptor from which to create the SGT.
 *
 * @return A pointer to the created SGT, or the pre-existing physical SGT if
 *         #NVGPU_MEM_FLAG_NO_DMA flag is set.
 */
struct nvgpu_sgt *nvgpu_sgt_create_from_mem(struct gk20a *g,
					    struct nvgpu_mem *mem);

/**
 * @brief Retrieve the next scatter gather list entry.
 *
 * This function returns the next entry in the scatter gather list by invoking
 * the sgl_next() operation from the scatter gather table's operation structure.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the next scatter gather list entry by calling the sgl_next()
 *    function pointer from the operations structure within the scatter gather
 *    table object.
 *
 * @param [in]  sgt  Pointer to the scatter gather table object.
 *                   Range: Valid pointer to a struct nvgpu_sgt.
 * @param [in]  sgl  Pointer to the current scatter gather list entry.
 *                   Range: Valid pointer to a scatter gather list entry or NULL.
 *
 * @return Pointer to the next scatter gather list entry.
 *         Range: Valid pointer to a scatter gather list entry or NULL if there
 *         is no next entry.
 */
void *nvgpu_sgt_get_next(struct nvgpu_sgt *sgt, void *sgl);

/**
 * @brief Retrieves the intermediate physical address (IPA) associated with a
 *        scatter gather list (SGL) entry.
 *
 * This function is a wrapper that calls the underlying implementation specific to the
 * scatter gather table (SGT) to retrieve the intermediate physical address for a given
 * scatter gather list entry. The intermediate physical address is a hardware-specific
 * address that may require further translation to a physical address depending on the
 * system's memory management configuration.
 *
 * The steps performed by the function are as follows:
 * -# Call the sgl_ipa() function from the sgt->ops structure, passing the GPU structure
 *    and the scatter gather list entry as arguments.
 *    -- sgl_ipa() will check whether given list is NULL or not. If it is
 *       not null it will return the field phys associated with the scatter
 *       gather list.
 *
 * @param [in] g      Pointer to the GPU structure.
 * @param [in] sgt    Pointer to the scatter gather table object.
 * @param [in] sgl    Pointer to the scatter gather list entry.
 *
 * @return The intermediate physical address associated with the scatter gather list entry.
 */
u64 nvgpu_sgt_get_ipa(struct gk20a *g, struct nvgpu_sgt *sgt, void *sgl);

/**
 * @brief Converts an intermediate physical address (IPA) to a physical address (PA).
 *
 * This function delegates the conversion of an IPA to a PA to the specific
 * scatter-gather table (SGT) operation provided by the GPU architecture. It
 * is a wrapper that calls the architecture-specific implementation to perform
 * the actual conversion.
 *
 * The steps performed by the function are as follows:
 * -# Call the architecture-specific 'sgl_ipa_to_pa()' function through the
 *    function pointer in the SGT operations structure, passing the GPU
 *    structure, the scatter-gather list entry, the IPA, and the pointer to
 *    store the length of the physical memory chunk.
 *    - Query the hypervisor to convert given ipa to pa by calling the
 *      function associated with module descriptor.
 *
 * @param [in]  g         Pointer to the GPU structure.
 * @param [in]  sgt       Pointer to the scatter-gather table structure.
 * @param [in]  sgl       Pointer to the scatter-gather list entry.
 * @param [in]  ipa       Intermediate physical address to be converted.
 * @param [out] pa_len    Pointer to store the length of the physical memory chunk.
 *
 * @return The physical address corresponding to the given intermediate physical address.
 */
u64 nvgpu_sgt_ipa_to_pa(struct gk20a *g, struct nvgpu_sgt *sgt,
				void *sgl, u64 ipa, u64 *pa_len);

/**
 * @brief Retrieves the physical address of a scatter-gather list entry.
 *
 * This function obtains the physical address associated with a scatter-gather
 * list entry by invoking the corresponding operation defined in the scatter-
 * gather table's operations structure.
 *
 * The steps performed by the function are as follows:
 * -# Call the sgl_phys() function pointer from the scatter-gather table's
 *    operations structure, passing the GPU and scatter-gather list entry as
 *    arguments.
 *    -- nvgpu_sgl_phys() will get the module descriptor from the given struct
 *       GPU. Query the hypervisor to convert given ipa to pa by calling the
 *       function associated with module descriptor.
 *
 * @param [in] g    Pointer to the GPU structure.
 * @param [in] sgt  Pointer to the scatter-gather table object.
 * @param [in] sgl  Pointer to the scatter-gather list entry.
 *
 * @return Physical address associated with the scatter-gather list entry.
 */
u64 nvgpu_sgt_get_phys(struct gk20a *g, struct nvgpu_sgt *sgt, void *sgl);

/**
 * @brief Retrieves the I/O Virtual Address (IOVA) associated with the
 *        scatter gather list entry.
 *
 * The steps performed by the function are as follows:
 * -# Call the function pointed by 'sgl_dma()' in the 'ops' field of the
 *    scatter gather table object to retrieve the IOVA address of the provided
 *    scatter gather list entry.
 *
 * @param [in] sgt  Pointer to the scatter gather table object.
 * @param [in] sgl  Pointer to the scatter gather list entry.
 *
 * @return IOVA address associated with the scatter gather list entry.
 */
u64 nvgpu_sgt_get_dma(struct nvgpu_sgt *sgt, void *sgl);

/**
 * @brief Retrieves the length associated with a scatter gather list entry.
 *
 * This function returns the length of memory covered by a scatter gather list
 * entry. It does this by invoking the sgl_length operation from the sgt's ops
 * structure, which is expected to be implemented by the underlying OS-specific
 * code.
 *
 * The steps performed by the function are as follows:
 * -# Call the sgl_length() function through the ops structure pointer of the
 *    scatter gather table object, passing the scatter gather list entry as an
 *    argument to obtain the length of memory it covers.
 *
 * @param [in] sgt  Pointer to scatter gather table object. It must not be NULL
 *                  and must contain valid ops and sgl_length function pointers.
 *                  Range: Non-NULL pointers.
 * @param [in] sgl  Pointer to scatter gather list entry. It must not be NULL.
 *                  Range: Non-NULL pointers.
 *
 * @return The length of memory covered by the scatter gather list entry.
 */
u64 nvgpu_sgt_get_length(struct nvgpu_sgt *sgt, void *sgl);

/**
 * @brief Retrieves the GPU address for a given scatter gather list entry.
 *
 * This function delegates the retrieval of the GPU address to the specific
 * implementation provided by the scatter gather table operations. It is a
 * wrapper that abstracts the underlying method used to obtain the GPU address
 * associated with a scatter gather list entry.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'sgl_gpu_addr()' function pointed to by the 'ops' field in the
 *    scatter gather table structure, passing the GPU structure, the scatter
 *    gather list entry, and the GMMU attributes as arguments.
 *    - Check the address associated with given scatter list is io virtual address
 *      or intermediate physical address(ipa).
 *      - If the address is ipa, Query hypervisor to convert to physical address.
 *      - If the address is iova, call nvgpu_mem_iommu_translate() to get physical
 *        address.
 *
 * @param [in]  g       The GPU structure.
 * @param [in]  sgt     The scatter gather table structure.
 * @param [in]  sgl     The scatter gather list entry.
 * @param [in]  attrs   The GMMU attributes.
 *
 * @return The GPU address associated with the scatter gather list entry.
 */
u64 nvgpu_sgt_get_gpu_addr(struct gk20a *g, struct nvgpu_sgt *sgt,
			void*sgl, struct nvgpu_gmmu_attrs *attrs);

/**
 * @brief Free the scatter gather table object.
 *
 * This function is responsible for freeing the memory associated with a
 * scatter gather table object. It checks if the scatter gather table object
 * and its associated operations are not NULL and then calls the appropriate
 * free operation to release the memory.
 *
 * The steps performed by the function are as follows:
 * -# Check if the scatter gather table object is not NULL.
 * -# Check if the free operation for the scatter gather table object is not NULL.
 * -# If both checks pass, call the free operation for the scatter gather table
 *    object using the provided GPU and scatter gather table object.
 * -# Walk through the given sgl list by sgl.next and free the sgl.next.
 *
 * @param [in] g    The GPU.
 * @param [in] sgt  Pointer to scatter gather table object.
 *
 * @return None.
 */
void nvgpu_sgt_free(struct gk20a *g, struct nvgpu_sgt *sgt);

/**
 * @brief Check if the scatter gather table (SGT) is iommuable.
 *
 * This function determines if the SGT is iommuable, which means it checks if
 * the SGT can be used with an I/O Memory Management Unit (IOMMU). The function
 * will return true if the SGT is iommuable, otherwise false.
 *
 * The steps performed by the function are as follows:
 * -# Check if the 'sgt_iommuable' operation is implemented for the given SGT.
 * -# If the operation is implemented, invoke the 'sgt_iommuable()' operation
 *    to determine if the SGT is iommuable.
 * -# If the operation is not implemented, return false indicating that the SGT
 *    is not iommuable.
 *
 * @param [in] g   The GPU.
 * @param [in] sgt The scatter gather table object.
 *
 * @return True if the SGT is iommuable, false otherwise.
 */
bool nvgpu_sgt_iommuable(struct gk20a *g, struct nvgpu_sgt *sgt);

/**
 * @brief Determine the minimum alignment requirement for a scatter gather table.
 *
 * This function calculates the minimum alignment requirement for the buffer
 * described by a scatter gather table (SGT). If the SGT is iommuable and an
 * IOMMU address is to be used, the alignment is based on the IOMMU address.
 * Otherwise, the function iterates over each scatter gather list (SGL) entry
 * within the SGT to determine the minimum alignment based on the physical
 * addresses and lengths of the SGL entries.
 *
 * This is necessary since the buffer may appear big enough to be mapped with
 * large pages. However, the SGL may have chunks that are not aligned on 64/128kB
 * large page boundary. There's also a possibility that chunks are of odd sizes
 * which will necessitate small page mappings to correctly glue them together into
 * a contiguous virtual mapping. If this SGT is iommuable and we want to use the IOMMU
 * address, then the SGT's first entry has the IOMMU address. We will align on this
 * and double check the length of the buffer later. In addition, we know that this DMA
 * address is contiguous since there's an IOMMU.
 *
 * @param [in] g    The GPU for which the alignment is being determined.
 *                   Range: A valid pointer to a struct gk20a.
 * @param [in] sgt  The scatter gather table for which the alignment is being
 *                  determined.
 *                   Range: A valid pointer to a struct nvgpu_sgt.
 *
 * @return The minimum alignment requirement in bytes.
 */
u64 nvgpu_sgt_alignment(struct gk20a *g, struct nvgpu_sgt *sgt);

/** @cond DOXYGEN_SHOULD_SKIP_THIS */
#if defined(__NVGPU_POSIX__)
void *nvgpu_mem_sgl_next(void *sgl);
u64 nvgpu_mem_sgl_phys(struct gk20a *g, void *sgl);
u64 nvgpu_mem_sgl_ipa_to_pa(struct gk20a *g, void *sgl, u64 ipa,
				u64 *pa_len);
u64 nvgpu_mem_sgl_dma(void *sgl);
u64 nvgpu_mem_sgl_length(void *sgl);
u64 nvgpu_mem_sgl_gpu_addr(struct gk20a *g, void *sgl,
				  struct nvgpu_gmmu_attrs *attrs);
bool nvgpu_mem_sgt_iommuable(struct gk20a *g, struct nvgpu_sgt *sgt);
void nvgpu_mem_sgt_free(struct gk20a *g, struct nvgpu_sgt *sgt);
#endif
/** @endcond DOXYGEN_SHOULD_SKIP_THIS */

#endif /* NVGPU_SGT_H */
