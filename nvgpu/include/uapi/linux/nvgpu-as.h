/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _UAPI__LINUX_NVGPU_AS_H__
#define _UAPI__LINUX_NVGPU_AS_H__

#include "nvgpu-uapi-common.h"

#define NVGPU_AS_IOCTL_MAGIC 'A'

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_ALLOC_SPACE IOCTL: NVGPU_AS_IOCTL_ALLOC_SPACE
 *
 * @brief Reserves a range of addresses for fixed-address mappings. The
 * reserved range will not be used by unrelated mappings.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_alloc_space_args</td>
 *   <td>@c #nvgpu_as_alloc_space_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Input out of range.
 * - @c ENOMEM ---  Range allocation failed.
 * - @c ENOMEM ---  If GMMU map fails in case of @c #NVGPU_AS_ALLOC_SPACE_FLAGS_SPARSE allocation.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @pre When allocating a fixed-address range (@c
 * #NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET), the requested page size (@c
 * #nvgpu_as_alloc_space_args::page_size) must match with the existing
 * allocations and mappings (if any) residing on the shared last level PDEs. See
 * @c #nvgpu_gpu_characteristics::pde_coverage_bit_count.
 *
 * @post Returned range may not overlap with a previous allocation or mapping in
 * the GPU virtual address space.
 *
 * @post Returned range is within the address space limits (see @ref
 * NVGPURM_INTF_ALLOC_AS and @c nvgpu_alloc_as_args::va_range_start /
 * @c nvgpu_alloc_as_args::va_range_end).
 *
 * @note This IOCTL has following caveats:
 * -# Address alignment is silently ignored by NvGPU RM.
 *
 * @remark Uses for range allocations and fixed-address mappings include:
 * -# Matching CPU and GPU virtual addresses for buffer mappings (e.g., CUDA
 *    UVM-lite). To support this, the address range itself is typically
 *    requested at a fixed base address with size covering the whole usable CPU
 *    virtual address range. (See also @c
 *    #NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET.)
 * -# Sparse resources in graphics (e.g., sparse textures). (See also @c
 *    #NVGPU_AS_ALLOC_SPACE_FLAGS_SPARSE.)
 *
 * @sa @ref NVGPURM_INTF_AS_FREE_SPACE
 */

/**
 * @ingroup NVGPURM_INTF_AS_ALLOC_SPACE
 * @{
 */

/**
 * @brief Request allocation at fixed address.
 *
 * When this flag is specified, allocation is requested at fixed address
 * @c #nvgpu_as_alloc_space_args::offset.
 */
#define NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET	NVGPU_BIT32(0)

/**
 * @brief Request sparse allocation.
 *
 * When this flag is specified, GPU accesses to unmapped regions of the
 * allocation are allowed. Any reads from unmapped regions return value 0, and
 * all writes are discarded.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_SPARSE_ALLOCS
 */
#define NVGPU_AS_ALLOC_SPACE_FLAGS_SPARSE	NVGPU_BIT32(1)

/** @} */

/*
 * Allocating an address space range:
 *
 * Address ranges created with this ioctl are reserved for later use with
 * fixed-address buffer mappings.
 *
 * If _FLAGS_FIXED_OFFSET is specified then the new range starts at the 'offset'
 * given.  Otherwise the address returned is chosen to be a multiple of 'align.'
 *
 */

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM32_INTF_AS_ALLOC_SPACE IOCTL: NVGPU32_AS_IOCTL_ALLOC_SPACE
 *
 * This interface is almost same as @ref NVGPURM_INTF_AS_ALLOC_SPACE except for
 * @c #nvgpu32_as_alloc_space_args::pages argument which is unsigned 32 bit instead
 * of unsigned 64 bit. This is required for legacy support.
 *
 * @brief Header struct for @ref NVGPURM32_INTF_AS_ALLOC_SPACE.
 */
struct nvgpu32_as_alloc_space_args {
	/**
	 * @brief Allocation size in pages
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Allocation size</dd>
	 * </dl>
	 *
	 * Range: &ge; 1
	 */
	__u32 pages;

	/**
	 * @brief Page size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Page size</dd>
	 * </dl>
	 *
	 * Range: a valid page size for the address space (see @ref
	 * NVGPURM_INTF_ALLOC_AS)
	 */
	__u32 page_size;

	/**
	 * @brief Range allocation flags
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Bit mask of flags (@c NVGPU_AS_ALLOC_SPACE_FLAGS_)</dd>
	 * </dl>
	 *
	 * Range: Bit mask of @c NVGPU_AS_ALLOC_SPACE_FLAGS_*
	 */
	__u32 flags;

	union {
		/**
		 * @brief Start address of range allocation (GPU virtual address)
		 *
		 * <dl>
		 *   <dt>[in]</dt>
		 *   <dd>When @c NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET is:
		 *       - specified: Requested start address.
		 *       - not specified: inactive union member, do not set.</dd>
		 *   <dt>[out]</dt>
		 *   <dd>Start address of allocation</dd>
		 * </dl>
		 *
		 * Range: GPU virtual address multiple of @c #page_size.
		 *
		 * @remark Despite naming, this is an absolute GPU virtual address.
		 */
		__u64 offset;

		/**
		 * @brief Alignment for allocation
		 *
		 * <dl>
		 *   <dt>[in]</dt>
		 *   <dd>When @c NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET is:
		 *       - specified: inactive union member, do not set.
		 *       - not specified: alignment requirement.</dd>
		 *   <dt>[out]</dt>
		 *   <dd>Inactive union member.</dd>
		 * </dl>
		 *
		 * Range: GPU virtual address multiple of @c #page_size.
		 *
		 * @note This is silently ignored by NvGPU resource manager.
		 */
		__u64 align;
	} o_a;
};

/**
 * @ingroup NVGPURM_INTF_AS_ALLOC_SPACE
 * @brief Header struct for @ref NVGPURM_INTF_AS_ALLOC_SPACE.
 */
struct nvgpu_as_alloc_space_args {

	/**
	 * @brief Allocation size in pages
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Allocation size</dd>
	 * </dl>
	 *
	 * Range: &ge; 1
	 */
	__u64 pages;

	/**
	 * @brief Page size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Page size</dd>
	 * </dl>
	 *
	 * Range: a valid page size for the address space (see @ref
	 * NVGPURM_INTF_ALLOC_AS)
	 */
	__u32 page_size;

	/**
	 * @brief Range allocation flags
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Bit mask of flags (@c NVGPU_AS_ALLOC_SPACE_FLAGS_)</dd>
	 * </dl>
	 *
	 * Range: Bit mask of @c NVGPU_AS_ALLOC_SPACE_FLAGS_*
	 */
	__u32 flags;

	union {
		/**
		 * @brief Start address of range allocation (GPU virtual address)
		 *
		 * <dl>
		 *   <dt>[in]</dt>
		 *   <dd>When @c NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET is:
		 *       - specified: Requested start address.
		 *       - not specified: inactive union member, do not set.</dd>
		 *   <dt>[out]</dt>
		 *   <dd>Start address of allocation</dd>
		 * </dl>
		 *
		 * Range: GPU virtual address multiple of @c #page_size.
		 *
		 * @remark Despite naming, this is an absolute GPU virtual address.
		 */
		__u64 offset;

		/**
		 * @brief Alignment for allocation
		 *
		 * <dl>
		 *   <dt>[in]</dt>
		 *   <dd>When @c NVGPU_AS_ALLOC_SPACE_FLAGS_FIXED_OFFSET is:
		 *       - specified: inactive union member, do not set.
		 *       - not specified: alignment requirement.</dd>
		 *   <dt>[out]</dt>
		 *   <dd>Inactive union member.</dd>
		 * </dl>
		 *
		 * Range: GPU virtual address multiple of @c #page_size.
		 *
		 * @note This is silently ignored by NvGPU resource manager.
		 */
		__u64 align;
	} o_a;

	/**
	 * @brief Padding
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 padding[2];
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_FREE_SPACE IOCTL: NVGPU_AS_IOCTL_FREE_SPACE
 * @brief Frees an address range that was previously allocated with @ref
 * NVGPURM_INTF_AS_ALLOC_SPACE.
 *
 * The previously allocated region starting at @c
 * #nvgpu_as_free_space_args::offset is freed. Any existing mappings in the
 * reserved range will be automatically removed.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_free_space_args</td>
 *   <td>@c #nvgpu_as_free_space_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @note This IOCTL has following caveats:
 * -# No error is returned in case @c #nvgpu_as_free_space_args::offset is not a
 *    valid allocation.
 */

/**
 * @ingroup NVGPURM_INTF_AS_FREE_SPACE
 * @brief Header struct for @ref NVGPURM_INTF_AS_FREE_SPACE.
 */
struct nvgpu_as_free_space_args {
	/**
	 * @brief Start address of an address range allocation previously returned
	 * by @ref NVGPURM_INTF_AS_ALLOC_SPACE.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Start address of address range allocation.</dd>
	 * </dl>
	 *
	 * Range: Start address of an existing range allocation.
	 *
	 * @remark Despite naming, this is an absolute GPU virtual address.
	 */
	__u64 offset;

	/** @brief Unused */
	__u64 pages;

	/** @brief Unused */
	__u32 page_size;

	/**
	 * @brief Padding
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 padding[3];
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_BIND_CHANNEL IOCTL: NVGPU_AS_IOCTL_BIND_CHANNEL
 * @brief Binds a channel to the address space.
 *
 * The address space specifies the data context for the channel. A channel must
 * be bound to an address space before the channel GP FIFO is allocated. See
 * @ref NVGPURM_LINUX_IOCTL_CHANNEL for the channel open sequence.
 *
 * Address space binding has the following restrictions:
 *
 * -# In case multiple channels are bound to a TSG subcontext, they must all be
 *    bound to the same address space as the first channel of the subcontext.
 *
 * -# Similarly, in case multiple channels are bound to a TSG without a
 *    subcontext, they must all be bound to the same address space as the
 *    first channel of the TSG.
 *
 * Once a channel has been bound to an address space, the only way to unbind is
 * to close the channel.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_bind_channel_args</td>
 *   <td>@c #nvgpu_as_bind_channel_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Invalid channel fd
 *                 (@c #nvgpu_as_bind_channel_args::channel_fd)
 * - @c EINVAL ---  Channel already bound to an address space
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from userspace fails.
 */

/**
 * @ingroup NVGPURM_INTF_AS_BIND_CHANNEL
 * @brief Header struct for @ref NVGPURM_INTF_AS_BIND_CHANNEL.
 */
struct nvgpu_as_bind_channel_args {
	/**
	 * @brief Channel fd to bind
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     Channel fd to bind.
	 *   </dd>
	 * </dl>
	 *
	 * Range: Valid channel fd (Returned by ioctl @ref
	 * NVGPURM_INTF_ALLOC_AS).
	 */
	__u32 channel_fd;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_MAP_BUFFER_EX IOCTL: NVGPU_AS_IOCTL_MAP_BUFFER_EX
 * @brief Maps a buffer to the GPU virtual address space.
 *
 * A buffer mapping adds a physical buffer to the GPU virtual address
 * space. This allows the channels bound to the address space to access the
 * buffer using GPU virtual addresses of the mapping.
 *
 * The contents of a GPU virtual address space are defined by the associated
 * page tables. A page table entry defines how a page of virtual addresses is
 * translated to physical addresses. In addition to virtual-to-physical address
 * translation, a page table entry contains attributes such as GPU L2
 * cacheability, the memory kind, and whether the page is read-only or
 * read-write accessible. See @c #nvgpu_as_map_buffer_ex_args for details.
 *
 * By default, this call performs a non-fixed-address mapping. This means that
 * first, the resource manager allocates an address range for the mapping, and
 * then, mapping is added in the allocation. This behavior can be changed by
 * specifying @c #NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET, which uses a
 * user-allocated address range allocation, instead.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_map_buffer_ex_args</td>
 *   <td>@c #nvgpu_as_map_buffer_ex_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Bad import id.
 * - @c EINVAL ---  Buffer parameter query failed.
 * - @c EINVAL ---  Inputs out of range.
 * - @c ENOMEM ---  Memory allocation for page tables failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @pre When @c #NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET is specified in @c
 * #nvgpu_as_map_buffer_ex_args::flags, then:
 * -# [@c #nvgpu_as_map_buffer_ex_args::offset,
 *    (@c #nvgpu_as_map_buffer_ex_args::offset +
 *    @c #nvgpu_as_map_buffer_ex_args::mapping_size - @c 1)] must be within an
 *    existing address range allocation. (See @ref NVGPURM_INTF_AS_ALLOC_SPACE.)
 * -# [@c #nvgpu_as_map_buffer_ex_args::offset,
 *    (@c #nvgpu_as_map_buffer_ex_args::offset +
 *    @c #nvgpu_as_map_buffer_ex_args::mapping_size - @c 1)] must not overlap
 *    with an existing mapping.
 * -# @c #nvgpu_as_map_buffer_ex_args::page_size must equal to the page size of
 *    the existing address range allocation. (See @ref
 *    NVGPURM_INTF_AS_ALLOC_SPACE.)
 *
 * @pre When compressible-kind mapping is required, compression tags must have
 * been previously allocated for the buffer. Notes:
 * - Compression is required by setting @c #nvgpu_as_map_buffer_ex_args::compr_kind &ne;
 *   @c #NV_KIND_INVALID AND @c #nvgpu_as_map_buffer_ex_args::incompr_kind = @c
 *   #NV_KIND_INVALID.
 * - Compression tags are allocated by a call to
 *   @ref NVGPURM_INTF_REGISTER_BUFFER.
 */

/**
 * @ingroup NVGPURM_INTF_AS_MAP_BUFFER_EX
 * @{
 */

/** @brief Fixed-address mapping. */
#define NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET		NVGPU_BIT32(0)

/**
 * @brief Enable GPU L2 caching for the mapping.
 *
 * When not specified, GPU L2-caching is disabled for the mapping.
 *
 * @remark GPU L2 caching should be enabled when GPU compression is used. See @c
 * #nvgpu_as_map_buffer_ex_args::compr_kind.
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_CACHEABLE		NVGPU_BIT32(2)

/**
 * @brief Request unmapped PTEs. **Not for production use!**
 *
 * @remark Not supported in safety.
 *
 * @sa @c #NVGPU_ALLOC_GPFIFO_EX_FLAGS_REPLAYABLE_FAULTS_ENABLE
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_UNMAPPED_PTE		NVGPU_BIT32(5)

/** @brief Request mappable compbits (unsupported) */
#define NVGPU_AS_MAP_BUFFER_FLAGS_MAPPABLE_COMPBITS	NVGPU_BIT32(6)

/**
 * @brief Specifies that the memory accesses may be cached by the SoC last-level
 * cache.
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_L3_ALLOC		NVGPU_BIT32(7)

/**
 * @brief Specifies that the SYSTEM COHERENT aperture is used for buffers in
 * system memory. Default is SYSTEM non-coherent.
 *
 * In Tegra, the characteristics of the apertures are SoC-specific:
 *
 * - On T194/T234, both apertures are I/O-coherent. SYSTEM coherent enables
 *   platform atomics.
 *
 * - On T26x, both apertures are I/O-coherent. SYSTEM coherent enables platform
 *   atomics and full coherency.
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_SYSTEM_COHERENT	NVGPU_BIT32(9)

#define NVGPU_AS_MAP_BUFFER_FLAGS_TEGRA_RAW		NVGPU_BIT32(12)

/**
 * @brief Map access field bit offset
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_OFFSET 10U

/**
 * @brief Map access field bit width
 */
#define NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_SIZE   2U

/**
 * @brief Specifies default access.
 *
 * Default access is determined by the buffer to be mapped. In case the buffer
 * handle specifies read-only access, then the mapping uses read-only
 * access. Similarly, for a read-write handle, read-write mapping is used.
 *
 * @sa @c #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_OFFSET, @c
 * #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_SIZE
 */
#define NVGPU_AS_MAP_BUFFER_ACCESS_DEFAULT              0U

/**
 * @brief Specifies read-only access.
 *
 * Read-only access is used regardless of the buffer handle access.
 *
 * @sa @c #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_OFFSET, @c
 * #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_SIZE
 */
#define NVGPU_AS_MAP_BUFFER_ACCESS_READ_ONLY            1U

/**
 * @brief Specifies read-write access.
 *
 * Read-write access is required. In case the buffer handle specifies read-only
 * access, the map call will fail.
 *
 * @sa @c #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_OFFSET, @c
 * #NVGPU_AS_MAP_BUFFER_FLAGS_ACCESS_BITMASK_SIZE
 */
#define NVGPU_AS_MAP_BUFFER_ACCESS_READ_WRITE           2U


/**
 * @brief Invalid kind number
 *
 * @sa @c #nvgpu_as_map_buffer_ex_args::compr_kind
 * @sa @c #nvgpu_as_map_buffer_ex_args::incompr_kind
 */
#define NV_KIND_INVALID	-1


/** @}*/

/**
 * @ingroup NVGPURM_INTF_AS_MAP_BUFFER_EX
 * @brief Header struct for @ref NVGPURM_INTF_AS_MAP_BUFFER_EX.
 */
struct nvgpu_as_map_buffer_ex_args {
	/**
	 * @brief Mapping flags
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Bit mask of mapping flags (@c NVGPU_AS_MAP_BUFFER_FLAGS_*)</dd>
	 * </dl>
	 *
	 * Range: Bit mask of mapping flags (@c NVGPU_AS_MAP_BUFFER_FLAGS_*). Note:
	 * NVGPU_AS_MAP_BUFFER_ACCESS_* values are specified in a bit field.
	 */
	__u32 flags;

	/**
	 * @brief Kind number for compressible mapping.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Compressible kind number for the mapping or @c #NV_KIND_INVALID if
	 *     compression is not required.</dd>
	 *   <dt>[out]</dt>
	 *   <dd>When the value is:
	 *       - @c #NV_KIND_INVALID: compression tags are not assigned.
	 *       - Otherwise, compression tags are assigned for the mapping. The map
	 *         kind number used is @c #compr_kind [in] = @c #compr_kind [out].
	 *   </dd>
	 * </dl>
	 *
	 * Range: HW-specific kind number or @c #NV_KIND_INVALID
	 */
	__s16 compr_kind;

	/**
	 * @brief Incompressible kind number for the mapping.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Incompressible kind number for the mapping or @c #NV_KIND_INVALID if
	 *     incompressible mapping is not allowed.</dd>
	 * </dl>
	 *
	 * Range: HW-specific kind number or @c #NV_KIND_INVALID
	 *
	 * @remark When @c #compr_kind &ne; @c #NV_KIND_INVALID, then this field specifies
	 * the incompressible fallback kind number in case compression tags are not
	 * available. If incompressible fallback is not allowed, this field is set
	 * to @c #NV_KIND_INVALID.
	 *
	 * @remark Specifying a compressible kind number in this field will result
	 * in corruption.
	 */
	__s16 incompr_kind;

	/**
	 * @brief dma-buf file descriptor of the buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     dma-buf fd.
	 *   </dd>
	 * </dl>
	 */
	__u32 dmabuf_fd;

	/**
	 * @brief Page size for the mapping
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Page size for the mapping.</dd>
	 * </dl>
	 *
	 * Range: In case @c #NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET is
	 * -# specified: page size must match with the underlying address range
	 *    allocation (see @c #nvgpu_as_alloc_space_args::page_size).
	 * -# not specified: page size must be a valid page size for the address
	 *    space (see @ref NVGPURM_INTF_ALLOC_AS)
	 *
	 * @remark There may be a HW-specific minimum page size for the used
	 * kind. In particular, many dGPUs require that compressible memory kinds
	 * use big pages.
	 */
	__u32 page_size;

	/**
	 * @brief Buffer offset (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Starting offset of the buffer to be mapped</dd>
	 * </dl>
	 *
	 * Range: [0, (@c Buffer_Size - @c #mapping_size)]. Must be a multiple of @c
	 * #page_size.
	 */
	__u64 buffer_offset;

	/**
	 * @brief Mapping size (in bytes)
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Number of bytes to be mapped</dd>
	 * </dl>
	 *
	 * Range: [4096, (@c Buffer_Size - @c #buffer_offset)]. Must be a multiple
	 * of @c #page_size.
	 */
	__u64 mapping_size;

	/**
	 * @brief GPU virtual address of the mapping.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>When @c #NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET is specified, GPU virtual
	 *     address of the mapping. Otherwise, ignored.
	 *   </dd>
	 *   <dt>[out]</dt>
	 *   <dd>GPU virtual address of the mapping.</dd>
	 * </dl>
	 *
	 * Range: When @c #NVGPU_AS_MAP_BUFFER_FLAGS_FIXED_OFFSET is specified, see
	 * preconditions of @ref NVGPURM_INTF_ALLOC_AS.
	 */
	__u64 offset;
};

/*
 * Get info about buffer compbits. Requires that buffer is mapped with
 * NVGPU_AS_MAP_BUFFER_FLAGS_MAPPABLE_COMPBITS.
 *
 * The compbits for a mappable buffer are organized in a mappable
 * window to the compbits store. In case the window contains comptags
 * for more than one buffer, the buffer comptag line index may differ
 * from the window comptag line index.
 */
struct nvgpu_as_get_buffer_compbits_info_args {

	/* in: address of an existing buffer mapping */
	__u64 mapping_gva;

	/* out: size of compbits mapping window (bytes) */
	__u64 compbits_win_size;

	/* out: comptag line index of the window start */
	__u32 compbits_win_ctagline;

	/* out: comptag line index of the buffer mapping */
	__u32 mapping_ctagline;

/* Buffer uses compbits */
#define NVGPU_AS_GET_BUFFER_COMPBITS_INFO_FLAGS_HAS_COMPBITS    (1 << 0)

/* Buffer compbits are mappable */
#define NVGPU_AS_GET_BUFFER_COMPBITS_INFO_FLAGS_MAPPABLE        (1 << 1)

/* Buffer IOVA addresses are discontiguous */
#define NVGPU_AS_GET_BUFFER_COMPBITS_INFO_FLAGS_DISCONTIG_IOVA  (1 << 2)

	/* out */
	__u32 flags;

	__u32 reserved1;
};

/*
 * Map compbits of a mapped buffer to the GPU address space. The
 * compbits mapping is automatically unmapped when the buffer is
 * unmapped.
 *
 * The compbits mapping always uses small pages, it is read-only, and
 * is GPU cacheable. The mapping is a window to the compbits
 * store. The window may not be exactly the size of the cache lines
 * for the buffer mapping.
 */
struct nvgpu_as_map_buffer_compbits_args {

	/* in: address of an existing buffer mapping */
	__u64 mapping_gva;

	/* in: gva to the mapped compbits store window when
	 * FIXED_OFFSET is set. Otherwise, ignored and should be be 0.
	 *
	 * For FIXED_OFFSET mapping:
	 * - If compbits are already mapped compbits_win_gva
	 *   must match with the previously mapped gva.
	 * - The user must have allocated enough GVA space for the
	 *   mapping window (see compbits_win_size in
	 *   nvgpu_as_get_buffer_compbits_info_args)
	 *
	 * out: gva to the mapped compbits store window */
	__u64 compbits_win_gva;

	/* in: reserved, must be 0
	   out: physical or IOMMU address for mapping */
	union {
		/* contiguous iova addresses */
		__u64 mapping_iova;

		/* buffer to receive discontiguous iova addresses (reserved) */
		__u64 mapping_iova_buf_addr;
	};

	/* in: Buffer size (in bytes) for discontiguous iova
	 * addresses. Reserved, must be 0. */
	__u64 mapping_iova_buf_size;

#define NVGPU_AS_MAP_BUFFER_COMPBITS_FLAGS_FIXED_OFFSET        (1 << 0)
	__u32 flags;
	__u32 reserved1;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_UNMAP_BUFFER IOCTL: NVGPU_AS_IOCTL_UNMAP_BUFFER
 * @brief Removes a buffer mapping. In case of non-fixed-address mapping,
 * removes also the underlying address range allocation.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_unmap_buffer_args</td>
 *   <td>@c #nvgpu_as_unmap_buffer_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c ENODEV ---  GPU not ready
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @note This IOCTL has following caveats:
 * -# No error is returned in case @c #nvgpu_as_unmap_buffer_args::offset is not a
 *    valid mapping.
 */

/**
 * This struct is associated with IOCTL command #NVGPU_AS_IOCTL_UNMAP_BUFFER
 * that unmaps a mapped buffer.
 * @ingroup NVGPURM_INTF_AS_UNMAP_BUFFER
 */
struct nvgpu_as_unmap_buffer_args {

	/**
	 * @brief GPU virtual address of a mapping.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Address of the mapping to remove.</dd>
	 * </dl>
	 *
	 * Range: Address of a mapped buffer as returned by a previous call to @ref
	 * NVGPURM_INTF_AS_MAP_BUFFER_EX.
	 */
	__u64 offset;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_GET_VA_REGIONS IOCTL: NVGPU_AS_IOCTL_GET_VA_REGIONS
 * @brief Queries the user-accessible GPU VA regions.
 *
 * The user-accessible GPU VA regions are usable by @ref
 * NVGPURM_INTF_AS_MAP_BUFFER_EX, @ref NVGPURM_INTF_AS_ALLOC_SPACE. For unified
 * address spaces, the unified region is reported once per every supported map
 * page size.
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>REQUEST</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_get_va_regions_args</td>
 *   <td>@c #nvgpu_as_get_va_regions_args</td>
 * </tr>
 * <tr>
 *   <td>Additional buffer 1(@c #nvgpu_as_get_va_regions_args::buf_addr)</td>
 *   <td>(none)</td>
 *   <td>@c #nvgpu_as_va_region[N]. See
 *       @c #nvgpu_as_get_va_regions_args::buf_size.</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EIO ---     Writing VA region info to output buffer failed.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @remark This IOCTL may be removed from safety in DRIVE 6.5.
 *
 * @remark Currently, the number of reported regions is:
 * - 1, when only small pages are supported
 * - 2, when both small and big pages are supported
 * - 3, when all small, big and huge pages are supported
 */

/**
 * @ingroup NVGPURM_INTF_AS_GET_VA_REGIONS
 * @brief Region descriptor
 */
struct nvgpu_as_va_region {

	/**
	 * @brief Starting address of the region (in bytes, page-aligned).
	 */
	__u64 offset;

	/**
	 * @brief Page size of the region.
	 *
	 * @remark When the region supports multiple page sizes, it is reported once
	 * per page size.
	 */
	__u32 page_size;

	/**
	 * @brief Reserved, ignore.
	 */
	__u32 reserved;

	/**
	 * @brief Size of the region in pages.
	 */
	__u64 pages;
};

/**
 * @ingroup NVGPURM_INTF_AS_GET_VA_REGIONS
 * @brief Header struct for @ref NVGPURM_INTF_AS_GET_VA_REGIONS.
 */
struct nvgpu_as_get_va_regions_args {

	/**
	 * @brief address of va region buffer
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>
	 *     The buffer size must be sufficient to hold array of va region entries.
	 *   </dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u64 buf_addr;

	/**
	 * @brief Region descriptor buffer size in bytes.
	 *
	 * <dl>
	 *   <dt>[in]</dt>
	 *   <dd>Size of the receive buffer allocated by the caller.</dd>
	 *   <dt>[out]</dt>
	 *   <dd>Size of the kernel buffer. Number of descriptors may be computed as
	 *       follows:
	 * @code
	 * num_descriptors = buf_size / sizeof(nvgpu_as_va_region)
	 * @endcode
	 *   </dd>
	 * </dl>
	 *
	 */
	__u32 buf_size;

	/**
	 * @brief Reserved
	 *
	 * <dl>
	 *   <dt>[in]</dt><dd>Zeroed</dd>
	 *   <dt>[out]</dt><dd>Ignore</dd>
	 * </dl>
	 */
	__u32 reserved;
};
struct nvgpu_as_map_buffer_batch_args {
	__u64 unmaps; /* ptr to array of nvgpu_as_unmap_buffer_args */
	__u64 maps;   /* ptr to array of nvgpu_as_map_buffer_ex_args */
	__u32 num_unmaps; /* in: number of unmaps
			   * out: on error, number of successful unmaps */
	__u32 num_maps;   /* in: number of maps
			   * out: on error, number of successful maps */
	__u64 reserved;
};

/**
 * @ingroup NVGPURM_LINUX_IOCTL_AS
 * @defgroup NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP IOCTL: NVGPU_AS_IOCTL_GET_SYNC_RO_MAP
 * @brief Returns the GPU virtual address to the syncpoint-semaphore shim
 * (read-only access). The shim is mapped to the GPU virtual address space if
 * necessary.
 *
 * The syncpoint-semaphore RO shim can be used to read syncpoint values using
 * regular memory reads. The typical use case is waiting for the syncpoint using
 * the GPU semaphore acquire methods.
 *
 * The GPU mapping for the syncpoint RO shim has the following properties:
 * - Memory kind is pitch linear (&le; Volta) or generic incompressible memory
 *   kind (Turing+)
 * - GPU L2 caching disabled (volatile)
 *
 * GPU virtual address of a syncpoint is computed as follows:
 * @code
 * gpu_va = nvgpu_as_get_sync_ro_map_args::base_gpuva +
 *          (nvgpu_as_get_sync_ro_map_args::sync_size * syncpoint_id)
 * @endcode
 *
 * <table><caption>IOCTL message structure</caption>
 * <tr>
 *   <th>Header struct or additional buffer</th><th>Usermode to RM</th><th>RESPONSE</th>
 * </tr>
 * <tr>
 *   <td>Header struct</td>
 *   <td>@c #nvgpu_as_get_sync_ro_map_args</td>
 *   <td>@c #nvgpu_as_get_sync_ro_map_args</td>
 * </tr>
 * </table>
 *
 * @return Call status
 * @retval 0        Call succeeded
 * @retval -1       Call failed (errno is set)
 *
 * Errors:
 * - @c EINVAL ---  Syncpoint RO shim not supported.
 * - @c ENOMEM ---  Failed to map the shim to GPU virtual address space.
 * - @c ENODEV ---  GPU not ready.
 * - @c EFAULT ---  Copy from userspace fails.
 *
 * @pre @c #NVGPU_GPU_FLAGS_SUPPORT_SYNCPOINT_ADDRESS
 */

/**
 * @ingroup NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP
 * @brief Header struct for @ref NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP.
 */
struct nvgpu_as_get_sync_ro_map_args {

	/**
	 * @brief Base address of the syncpoint-semaphore RO shim
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Syncpoint-semaphore RO shim base address</dd>
	 * </dl>
	 *
	 * Range: Kernel-private region (see @ref NVGPURM_INTF_ALLOC_AS).
	 */
	__u64 base_gpuva;

	/**
	 * @brief Syncpoint stride (in bytes).
	 *
	 * Syncpoint stride specifies the distance between adjacent syncpoints.
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Syncpoint stride</dd>
	 * </dl>
	 *
	 * Range: HW-specific
	 * - On T194: 4096
	 * - On T234: 65536
	 * - On T264: 65536
	 */
	__u32 sync_size;

	/**
	 * @brief Number of syncpoints in the shim.
	 *
	 * <dl>
	 *   <dt>[out]</dt>
	 *   <dd>Number of syncpoints</dd>
	 * </dl>
	 *
	 * Range: Platform-specific
	 */
	__u32 num_syncpoints;
};
/*
 * Macro defining the maximum number of syncpoint regions that are
 * reported.
 */
#define NVGPU_SYNC_RO_MAP_MAX_SYNCPOINT_REGIONS 4

/*
 * Structure describing a single syncpoint region.
 */
struct nvgpu_as_syncpoint_region_description {
	__u64 base_gpuva;
	__u32 sync_size;
	__u32 num_syncpoints;
};

/*
 * Structure containing an array of syncpoint regions.
 *
 * Each reported syncpoint region is indexable by the
 * Host1x instance ID.
 *
 * @num_syncpoint_regions number of reported syncpoint regions
 *
 * @regions array of syncpoint regions
 */
struct nvgpu_as_get_sync_ro_map_v2_args {
	__u32 num_syncpoint_regions; /* out */
	struct nvgpu_as_syncpoint_region_description regions[NVGPU_SYNC_RO_MAP_MAX_SYNCPOINT_REGIONS]; /* out */
};

/*
 * VM mapping modify IOCTL
 *
 * This ioctl changes the kind of an existing mapped buffer region.
 *
 * Usage of this API is as follows.
 *
 * @compr_kind  [IN]
 *
 *   Specify the new compressed kind to be used for the mapping.  This
 *   parameter is only valid if compression resources are allocated to the
 *   underlying physical buffer. If NV_KIND_INVALID is specified then the
 *   fallback incompr_kind parameter is used.
 *
 * @incompr_kind  [IN]
 *
 *   Specify the new kind to be used for the mapping if compression is not
 *   to be used.  If NV_KIND_INVALID is specified then incompressible fallback
 *   is not allowed.
 *
 * @buffer_offset  [IN]
 *
 *   Specifies the beginning offset of the region within the existing buffer
 *   for which the kind should be modified.  This field is in bytes.
 *
 * @buffer_size  [IN]
 *
 *   Specifies the size of the region within the existing buffer for which the
 *   kind should be updated.  This field is in bytes.  Note that the region
 *   described by <buffer_offset, buffer_offset + buffer_size> must reside
 *   entirely within the existing buffer.
 *
 * @map_address  [IN]
 *
 *   The address of the existing buffer in the GPU virtual address space
 *   specified in bytes.
 */
struct nvgpu_as_mapping_modify_args {
	__s16 compr_kind;       /* in */
	__s16 incompr_kind;     /* in */

	__u64 buffer_offset;	/* in, offset of mapped buffer region */
	__u64 buffer_size;	/* in, size of mapped buffer region */

	__u64 map_address;	/* in, base virtual address of mapped buffer */
};

/*
 * VM remap operation.
 *
 * The VM remap operation structure represents a single map or unmap operation
 * to be executed by the NVGPU_AS_IOCTL_REMAP ioctl.
 *
 * The format of the structure is as follows:
 *
 * @flags [IN]
 *
 *   The following remap operation flags are supported:
 *
 *     %NVGPU_AS_REMAP_OP_FLAGS_CACHEABLE
 *
 *       Specify that the associated mapping shall be GPU cachable.
 *
 *     %NVGPU_AS_REMAP_OP_FLAGS_ACCESS_NO_WRITE
 *
 *       Specify that the associated mapping shall be read-only.  This flag
 *       must be set if the physical memory buffer represented by @mem_handle
 *       is mapped read-only.
 *
 *     %NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_4K
 *     %NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_64K
 *     %NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_128K
 *     %NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_2048K
 *
 *       One, and only one, of these flags must be set for both map/unmap
 *       ops and indicates the assumed page size of the mem_offset_in_pages
 *       and virt_offset_in_pages. This value is also verified against the
 *       page size of the address space.
 *
 * @compr_kind  [IN/OUT]
 * @incompr_kind  [IN/OUT]
 *
 *   On input these fields specify the compressible and incompressible kinds
 *   to be used for the mapping.  If @compr_kind is not set to NV_KIND_INVALID
 *   then nvgpu will attempt to allocate compression resources.  If
 *   @compr_kind is set to NV_KIND_INVALID or there are no compression
 *   resources then nvgpu will attempt to use @incompr_kind.  If both
 *   @compr_kind and @incompr_kind are set to NV_KIND_INVALID then -EINVAL is
 *   returned.  These fields must be set to NV_KIND_INVALID for unmap
 *   operations.  On output these fields return the selected kind.  If
 *   @compr_kind is set to a valid compressible kind but the required
 *   compression resources are not available then @compr_kind will return
 *   NV_INVALID_KIND and the @incompr_kind value will be used for the mapping.
 *
 * @mem_handle [IN]
 *
 *   Specify the memory handle (dmabuf_fd) associated with the physical
 *   memory buffer to be mapped.  This field must be zero for unmap
 *   operations.
 *
 * @mem_offset_in_pages [IN]
 *
 *   Specify an offset (in pages) into the physical buffer associated with
 *   mem_handle at which to start the mapping.  This value must be zero for
 *   unmap operations.
 *
 * @virt_offset_in_pages [IN]
 *
 *   Specify the virtual memory start offset (in pages) of the region to map
 *   or unmap.
 *
 * @num_pages [IN]
 *   Specify the number of pages to map or unmap.
 */
struct nvgpu_as_remap_op {
#define NVGPU_AS_REMAP_OP_FLAGS_CACHEABLE               (1 << 2)
#define NVGPU_AS_REMAP_OP_FLAGS_ACCESS_NO_WRITE         (1 << 10)
#define NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_4K             (1 << 15)
#define NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_64K            (1 << 16)
#define NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_128K           (1 << 17)
#define NVGPU_AS_REMAP_OP_FLAGS_PAGESIZE_2048K          (1 << 18)

	/* in: For map and unmap (one and only one) of the _PAGESIZE_ flags is
	 * required to interpret the mem_offset_in_pages and virt_offset_in_pages
	 * correctly. The other flags are used only with map operations. */
	__u32 flags;

	/* in: For map operations, this field specifies the desired
	 * compressible kind.  For unmap operations this field must be set
	 * to NV_KIND_INVALID.
	 * out: For map operations this field returns the actual kind used
	 * for the mapping.  This can be useful for detecting if a compressed
	 * mapping request was forced to use the fallback incompressible kind
	 * value because sufficient compression resources are not available. */
	__s16 compr_kind;

	/* in: For map operations, this field specifies the desired
	 * incompressible kind.  This value will be used as the fallback kind
	 * if a valid compressible kind value was specified in the compr_kind
	 * field but sufficient compression resources are not available.  For
	 * unmap operations this field must be set to NV_KIND_INVALID. */
	__s16 incompr_kind;

	/* in: For map operations, this field specifies the handle (dmabuf_fd)
	 * for the physical memory buffer to map into the specified virtual
	 * address range.  For unmap operations, this field must be set to
	 * zero. */
	__u32 mem_handle;

	/* This field is reserved for padding purposes. */
	__s32 reserved;

	/* in: For map operations this field specifies the offset (in pages)
	 * into the physical memory buffer associated with mem_handle from
	 * from which physical page information should be collected for
	 * the mapping.  For unmap operations this field must be zero. */
	__u64 mem_offset_in_pages;

	/* in: For both map and unmap operations this field specifies the
	 * virtual address space start offset in pages for the operation. */
	__u64 virt_offset_in_pages;

	/* in: For both map and unmap operations this field specifies the
	 * number of pages to map or unmap. */
	__u64 num_pages;
};

/*
 * VM remap IOCTL
 *
 * This ioctl can be used to issue multiple map and/or unmap operations in
 * a single request.  VM remap operations are only valid on address spaces
 * that have been allocated with NVGPU_AS_ALLOC_SPACE_FLAGS_SPARSE.
 * Validation of remap operations is performed before any changes are made
 * to the associated sparse address space so either all map and/or unmap
 * operations are performed or none of them are.
 */
struct nvgpu_as_remap_args {
	/* in: This field specifies a pointer into the caller's address space
	 * containing an array of one or more nvgpu_as_remap_op structures. */
	__u64 ops;

	/* in/out: On input this field specifies the number of operations in
	 * the ops array.  On output this field returns the successful
	 * number of remap operations. */
	__u32 num_ops;
};

/**
 * @ingroup NVGPURM_INTF_AS_BIND_CHANNEL
 * @brief Command code for @ref NVGPURM_INTF_AS_BIND_CHANNEL
 */
#define NVGPU_AS_IOCTL_BIND_CHANNEL \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 1, struct nvgpu_as_bind_channel_args)
/**
 * @ingroup NVGPURM32_INTF_AS_ALLOC_SPACE
 * @brief Command code for @ref NVGPURM32_INTF_AS_ALLOC_SPACE
 */
#define NVGPU32_AS_IOCTL_ALLOC_SPACE \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 2, struct nvgpu32_as_alloc_space_args)
/**
 * @ingroup NVGPURM_INTF_AS_FREE_SPACE
 * @brief Command code for @ref NVGPURM_INTF_AS_FREE_SPACE
 */
#define NVGPU_AS_IOCTL_FREE_SPACE \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 3, struct nvgpu_as_free_space_args)
/**
 * @ingroup NVGPURM_INTF_AS_UNMAP_BUFFER
 * @brief Command code for @ref NVGPURM_INTF_AS_UNMAP_BUFFER.
 */
#define NVGPU_AS_IOCTL_UNMAP_BUFFER \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 5, struct nvgpu_as_unmap_buffer_args)
/**
 * @ingroup NVGPURM_INTF_AS_ALLOC_SPACE
 * @brief Command code for @ref NVGPURM_INTF_AS_ALLOC_SPACE
 */
#define NVGPU_AS_IOCTL_ALLOC_SPACE \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 6, struct nvgpu_as_alloc_space_args)
/**
 * @ingroup NVGPURM_INTF_AS_MAP_BUFFER_EX
 * @brief Command code for @ref NVGPURM_INTF_AS_MAP_BUFFER_EX
 */
#define NVGPU_AS_IOCTL_MAP_BUFFER_EX \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 7, struct nvgpu_as_map_buffer_ex_args)
/**
 * @ingroup NVGPURM_INTF_AS_GET_VA_REGIONS
 * @brief Command code for @ref NVGPURM_INTF_AS_GET_VA_REGIONS.
 */
#define NVGPU_AS_IOCTL_GET_VA_REGIONS \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 8, struct nvgpu_as_get_va_regions_args)
#define NVGPU_AS_IOCTL_GET_BUFFER_COMPBITS_INFO \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 9, struct nvgpu_as_get_buffer_compbits_info_args)
#define NVGPU_AS_IOCTL_MAP_BUFFER_COMPBITS \
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 10, struct nvgpu_as_map_buffer_compbits_args)
#define NVGPU_AS_IOCTL_MAP_BUFFER_BATCH	\
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 11, struct nvgpu_as_map_buffer_batch_args)
/**
 * @ingroup NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP
 * @brief Command code for @ref NVGPURM_INTF_AS_GET_SYNCPT_RO_MAP
 */
#define NVGPU_AS_IOCTL_GET_SYNC_RO_MAP	\
	_IOR(NVGPU_AS_IOCTL_MAGIC,  12, struct nvgpu_as_get_sync_ro_map_args)
#define NVGPU_AS_IOCTL_MAPPING_MODIFY	\
	_IOWR(NVGPU_AS_IOCTL_MAGIC,  13, struct nvgpu_as_mapping_modify_args)
#define NVGPU_AS_IOCTL_REMAP		\
	_IOWR(NVGPU_AS_IOCTL_MAGIC, 14, struct nvgpu_as_remap_args)
#define NVGPU_AS_IOCTL_GET_SYNC_RO_MAP_V2	\
	_IOR(NVGPU_AS_IOCTL_MAGIC,  15, struct nvgpu_as_get_sync_ro_map_v2_args)

#define NVGPU_AS_IOCTL_LAST		\
	_IOC_NR(NVGPU_AS_IOCTL_GET_SYNC_RO_MAP_V2)
#define NVGPU_AS_IOCTL_MAX_ARG_SIZE	\
	sizeof(struct nvgpu_as_get_sync_ro_map_v2_args)

#endif /* #define _UAPI__LINUX_NVGPU_AS_H__ */
