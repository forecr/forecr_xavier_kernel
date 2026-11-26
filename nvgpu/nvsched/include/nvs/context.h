/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVS_CONTEXT_H
#define NVS_CONTEXT_H

#include <nvs/types-internal.h>

struct nvs_context;

/**
 * Similar to a nvs_domain_list this is a singly linked list of contexts.
 * If sub-scheduler algorithms ever want something more sophisticated they'll
 * likely have the build it themselves.
 */
struct nvs_context_list {
	u32			 nr;
	struct nvs_context	*contexts;
};

#endif
