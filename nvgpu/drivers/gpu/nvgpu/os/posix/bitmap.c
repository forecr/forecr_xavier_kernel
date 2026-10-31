// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <stdio.h>
#include <stdlib.h>
#include <nvgpu/cov_whitelist.h>

NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 1_4), "TID-1591")
#include <stdatomic.h>
NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 1_4))

#include <nvgpu/posix/utils.h>
#include <nvgpu/posix/bitops.h>
#include <nvgpu/posix/atomic.h>

/**
 * @brief Generate a bitmask with a single bit set.
 *
 * This function calculates a bitmask with only one bit set, based on the bit position
 * specified by the input parameter. The bit position is taken modulo the number of bits
 * in an unsigned long to ensure it is within the valid range. The resulting bitmask has
 * the bit at the specified position set to 1, and all other bits set to 0.
 *
 * The steps performed by the function are as follows:
 * -# Convert the input bit position to an equivalent position within the range of an
 *    unsigned long by taking the modulo with the constant BITS_PER_LONG.
 * -# Shift the value 1 left by the number of positions obtained in the previous step
 *    to create the bitmask.
 *
 * @param [in] bit The bit position for which to create the bitmask. Range: [0, BITS_PER_LONG-1]
 *
 * @return The bitmask with the specified bit set.
 */
static inline unsigned long get_mask(unsigned int bit)
{
	unsigned long lbit = bit;
	lbit %= BITS_PER_LONG;
	return (1UL << lbit);
}

/**
 * @brief Calculate the index in an array of unsigned longs corresponding to a bit position.
 *
 * This function computes the index in an array of unsigned longs that would contain
 * the specified bit position. It divides the bit position by the number of bits in
 * an unsigned long, effectively mapping the bit position to the index of the array
 * where that bit would be stored.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the number of bits in an unsigned long and store it in a variable.
 * -# Divide the input bit position by the number of bits in an unsigned long to
 *    find the index in the array of unsigned longs.
 * -# Cast the result of the division to an unsigned int using the function
 *    'nvgpu_safe_cast_u64_to_u32()'.
 * -# Return the calculated index.
 *
 * @param [in] bit The bit position for which to calculate the index.
 *                 Range: [0, UINT_MAX]
 *
 * @return The index in an array of unsigned longs where the specified bit position
 *         would be located.
 */
static inline unsigned int get_index(unsigned int bit)
{
	unsigned long bpl = BITS_PER_LONG;
	return (bit / nvgpu_safe_cast_u64_to_u32(bpl));
}

unsigned long nvgpu_posix_ffs(unsigned long word)
{
	int ret = 0;
	const int maxvalue = 64;

	if ((word & (unsigned long) LONG_MAX) != 0UL) {
		ret = __builtin_ffsl(
			nvgpu_safe_cast_u64_to_s64(
				(word & (unsigned long) LONG_MAX)));
	} else {
		if (word > (unsigned long) LONG_MAX) {
			ret = maxvalue;
		}
	}

	return nvgpu_safe_cast_s32_to_u64(ret);
}

unsigned long nvgpu_posix_fls(unsigned long word)
{
	unsigned long ret;

	if (word == 0UL) {
		/* __builtin_clzl() below is undefined for 0, so we have
		 * to handle that as a special case.
		 */
		ret = 0UL;
	} else {
		ret = (sizeof(unsigned long) * 8UL) -
			(nvgpu_safe_cast_s32_to_u64(__builtin_clzl(word)));
	}

	return ret;
}

/**
 * @brief Find the next set or unset bit in a bitmap starting from a given position.
 *
 * This function searches for the next bit in a bitmap that is set (or unset if
 * the invert parameter is true) starting from a specified position. It uses an
 * optimization to avoid branching when inverting the bitmap by XORing with a
 * mask. The function returns the index of the found bit or the size of the
 * bitmap if no such bit is found.
 *
 * The steps performed by the function are as follows:
 * -# Check if the start position is greater than or equal to the size of the
 *    bitmap. If it is, return the size of the bitmap as no further search is
 *    needed.
 * -# Create a mask to isolate bits starting from the start position within the
 *    first word of the bitmap.
 * -# Calculate the index of the word in the bitmap where the start position is
 *    located.
 * -# Calculate the index of the last word in the bitmap that needs to be
 *    searched.
 * -# XOR the word at the calculated index with the invert mask and apply the
 *    start mask to isolate relevant bits.
 * -# Loop through the words in the bitmap starting from the calculated index:
 *    -# Increment the index to move to the next word.
 *    -# If the index exceeds the index of the last word, return the size of the
 *       bitmap as no set/unset bit is found.
 *    -# XOR the current word with the invert mask to account for inversion if
 *       needed.
 *    -# If the current word is not zero, break the loop as a set/unset bit is
 *       found.
 * -# Use the 'nvgpu_ffs()' function to find the first set bit in the non-zero
 *    word.
 * -# Calculate the bit index within the bitmap using the index of the word and
 *    the position of the first set bit within the word.
 * -# Return the minimum of the size of the bitmap and the calculated bit index.
 *
 * @param [in] address   Pointer to the start of the bitmap.
 * @param [in] n         Size of the bitmap in bits.
 *                       Range: [0, ULONG_MAX]
 * @param [in] start     Starting position for the search within the bitmap.
 *                       Range: [0, n-1]
 * @param [in] invert    Boolean indicating whether to search for set bits (false)
 *                       or unset bits (true).
 *
 * @return Index of the next set/unset bit found in the bitmap starting from the
 *         start position, or the size of the bitmap if no such bit is found.
 */
static unsigned long nvgpu_posix_find_next_bit(const unsigned long *address,
				     unsigned long n,
				     unsigned long start,
				     bool invert)
{
	unsigned long idx, idx_max;
	unsigned long w;
	unsigned long start_mask;
	const unsigned long *base_addr = (const unsigned long *)&address[0];

	/*
	 * We make a mask we can XOR into the word so that we can invert the
	 * word without requiring a branch. I.e instead of doing:
	 *
	 *   w = invert ? ~addr[idx] : addr[idx]
	 *
	 * We can do:
	 *
	 *   w = addr[idx] ^= invert_mask
	 *
	 * This saves us a branch every iteration through the loop. Now we can
	 * always just look for 1s.
	 */
	unsigned long invert_mask = invert ? ~0UL : 0UL;

	if (start >= n) {
		return n;
	}

	start_mask = ~0UL << (start & (BITS_PER_LONG - 1UL));

	idx = start / BITS_PER_LONG;
	w = (base_addr[idx] ^ invert_mask) & start_mask;

	idx_max = (n - 1UL) / BITS_PER_LONG;

	/*
	 * Find the first non-zero word taking into account start and
	 * invert.
	 */
	while (w == 0UL) {
		idx = nvgpu_safe_add_u64(idx, 1UL);
		if (idx > idx_max) {
			return n;
		}

		w = base_addr[idx] ^ invert_mask;
	}

	return min(n, (nvgpu_safe_add_u64(((nvgpu_ffs(w)) - 1UL),
				(nvgpu_safe_mult_u64(idx, BITS_PER_LONG)))));
}

unsigned long find_first_bit(const unsigned long *address, unsigned long size)
{
	return nvgpu_posix_find_next_bit(address, size, 0, false);
}

unsigned long find_first_zero_bit(const unsigned long *address, unsigned long size)
{
	return nvgpu_posix_find_next_bit(address, size, 0, true);
}

unsigned long find_next_bit(const unsigned long *address, unsigned long size,
			    unsigned long offset)
{
	return nvgpu_posix_find_next_bit(address, size, offset, false);
}

/**
 * @brief Finds the position of the first zero bit in a bitmap starting from a given offset.
 *
 * This function searches for the first zero bit in a bitmap starting from a specified offset
 * within the bitmap. The bitmap is represented as an array of unsigned longs.
 *
 * The steps performed by the function are as follows:
 * -# Call the 'nvgpu_posix_find_next_bit()' function with the bitmap address, size of the bitmap,
 *    offset to start the search from, and a boolean value 'true' indicating that the search is
 *    for a zero bit.
 *
 * @param [in] address  Pointer to the start of the bitmap.
 *                      Range: Valid pointer to an array of unsigned long.
 * @param [in] size     Size of the bitmap in bits.
 *                      Range: 0 to the maximum number of bits that can be represented by
 *                      unsigned long.
 * @param [in] offset   Offset within the bitmap to start the search.
 *                      Range: 0 to size - 1.
 *
 * @return The position of the first zero bit found after the offset within the bitmap. If no zero
 *         bit is found, the function returns the size of the bitmap.
 */
static unsigned long find_next_zero_bit(const unsigned long *address,
					unsigned long size,
					unsigned long offset)
{
	return nvgpu_posix_find_next_bit(address, size, offset, true);
}

void nvgpu_bitmap_set(unsigned long *map, unsigned int start, unsigned int len)
{
	unsigned int end = start + len;

	/*
	 * Super slow naive implementation. But speed isn't what matters here.
	 */
	while (start < end) {
		nvgpu_set_bit(start++, map);
	}
}

void nvgpu_bitmap_clear(unsigned long *map,
				unsigned int start, unsigned int len)
{
	unsigned int end = start + len;

	while (start < end) {
		nvgpu_clear_bit(start++, map);
	}
}

/*
 * This is essentially a find-first-fit allocator: this searches a bitmap for
 * the first space that is large enough to satisfy the requested size of bits.
 * That means that this is not a vary smart allocator. But it is fast relative
 * to an allocator that goes looking for an optimal location.
 */
unsigned long bitmap_find_next_zero_area(unsigned long *map,
					 unsigned long size,
					 unsigned long start,
					 unsigned int bit,
					 unsigned long align_mask)
{
	unsigned long offs;

	while ((nvgpu_safe_add_u64(start, (unsigned long)bit)) <= size) {
		start = find_next_zero_bit(map, size, start);

		start = ALIGN_MASK(start, align_mask);

		/*
		 * Not enough space left to satisfy the requested area.
		 */
		if ((nvgpu_safe_add_u64(start, (unsigned long)bit)) > size) {
			return size;
		}

		offs = find_next_bit(map, size, start);

		if ((offs - start) >= bit) {
			return start;
		}

		start = offs + 1UL;
	}

	return size;
}

bool nvgpu_test_bit(unsigned int bit, const volatile unsigned long *address)
{
	return (1UL & (address[get_index(bit)] >>
			(bit & (BITS_PER_LONG-1UL)))) != 0UL;
}

bool nvgpu_test_and_set_bit(unsigned int bit, volatile unsigned long *address)
{
	unsigned long mask = get_mask(bit);

	NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 1_4), "TID-1591")
	volatile unsigned _Atomic long *p =
		(volatile unsigned _Atomic long *)address + get_index(bit);
	NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 1_4))

	return (atomic_fetch_or(p, mask) & mask) != 0ULL;
}

bool nvgpu_test_and_clear_bit(unsigned int bit, volatile unsigned long *address)
{
	unsigned long mask = get_mask(bit);

	NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 1_4), "TID-1591")
	volatile unsigned _Atomic long *p =
		(volatile unsigned _Atomic long *)address + get_index(bit);
	NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 1_4))

	return (atomic_fetch_and(p, ~mask) & mask) != 0ULL;
}

void nvgpu_set_bit(unsigned int bit, volatile unsigned long *address)
{
	unsigned long mask = get_mask(bit);

	NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 1_4), "TID-1591")
	volatile unsigned _Atomic long *p =
		(unsigned volatile _Atomic long *)address + get_index(bit);
	NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 1_4))

	(void)atomic_fetch_or(p, mask);
}

void nvgpu_clear_bit(unsigned int bit, volatile unsigned long *address)
{
	unsigned long mask = get_mask(bit);

	NVGPU_COV_WHITELIST_BLOCK_BEGIN(deviate, 1, NVGPU_MISRA(Rule, 1_4), "TID-1591")
	volatile unsigned _Atomic long *p =
		(volatile unsigned _Atomic long *)address + get_index(bit);
	NVGPU_COV_WHITELIST_BLOCK_END(NVGPU_MISRA(Rule, 1_4))

	(void)atomic_fetch_and(p, ~mask);
}
