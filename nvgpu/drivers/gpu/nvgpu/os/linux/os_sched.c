/*
 * Copyright (c) 2017-2023, NVIDIA CORPORATION.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

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
