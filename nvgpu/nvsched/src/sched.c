// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvs/log.h>
#include <nvs/nvs_sched.h>
#include <nvs/domain.h>

int nvs_sched_create(struct nvs_sched *sched,
		     struct nvs_sched_ops *ops, void *priv)
{
	int err;

	if (ops == NULL) {
		return -EINVAL;
	}

	(void)nvs_memset(sched, 0, sizeof(*sched));

	sched->ops  = ops;
	sched->priv = priv;

	sched->domain_list = nvs_malloc(sched, sizeof(*sched->domain_list));
	if (sched->domain_list == NULL) {
		return -ENOMEM;
	}

	(void)nvs_memset(sched->domain_list, 0, sizeof(*sched->domain_list));

	err = nvs_log_init(sched);
	if (err != 0) {
		nvs_free(sched, sched->domain_list);
		return err;
	}

	nvs_log_event_func(sched, NVS_EV_CREATE_SCHED, 0U);

	return 0;
}

void nvs_sched_close(struct nvs_sched *sched)
{
	nvs_domain_clear_all(sched);
	nvs_free(sched, sched->domain_list);
	nvs_log_destroy(sched);

	(void)nvs_memset(sched, 0, sizeof(*sched));
}
