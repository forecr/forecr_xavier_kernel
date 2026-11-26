// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bsearch.h>
#include <stdlib.h>

void *nvgpu_bsearch(const void *key, const void *base, size_t nitems, size_t size,
		int (*compar)(const void *a, const void *b))
{
	return bsearch(key, base, nitems, size, compar);
}
