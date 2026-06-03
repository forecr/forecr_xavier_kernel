/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_BITOPS_H
#define NVGPU_POSIX_BITOPS_H

#include <nvgpu/types.h>

/*
 * Assume an 8 bit byte, of course.
 */

/**
 * Number of bits in a byte.
 */
#define BITS_PER_BYTE	8UL

/**
 * Number of bits in data type long.
 */
#define BITS_PER_LONG 	((unsigned long)__SIZEOF_LONG__ * BITS_PER_BYTE)

/**
 * @brief Convert number of bits into long.
 *
 * Converts the input param \a bits into equivalent number of long.
 * Uses the safe add API #nvgpu_safe_add_u64() with \a bits and
 * #BITS_PER_LONG - 1 as parameters to round up the value of \a bits.
 * The number of long variables required to store \a bits is calculated
 * from the rounded up value of \a bits. Macro does not perform any validation
 * of the input parameters.
 *
 * @param bits [in]	Number of bits to convert.
 */
#define BITS_TO_LONGS(bits)			\
	(nvgpu_safe_add_u64(bits, BITS_PER_LONG - 1UL) / BITS_PER_LONG)

/**
 * @brief Set the bit value.
 *
 * Invokes the macro #BIT64() to set the value at bit position indicated by
 * \a i. Deprecated; use the explicit BITxx() macros instead. Macro does not
 * perform any validation of the input parameter.
 *
 * @param i [in]	Bit position to set.
 */
#define BIT(i)		BIT64(i)

/**
 * @brief Create a bit mask.
 *
 * Creates a bit mask starting at bit position \a lo and ending at \a hi.
 * Macro does not perform any validation of the input parameters.
 *
 * @param hi [in]	MSB position of the bit mask.
 * @param lo [in]	LSB position of the bit mask.
 */
#define GENMASK(hi, lo) \
	(((~0UL) - (1UL << (lo)) + 1UL) &	\
		(~0UL >> (BITS_PER_LONG - 1UL - (unsigned long)(hi))))

/*
 * Can't use BITS_TO_LONGS to declare arrays where we can't use BUG(), so if the
 * range is invalid, use -1 for the size which will generate a compiler error.
 */

/**
 * @brief Create an array of unsigned long to represent a bitmap.
 *
 * Creates an array of data type unsigned long by name \a bmap. The size of the
 * array is taken as \a bits value converted into an equivalent rounded up long
 * value if the rounded up value is less than or equal to LONG_MAX. Otherwise
 * the array declaration will generate a compiler error. Macro does not perform
 * any validation of the input parameters.
 *
 * @param bmap [in]	Bitmap to create.
 * @param bits [in]	Number of bits.
 */
#define DECLARE_BITMAP(bmap, bits)					\
	unsigned long bmap[(((LONG_MAX - (bits)) < (BITS_PER_LONG - 1UL)) ? \
		-1 :							\
		(long int)(((bits) + BITS_PER_LONG - 1UL) /		\
						BITS_PER_LONG))]

/**
 * @brief Loop for each set bit.
 *
 * Macro does not perform any validation of the parameters.
 *
 * @param bit [in]	Each set bit, this is the loop index.
 * @param address [in]	Starting of the bitmap.
 * @param size [in]	Size of the bitmap.
 */
#define for_each_set_bit(bit, address, size) \
	for ((bit) = find_first_bit((address), (size));		\
	     (bit) < (size);					\
	     (bit) = find_next_bit((address), (size), (bit) + 1U))

/**
 * @brief Find first set bit.
 *
 * Function returns the position of the first set bit in \a word. This function
 * internally uses the builtin function __builtin_ffsl. Function does not
 * perform any validation of the parameter.
 *
 * @param word [in]	Input of datatype long.
 *
 * @return Returns one plus the index of the least significant 1-bit of input
 * param \a word. Returns 0 if input param is 0.
 */
unsigned long nvgpu_posix_ffs(unsigned long word);

/**
 * @brief Find last set bit.
 *
 * Function returns the position of the last set bit in \a word. This function
 * internally uses the builtin function __builtin_clzl. Function does not
 * perform any validation of the input parameter.
 *
 * @param word [in]	Input of datatype long.
 *
 * @return Returns one plus the index of the most significant 1-bit of input
 * param word. If input param is 0, returns 0.
 */
unsigned long nvgpu_posix_fls(unsigned long word);

/**
 * Wrapper define for #nvgpu_posix_ffs.
 */
#define nvgpu_ffs(word)	nvgpu_posix_ffs(word)

/**
 * Wrapper define for #nvgpu_posix_fls.
 */
#define nvgpu_fls(word)	nvgpu_posix_fls(word)

/**
 * @brief Find first zero bit.
 *
 * Macro to find the position of first zero bit in input data \a word.
 * Uses the macro #nvgpu_ffs internally. Macro does not perform any
 * validation of the input parameter.
 *
 * @param word [in]	Input value to search for the bit.
 *
 * @return Returns the bit position of first zero bit in the input data.
 */
#define ffz(word)	(nvgpu_ffs(~(word)) - 1UL)

/**
 * @brief Find the first set bit.
 *
 * Finds the first set bit position in the input bitmap pointed by \a address.
 * Function does not perform any validation of the input parameter.
 *
 * @param address [in]	Input value to search for set bit.
 * @param size [in]	Size of the input value in bits.
 *
 * @return Returns the position of first set bit.
 */
unsigned long find_first_bit(const unsigned long *address, unsigned long size);

/**
 * @brief Finds the next set bit.
 *
 * Finds the next set bit position in the input data \a address.
 * Function does not perform any validation of the input parameter.
 *
 * @param address [in]	Input value to search for next set bit.
 * @param size [in]	Size of the input value in bits.
 * @param offset [in]	Offset to start from the input data.
 *
 * @return Returns the position of next set bit.
 */
unsigned long find_next_bit(const unsigned long *address, unsigned long size,
			    unsigned long offset);

/**
 * @brief Finds the first zero bit.
 *
 * Finds the first zero bit position in the input data \a address.
 * Function does not perform any validation of the input parameter.
 *
 * @param address [in]	Input value to search.
 * @param size [in]	Size of the input value in bits.
 *
 * @return Returns the position of first zero bit.
 */
unsigned long find_first_zero_bit(const unsigned long *address,
				  unsigned long size);

/**
 * @brief Test the bit value at given position.
 *
 * Checks if the bit at position mentioned by \a bit in \a address is set or
 * not. Function does not perform any validation of the input parameter.
 *
 * @param bit [in]	Bit position to check.
 * @param address [in]	Input data stream.
 *
 * @return Boolean value which indicates the status of the bit.
 *
 * @retval TRUE if the bit position is set.
 * @retval FALSE if the bit position is not set.
 */
bool nvgpu_test_bit(unsigned int bit, const volatile unsigned long *address);

/**
 * @brief Test and set the bit at given position.
 *
 * Tests and sets the bit at position \a bit in \a address. Uses the library
 * function \a atomic_fetch_or internally with a long pointer pointing to
 * the location where the bit position \a bit is stored and the mask value
 * of the bit position as parameters. Function does not perform any
 * validation of the input parameter.
 *
 * @param bit [in]	Bit position to test and set.
 * @param address [in]	Input data stream.
 *
 * @return Boolean value which indicates the status of the bit before the set
 * operation.
 *
 * @retval TRUE if the bit position was already set.
 * @retval FALSE if the bit position was not set before.
 */
bool nvgpu_test_and_set_bit(unsigned int bit, volatile unsigned long *address);

/**
 * @brief Test and clear the bit at given position.
 *
 * Tests and clears the bit at position \a bit in \a address. Uses the library
 * function \a atomic_fetch_and internally with a long pointer pointing to
 * the location where the bit position \a bit is stored and the mask value
 * of the bit position as parameters. Function does not perform any
 * validation of the input parameter.
 *
 * @param bit [in]	Bit position to test and clear.
 * @param address [in]	Input data stream.
 *
 * @return Boolean value which indicates the status of the bit before the clear
 * operation.
 *
 * @retval TRUE if the bit position was already set.
 * @retval FALSE if the bit position was not set before.
 */
bool nvgpu_test_and_clear_bit(unsigned int bit,
			      volatile unsigned long *address);

/*
 * These two are atomic.
 */

/**
 * @brief Sets the bit at given position.
 *
 * Sets the bit atomically at bit position \a bit in \a address. Uses the
 * library function \a atomic_fetch_or internally with a long pointer pointing
 * to the location where the bit position \a bit is stored and the mask value
 * of the bit position as parameters. Function does not perform any
 * validation of the input parameter.
 *
 * @param bit [in]	Bit position to set.
 * @param address [in]	Input data stream.
 */
void nvgpu_set_bit(unsigned int bit, volatile unsigned long *address);

/**
 * @brief Clears the bit at given position.
 *
 * Clears the bit atomically at bit position \a bit in \a address. Uses the
 * library function \a atomic_fetch_and internally with a long pointer pointing
 * to the location where the bit position \a bit is stored and the mask value
 * of the bit position as parameters. Function does not perform any
 * validation of the input parameter.
 *
 * @param bit [in]	Bit position to clear.
 * @param address [in]	Input data stream.
 */
void nvgpu_clear_bit(unsigned int bit, volatile unsigned long *address);

/**
 * @brief Sets a bitmap.
 *
 * Sets a bitmap of length \a len starting from bit position \a start in
 * \a map. Uses the function #nvgpu_set_bit() internally in a loop with the bit
 * positions and \a map as input parameters. Function does not perform any
 * validation of the input parameters.
 *
 * @param map [in,out]	Input data to set bitmap.
 * @param start [in]	Start position of the bitmap.
 * @param len [in]	Length of the bitmap.
 */
void nvgpu_bitmap_set(unsigned long *map, unsigned int start, unsigned int len);

/**
 * @brief Clears a bitmap.
 *
 * Clears a bitmap of length \a len starting from bit position \a start in \a
 * map. Uses the function #nvgpu_clear_bit() internally in a loop with the bit
 * positions and \a map as input parameters. Function does not perform any
 * validation of the input parameters.
 *
 * @param map [in,out]	Input data to clear bitmap.
 * @param start [in]	Start position of the bitmap.
 * @param len [in]	Length of the bitmap.
 */
void nvgpu_bitmap_clear(unsigned long *map, unsigned int start,
			unsigned int len);

/**
 * @brief Find the starting index of a contiguous area of unset (zero) bits.
 *
 * This function searches for a contiguous area of unset bits of a given size
 * within a bitmap, starting from a specified index, and optionally aligned to
 * a specified mask. It returns the starting index of the first area found that
 * matches the criteria or the size of the bitmap if no such area is found.
 *
 * The steps performed by the function are as follows:
 * -# Loop until the sum of the starting index and the size of the area to find
 *    is less than or equal to the size of the bitmap.
 * -# Find the index of the first unset bit in the bitmap starting from the
 *    current starting index using find_next_zero_bit().
 * -# Align the starting index to the specified alignment mask using ALIGN_MASK.
 * -# Check if there is enough space left in the bitmap to satisfy the requested
 *    area size. If not, return the size of the bitmap as no suitable area can
 *    be found.
 * -# Find the index of the first set bit in the bitmap starting from the newly
 *    aligned starting index using find_next_bit().
 * -# Check if the number of unset bits from the aligned starting index to the
 *    index of the first set bit is greater than or equal to the size of the
 *    area to find. If so, return the aligned starting index as the beginning
 *    of the suitable area.
 * -# If a suitable area is not found, update the starting index to one more
 *    than the index of the first set bit and continue the loop.
 * -# If the loop exits without finding a suitable area, return the size of the
 *    bitmap to indicate failure.
 *
 * @param [in] map          Pointer to the bitmap.
 * @param [in] size         Size of the bitmap in bits.
 * @param [in] start        Index to start the search from within the bitmap.
 * @param [in] bit          Size of the area to find in bits.
 * @param [in] align_mask   Mask for aligning the starting index.
 *
 * @return Starting index of the area found, or the size of the bitmap if no
 *         suitable area is found.
 *
 * @note The function assumes that the bitmap is large enough to include the
 *       start index and the size of the area to find.
 */
unsigned long bitmap_find_next_zero_area(unsigned long *map,
					 unsigned long size,
					 unsigned long start,
					 unsigned int bit,
					 unsigned long align_mask);

#endif /* NVGPU_POSIX_BITOPS_H */
