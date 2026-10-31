// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/bug.h>
#include <nvgpu/log.h>

#include <nvgpu/posix/file_ops.h>
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
#include <nvgpu/posix/posix-fault-injection.h>
#endif


#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_file_ops_get_fstat_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
			nvgpu_posix_fault_injection_get_container();

	return &c->fstat_op;
}

struct nvgpu_posix_fault_inj *nvgpu_file_ops_get_fread_injection(void)
{
	struct nvgpu_posix_fault_inj_container *c =
			nvgpu_posix_fault_injection_get_container();

	return &c->fread_op;
}
#endif

int nvgpu_fstat(int fd, struct stat *buf)
{
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(
					nvgpu_file_ops_get_fstat_injection())) {
		return -1;
	}
#endif
	return fstat(fd, buf);
}

ssize_t nvgpu_fread(int fildes, void* buf, size_t nbytes)
{
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	if (nvgpu_posix_fault_injection_handle_call(
					nvgpu_file_ops_get_fread_injection())) {
		errno = -1;
		return -1;
	}
#endif
	return read(fildes, buf, nbytes);
}

void nvgpu_close(int fd)
{
	int ret;
	ret = close(fd);
	if (ret != 0) {
		nvgpu_err(NULL, "close() failed %d", ret);
	}
}
