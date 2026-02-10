/*
 * Copyright (c) 2022, NVIDIA CORPORATION.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NVS_CONTROL_INTERFACE_H
#define NVS_CONTROL_INTERFACE_H

/**
 * @file
 * @brief <b>NVIDIA GPU domain scheduler asynchronous messaging interface</b>
 */

#include "nvs-control-messages.h"

#if defined(__cplusplus)
#include <cstdint>
#elif !defined(__KERNEL__)
#include <stdint.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif


/**
 * @brief Value for nvs_domain_msg_fifo_control::get to indicate disabled
 * flow control.
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED (0xFFFFFFFFU)

/**
 * @brief Least significant bit of field PUT in nvs_domain_msg_fifo_control::put_revolutions
 */
#define NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_PUT_LSB (0U)

/**
 * @brief Most significant bit of field PUT in nvs_domain_msg_fifo_control::put_revolutions
 */
#define NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_PUT_MSB (31U)

/**
 * @brief Least significant bit of field REVOLUTIONS in nvs_domain_msg_fifo_control::put_revolutions
 */
#define NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_REVOLUTIONS_LSB (32U)

/**
 * @brief Most significant bit of field REVOLUTIONS in nvs_domain_msg_fifo_control::put_revolutions
 */
#define NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_REVOLUTIONS_MSB (63U)

/**
 * @brief The FIFO ring buffer control block
 *
 * FIFO ring buffer initialization
 * -------------------------------
 *
 * The FIFO ring buffer control block should be initialized on allocation as
 * follows:
 * - client --> scheduler:
 *   - all zeroes
 * - scheduler --> client:
 *   - nvs_domain_msg_fifo_control::get =
 *     #NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED.
 *   - everything else zeroes
 *
 * The FIFO size in entries is calculated as follows from the buffer size:
 *
 * @code
 * num_messages = (buffer_size - sizeof(nvs_domain_msg_fifo_control)) / sizeof(nvs_domain_message)
 * @endcode
 *
 * With the current data structure layouts, this formula with sizeof substitutions is as:
 *
 * @code
 * num_messages = (buffer_size - 128U) / 64U
 * @endcode
 *
 * For example, the number of messages in the FIFO is 1022 for a buffer of size 64K.
 *
 *
 * Adding a message in the FIFO ring buffer
 * ----------------------------------------
 *
 * The sender is suggested to maintain the following variables locally:
 * - @c local_put --- local copy of put index
 * - @c local_revolution_count --- local copy of revolution count
 * - @c local_num_dropped_messages --- local copy of number of dropped messages
 * - @c local_fifo_size_entries --- local copy of the FIFO ring buffer size
 *
 * When the sender is initialized, the local variables should be initialized
 * from the fields in #nvs_domain_msg_fifo_control, except for the FIFO size. The
 * FIFO size is received from nvgpu-rm as part of the physical buffer
 * parameters.
 *
 * The sequence for adding a message in the FIFO ring buffer is as follows:
 *
 *   1. Read nvs_domain_msg_fifo_control::get as @c get
 *   2. Determine whether there is space for the message. There are two cases:
 *      - @c get == #NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED: flow control disabled, space always available
 *      - otherwise: space is available when
 *        <tt>((local_put + 1) % local_fifo_size_entries) != get</tt>
 *   3. If out of space, then:
 *      - increment @c local_num_dropped_messages
 *      - write it to nvs_domain_msg_fifo_control::num_dropped_messages
 *      - do not proceed with this sequence
 *      - Notes:
 *        - scheduler --> client: nvs_domain_msg_fifo_control::num_dropped_messages
 *          indicates that messages were dropped. Logging an
 *          overflow event may be applicable.
 *        - client --> scheduler: The usermode client should return an error.
 *   4. Read-write memory barrier
 *   5. Write the message to nvs_domain_msg_fifo::messages[local_put]
 *      - NOTE: if the message size is less than the size of the array entry, it
 *        should be appended with 0 to fill the entry. This allows extending the
 *        messages with new fields in later protocol versions in a backwards
 *        compatible manner.
 *      - NOTE: this update does not need to be atomic
 *   6. Write memory barrier
 *   7. Increment @c local_put (modulo fifo size). In case of a wrap-around,
 *      increment also @c local_revolution_count
 *   8. Write <tt>local_put | (local_revolution_count << 32)</tt>
 *      to nvs_domain_msg_fifo_control::put_revolutions. This write
 *      should be atomic 64-bit write.
 *
 *
 * Receiving a message in the FIFO ring buffer (read-write access client)
 * ---------------------------------------------------------------
 *
 * The receiver is suggested to maintain the following variables locally:
 * - @c local_get --- local copy of get index
 * - @c local_fifo_size_entries --- local copy of the FIFO ring buffer size
 *
 * When the reader is initialized, it should:
 *
 *   1. atomic read nvs_domain_msg_fifo_control::put_revolutions and store the
 *      32-bit lower bits as @c local_get
 *   2. store @c local_get to nvs_domain_msg_fifo_control::get. This enables
 *      flow control.
 *
 * When the reader exits, it should write
 * #NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED in
 * nvs_domain_msg_fifo_control::get. This disables flow control. Further,
 * nvgpu-rm should also write
 * #NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED in
 * nvs_domain_msg_fifo_control::get when the R/W reader client exits. This
 * is to ensure that abnormal client exit (e.g., process crash) disables flow
 * control.
 *
 * The sequence for reading a message:
 *
 *   1. Atomic read the bottom 32 bits (or whole field) of
 *      nvs_domain_msg_fifo_control::put_revolutions as @c put
 *   2. If local_get == put, there are no more messages. Exit this
 *      sequence.
 *   3. Read memory barrier
 *   4. Read the message from nvs_domain_msg_fifo::messages[local_get]
 *      (non-atomic read ok)
 *   5. Read-write memory barrier
 *   6. Increment local_get (mod FIFO size)
 *   7. Atomic write nvs_domain_msg_fifo_control::get
 *
 *
 * Receiving a message in the FIFO ring buffer (read-only access client)
 * ---------------------------------------------------------------
 *
 * <b>NOTE:</b> The read-only reader client should not be used for
 * safety-critical operation. It does not have flow control and it is subject to
 * FIFO overruns. The read-only client is intended for diagnostics and tracing.
 *
 * The receiver is suggested to maintain the following variables locally:
 * - @c local_get --- local copy of get index
 * - @c local_revolutions --- local copy of FIFO revolutions
 * - @c local_fifo_size_entries --- local copy of the FIFO ring buffer size
 *
 * When the reader is initialized, it should:
 *
 *   1. atomic read nvs_domain_msg_fifo_control::put_revolutions and store the
 *      lower 32 bits as @c local_get, and the upper 32 bits as @c
 *      local_revolutions
 *
 * The sequence for reading a message:
 *
 *   1. Atomic read nvs_domain_msg_fifo_control::put_revolutions as @c put and
 *      @c revolutions
 *   2. Determine whether the reader is more than a full revolution behind the
 *      writer. One way to do this is to calculate the total number of messages
 *      read and written, and then calculate the cyclic difference of the
 *      totals.
 *      @code
 *      cycleSize = FIFO_entries * (1 << 32)  ; num messages until 'revolutions' wraps around
 *      messagesRead = local_get + (local_revolutions * FIFO_entries)
 *      messagesWritten = put + (revolutions * FIFO_entries)
 *      unreadMessages = (messagesWritten - messagesRead) mod cycleSize
 *      @endcode
 *      In case <tt>unreadMessages > FIFO_entries</tt>, a FIFO overrun has
 *      occurred. Note that in this formulation, care must be taken to avoid
 *      integer overflows during computation.
 *
 *      It is up to the implementation what to do on FIFO overflow. Possibly,
 *      report an error and reset <tt>local_revolutions = revolutions</tt> and
 *      <tt>local_get = @c put</tt>.
 *   3. If <tt>local_get == put</tt>, the are no unread messages. Exit this
 *      sequence.
 *   4. Read memory barrier
 *   5. Read the message from nvs_domain_msg_fifo::messages[local_get]
 *      (non-atomic read ok)
 *   6. Read memory barrier
 *   7. Atomic read nvs_domain_msg_fifo_control::put_revolutions as @c put and
 *      @c revolutions
 *   8. Perform the FIFO overrun check again as in step 2. In case overrun is
 *      detected, then the message read on step 5 may have been overwritten
 *      while reading.
 *
 *      It is up to the implementation what to do on FIFO overflow. Possibly,
 *      report an error and reset <tt>local_revolutions = revolutions</tt> and
 *      <tt>local_get = @c put</tt>.
 *   9. Increment <tt>local_get (mod FIFO_entries)</tt>. If there is a wrap-around,
 *      increment @c local_revolutions.
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_fifo_control {

	/**
	 * @brief Get index (updated by the RW consumer)
	 *
	 * @remark Special value
	 * #NVS_DOMAIN_MSG_FIFO_CONTROL_GET_FLOW_CTRL_DISABLED means no flow
	 * control (scheduler --> client buffers only)
	 *
	 * @since 1.0.0
	 */
	uint32_t get;

	/**
	 * @brief Padding to fill up 64B
	 */
	uint32_t reserved0[15];

	/**
	 * @brief Message put index and revolution count (updated by the producer)
	 *
	 * This member consists of two fields:
	 *
	 *   - The lower 32 bits is the put index
	 *   - The upper 32 bits is the revolution count, i.e., how many times the
	 *     put index has wrapped around
	 *
	 * For example, this field would be incremented as follows for a FIFO of size 5:
	 *
	 *   - 0x0000'0000'0000'0000 (initial value)
	 *   - 0x0000'0000'0000'0001
	 *   - 0x0000'0000'0000'0002
	 *   - 0x0000'0000'0000'0003
	 *   - 0x0000'0000'0000'0004
	 *   - 0x0000'0001'0000'0000 (wrap-around, revolution count incremented)
	 *   - 0x0000'0001'0000'0001
	 *
	 * The intention of the revolution count is to provide read-only
	 * observers a mechanism to detect dropped messages.
	 *
	 * @sa NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_PUT_LSB
	 * @sa NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_PUT_MSB
	 * @sa NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_REVOLUTIONS_LSB
	 * @sa NVS_DOMAIN_MSG_FIFO_CONTROL_PUT_REVOLUTIONS_REVOLUTIONS_MSB
	 *
	 * @since 1.0.0
	 */
	uint64_t put_revolutions;

	/**
	 * @brief Number of dropped messages due to overrun (updated by the
	 * producer)
	 *
	 * @since 1.0.0
	 */
	uint64_t num_dropped_messages; // number of lost messages due to buffer overrun

	/**
	 * @brief Padding to fill up 64B
	 */
	uint32_t reserved1[12];
};

#if (defined(__cplusplus) && (__cplusplus >= 201103L))
/* Double-check that the example in the documentation is correct */
static_assert(
	sizeof(nvs_domain_msg_fifo_control) == 128U,
	"Verify the documented substitution (1)");
static_assert(
	sizeof(nvs_domain_message) == 64U,
	"Verify the documented substitution (2)");
static_assert(
	(65536U - sizeof(nvs_domain_msg_fifo_control)) / sizeof(nvs_domain_message) == 1022U,
	"Verify the documented example");
#endif

/**
 * @brief The general FIFO ring buffer format
 *
 * The following FIFO ring buffers are specified:
 *
 * - client --> scheduler control request messages
 * - scheduler --> client control response messages
 *
 * The FIFO must be aligned by 64 bytes.
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_fifo {

	/**
	 * @brief Message ring buffer control
	 *
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_fifo_control control;

	/**
	 * @brief Message ring buffer data
	 *
	 * @since 1.0.0
	 */
	struct nvs_domain_message messages[];
};

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
