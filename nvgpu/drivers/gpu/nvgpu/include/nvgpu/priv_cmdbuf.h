/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_PRIV_CMDBUF_H
#define NVGPU_PRIV_CMDBUF_H

#include <nvgpu/types.h>

struct gk20a;
struct vm_gk20a;
struct priv_cmd_entry;
struct priv_cmd_queue;

#ifdef CONFIG_NVGPU_NON_FUSA
int nvgpu_priv_cmdbuf_queue_alloc(struct vm_gk20a *vm,
		u32 job_count, struct priv_cmd_queue **queue);
void nvgpu_priv_cmdbuf_queue_free(struct priv_cmd_queue *q);

int nvgpu_priv_cmdbuf_alloc(struct priv_cmd_queue *q, u32 size,
		struct priv_cmd_entry **e);
void nvgpu_priv_cmdbuf_rollback(struct priv_cmd_queue *q,
		struct priv_cmd_entry *e);
void nvgpu_priv_cmdbuf_free(struct priv_cmd_queue *q,
		struct priv_cmd_entry *e);

void nvgpu_priv_cmdbuf_append(struct gk20a *g, struct priv_cmd_entry *e,
		u32 *data, u32 entries);
void nvgpu_priv_cmdbuf_append_zeros(struct gk20a *g, struct priv_cmd_entry *e,
		u32 entries);

void nvgpu_priv_cmdbuf_finish(struct gk20a *g, struct priv_cmd_entry *e,
		u64 *gva, u32 *size);
#endif /* CONFIG_NVGPU_NON_FUSA */
#endif
