/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2016-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_BOARDOBJGRPMASK_H
#define NVGPU_BOARDOBJGRPMASK_H

struct ctrl_boardobjgrp_mask;

/*
* Board Object Group Mask super-structure.
* Used to unify access to all BOARDOBJGRPMASK_E** child classes
*/
struct boardobjgrpmask {
	/* Number of bits supported by the mask */
	u8  bitcount;
	/* Number of 32-bit words required to store all @ref bitCount bits */
	u8  maskdatacount;
	/*
	* Bit-mask of used-bits within last 32-bit word. Used to
	* normalize data
	*/
	u32 lastmaskfilter;
	/*
	* Start of the array of 32-bit words representing the bit-mask
	* Must be the last element of the structure.
	*/
	/* 1 will be replace with CTRL_BOARDOBJGRP_MASK_ARRAY_START_SIZE */
	 u32 data[1];
};

struct boardobjgrpmask_e32 {
	/*
	* BOARDOBJGRPMASK super-class. Must be the first element of the
	* structure.
	*/
	struct boardobjgrpmask super;
	 /*u32   data_e32[1]; */
};

struct boardobjgrpmask_e255 {
	/*
	* BOARDOBJGRPMASK super-class. Must be the first element of the
	* structure.
	*/
	struct boardobjgrpmask super;
	u32   data_e255[254];
};


struct boardobjgrpmask_e2048 {
	/*
	* BOARDOBJGRPMASK super-class. Must be the first element of the
	* structure.
	*/
	struct boardobjgrpmask super;
	u32   data_e2048[2047];
};




/* Init and I/O operations.*/
int nvgpu_boardobjgrpmask_init(struct boardobjgrpmask *mask, u8 bitsize,
		struct ctrl_boardobjgrp_mask *extmask);
int nvgpu_boardobjgrpmask_export(struct boardobjgrpmask *mask, u8 bitsize,
		struct ctrl_boardobjgrp_mask *extmask);

/* Operations on all bits of a single mask.*/
u8 nvgpu_boardobjgrpmask_bit_set_count(struct boardobjgrpmask *mask);
u8 nvgpu_boardobjgrpmask_bit_idx_highest(struct boardobjgrpmask *mask);

/* Operations on a single bit of a single mask */
int nvgpu_boardobjgrpmask_bit_clr(struct boardobjgrpmask *mask, u8 bitidx);
int nvgpu_boardobjgrpmask_bit_set(struct boardobjgrpmask *mask, u8 bitidx);
bool nvgpu_boardobjgrpmask_bit_get(struct boardobjgrpmask *mask, u8 bitidx);

int nvgpu_boardobjmask_or(struct boardobjgrpmask *dst,
		struct boardobjgrpmask *mask1, struct boardobjgrpmask *mask2);
int nvgpu_boardobjmask_and(struct boardobjgrpmask *dst,
		struct boardobjgrpmask *mask1, struct boardobjgrpmask *mask2);
int nvgpu_boardobjgrpmask_copy(struct boardobjgrpmask *dst,
		struct boardobjgrpmask *src);

/* Special interfaces */
bool nvgpu_boardobjgrpmask_sizeeq(struct boardobjgrpmask *op1,
		struct boardobjgrpmask *op2);
/* init boardobjgrpmask_e32 structure */
#define boardobjgrpmask_e32_init(pmaske32, pextmask)                           \
	nvgpu_boardobjgrpmask_init(&(pmaske32)->super,                         \
		CTRL_BOARDOBJGRP_E32_MAX_OBJECTS, (pextmask))

/* init boardobjgrpmask_e255 structure */
#define boardobjgrpmask_e255_init(pmaske255, pextmask)                         \
	nvgpu_boardobjgrpmask_init(&(pmaske255)->super,                        \
		CTRL_BOARDOBJGRP_E255_MAX_OBJECTS, (pextmask))

#define boardobjgrpmask_e2048_init(pmaske255, pextmask)                         \
	nvgpu_boardobjgrpmask_init(&(pmaske255)->super,                        \
		CTRL_BOARDOBJGRP_E255_MAX_OBJECTS, (pextmask))

#endif /* NVGPU_BOARDOBJGRPMASK_H */
