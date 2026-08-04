// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/string.h>
#include <nvgpu/kmem.h>
#include <nvgpu/log.h>
#include <nvgpu/errno.h>
#include <nvgpu/gsp.h>

#include "gsp_seq.h"

/**
 * @brief Initializes the GSP sequences structure.
 *
 * The function initializes the GSP sequences structure by setting all sequence
 * entries and the sequence table to zero and assigning a unique ID to each
 * sequence entry.
 *
 * The steps performed by the function are as follows:
 * -# Zero out the memory for the sequence entries array using memset().
 * -# Zero out the memory for the GSP sequence table using memset().
 * -# Assign a unique ID to each sequence entry in the array, starting from 0
 *    up to the maximum number of GSP sequences.
 *
 * @Param [in]  g          Pointer to the GPU structure.
 * @Param [out] sequences  Pointer to the pointer of the GSP sequences structure.
 */
static void gsp_sequences_init(struct gk20a *g,
			struct gsp_sequences **sequences)
{
	u16 i = 0;

	nvgpu_gsp_dbg(g, " ");

	(void) memset((*sequences)->seq, 0,
		sizeof(struct gsp_sequence) * GSP_MAX_NUM_SEQUENCES);

	(void) memset((*sequences)->gsp_seq_tbl, 0,
		sizeof((*sequences)->gsp_seq_tbl));

	for (i = 0; i < GSP_MAX_NUM_SEQUENCES; i++) {
		(*sequences)->seq[i].id = (u8)i;
	}
}

int nvgpu_gsp_sequences_init(struct gk20a *g, struct nvgpu_gsp_ipc **gsp_ipc)
{
	int err = 0;
	struct gsp_sequences *seqs = NULL;

	nvgpu_gsp_dbg(g, " ");

	seqs = (struct gsp_sequences *) nvgpu_kzalloc(g, sizeof(struct gsp_sequences));
	if (seqs == NULL) {
		nvgpu_err(g, "GSP sequences allocation failed");
		return -ENOMEM;
	}

	seqs->seq = (struct gsp_sequence *) nvgpu_kzalloc(g,
			GSP_MAX_NUM_SEQUENCES * sizeof(struct gsp_sequence));
	if (seqs->seq == NULL) {
		nvgpu_err(g, "GSP sequence allocation failed");
		nvgpu_kfree(g, seqs);
		return -ENOMEM;
	}

	nvgpu_mutex_init(&seqs->gsp_seq_lock);

	gsp_sequences_init(g, &seqs);

	(*gsp_ipc)->sequences = seqs;

	return err;
}

void nvgpu_gsp_sequences_free(struct gk20a *g,
			struct gsp_sequences **sequences)
{
	nvgpu_mutex_destroy(&(*sequences)->gsp_seq_lock);
	nvgpu_kfree(g, ((*sequences)->seq));
	(*sequences)->seq = NULL;
	nvgpu_kfree(g, (*sequences));
	*sequences = NULL;
}

int nvgpu_gsp_seq_acquire(struct gk20a *g,
			struct gsp_sequences *sequences,
			struct gsp_sequence **pseq,
			gsp_callback callback, void *cb_params)
{
	struct gsp_sequence *seq;
	u32 index = 0;
	int err = 0;

	nvgpu_gsp_dbg(g, " ");

	nvgpu_mutex_acquire(&sequences->gsp_seq_lock);

	index = nvgpu_safe_cast_u64_to_u32(find_first_zero_bit(sequences->gsp_seq_tbl,
			GSP_MAX_NUM_SEQUENCES));

	if (index >= GSP_MAX_NUM_SEQUENCES) {
		nvgpu_err(g, "no free sequence available");
		nvgpu_mutex_release(&sequences->gsp_seq_lock);
		err = -EAGAIN;
		goto exit;
	}

	nvgpu_assert(index < U64(INT_MAX));
	nvgpu_set_bit(index, sequences->gsp_seq_tbl);

	nvgpu_mutex_release(&sequences->gsp_seq_lock);

	seq = &sequences->seq[index];

	seq->state = GSP_SEQ_STATE_PENDING;
	seq->callback = callback;
	seq->cb_params = cb_params;
	seq->out_payload = NULL;

	*pseq = seq;

exit:
	return err;
}

void gsp_seq_release(struct gsp_sequences *sequences,
			struct gsp_sequence *seq)
{
	seq->state	= GSP_SEQ_STATE_FREE;
	seq->callback	= NULL;
	seq->cb_params	= NULL;
	seq->out_payload = NULL;

	nvgpu_mutex_acquire(&sequences->gsp_seq_lock);
	nvgpu_clear_bit(seq->id, sequences->gsp_seq_tbl);
	nvgpu_mutex_release(&sequences->gsp_seq_lock);
}

int nvgpu_gsp_seq_response_handle(struct gk20a *g,
				struct gsp_sequences *sequences,
				struct nv_flcn_msg_gsp *msg, u32 seq_id)
{
	struct gsp_sequence *seq;

	nvgpu_gsp_dbg(g, " ");

	/* get the sequence info data associated with this message */
	seq = &sequences->seq[seq_id];


	if (seq->state != GSP_SEQ_STATE_USED) {
		nvgpu_err(g, "msg for an unknown sequence %d", seq->id);
		return -EINVAL;
	}

	if (seq->callback != NULL) {
		seq->callback(g, msg, seq->cb_params, 0);
	}

	/* release the sequence so that it may be used for other commands */
	gsp_seq_release(sequences, seq);

	return 0;
}

u8 nvgpu_gsp_seq_get_id(struct gsp_sequence *seq)
{
	return seq->id;
}

void nvgpu_gsp_seq_set_state(struct gsp_sequence *seq, enum gsp_seq_state state)
{
	seq->state = state;
}
