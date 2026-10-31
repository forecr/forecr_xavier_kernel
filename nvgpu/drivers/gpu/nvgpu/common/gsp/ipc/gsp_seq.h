/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_GSP_SEQ_H
#define NVGPU_GSP_SEQ_H

#include <nvgpu/types.h>
#include <nvgpu/lock.h>
#include "gsp_ipc.h"

struct gk20a;
struct nv_flcn_msg_gsp;

#define GSP_MAX_NUM_SEQUENCES	256U
#define GSP_SEQ_BIT_SHIFT	5U
#define GSP_SEQ_TBL_SIZE	(GSP_MAX_NUM_SEQUENCES >> GSP_SEQ_BIT_SHIFT)

enum gsp_seq_state {
	GSP_SEQ_STATE_FREE = 0U,
	GSP_SEQ_STATE_PENDING,
	GSP_SEQ_STATE_USED
};

typedef void (*gsp_callback)(struct gk20a *g, struct nv_flcn_msg_gsp *msg,
	void *param, u32 status);

struct gsp_sequence {
	u8 id;
	enum gsp_seq_state state;
	u8 *out_payload;
	gsp_callback callback;
	void *cb_params;
};

struct gsp_sequences {
	struct gsp_sequence *seq;
	unsigned long gsp_seq_tbl[GSP_SEQ_TBL_SIZE];
	struct nvgpu_mutex gsp_seq_lock;
};

/**
 * @brief Initializes the GSP ipc sequences.
 *
 * Allocates and initializes the GSP sequences structure, sets up the sequence
 * array, initializes the mutex for GSP sequence operations, and assigns the
 * initialized sequences to the GSP ipc.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for the GSP sequences structure using nvgpu_kzalloc().
 * -# If the allocation fails, log an error and return -ENOMEM.
 * -# Allocate memory for the array of GSP sequence structures within the GSP
 *    sequences structure using nvgpu_kzalloc().
 * -# If the allocation fails, log an error, free the previously allocated GSP
 *    sequences structure using nvgpu_kfree(), and return -ENOMEM.
 * -# Initialize the mutex within the GSP sequences structure using
 *    nvgpu_mutex_init().
 * -# Initialize the GSP sequences using gsp_sequences_init().
 * -# Assign the pointer to the initialized GSP sequences structure to the
 *    sequences field of the GSP ipc structure.
 * -# Return 0 to indicate success.
 *
 * @Param [in]  g          Pointer to the GPU structure.
 * @Param [out] gsp_ipc    Pointer to the pointer of the GSP ipc structure.
 *
 * @return 0 on success, or -ENOMEM if memory allocation fails.
 */
int nvgpu_gsp_sequences_init(struct gk20a *g, struct nvgpu_gsp_ipc **gsp_ipc);

/**
 * @brief Frees the memory and resources associated with GSP sequences.
 *
 * This function releases the memory allocated for the GSP sequences and destroys
 * the associated mutex lock. It ensures that all resources are properly cleaned
 * up and the pointers are set to NULL to prevent any dangling references.
 *
 * The steps performed by the function are as follows:
 * -# Destroy the mutex lock associated with the GSP sequences using
 *    nvgpu_mutex_destroy().
 * -# Free the memory allocated for the sequence array within the GSP sequences
 *    using nvgpu_kfree().
 * -# Set the pointer to the sequence array within the GSP sequences to NULL.
 * -# Free the memory allocated for the GSP sequences structure using nvgpu_kfree().
 * -# Set the pointer to the GSP sequences structure to NULL.
 *
 * @Param [in] g            Pointer to the GPU driver structure.
 * @Param [out] sequences   Pointer to the pointer of the GSP sequences structure
 *                          to be freed.
 */
void nvgpu_gsp_sequences_free(struct gk20a *g,
			struct gsp_sequences **sequences);

/**
 * @brief Acquires a free GSP sequence and initializes it with a callback and parameters.
 *
 * This function searches for a free GSP sequence within the provided GSP sequences
 * structure, marks it as pending, sets up a callback function with parameters for
 * the sequence, and returns the initialized sequence. If no free sequence is
 * available, it returns an error.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the mutex lock for the GSP sequences to ensure exclusive access using
 *    nvgpu_mutex_acquire().
 * -# Find the index of the first zero bit in the GSP sequence table which represents
 *    a free sequence using find_first_zero_bit() and cast the result to u32 using
 *    nvgpu_safe_cast_u64_to_u32().
 * -# Check if the index is within the range of maximum GSP sequences. If not, release
 *    the mutex lock using nvgpu_mutex_release(), set the error code to -EAGAIN, and
 *    exit the function.
 * -# Set the bit at the found index in the GSP sequence table to mark the sequence as
 *    acquired using nvgpu_set_bit().
 * -# Release the mutex lock for the GSP sequences using nvgpu_mutex_release().
 * -# Initialize the GSP sequence at the found index with the state set to pending,
 *    the provided callback, and callback parameters. The output payload pointer is
 *    set to NULL.
 * -# Set the provided GSP sequence pointer to point to the initialized sequence.
 *
 * @Param [in] g            Pointer to the GPU driver structure.
 * @Param [in] sequences    Pointer to the GSP sequences structure.
 * @Param [out] pseq        Pointer to the pointer of the GSP sequence that has been
 *                          acquired and initialized.
 * @Param [in] callback     Callback function to be associated with the acquired GSP
 *                          sequence.
 * @Param [in] cb_params    Pointer to the parameters to be passed to the callback
 *                          function.
 *
 * @return 0 on success, indicating that a GSP sequence has been successfully acquired
 *         and initialized.
 * @return -EAGAIN if no free GSP sequence is available.
 */
int nvgpu_gsp_seq_acquire(struct gk20a *g,
			struct gsp_sequences *sequences,
			struct gsp_sequence **pseq,
			gsp_callback callback, void *cb_params);

/**
 * @brief Handles the response for a GSP sequence and invokes the associated callback.
 *
 * This function retrieves the GSP sequence associated with the given sequence ID,
 * verifies its state, calls the registered callback function if present, and then
 * releases the sequence for future use. If the sequence is not in the expected state,
 * it logs an error and returns an invalid argument error code.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GSP sequence from the sequences array using the provided sequence ID.
 * -# Check if the state of the GSP sequence is not set to used. If it is not, log an
 *    error and return -EINVAL.
 * -# If a callback function is registered for the GSP sequence, call the callback
 *    function with the GPU driver structure, message, callback parameters, and a status
 *    code of 0.
 * -# Release the GSP sequence by calling gsp_seq_release() so it can be reused for
 *    other commands.
 *
 * @Param [in] g            Pointer to the GPU driver structure.
 * @Param [in] sequences    Pointer to the GSP sequences structure.
 * @Param [in] msg          Pointer to the message received from the GSP.
 * @Param [in] seq_id       ID of the GSP sequence for which the response is handled.
 *
 * @return 0 on successful handling of the response and invocation of the callback.
 * @return -EINVAL if the sequence state is not used or the sequence is unknown.
 */
int nvgpu_gsp_seq_response_handle(struct gk20a *g,
			struct gsp_sequences *sequences,
			struct nv_flcn_msg_gsp *msg, u32 seq_id);

/**
 * @brief Retrieves the ID of a GSP sequence.
 *
 * This function returns the ID field from the provided GSP sequence structure.
 * The ID is used to identify the sequence within the GSP sequences array.
 *
 * The steps performed by the function are as follows:
 * -# Return the ID of the GSP sequence.
 *
 * @Param [in] seq          Pointer to the GSP sequence structure from which to
 *                          retrieve the ID.
 *
 * @return The ID of the GSP sequence.
 */
u8 nvgpu_gsp_seq_get_id(struct gsp_sequence *seq);

/**
 * @brief Sets the state of a GSP sequence.
 *
 * This function assigns a new state to the provided GSP sequence. The state
 * indicates the current usage status of the sequence, such as whether it is free,
 * pending, or in use.
 *
 * The steps performed by the function are as follows:
 * -# Assign the provided state to the state field of the GSP sequence.
 *
 * @Param [in] seq          Pointer to the GSP sequence structure whose state is
 *                          to be set.
 * @Param [in] state        The new state to be assigned to the GSP sequence.
 */
void nvgpu_gsp_seq_set_state(struct gsp_sequence *seq,
			enum gsp_seq_state state);

/**
 * @brief Releases a GSP sequence and marks it as free.
 *
 * This function resets the state of a GSP sequence to free, clears its callback
 * and parameters, and updates the GSP sequence table to indicate the sequence is
 * available for reuse.
 *
 * The steps performed by the function are as follows:
 * -# Set the state of the GSP sequence to free, indicating it is no longer in use.
 * -# Clear the callback function pointer of the GSP sequence to NULL.
 * -# Clear the callback parameters pointer of the GSP sequence to NULL.
 * -# Clear the output payload pointer of the GSP sequence to NULL.
 * -# Acquire the mutex lock for the GSP sequences to ensure exclusive access using
 *    nvgpu_mutex_acquire().
 * -# Clear the bit in the GSP sequence table at the index corresponding to the GSP
 *    sequence's ID, marking it as available using nvgpu_clear_bit().
 * -# Release the mutex lock for the GSP sequences using nvgpu_mutex_release().
 *
 * @Param [in] sequences    Pointer to the GSP sequences structure containing the
 *                          sequence to be released.
 * @Param [in] seq          Pointer to the GSP sequence to be released.
 */
void gsp_seq_release(struct gsp_sequences *sequences,
			struct gsp_sequence *seq);
#endif /* NVGPU_GSP_SEQ_H */
