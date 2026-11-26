/* SPDX-License-Identifier: GPL-2.0-only
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef _UAPI__LINUX_NVGPU_UAPI_COMMON_H

#include <linux/ioctl.h>
#include <linux/types.h>

#if !defined(__KERNEL__)
#define __user
#define nvgpu_user

/* Some userspace builds have __packed defined already */
#if !defined(__packed)
#define __packed __attribute__((packed))
#endif /* __packed */

#endif /* __KERNEL__ */

/**
 * @ingroup NVGPURM_LINUX_IOCTL_COMMON
 * @{
 */

/*
 * Note: These NVGPU_U*()/NVGPU_S*() macros are only meant for casting integer
 * literals. They are not meant for casting variables because casting variables
 * without any checks can cause CERT C violations.
 */

/**
 * @brief Typecasts a value to the 8-bit unsigned nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__u8
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_U8(x)		((__u8)(x))

/**
 * @brief Typecasts a value to the 16-bit unsigned nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__u16
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_U16(x)		((__u16)(x))

/**
 * @brief Typecasts a value to the 32-bit unsigned nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__u32
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_U32(x)		((__u32)(x))

/**
 * @brief Typecasts a value to the 64-bit unsigned nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__u64
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_U64(x)		((__u64)(x))

/**
 * @brief Typecasts a value to the 8-bit signed nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__s8
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_S8(x)		((__s8)(x))

/**
 * @brief Typecasts a value to the 16-bit signed nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__s16
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_S16(x)		((__s16)(x))

/**
 * @brief Typecasts a value to the 32-bit signed nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__s32
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_S32(x)		((__s32)(x))

/**
 * @brief Typecasts a value to the 64-bit signed nvgpu type.
 *
 * @param[in] x  Integer value (literal)
 * @return       Integer cast to @c #__s64
 *
 * @remark This macro is intended to be used only to cast literal constants for
 * precise constant value typing.
 */
#define NVGPU_S64(x)		((__s64)(x))

/**
 * @brief Returns a 8-bit value with @c i<sup>th</sup> bit set.
 *
 * @param[in] i  Bit number. Range: [0, 7] (unchecked)
 * @return       Value with @c i<sup>th</sup> bit set.
 */
#define NVGPU_BIT8(i)		(NVGPU_U8(1) << NVGPU_U8(i))

/**
 * @brief Returns a 16-bit value with @c i<sup>th</sup> bit set.
 *
 * @param[in] i  Bit number. Range: [0, 15] (unchecked)
 * @return       Value with @c i<sup>th</sup> bit set.
 */
#define NVGPU_BIT16(i)		(NVGPU_U16(1) << NVGPU_U16(i))

/**
 * @brief Returns a 32-bit value with @c i<sup>th</sup> bit set.
 *
 * @param[in] i  Bit number. Range: [0, 31] (unchecked)
 * @return       Value with @c i<sup>th</sup> bit set.
 */
#define NVGPU_BIT32(i)		(NVGPU_U32(1) << NVGPU_U32(i))

/**
 * @brief Returns a 64-bit value with @c i<sup>th</sup> bit set.
 *
 * @param[in] i  Bit number. Range: [0, 63] (unchecked)
 * @return       Value with @c i<sup>th</sup> bit set.
 */
#define NVGPU_BIT64(i)		(NVGPU_U64(1) << NVGPU_U64(i))

/** @} */

#endif
