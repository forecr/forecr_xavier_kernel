// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <stdlib.h>
#include <stdbool.h>
#include <nvgpu/posix/posix-fault-injection.h>

_Thread_local struct nvgpu_posix_fault_inj_container *thread_fi;

void nvgpu_posix_init_fault_injection(struct nvgpu_posix_fault_inj_container *c)
{
	thread_fi = c;
}

struct nvgpu_posix_fault_inj_container
	*nvgpu_posix_fault_injection_get_container(void)
{
	return thread_fi;
}

void nvgpu_posix_enable_fault_injection(struct nvgpu_posix_fault_inj *fi,
					bool enable, unsigned int number)
{
	if (number == 0U) {
		fi->enabled = enable;
		fi->counter = 0U;
	} else {
		fi->enabled = !enable;
		fi->counter = number;
	}
}

bool nvgpu_posix_is_fault_injection_triggered(struct nvgpu_posix_fault_inj *fi)
{
	return fi->enabled;
}

bool nvgpu_posix_is_fault_injection_cntr_set(struct nvgpu_posix_fault_inj *fi)
{
	return (fi->counter > 0U);
}

void nvgpu_posix_set_fault_injection_bitmask(struct nvgpu_posix_fault_inj *fi,
	unsigned long *bitmask, unsigned int number)
{
/* TODO check for input validity */

	fi->bitmask = *bitmask;
	fi->counter = number;
}

void nvgpu_posix_reset_fault_injection_bitmask(struct nvgpu_posix_fault_inj *fi)
{
	fi->bitmask = 0UL;
	fi->counter = 0U;
}

/*
 * Return status of fault injection.
 * Decrement fault injection count for each call.
 */
bool nvgpu_posix_fault_injection_handle_call(struct nvgpu_posix_fault_inj *fi)
{
	bool current_state = fi->enabled;

	if (fi->bitmask != 0UL && fi->counter > 0U &&
		fi->counter <= sizeof(fi->bitmask) * 8) {
		fi->counter--;
		if ((1UL << fi->counter) & fi->bitmask) {
			return true;
		} else {
			return false;
		}
	}

	if (fi->counter > 0U) {
		fi->counter--;
		if (fi->counter == 0U) {
			/* for next time */
			fi->enabled = !fi->enabled;
		}
	}

	return current_state;
}
