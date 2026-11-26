/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_MEM_H
#define NVGPU_MEM_H

#include <nvgpu/types.h>
#include <nvgpu/list.h>
#include <nvgpu/enabled.h>
#include <nvgpu/gmmu.h>

#ifdef __KERNEL__
#include <nvgpu/linux/nvgpu_mem.h>
#elif defined(__NVGPU_POSIX__)
#include <nvgpu/posix/nvgpu_mem.h>
#elif defined(NVGPU_HVRTOS)
#include <nvgpu_hvrtos/nvgpu_mem.h>
#else
#include <nvgpu/qnx/nvgpu_mem.h>
#endif

/**
 * Memory Interface for all GPU accessible memory.
 */

struct page;
struct sg_table;
struct gk20a;
struct nvgpu_allocator;
struct nvgpu_gmmu_attrs;
struct nvgpu_page_alloc;
struct nvgpu_sgt;

/**
 * The nvgpu_mem structure defines abstracted GPU accessible memory regions.
 */
struct nvgpu_mem {
	/**
	 * Indicates memory type of original allocation.
	 */
	enum nvgpu_aperture			 aperture;
	/**
	 * Size of memory segment requested during creation.
	 */
	size_t					 size;
	/**
	 * Total amount of memory allocated after aligning requested size.
	 */
	size_t					 aligned_size;
	/**
	 * Address of mapped GPU memory, if any.
	 */
	u64					 gpu_va;
	/**
	 * Flag to indicate write memory barrier requirement.
	 */
	bool					 skip_wmb;
	/**
	 * Indicates if gpu_va address is valid.
	 */
	bool					 free_gpu_va;

	/**
	 * Set when a nvgpu_mem struct is not a "real" nvgpu_mem struct. Instead
	 * the struct is just a copy of another nvgpu_mem struct.
	 */
#define NVGPU_MEM_FLAG_SHADOW_COPY		 BIT64(0)

	/**
	 * Specify that the GVA mapping is a fixed mapping - that is the caller
	 * chose the GPU VA, not the GMMU mapping function. Only relevant for
	 * VIDMEM.
	 */
#define NVGPU_MEM_FLAG_FIXED			 BIT64(1)

	/**
	 * Set for user generated VIDMEM allocations. This triggers a special
	 * cleanup path that clears the vidmem on free. Given that the VIDMEM is
	 * zeroed on boot this means that all user vidmem allocations are
	 * therefor zeroed (to prevent leaking information in VIDMEM buffers).
	 */
#define NVGPU_MEM_FLAG_USER_MEM			 BIT64(2)

	/**
	 * Internal flag that specifies this struct has not been made with DMA
	 * memory and as a result should not try to use the DMA routines for
	 * freeing the backing memory.
	 *
	 * However, this will not stop the DMA API from freeing other parts of
	 * nvgpu_mem in a system specific way.
	 */
#define NVGPU_MEM_FLAG_NO_DMA			 BIT64(3)

	/**
	 * Some nvgpu_mem objects act as facades to memory buffers owned by
	 * someone else. This internal flag specifies that the sgt field is
	 * "borrowed", and it must not be freed by us.
	 *
	 * Of course the caller will have to make sure that the sgt owner
	 * outlives the nvgpu_mem.
	 */
#define NVGPU_MEM_FLAG_FOREIGN_SGT		 BIT64(4)
	/**
	 * Some nvgpu_mem objects act as facades to memory buffers owned by
	 * sync_point shim and need to freed when dma_free_sys invokes.
	 */
#define NVGPU_MEM_FLAG_SYNCPT_IOVA               BIT64(5)

	/**
	 * Store flag bits indicating conditions for nvgpu_mem struct instance.
	 */
	unsigned long				 mem_flags;

	/**
	 * Pointer to sysmem memory address. Only populated for a sysmem
	 * allocation.
	 */
	void					*cpu_va;

#ifdef CONFIG_NVGPU_DGPU

	/**
	 * Pointer to allocated chunks of pages constituting requested vidmem
	 * type memory. This memory is allocated from GPU vidmem memory.
	 * Evidently, only populated for vidmem allocations.
	 */
	struct nvgpu_page_alloc			*vidmem_alloc;

	/**
	 * Pointer to GPU vidmem allocator. This allocator is used to allocate
	 * memory pointed by #vidmem_alloc. Only populated for vidmem
	 * allocations.
	 */
	struct nvgpu_allocator			*allocator;

	/**
	 * Clear list entry node. This node is used to register this nvgpu_mem
	 * vidmem instance in GPU vidmem allocations list. This node can be used
	 * to search outstanding vidmem allocations. Only populated for vidmem
	 * allocations.
	 */
	struct nvgpu_list_node			 clear_list_entry;
#endif

	/**
	 * Pointer to scatter gather table for direct "physical" nvgpu_mem
	 * structs.
	 */
	struct nvgpu_sgt			*phys_sgt;

	/**
	 * Structure containing system specific memory pointers.
	 * This is defined by the system specific header. It can be empty if
	 * there's no system specific stuff for a given system.
	 */
	struct nvgpu_mem_priv			 priv;
};

#ifdef CONFIG_NVGPU_DGPU
static inline struct nvgpu_mem *
nvgpu_mem_from_clear_list_entry(struct nvgpu_list_node *node)
{
	return (struct nvgpu_mem *)
		((uintptr_t)node - offsetof(struct nvgpu_mem,
					    clear_list_entry));
};
#endif

/**
 * @brief Convert aperture type to string.
 *
 * @param[in] aperture		Aperture type.
 *
 * @return Aperture type string.
 */
static inline const char *nvgpu_aperture_str(enum nvgpu_aperture aperture)
{
	const char *aperture_name[APERTURE_MAX_ENUM + 1] = {
		[APERTURE_INVALID]	= "INVAL",
		[APERTURE_SYSMEM]	= "SYSTEM",
		[APERTURE_SYSMEM_COH]	= "SYSCOH",
		[APERTURE_VIDMEM]	= "VIDMEM",
		[APERTURE_MAX_ENUM]	= "UNKNOWN",
	};

	if ((aperture < APERTURE_INVALID) || (aperture >= APERTURE_MAX_ENUM)) {
		return aperture_name[APERTURE_MAX_ENUM];
	}
	return aperture_name[aperture];
}

/**
 * @brief Determine if the aperture is system memory.
 *
 * This function checks if the given aperture type represents system memory,
 * either coherent or non-coherent.
 *
 * The steps performed by the function are as follows:
 * -# Check if the aperture type is either coherent system memory or
 *    non-coherent system memory.
 * -# Return true if the aperture type matches either of the system memory
 *    types, otherwise return false.
 *
 * @param [in] ap  Aperture type to be checked.
 *                 Range: #APERTURE_SYSMEM_COH, #APERTURE_SYSMEM
 *
 * @return True if the aperture is system memory, false otherwise.
 */
bool nvgpu_aperture_is_sysmem(enum nvgpu_aperture ap);

/**
 * @brief Check if the memory is of SYSMEM type.
 *
 * This function determines if the memory represented by the given nvgpu_mem
 * structure is system memory (SYSMEM) by checking the aperture type.
 *
 * The steps performed by the function are as follows:
 * -# Call the function nvgpu_aperture_is_sysmem() with the aperture type of the
 *    memory to check if it is system memory.
 *
 * @param [in] mem  Pointer to the nvgpu_mem structure.
 *                   Range: Valid pointer to nvgpu_mem structure.
 *
 * @return True if the memory is system memory, false otherwise.
 */
bool nvgpu_mem_is_sysmem(struct nvgpu_mem *mem);

/**
 * @brief Check if given nvgpu_mem structure is valid for subsequent use.
 *
 * @param[in] mem	Pointer to nvgpu_mem structure.
 *			Cannot be NULL.
 *
 * @return True if the passed nvgpu_mem has been allocated, false otherwise.
 */
static inline bool nvgpu_mem_is_valid(struct nvgpu_mem *mem)
{
	/*
	 * Internally the DMA APIs must set/unset the aperture flag when
	 * allocating/freeing the buffer. So check that to see if the *mem
	 * has been allocated or not.
	 *
	 * This relies on mem_descs being zeroed before being initialized since
	 * APERTURE_INVALID is equal to 0.
	 */
	return mem->aperture != APERTURE_INVALID;

}
/**
 * @brief Create a new nvgpu_mem struct from an existing one.
 *
 * This function creates a new nvgpu_mem struct that describes a subsection of
 * the source nvgpu_mem. It calculates the start and size of the new memory
 * region based on the given page start and number of pages, validates the
 * aperture type and size constraints, and then sets up the destination
 * nvgpu_mem struct to share the mapping created by the source nvgpu_mem.
 *
 * There is a _major_ caveat to this API: if the source buffer is freed before
 * the copy is freed then the copy will become invalid. This is a result from
 * how typical DMA APIs work: we can't call free on the buffer multiple times.
 * Nor can we call free on parts of a buffer. Thus the only way to ensure that
 * the entire buffer is actually freed is to call free once on the source
 * buffer. Since these nvgpu_mem structs are not ref-counted in anyway it is up
 * to the caller of this API to _ensure_ that the resulting nvgpu_mem buffer
 * from this API is freed before the source buffer. Otherwise there can and will
 * be memory corruption.
 *
 * The resulting nvgpu_mem should be released with the nvgpu_dma_free() or the
 * nvgpu_dma_unmap_free() function depending on whether or not the resulting
 * nvgpu_mem has been mapped.
 *
 * The steps performed by the function are as follows:
 * -# Calculate the start byte offset of the new memory region by multiplying
 *    the start page number with the size of a CPU page using nvgpu_safe_mult_u64().
 * -# Calculate the size in bytes of the new memory region by multiplying the
 *    number of pages with the size of a CPU page using nvgpu_safe_mult_u64().
 * -# Validate that the source memory's aperture is of type SYSMEM.
 * -# Check if the calculated size is greater than the size of the source memory.
 * -# Check if the sum of the start offset and size exceeds the size of the source
 *    memory using nvgpu_safe_add_u64().
 * -# Retrieve the memory descriptor from the source memory based on the calculated
 *    offset and size using nvgpu_nvmap_mem_get_mdesc_from_offset_size().
 * -# If the retrieval is successful, set the CPU virtual address of the destination
 *    memory to the corresponding offset within the source memory's CPU virtual address.
 * -# Initialize the destination memory's private handle to zero to indicate it is
 *    sharing the source memory's mapping.
 * -# Set the destination memory's private descriptor offset to the calculated offset.
 * -# Copy the flags from the source memory to the destination memory.
 * -# Set the destination memory's flags to indicate it is a shadow copy of the source
 *    memory.
 * -# Copy the aperture type and write memory barrier flag from the source memory to the
 *    destination memory.
 * -# Set the size of the destination memory to the calculated size.
 * -# Return 0 to indicate success.
 *
 * @param [in]  g            Pointer to the GPU structure.
 * @param [out] dest         Pointer to the destination nvgpu_mem structure.
 * @param [in]  src          Pointer to the source nvgpu_mem structure.
 * @param [in]  start_page   Start page number of the memory to be created.
 *                           Range: [0, src->size / NVGPU_CPU_PAGE_SIZE)
 * @param [in]  nr_pages     Number of pages to be included in the new memory.
 *                           Range: [1, src->size / NVGPU_CPU_PAGE_SIZE - start_page]
 *
 * @return 0 on success.
 * @return -EINVAL if the input parameters are invalid or if the operation fails.
 */
int nvgpu_mem_create_from_mem(struct gk20a *g,
			      struct nvgpu_mem *dest, struct nvgpu_mem *src,
			      u64 start_page, size_t nr_pages);

/**
 * @brief Creates a memory descriptor from a given physical memory address.
 *
 * This function initializes a memory descriptor for a contiguous block of
 * physical memory. It allocates and sets up the scatter-gather table (SGT)
 * and scatter-gather list (SGL) for the memory region, and populates the
 * memory descriptor with the appropriate values including aperture type,
 * size, aligned size, and memory flags.
 *
 * The physical memory aperture needs to be contiguous for requested @nr_pages.
 * This API only works for SYSMEM. This also assumes a 4K page granule since the GMMU
 * always supports 4K pages. If _system_ pages are larger than 4K then the
 * resulting nvgpu_mem will represent less than 1 OS page worth of memory
 *
 * The resulting nvgpu_mem should be released with the nvgpu_dma_free() or the
 * nvgpu_dma_unmap_free() function depending on whether or not the resulting
 * nvgpu_mem has been mapped.
 *
 * The steps performed by the function are as follows:
 * -# Set the aperture type of the memory descriptor to indicate system memory.
 * -# Calculate the size of the memory region by multiplying the number of pages
 *    by the size of a CPU page.
 * -# Set the aligned size of the memory descriptor to the calculated size.
 * -# Set the memory flags of the memory descriptor to indicate no IOVA is used.
 * -# Set the SGT pointer in the memory descriptor to the allocated SGT.
 * -# Initialize the SGL with the provided physical address, set the next pointer
 *    to NULL, and set the length to the size of the memory region.
 * -# Set the SGL pointer in the SGT to the allocated SGL.
 * -# Set the operations for the SGT to the physical memory operations.
 * -# Return the success code.
 *
 * @param [in]  g         Pointer to the GPU structure.
 * @param [out] dest      Pointer to the memory descriptor to be populated.
 * @param [in]  src_phys  Physical address of the memory region.
 * @param [in]  nr_pages  Number of pages in the memory region.
 *
 * @return 0 on success, negative error code on failure.
 * @retval -ENOMEM if memory allocation for SGT or SGL fails.
 */
int nvgpu_mem_create_from_phys(struct gk20a *g, struct nvgpu_mem *dest,
			       u64 src_phys, u64 nr_pages);

#ifdef CONFIG_NVGPU_DGPU
/**
 * @brief Free vidmem buffer.
 *
 * @param[in] g			Pointer to GPU structure.
 * @param[in] vidmem		Pointer to nvgou_mem structure to be freed.
 *
 * Really free a vidmem buffer. There's a fair amount of work involved in
 * freeing vidmem buffers in the DMA API. This handles none of that - it only
 * frees the underlying vidmem specific structures used in vidmem buffers.
 *
 * This is implemented in the OS specific code. If it's not necessary it can
 * be a noop. But the symbol must at least be present.
 */
void nvgpu_mem_free_vidmem_alloc(struct gk20a *g, struct nvgpu_mem *vidmem);
#endif

/*
 * Buffer accessors. Sysmem buffers always have a CPU mapping and vidmem
 * buffers are accessed via PRAMIN.
 */

/**
 * @brief Reads a 32-bit word from the specified memory location.
 *
 * The function reads a 32-bit word from a memory buffer described by the
 * nvgpu_mem structure. The read operation is performed based on the aperture
 * type of the memory buffer. If the aperture is SYSMEM, the function reads
 * directly from the CPU virtual address. If the aperture is VIDMEM, the
 * function uses the nvgpu_pramin_rd_n() function to read from video memory.
 * The function asserts if the aperture type is neither SYSMEM nor VIDMEM,
 * indicating an attempt to access unallocated memory.
 *
 * The steps performed by the function are as follows:
 * -# Check if the aperture type of the memory buffer is SYSMEM.
 *    - If SYSMEM, read the data directly from the CPU virtual address.
 *    - If the pointer to the CPU virtual address is NULL, trigger a bug.
 * -# If the aperture type is VIDMEM (only for configurations with DGPU support),
 *    call nvgpu_pramin_rd_n() to read the data from video memory.
 * -# If the aperture type is neither SYSMEM nor VIDMEM, call nvgpu_do_assert_print()
 *    to handle the assertion for accessing unallocated memory.
 * -# Return the data read from the memory buffer.
 *
 * @param [in]  g         Pointer to the GPU structure.
 * @param [in]  mem       Pointer to the memory buffer structure.
 * @param [in]  w         Index of the word to read from the memory buffer.
 *
 * @return The 32-bit word read from the specified memory location.
 * @retval 0 in case of invalid mem.
 */
u32 nvgpu_mem_rd32(struct gk20a *g, struct nvgpu_mem *mem, u64 w);

/**
 * @brief Read two 32-bit values from GPU memory and combine into a 64-bit value.
 *
 * This function reads two 32-bit values from the specified locations in GPU
 * memory and combines them into a single 64-bit value. The lower 32 bits of the
 * result come from the first memory location, and the upper 32 bits come from
 * the second memory location.
 *
 * The steps performed by the function are as follows:
 * -# Read the 32-bit value from the memory location specified by the lower offset.
 * -# Convert the read 32-bit value to a 64-bit value.
 * -# Read the 32-bit value from the memory location specified by the higher offset.
 * -# Convert the read 32-bit value to a 64-bit value.
 * -# Shift the second 64-bit value left by 32 bits to prepare for combination.
 * -# Combine the two 64-bit values into one by performing a bitwise OR operation.
 *
 * @param [in] g        Pointer to the GPU structure.
 * @param [in] mem      Pointer to the GPU memory descriptor.
 * @param [in] lo       Lower offset within the GPU memory from where to read the
 *                      first 32-bit value.
 * @param [in] hi       Higher offset within the GPU memory from where to read the
 *                      second 32-bit value.
 *
 * @return Combined 64-bit value read from the two memory locations.
 * @retval 0 in case of invalid mem.
 */
u64 nvgpu_mem_rd32_pair(struct gk20a *g, struct nvgpu_mem *mem,
		u32 lo, u32 hi);

/**
 * @brief Read a 32-bit word from the specified memory offset.
 *
 * This function reads a 32-bit word from the memory region described by the
 * nvgpu_mem structure at the specified byte offset. It ensures the offset is
 * 32-bit aligned and then calls another function to perform the actual read.
 *
 * The steps performed by the function are as follows:
 * -# Check if the offset is 32-bit aligned by verifying the lower two bits are zero.
 * -# Call 'nvgpu_mem_rd32()' to read a 32-bit word from the memory at the given
 *    offset divided by the size of a 32-bit word, effectively converting the byte
 *    offset to a word offset.
 *
 * @param [in]  g        Pointer to the GPU structure.
 * @param [in]  mem      Pointer to the memory descriptor structure.
 * @param [in]  offset   Byte offset within the memory from where to read.
 *                       Must be a multiple of 4 (32-bit aligned).
 *
 * @return The 32-bit word read from the specified memory offset.
 * @retval 0 in case of invalid mem.
 */
u32 nvgpu_mem_rd(struct gk20a *g, struct nvgpu_mem *mem, u64 offset);

/**
 * @brief Reads a block of data from GPU accessible memory into CPU memory.
 *
 * This function reads a specified number of bytes from a given offset within a
 * GPU accessible memory region and copies it into a destination buffer in CPU
 * memory. It supports reading from system memory (SYSMEM) and video memory
 * (VIDMEM) depending on the aperture type of the memory region.
 *
 * The steps performed by the function are as follows:
 * -# Validate that the offset and size are 32-bit aligned by checking if the
 *    least significant 2 bits are zero.
 * -# If the memory region's aperture is system memory (SYSMEM), perform the
 *    following sub-steps:
 *    -# Ensure that the CPU virtual address (cpu_va) is not NULL, and if it is,
 *       trigger a bug indicating a critical error.
 *    -# Calculate the source address by adding the offset to the CPU virtual
 *       address.
 *    -# Copy the data from the source address to the destination buffer using
 *       nvgpu_memcpy().
 * -# If the memory region's aperture is video memory (VIDMEM) and the build
 *    configuration supports discrete GPUs (CONFIG_NVGPU_DGPU), perform the
 *    following sub-steps:
 *    -# Read the data from the video memory into the destination buffer using
 *       nvgpu_pramin_rd_n().
 * -# If the aperture type is neither SYSMEM nor VIDMEM, print an assertion
 *    message indicating that an unallocated memory region is being accessed.
 *
 * @param [in]  g         Pointer to the GPU structure.
 * @param [in]  mem       Pointer to the GPU accessible memory region.
 *                        The aperture field of this structure determines the
 *                        memory type (SYSMEM or VIDMEM).
 * @param [in]  offset    Byte offset within the memory region from where to
 *                        start reading data. Must be 32-bit aligned.
 * @param [out] dest      Pointer to the destination buffer in CPU memory where
 *                        the read data will be copied.
 * @param [in]  size      Number of bytes to read from the memory region. Must
 *                        be 32-bit aligned.
 * @return None.
 */
void nvgpu_mem_rd_n(struct gk20a *g, struct nvgpu_mem *mem, u64 offset,
		void *dest, u64 size);

/**
 * @brief Writes a 32-bit word to the specified memory location.
 *
 * This function writes a 32-bit data word to a memory location within a given
 * nvgpu_mem structure. The location to write to is determined by the word
 * index within the memory region. The function handles different memory
 * apertures (SYSMEM and VIDMEM) and performs the write accordingly.
 *
 * The steps performed by the function are as follows:
 * -# Check if the memory aperture is SYSMEM (system memory).
 *    - If true, get the CPU virtual address of the memory and write the data
 *      word to the location indexed by the word index.
 *    - If the CPU virtual address is NULL, trigger a bug check with BUG_ON().
 * -# If the memory aperture is not SYSMEM, check if it is VIDMEM (video memory).
 *    - If true, calculate the byte offset by multiplying the word index with
 *      the size of a 32-bit word and call nvgpu_pramin_wr_n() to write the
 *      data word to the video memory.
 *    - After writing, if the memory region is not marked to skip the write
 *      memory barrier, call nvgpu_wmb() to ensure the write is completed before
 *      continuing.
 * -# If the memory aperture is neither SYSMEM nor VIDMEM, print an assertion
 *    message indicating that an unallocated nvgpu_mem is being accessed.
 *
 * @param [in] g          Pointer to the GPU structure.
 * @param [in] mem        Pointer to the nvgpu_mem structure representing the
 *                        memory region to write to.
 * @param [in] w          Word index within the memory region where the data
 *                        word should be written.
 * @param [in] data       The 32-bit data word to write.
 *
 * @return None.
 */
void nvgpu_mem_wr32(struct gk20a *g, struct nvgpu_mem *mem, u64 w, u32 data);

/**
 * @brief Write a 32-bit word to the specified offset within the GPU memory.
 *
 * This function writes a 32-bit word to a given offset within a memory region
 * represented by an nvgpu_mem structure. It ensures the offset is 32-bit
 * aligned and then delegates the actual write operation to nvgpu_mem_wr32().
 *
 * The steps performed by the function are as follows:
 * -# Divide the offset by the size of a 32-bit word to convert the byte offset
 *    into a word offset.
 * -# Call nvgpu_mem_wr32() to perform the actual write operation using the
 *    calculated word offset and the data to be written.
 *
 * @param [in] g          Pointer to the GPU structure.
 * @param [in] mem        Pointer to the memory descriptor structure where the
 *                        data will be written.
 * @param [in] offset     Byte offset within the memory region where the data
 *                        will be written. Must be 32-bit aligned.
 * @param [in] data       32-bit data word to be written at the specified offset.
 *
 * @return None.
 */
void nvgpu_mem_wr(struct gk20a *g, struct nvgpu_mem *mem, u64 offset, u32 data);

/**
 * @brief Writes a block of data to GPU memory.
 *
 * This function writes a block of data from a source buffer to a specified
 * location in GPU memory, which can be either system memory (sysmem) or
 * video memory (vidmem). The function ensures that the offset and size are
 * 32-bit aligned and that the GPU memory is properly allocated before
 * performing the write operation. For vidmem, it also ensures a write memory
 * barrier if required.
 *
 * The steps performed by the function are as follows:
 * -# Check if both offset and size are 32-bit aligned, warn if it is not.
 * -# If the GPU memory is system memory, perform the following steps:
 *    -# Calculate the destination address by adding the offset to the CPU
 *       virtual address of the GPU memory.
 *    -# Copy the data from the source buffer to the calculated destination
 *       address using nvgpu_memcpy().
 * -# If the GPU memory is video memory, perform the following steps:
 *    -# Write the data from the source buffer to the GPU memory at the
 *       specified offset using nvgpu_pramin_wr_n().
 *    -# If a write memory barrier is not skipped, invoke nvgpu_wmb() to
 *       ensure the write operation is completed before proceeding.
 * -# If the GPU memory is neither system memory nor video memory, assert
 *    an error indicating that an attempt is made to access unallocated GPU
 *    memory.
 *
 * @param [in] g        Pointer to the GPU structure.
 * @param [in] mem      Pointer to the GPU memory structure.
 * @param [in] offset   Byte offset within the GPU memory where the data
 *                      should be written.
 * @param [in] src      Pointer to the source buffer containing the data
 *                      to be written.
 * @param [in] size     Number of bytes to be written to the GPU memory.
 *
 * @return None.
 */
void nvgpu_mem_wr_n(struct gk20a *g, struct nvgpu_mem *mem, u64 offset,
		void *src, u64 size);

/**
 * @brief Set a block of GPU memory to a specified value.
 *
 * This function fills a block of GPU memory with a specified value. It handles
 * both system memory (sysmem) and video memory (vidmem) depending on the
 * aperture type of the memory. It ensures the offset and size are 32-bit
 * aligned and the fill value is a valid 8-bit value.
 *
 * The steps performed by the function are as follows:
 * -# Check if both offset and size are 32-bit aligned, warn if it is not.
 * -# Check if the fill value is not an 8-bit value and warn if it is not.
 * -# If the aperture type is system memory, calculate the destination pointer
 *    by adding the offset to the CPU virtual address of the memory. Then use
 *    memset() to fill the memory with the specified value.
 * -# If the aperture type is video memory and the platform supports it,
 *    construct a 32-bit pattern from the 8-bit fill value and call
 *    nvgpu_pramin_memset() to fill the memory. If a write memory barrier is
 *    required, call nvgpu_wmb().
 * -# If the aperture type is neither system memory nor video memory, assert
 *    an error indicating an attempt to access unallocated memory.
 *
 * @param [in] g        Pointer to the GPU structure.
 * @param [in] mem      Pointer to the memory descriptor.
 * @param [in] offset   Byte offset within the memory where filling starts.
 * @param [in] c        The value to set the memory to. Only the least
 *                      significant 8 bits are used.
 * @param [in] size     Number of bytes to fill.
 *
 * @return None.
 */
void nvgpu_memset(struct gk20a *g, struct nvgpu_mem *mem, u64 offset,
		u32 c, u64 size);

/**
 * @brief Retrieve the base address of a memory allocation.
 *
 * This function returns the base address of a memory allocation described by
 * the nvgpu_mem structure. It handles both video memory (VIDMEM) and system
 * memory (SYSMEM) allocations. For VIDMEM, it asserts that the allocation
 * consists of a single chunk and returns the base address of that chunk. For
 * SYSMEM, it delegates to another function to retrieve the address.
 *
 * The steps performed by the function are as follows:
 * -# If the system is configured for discrete GPUs (CONFIG_NVGPU_DGPU - disabled for QNX Safety),
 *    check if the memory allocation is in the video memory aperture.
 * -# If the memory allocation is in the video memory aperture:
 *    -# Retrieve the video memory allocation structure from the nvgpu_mem
 *       structure.
 *    -# Assert that the number of chunks in the video memory allocation is one.
 *    -# Return the base address of the video memory allocation.
 * -# [For QNX Safety] If the memory allocation is not in the video memory aperture,
 *    call the function to retrieve the system memory address 'nvgpu_nvmap_get_addr_sysmem()'.
 *
 * @param [in]  g    Pointer to the GPU structure.
 * @param [in]  mem  Pointer to the memory allocation structure.
 *
 * @return The base address of the memory allocation.
 * @retval 0 in case of invalid #mem.
 */
u64 nvgpu_mem_get_addr(struct gk20a *g, struct nvgpu_mem *mem);

/**
 * @brief Retrieve the physical address of a memory allocation.
 *
 * This function is responsible for obtaining the physical address of a memory
 * allocation represented by the nvgpu_mem structure. It differentiates between
 * memory allocated in video memory (VIDMEM) and system memory (SYSMEM), and
 * retrieves the physical address accordingly. For VIDMEM, it calls another
 * function to get the address, and for SYSMEM, it retrieves the address
 * directly.
 *
 * This should only be used on contiguous buffers regardless of whether
 * there's an IOMMU present/enabled. This applies to both SYSMEM and
 * VIDMEM.
 *
 * The steps performed by the function are as follows:
 * -# If the system is configured for discrete GPUs (CONFIG_NVGPU_DGPU - disabled for QNX Safety),
 *    check if the memory allocation is in the video memory aperture.
 * -# If the memory allocation is in the video memory aperture, call the
 *    function 'nvgpu_mem_get_addr()' to retrieve the address of the memory
 *    allocation.
 * -# [For QNX Safety] If the memory allocation is not in the video memory aperture, call the
 *    function 'nvgpu_nvmap_get_phys_addr()' to retrieve the physical address
 *    of the system memory allocation.
 *
 * @param [in]  g    Pointer to the GPU structure.
 * @param [in]  mem  Pointer to the memory allocation structure.
 *
 * @return The physical address of the memory allocation.
 * @retval 0 in case of invalid #mem.
 */
u64 nvgpu_mem_get_phys_addr(struct gk20a *g, struct nvgpu_mem *mem);

/**
 * @brief Determines the aperture mask based on the given aperture type.
 *
 * This function selects the appropriate mask for the memory aperture type
 * specified. It handles special cases where system memory is treated as video
 * memory by certain integrated GPUs. The function also asserts if an invalid
 * aperture type is provided.
 *
 * The steps performed by the function are as follows:
 * -# Initialize the return mask to zero.
 * -# Switch on the aperture type provided:
 *    - If the aperture type is coherent system memory, set the return mask to
 *      the coherent system memory mask.
 *    - If the aperture type is non-coherent system memory, set the return mask
 *      to the system memory mask.
 *    - If the aperture type is video memory, set the return mask to the video
 *      memory mask.
 *    - If the aperture type is none of the above, assert with a message
 *      indicating a bad aperture type.
 * -# Check if the GPU honors the aperture setting:
 *    - If the GPU does not honor the aperture and the return mask is non-zero,
 *      override the return mask with the video memory mask.
 * -# Return the determined mask.
 *
 * @param [in]  g               Pointer to the GPU superstructure.
 * @param [in]  aperture        Aperture type to determine the mask for.
 *                              Range: #APERTURE_SYSMEM_COH, #APERTURE_SYSMEM,
 *                              #APERTURE_VIDMEM
 * @param [in]  sysmem_mask     Mask for non-coherent system memory.
 * @param [in]  sysmem_coh_mask Mask for coherent system memory.
 * @param [in]  vidmem_mask     Mask for video memory.
 *
 * @return Mask corresponding to the aperture type.
 * @retval 0 in case of invalid aperture.
 */
u32 nvgpu_aperture_mask_raw(struct gk20a *g, enum nvgpu_aperture aperture,
		u32 sysmem_mask, u32 sysmem_coh_mask, u32 vidmem_mask);

/**
 * @brief Get aperture mask value.
 *
 * This function retrieves the aperture mask value based on the memory type
 * (system memory, coherent system memory, or video memory) of the given memory
 * object. It calls nvgpu_aperture_mask_raw() to get the raw aperture mask value.
 *
 * Correct coherency aperture type should be used. This function doesn't add any checks.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the aperture type from the memory object.
 * -# Call nvgpu_aperture_mask_raw() to get the raw aperture mask value using the
 *    retrieved aperture type and the provided system memory masks and video memory
 *    mask.
 *
 * @param [in]  g               Pointer to the GPU structure.
 * @param [in]  mem             Pointer to the memory structure.
 * @param [in]  sysmem_mask     Mask for system memory.
 * @param [in]  sysmem_coh_mask Mask for coherent system memory.
 * @param [in]  vidmem_mask     Mask for video memory.
 *
 * @return Aperture mask value in case of success.
 * @retval 0 in case of invalid memory type.
 */
u32 nvgpu_aperture_mask(struct gk20a *g, struct nvgpu_mem *mem,
		u32 sysmem_mask, u32 sysmem_coh_mask, u32 vidmem_mask);

/**
 * @brief Translate a physical memory address to an IOMMU address if applicable.
 *
 * This function checks if the GPU memory management unit is IOMMU capable and
 * if so, it sets the appropriate IOMMU bit in the provided physical address.
 * This is typically used to modify the physical address before programming it
 * into GPU page tables. The function ensures that the provided physical address
 * does not correspond to a vidmem allocation.
 *
 * The steps performed by the function are as follows:
 * -# Check if the build configuration supports discrete GPUs and if the provided
 *    physical address corresponds to a vidmem page allocation. If so, issue a
 *    warning.
 * -# Check if the GPU is IOMMU capable and if the function pointer to retrieve
 *    the IOMMU bit is not NULL.
 * -# If the GPU is IOMMU capable, modify the physical address by setting the
 *    IOMMU bit.
 * -# Return the possibly modified physical address.
 *
 * @param [in] g            Pointer to the GPU structure.
 * @param [in] phys         Physical memory address to be translated.
 *                          Range: [0, ULONG_MAX]
 *
 * @return Translated IOMMU address if IOMMU is enabled, otherwise the original
 *         physical address.
 */
u64 nvgpu_mem_iommu_translate(struct gk20a *g, u64 phys);

/**
 * @brief Retrieve the physical address from a physical nvgpu_mem structure.
 *
 * This function returns the physical address associated with a physical
 * nvgpu_mem structure. It assumes that the nvgpu_mem structure represents
 * a physically contiguous memory region.
 *
 * This fuction should not be used for normal nvgpumem that holds
 * the sgt of intermediate or iova addresses.
 *
 * The steps performed by the function are as follows:
 * -# Cast the scatter gather list from the physical scatter gather table
 *    of the nvgpu_mem structure to a nvgpu_mem_sgl structure.
 * -# Return the physical address from the nvgpu_mem_sgl structure.
 *
 * @param [in]  g    Pointer to the GPU driver structure. This parameter is not
 *                    used in the function but is part of the function signature
 *                    for consistency.
 * @param [in]  mem  Pointer to the nvgpu_mem structure whose physical address
 *                    is to be retrieved.
 *                    - mem must not be NULL.
 *                    - mem->phys_sgt must not be NULL.
 *                    - mem->phys_sgt->sgl must not be NULL.
 *
 * @return The physical address associated with the nvgpu_mem structure.
 */
u64 nvgpu_mem_phys_get_addr(struct gk20a *g, struct nvgpu_mem *mem);

#endif /* NVGPU_MEM_H */
