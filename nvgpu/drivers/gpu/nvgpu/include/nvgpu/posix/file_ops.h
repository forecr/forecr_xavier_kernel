/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_FILE_OPS_H
#define NVGPU_POSIX_FILE_OPS_H

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <nvgpu/types.h>

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
#include <nvgpu/posix/posix-fault-injection.h>
#endif

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_file_ops_get_fstat_injection(void);
struct nvgpu_posix_fault_inj *nvgpu_file_ops_get_fread_injection(void);
#endif

/**
 * @brief Get file status.
 *
 * The function shall obtain information about an open file associated with the
 * file descriptor \a fd. The \a buf argument is a pointer to a stat structure,
 * into which the information regarding the file is placed. Invokes the library
 * function \a fstat with \a fd and \a buf as parameters to fetch the required
 * information. Function does not perform any validation of the parameters.
 *
 * @param fd [in]	File descriptor.
 * @param buf [out]	Pointer to a buffer to store the status information.
 *
 * @return Upon successful completion, 0 shall be returned. Otherwise, -1 shall
 * be returned.
 *
 * @retval 0 on success.
 * @retval -1 on failure.
 */
int nvgpu_fstat(int fd, struct stat *buf);

/**
 * @brief Read bytes from file.
 *
 * The function attempts to read \a nbytes bytes from the file associated with
 * the open file descriptor, \a fildes, into the buffer pointed by \a buf.
 * Invokes the library function \a read with \a fildes, \a buf and \a nbytes as
 * parameters to read the data. Function does no perform any validation of the
 * parameters.
 *
 * @param fildes [in]	Descriptor of the file to read from.
 * @param buf [out]	Pointer to a buffer to store the read data.
 * @param nbytes [in]	Number of bytes to read.
 *
 * @return Number of bytes actually read, or -1.
 */
ssize_t nvgpu_fread(int fildes, void* buf, size_t nbytes);

/**
 * @brief Close a file.
 *
 * The function closes the file specified by the file descriptor \a fd. Invokes
 * the library function \a close with parameter \a fd to close the file.
 * Function does not perform any validation of the parameter.
 *
 * @param fd [in]	Descriptor of the file to close.
 */
void nvgpu_close(int fd);
#endif /* NVGPU_POSIX_FILE_OPS_H */

