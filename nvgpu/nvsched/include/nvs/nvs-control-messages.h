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

#ifndef NVS_CONTROL_MESSAGES_H
#define NVS_CONTROL_MESSAGES_H

/**
 * @file
 * @brief <b>NVIDIA GPU domain scheduler asynchronous message definitions</b>
 */

#if defined(__cplusplus)
#include <cstdint>
#elif !defined(__KERNEL__)
#include <stdint.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Communication error
 *
 * @sa nvs_domain_msg_ctrl_error_resp
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_TYPE_CTRL_ERROR (0U)

/**
 * @brief Unknown/undefined error
 *
 * @sa nvs_domain_msg_ctrl_error_resp::error_code
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_CTRL_ERROR_UNKNOWN (0U)

/**
 * @brief Unhandled message
 *
 * The scheduler received a message that it does not know how to handle.
 *
 * @sa nvs_domain_msg_ctrl_error_resp::error_code
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_CTRL_ERROR_UNHANDLED_MESSAGE (1U)

/**
 * @brief Communication error
 *
 * This control message is sent as a response to an erroneous request.
 *
 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_ERROR
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_ctrl_error_resp {

	/**
	 * @brief Communication error code
	 *
	 * See NVS_DOMAIN_MSG_CTRL_ERROR_* error codes.
	 *
	 * @since 1.0.0
	 */
	uint32_t error_code;
};

/**
 * @brief Scheduler capability query
 *
 * @sa nvs_domain_msg_ctrl_get_caps_req
 * @sa nvs_domain_msg_ctrl_get_caps_resp
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_TYPE_CTRL_GET_CAPS_INFO (1U)

/**
 * @brief Scheduler capability query request
 *
 * The client version is used by the scheduler to check the client
 * compatibility, following the semantic versioning specification. (See
 * #NVS_DOMAIN_SCHED_VERSION_MAJOR for details.)
 *
 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_GET_CAPS_INFO
 * @sa nvs_domain_msg_ctrl_get_caps_resp::client_version_status
 *
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_ctrl_get_caps_req {

	/**
	 * @brief Client major version
	 *
	 * @since 1.0.0
	 */
	uint8_t client_version_major;

	/**
	 * @brief Client minor version
	 *
	 * @since 1.0.0
	 */
	uint8_t client_version_minor;

	/**
	 * @brief Client patch level
	 *
	 * @since 1.0.0
	 */
	uint8_t client_version_patch;
};

/**
 * @brief Major version of the domain scheduler interface
 *
 * The versioning scheme follows the Semantic Versioning 2.0.0
 * specification. See https://semver.org/spec/v2.0.0.html
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_SCHED_VERSION_MAJOR	(1U)

/**
 * @brief Minor version of the domain scheduler interface
 *
 * The versioning scheme follows the Semantic Versioning 2.0.0
 * specification. See https://semver.org/spec/v2.0.0.html
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_SCHED_VERSION_MINOR	(0U)

/**
 * @brief Patch version of the domain scheduler interface
 *
 * The versioning scheme follows the Semantic Versioning 2.0.0
 * specification. See https://semver.org/spec/v2.0.0.html
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_SCHED_VERSION_PATCH	(0U)

/**
 * @brief Client version check failed
 *
 * The client should not proceed with scheduler communication. The
 * scheduler is not expected to be compatible.
 */
#define NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_FAILED (0U)

/**
 * @brief Client version check passed
 */
#define NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_OK     (1U)

/**
 * @brief Scheduler capability query response
 *
 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_GET_CAPS_INFO
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_ctrl_get_caps_resp {

	/**
	 * @brief Scheduler major version
	 *
	 * @since 1.0.0
	 */
	uint8_t sched_version_major;

	/**
	 * @brief Scheduler minor version
	 *
	 * @since 1.0.0
	 */
	uint8_t sched_version_minor;

	/**
	 * @brief Scheduler patch level
	 *
	 * @since 1.0.0
	 */
	uint8_t sched_version_patch;

	/**
	 * @brief Client version check status
	 *
	 * This field indicates the scheduler-side check for client
	 * version compatibility. In general, the version numbering
	 * scheme follows the semantic versioning specification (see
	 * #NVS_DOMAIN_SCHED_VERSION_MAJOR). Additionally, the scheduler is
	 * allowed to fail this check for known incompatible or otherwise bad
	 * versions of the client.
	 *
	 * In general:
	 * - When the client and scheduler major version numbers match,
	 *   #NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_OK
	 *   is returned.
	 * - When the client and scheduler major versions do not match,
	 *   #NVS_DOMAIN_MSG_CTRL_GET_CAPS_RESP_CLIENT_VERSION_STATUS_FAILED
	 *   is returned. Future note: If the client supports multiple
	 *   scheduler major versions, the client is allowed to send
	 *   #nvs_domain_msg_ctrl_get_caps_req again with a compatible
	 *   version number.
	 *
	 * @since 1.0.0
	 */
	uint8_t client_version_status;
};

/**
 * @brief Switch to another domain
 *
 * @sa nvs_domain_msg_ctrl_switch_domain_req
 * @sa nvs_domain_msg_ctrl_switch_domain_resp
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN (2U)

/**
 * @brief Pseudo-domain ID for all TSGs over all domains
 *
 * @sa nvs_domain_msg_ctrl_switch_domain_req::domain_id
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_CTRL_DOMAIN_ID_ALL (~(uint64_t)0U)

/**
 * @brief Scheduler domain switch request
 *
 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_ctrl_switch_domain_req {

	/**
	 * @brief Domain id
	 *
	 * @remark Domain id #NVS_DOMAIN_CTRL_DOMAIN_ID_ALL has a special meaning. This
	 * is a request to switch to a runlist that contains all TSGs in all domains.
	 *
	 * @since 1.0.0
	 */
	uint64_t domain_id;
};

/**
 * @brief Scheduler domain switch succeeded
 *
 * @sa nvs_domain_msg_ctrl_switch_domain_resp::status
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_SUCCESS (0U)

/**
 * @brief Scheduler domain switch response
 *
 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN
 *
 * @since 1.0.0
 */
struct nvs_domain_msg_ctrl_switch_domain_resp {

	/**
	 * @brief Domain switch status
	 *
	 * @remark The domain scheduler may return a status code that
	 * is not listed below. In this case, domain switch failed and
	 * the status code is returned for diagnostic and debugging
	 * purposes.
	 *
	 * @sa NVS_DOMAIN_MSG_TYPE_CTRL_SWITCH_DOMAIN_STATUS_SUCCESS
	 *
	 * @since 1.0.0
	 */
	uint8_t status;

	/**
	 * @brief Padding to ensure 8B alignment for the next field
	 * for 32/64-bit compatibility
	 */
	uint8_t reserved[7];

	/**
	 * @brief Time it took to preempt and switch to the new domain (nanoseconds)
	 *
	 * @since 1.0.0
	 */
	uint64_t switch_ns;
};


/**
 * @brief Maximum payload size for #nvs_domain_message.
 *
 * @sa nvs_domain_message::payload
 *
 * @since 1.0.0
 */
#define NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE (48U)


/**
 * @brief Payload union
 *
 * Union that contains all defined messages.
 *
 * @since 1.0.0
 */
union nvs_domain_msg_payload_union {
	/**
	 * @brief Control response message: communication error
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_ctrl_error_resp resp_error;

	/**
	 * @brief Control request message: get_caps
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_ctrl_get_caps_req req_get_caps;

	/**
	 * @brief Control response message: get_caps
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_ctrl_get_caps_resp resp_get_caps;

	/**
	 * @brief Control request message: switch_domain
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_ctrl_switch_domain_req req_switch_domain;

	/**
	 * @brief Control response message: switch_domain
	 * @since 1.0.0
	 */
	struct nvs_domain_msg_ctrl_switch_domain_resp resp_switch_domain;


	/**
	 * @brief Raw message data
	 *
	 * @since 1.0.0
	 */
	uint8_t raw_data[NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE];
};

#if (defined(__cplusplus) && (__cplusplus >= 201103L))
static_assert(
	sizeof(nvs_domain_msg_payload_union) == NVS_DOMAIN_MESSAGE_MAX_PAYLOAD_SIZE,
	"Check for expected payload size");
#endif

/**
 * @brief The generic NVS domain message envelope
 *
 * @since 1.0.0
 */
struct nvs_domain_message {

	/**
	 * @brief FIFO-specific message type
	 *
	 * Message type defines:
	 * - control messages: NVS_DOMAIN_MSG_TYPE_CTRL_*
	 *
	 * @since 1.0.0
	 */
	uint32_t type;

	/**
	 * @brief Client-provided message tag
	 *
	 * - **Request-response messages.** The client provides a tag
	 *   which the scheduler copies to the corresponding
	 *   response.
	 *
	 * @remark It is recommended that the client uses a sequence
	 * to generate the tags for easier request/response tracing.
	 *
	 * @since 1.0.0
	 */
	uint32_t sequence_tag;

	/**
	 * @brief Message timestamp (nanoseconds)
	 *
	 * client --> scheduler: host CPU time (CLOCK_MONOTONIC)
	 *
	 * scheduler --> client: scheduler local time
	 *
	 * @remark The timestamp field is primarily intended for
	 * message tracing purposes for allowing the reconstruction of
	 * the timeline of events. For operational purposes such as
	 * measuring the request/response round-trips as a health
	 * indicator, the usermode software should use local CPU
	 * host-side clock sampling, instead. The GSP clock is not
	 * guaranteed to use the same time domain with the host
	 * CPU. Further, drift may occur between the host CPU and GSP
	 * time domains.
	 *
	 * @since 1.0.0
	 */
	uint64_t timestamp_ns;

	/**
	 * @brief Payload union
	 *
	 * @since 1.0.0
	 */
	union nvs_domain_msg_payload_union payload;
};

#if defined(__cplusplus)
} // extern "C"
#endif

#endif
