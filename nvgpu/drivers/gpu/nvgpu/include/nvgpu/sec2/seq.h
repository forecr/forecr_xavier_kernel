/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_SEC2_SEQ_H
#define NVGPU_SEC2_SEQ_H

#include <nvgpu/types.h>
#include <nvgpu/lock.h>

struct gk20a;
struct nv_flcn_msg_sec2;

#define SEC2_MAX_NUM_SEQUENCES	(256U)
#define SEC2_SEQ_BIT_SHIFT		(5U)
#define SEC2_SEQ_TBL_SIZE	\
	(SEC2_MAX_NUM_SEQUENCES >> SEC2_SEQ_BIT_SHIFT)

enum sec2_seq_state {
	SEC2_SEQ_STATE_FREE = 0U,
	SEC2_SEQ_STATE_PENDING,
	SEC2_SEQ_STATE_USED
};

typedef void (*sec2_callback)(struct gk20a *g, struct nv_flcn_msg_sec2 *msg,
	void *param, u32 status);

struct sec2_sequence {
	u8 id;
	enum sec2_seq_state state;
	u8 *out_payload;
	sec2_callback callback;
	void *cb_params;
};

struct sec2_sequences {
	struct sec2_sequence *seq;
	unsigned long sec2_seq_tbl[SEC2_SEQ_TBL_SIZE];
	struct nvgpu_mutex sec2_seq_lock;
};

int nvgpu_sec2_sequences_alloc(struct gk20a *g,
			       struct sec2_sequences *sequences);
void nvgpu_sec2_sequences_init(struct gk20a *g,
			       struct sec2_sequences *sequences);
void nvgpu_sec2_sequences_free(struct gk20a *g,
			       struct sec2_sequences *sequences);
int nvgpu_sec2_seq_acquire(struct gk20a *g,
			   struct sec2_sequences *sequences,
			   struct sec2_sequence **pseq,
			   sec2_callback callback, void *cb_params);
int nvgpu_sec2_seq_response_handle(struct gk20a *g,
				   struct sec2_sequences *sequences,
				   struct nv_flcn_msg_sec2 *msg, u32 seq_id);
u8 nvgpu_sec2_seq_get_id(struct sec2_sequence *seq);
void nvgpu_sec2_seq_set_state(struct sec2_sequence *seq,
			      enum sec2_seq_state state);

#endif /* NVGPU_SEC2_SEQ_H */
