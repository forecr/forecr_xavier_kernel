// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvs/log.h>
#include <nvs/nvs_sched.h>
#include <nvs/impl-internal.h>

#define LOG_INC(value, max)					\
	do {							\
		(value) = nvgpu_safe_add_u32((value), 1U);	\
		if ((value) >= (max)) {				\
			(value) = 0U;				\
		}						\
	} while (false)

static bool nvs_log_full(struct nvs_log_buffer *logger)
{
	u32 updated_put = logger->put;

	LOG_INC(updated_put, logger->entries);

	/*
	 * If the next put is the same as get, then put has caught up to get,
	 * and the log is therefore full.
	 */
	return updated_put == logger->get;
}

int nvs_log_init(struct nvs_sched *sched)
{
	struct nvs_log_buffer *logger;

	logger = nvs_malloc(sched, sizeof(*logger));
	if (logger == NULL) {
		return -ENOMEM;
	}

	(void)nvs_memset(logger, 0, sizeof(*logger));

	logger->ts_offset = nvs_timestamp();
	logger->entries = NVS_LOG_ENTRIES;
	logger->events = nvs_malloc(sched,
				    NVS_LOG_ENTRIES * sizeof(*logger->events));
	if (logger->events == NULL) {
		nvs_free(sched, logger);
		return -ENOMEM;
	}

	(void)nvs_memset(logger->events, 0,
		   NVS_LOG_ENTRIES * sizeof(*logger->events));

	sched->logger = logger;

	return 0;
}

void nvs_log_destroy(struct nvs_sched *sched)
{
	nvs_free(sched, sched->logger->events);
	nvs_free(sched, sched->logger);
	sched->logger = NULL;
}

void nvs_log_event_func(struct nvs_sched *sched, enum nvs_event event, u32 data)
{
	struct nvs_log_buffer *logger = sched->logger;
	struct nvs_log_event *ev;

	nvs_log(sched, "ev: %d", event);
	nvs_log(sched, "  Starting: G=%05u P=%05u", logger->get, logger->put);

	/*
	 * If the log fills, just consume the oldest entry like with nvs_log_get().
	 *
	 * TODO: insert a "log overrun" entry, too, so readers will know.
	 */
	if (nvs_log_full(logger)) {
		nvs_log(sched, "Log full; killing entry.");
		LOG_INC(logger->get, logger->entries);
	}

	ev = &logger->events[logger->put];
	ev->data  = data;
	ev->event = event;
	ev->timestamp = nvs_timestamp() - logger->ts_offset;

	LOG_INC(logger->put, logger->entries);
	nvs_log(sched, "  New:      G=%05u P=%05u", logger->get, logger->put);
}

void nvs_log_get(struct nvs_sched *sched, struct nvs_log_event *ev)
{
	struct nvs_log_buffer *logger = sched->logger;

	nvs_log(sched, "Getting log event.");
	nvs_log(sched, "  Starting: G=%05u P=%05u", logger->get, logger->put);

	/*
	 * Check if the log is empty; if so, clear *ev to signal that.
	 */
	if (logger->get == logger->put) {
		ev->event = NVS_EV_NO_EVENT;
		nvs_log(sched, "  Log empty!");
		return;
	}

	*ev = logger->events[logger->get];
	LOG_INC(logger->get, logger->entries);

	nvs_log(sched, "  New:      G=%05u P=%05u", logger->get, logger->put);
}

const char *nvs_log_event_string(enum nvs_event ev)
{
	const char *log_event_string = NULL;

	switch (ev) {
	case NVS_EV_NO_EVENT:
		log_event_string = "No event";
		break;
	case NVS_EV_CREATE_SCHED:
		log_event_string = "Create scheduler";
		break;
	case NVS_EV_CREATE_DOMAIN:
		log_event_string = "Create domain";
		break;
	case NVS_EV_REMOVE_DOMAIN:
		log_event_string = "Remove domain";
		break;
	case NVS_EV_CTRL_QUEUE:
		log_event_string = "Control Queue";
		break;
	case NVS_EV_MAX:
		log_event_string = "Invalid MAX event";
		break;
	default:
		log_event_string = "Undefined event";
		break;
	}

	return log_event_string;
}
