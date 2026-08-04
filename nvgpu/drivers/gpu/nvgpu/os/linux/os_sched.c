// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/os_sched.h>

#include <linux/sched.h>

int nvgpu_current_tid(struct gk20a *g)
{
	return current->pid;
}

int nvgpu_current_pid(struct gk20a *g)
{
	return current->tgid;
}

void nvgpu_print_current_impl(struct gk20a *g, const char *func_name, int line,
		void *ctx, enum nvgpu_log_type type)
{
	nvgpu_log_msg_impl(g, func_name, line, type, current->comm);
}

void nvgpu_get_thread_name(char *dest)
{
	char buf[TASK_COMM_LEN];

	get_task_comm(buf, current);
	strncpy(dest, buf, TASK_COMM_LEN);
	/* Ensure buffer is null terminated */
	dest[TASK_COMM_LEN-1] = '\0';
}
