/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */
#ifndef NVGPU_BSEARCH_H
#define NVGPU_BSEARCH_H

#include <nvgpu/types.h>

void *nvgpu_bsearch(const void *key, const void *base, size_t nitems, size_t size,
		int (*compar)(const void *a, const void *b));

#endif /*NVGPU_BSEARCH_H*/
