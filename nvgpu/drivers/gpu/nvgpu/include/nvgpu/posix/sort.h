/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_SORT_H
#define NVGPU_POSIX_SORT_H

#include <stdlib.h>

/**
 * @brief Sort a given set of data.
 *
 * @param base [in,out]		Pointer to the first element in the list.
 * @param num [in]		Number of elements in the list pointed by base.
 * @param size [in]		Size in bytes of each element in the list.
 * @param cmp [in]		Function to compare two items in the list.
 *
 * The function sorts the given set of data referenced by \a base. The internal
 * implementation used for sorting is qsort.
 */
static void sort(void *base, size_t num, size_t size,
		 int (*cmp)(const void *a, const void *b),
		 void (*swap)(void *a, void *b, int n) __attribute__((unused)))
{
	qsort(base, num, size, cmp);
}

#endif /* NVGPU_POSIX_SORT_H */
