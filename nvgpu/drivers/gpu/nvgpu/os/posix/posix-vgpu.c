// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/bug.h>
#include <nvgpu/vgpu/vgpu.h>
#include <nvgpu/vgpu/vgpu_ivc.h>
#include <nvgpu/nvgpu_ivm.h>
#include <nvgpu/vgpu/os_init_hal_vgpu.h>
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
#include <nvgpu/posix/posix-fault-injection.h>
#endif

#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
struct nvgpu_posix_fault_inj *nvgpu_ivm_reserve_get_fault_injection(void)
{
        struct nvgpu_posix_fault_inj_container *c =
                        nvgpu_posix_fault_injection_get_container();

        return &c->nvgpu_ivm_reserve_fi;
}

struct nvgpu_posix_fault_inj *nvgpu_ivm_unreserve_get_fault_injection(void)
{
        struct nvgpu_posix_fault_inj_container *c =
                        nvgpu_posix_fault_injection_get_container();

        return &c->nvgpu_ivm_unreserve_fi;
}

struct nvgpu_posix_fault_inj *nvgpu_ivm_map_get_fault_injection(void)
{
        struct nvgpu_posix_fault_inj_container *c =
                        nvgpu_posix_fault_injection_get_container();

        return &c->nvgpu_ivm_map_fi;
}

struct nvgpu_posix_fault_inj *nvgpu_nvlink_prot_get_fault_injection(void)
{
        struct nvgpu_posix_fault_inj_container *c =
                        nvgpu_posix_fault_injection_get_container();

        return &c->nvgpu_nvlink_prot_fi;
}

struct nvgpu_posix_fault_inj *nvgpu_ivm_unmap_get_fault_injection(void)
{
        struct nvgpu_posix_fault_inj_container *c =
                        nvgpu_posix_fault_injection_get_container();

        return &c->nvgpu_ivm_unmap_fi;
}
#endif

struct vgpu_priv_data *vgpu_get_priv_data(struct gk20a *g)
{
	(void)g;
	BUG();
	return NULL;
}

int vgpu_ivc_send(struct vgpu_ivc *ivc, void *data, u32 size)
{
	(void)ivc;
	(void)data;
	(void)size;
	return 0;
}

int vgpu_ivc_recv(struct vgpu_ivc *ivc, void *data, u32 size)
{
	(void)ivc;
	(void)data;
	(void)size;
	return 0;
}

int vgpu_ivc_init_support(struct gk20a *g)
{
	(void)g;
	return 0;
}

void vgpu_ivc_remove_support(struct gk20a *g)
{
	(void)g;
}

struct tegra_hv_ivm_cookie *nvgpu_ivm_mempool_reserve(unsigned int id)
{
	(void)id;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
static int temp_cookie;
if (nvgpu_posix_fault_injection_handle_call(
                nvgpu_ivm_reserve_get_fault_injection())) {
        return NULL;
} else {
/* This address will not be used. */
	return (struct tegra_hv_ivm_cookie *)&temp_cookie;
}
#endif
#else
        BUG();
#endif
	return NULL;
}

int nvgpu_ivm_mempool_unreserve(struct tegra_hv_ivm_cookie *cookie)
{
	(void)cookie;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
if (nvgpu_posix_fault_injection_handle_call(
                nvgpu_ivm_map_get_fault_injection())) {
        return -1;
} else {
	return 0;
}
#endif
#endif
	BUG();
	return 0;
}

u64 nvgpu_ivm_get_ipa(struct tegra_hv_ivm_cookie *cookie)
{
	(void)cookie;
	BUG();
	return 0ULL;
}

u64 nvgpu_ivm_get_size(struct tegra_hv_ivm_cookie *cookie)
{
	(void)cookie;
	BUG();
	return 0ULL;
}

void *nvgpu_ivm_mempool_map(struct tegra_hv_ivm_cookie *cookie)
{
	(void)cookie;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	static int temp;
if (nvgpu_posix_fault_injection_handle_call(
                nvgpu_ivm_map_get_fault_injection())) {
        return NULL;
} else {
/* This address will not be used */
	return (void *)&temp;
}
#endif
#else
        BUG();
#endif
	return NULL;
}

void nvgpu_ivm_mempool_unmap(struct tegra_hv_ivm_cookie *cookie,
		void *addr)
{
	(void)cookie;
	(void)addr;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
	return;
#endif
#else
	BUG();
#endif
}
int vgpu_init_hal_os(struct gk20a *g)
{
	(void)g;
	BUG();
	return -ENOSYS;
}

int nvgpu_nvlink_prot_update(struct nvgpu_contig_cbcmempool *contig_pool)
{

	(void)contig_pool;
#ifdef __NVGPU_UNIT_TEST__
#ifdef NVGPU_UNITTEST_FAULT_INJECTION_ENABLEMENT
if (nvgpu_posix_fault_injection_handle_call(
                nvgpu_nvlink_prot_get_fault_injection())) {
        return -1;
} else {
       return 0;
}
#endif
#else
        BUG();
#endif
	return 0;
}


