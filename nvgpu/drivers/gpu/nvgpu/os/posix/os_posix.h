/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_OS_POSIX_H
#define NVGPU_OS_POSIX_H

#include <nvgpu/gk20a.h>

struct nvgpu_posix_io_callbacks;

struct nvgpu_os_posix {
	struct gk20a g;


	/*
	 * IO callbacks for handling the nvgpu IO accessors.
	 */
	struct nvgpu_posix_io_callbacks *callbacks;

	/*
	 * Memory-mapped register space for unit tests.
	 */
	struct nvgpu_list_node reg_space_head;
	int error_code;


	/*
	 * List to record sequence of register writes.
	 */
	struct nvgpu_list_node recorder_head;
	bool recording;

	/*
	 * Parameters to change the behavior of MM-related functions
	 */
	bool mm_is_iommuable;
	bool mm_sgt_is_iommuable;

	/*
	 * Parameters to change SOC behavior
	 */
	bool is_soc_t194_a01;
	bool is_silicon;
	bool is_fpga;
	bool is_simulation;
};

static inline struct nvgpu_os_posix *nvgpu_os_posix_from_gk20a(struct gk20a *g)
{
	return container_of(g, struct nvgpu_os_posix, g);
}

#endif /* NVGPU_OS_POSIX_H */
