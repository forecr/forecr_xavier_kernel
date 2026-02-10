/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NVGPU_DMABUF_NVS_H
#define NVGPU_DMABUF_NVS_H

#include <nvgpu/types.h>
#include <nvgpu/list.h>

struct dma_buf;
struct gk20a;

struct nvgpu_nvs_linux_buf_priv {
	/* This is used to temporarily contain the dmabuf for handling failure */
	struct dma_buf	*dmabuf_temp;
	bool read_only;
	u32 mapped_ref;

	struct nvgpu_list_node list_mapped_user_vmas;
};

struct nvgpu_nvs_domain_ctrl_fifo_user_vma {
	bool read_only;
	struct nvgpu_nvs_ctrl_queue *buf;
	struct vm_area_struct *vma;
	struct nvgpu_list_node node;
};

int nvgpu_nvs_get_buf(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *buf,
		bool read_only);

/**
 * @brief Construct a buffer for use as a shared message passing
 *        queue between user and backend scheduler. Function is
 *        not safe from concurrent access by multiple external
 *        users. Must be invoked between the calls of
 *        nvgpu_nvs_ctrl_fifo_lock_queues() and
 *        nvgpu_nvs_ctrl_fifo_unlock_queues().
 *
 * @param g instance of struct gk20a.
 * @param buf instance of struct nvgpu_nvs_ctrl_queue to contain
 *            the constructed buffer metadata.
 * @param bytes size of buffer requested.
 * @param mask Mask of queue requested.
 * @param read_only Indicates whether a read-only buffer is requested.
 * @return int 0 on success, else fail.
 */
int nvgpu_nvs_alloc_and_get_buf(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *buf,
		size_t bytes, u8 mask, bool read_only);

/**
 * @brief Check whether user mappings exist for this buffer.
 *
 * @param g instance of struct gk20a.
 * @param buf instance of struct nvgpu_nvs_ctrl_queue.
 * @return true User mappings exist.
 * @return false User mappings doesn't exist.
 */
bool nvgpu_nvs_buf_linux_is_mapped(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *buf);

#endif /* NVGPU_DMABUF_NVS_H */
