// SPDX-License-Identifier: GPL-2.0-only OR MIT
// SPDX-FileCopyrightText: Copyright (c) 2023-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <nvgpu/gk20a.h>
#include <nvgpu/log.h>
#include <nvgpu/gsp.h>
#include <nvgpu/runlist.h>
#include <nvgpu/string.h>

#ifdef CONFIG_NVS_PRESENT
#include <nvgpu/nvs.h>
#endif

#include <nvgpu/gsp_sched.h>
#include <nvgpu/device.h>
#include <nvgpu/utils.h>
#include "gsp_runlist.h"

/**
 * @brief Updates the GSP runlist information with the current runlist domain data.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the IOVA (Input/Output Virtual Address) of the runlist from the
 *    runlist domain's hardware memory structure by calling nvgpu_mem_get_addr().
 * -# Retrieve the number of entries in the runlist domain's hardware memory structure.
 * -# Retrieve the aperture of the runlist domain's hardware memory structure using
 *    the GPU operations structure by calling gops_runlist.get_runlist_aperture().
 * -# Populate the GSP runlist information structure with the domain ID, runlist ID,
 *    aperture, lower and higher 32 bits of the runlist IOVA, number of entries, and
 *    set the runlist as valid.
 * -# Retrieve the device ID associated with the runlist using the GPU operations
 *    by calling nvgpu_runlist_get_device_id().
 * -# If retrieving the device ID fails, log an error.
 * -# Populate the GSP runlist information structure with the device ID.
 * -# Return the error code, which will be 0 if no errors occurred during the process.
 *
 * @param [in] g             The GPU structure.
 * @param [out] gsp_runlist  The GSP runlist information structure to be updated.
 * @param [in] rl            The runlist structure associated with the runlist domain.
 * @param [in] domain        The runlist domain containing the runlist data.
 *
 * @return 0 if the GSP runlist information is successfully updated.
 * @return Non-zero error code if retrieving the device ID fails.
 */
static int gsp_nvs_update_runlist_info(struct gk20a *g,
    struct nvgpu_gsp_runlist_info *gsp_runlist, struct nvgpu_runlist *rl,
    struct nvgpu_runlist_domain *domain)
{
    int err = 0;
    u64 runlist_iova = nvgpu_mem_get_addr(g, &domain->mem_hw->mem);
    u32 num_entries = domain->mem_hw->count;
    u32 aperture = g->ops.runlist.get_runlist_aperture(g, &domain->mem_hw->mem);
    u32 device_id = 0;

    nvgpu_gsp_dbg(g, " ");

    gsp_runlist->domain_id = u64_lo32(domain->domain_id);
    gsp_runlist->runlist_id = rl->id;
    gsp_runlist->aperture = aperture;
    gsp_runlist->runlist_base_lo = u64_lo32(runlist_iova);
    gsp_runlist->runlist_base_hi = u64_hi32(runlist_iova);
    gsp_runlist->num_entries = num_entries;
    gsp_runlist->is_runlist_valid = true;

    err = nvgpu_runlist_get_device_id(g, rl, &device_id);
    if (err != 0) {
        nvgpu_err(g, "updating engine ID to gsp runlist info failed");
    }
    gsp_runlist->device_id = nvgpu_safe_cast_u32_to_u8(device_id);
    return err;

}

#ifdef CONFIG_NVS_PRESENT

/**
 * @brief Retrieves and populates runlist information for a specific GSP domain.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the total number of runlists for the GPU using nvgpu_runlist_get_num_runlists().
 * -# Iterate over each runlist index up to the total number of runlists.
 *  - For each runlist:
 *    -# Obtain a pointer to the current GSP runlist information structure.
 *    -# Retrieve domain-specific information for the current runlist index using
 *       nvgpu_nvs_gsp_get_runlist_domain_info(), which includes the number of entries,
 *       the IOVA, and the aperture.
 *    -# If there is an error in retrieving domain-specific information, log the error
 *       and continue to the next runlist.
 *    -# Retrieve runlist-specific information, including the runlist ID and device ID,
 *       using nvgpu_runlist_get_runlist_info().
 *    -# If there is an error in retrieving runlist-specific information, log the error
 *      and continue to the next runlist.
 *    -# Update the GSP runlist information structure with the retrieved information,
 *       marking the runlist as valid and populating it with the aperture, device ID,
 *       domain ID, number of entries, and the lower and higher 32 bits of the runlist IOVA.
 * -# Return the error code, which will be 0 if all runlists are processed successfully.
 *
 * @param [in] g              The GPU structure.
 * @param [out] gsp_domain    The GSP domain info structure to be updated with runlist information.
 * @param [in] nvgpu_domain_id The domain ID for which runlist information is being retrieved.
 *
 * @return 0 if the runlist information is successfully retrieved and updated for all runlists.
 * @return Non-zero error code if retrieving domain-specific or runlist-specific information fails.
 */
static int gsp_nvs_get_runlist_info(struct gk20a *g, struct nvgpu_gsp_domain_info *gsp_domain,
        u64 nvgpu_domain_id)
{
    u32 num_runlists;
    int err = 0;
    u64 runlist_iova = 0;
    u32 num_entries = 0;
    u32 aperture = 0;
    u32 runlist_id;
    u8 device_id;
    u32 i;
    struct nvgpu_gsp_runlist_info *gsp_runlist;
    nvgpu_gsp_dbg(g, " ");

    num_runlists = nvgpu_runlist_get_num_runlists(g);
    for (i = 0; i < num_runlists; i++) {
        gsp_runlist = &gsp_domain->runlist_info[i];

        err = nvgpu_nvs_gsp_get_runlist_domain_info(g, nvgpu_domain_id, &num_entries,
                &runlist_iova, &aperture, i);
        if (err != 0) {
            nvgpu_err(g, "gsp error in getting domain info ID: %u", gsp_domain->domain_id);
            continue;
        }

        err = nvgpu_runlist_get_runlist_info(g, i, &runlist_id, &device_id);
        if( err != 0) {
            nvgpu_err(g, "gsp error in getting runlist info Index: %u", i);
            continue;
        }
        gsp_runlist->aperture = aperture;
        gsp_runlist->device_id = device_id;
        gsp_runlist->domain_id = gsp_domain->domain_id;
	gsp_runlist->is_runlist_valid = false;
        gsp_runlist->num_entries = num_entries;
        gsp_runlist->runlist_base_lo = u64_lo32(runlist_iova);
        gsp_runlist->runlist_base_hi = u64_hi32(runlist_iova);
        gsp_runlist->runlist_id = runlist_id;

    }
return err;
}

/**
 * @brief Retrieves domain information for the GSP and updates the GSP domain structure.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the GSP domain information, including the domain ID and timeslice in
 *    nanoseconds, using nvgpu_nvs_get_gsp_domain_info().
 * -# If an error occurs while retrieving the GSP domain information, log the error
 *    and exit the function.
 * -# Update the GSP domain structure with the retrieved domain ID and timeslice.
 *    Set the priority to 0 as a default value.
 * -# Retrieve the runlist information associated with the GSP domain using
 *    gsp_nvs_get_runlist_info().
 * -# If an error occurs while retrieving the runlist information, log the error
 *    and exit the function.
 * -# Return the error code, which will be 0 if no errors occurred during the process.
 *
 * @param [in] g              The GPU structure.
 * @param [in] nvgpu_domain_id The domain ID for which information is being retrieved.
 * @param [out] gsp_domain    The GSP domain structure to be updated with domain information.
 *
 * @return 0 if the GSP domain information is successfully retrieved and updated.
 * @return Non-zero error code if retrieving the GSP domain information or runlist information fails.
 */
static int gsp_nvs_get_domain_info(struct gk20a *g, u64 nvgpu_domain_id,
                                struct nvgpu_gsp_domain_info *gsp_domain)
{
    int err = 0;
    u32 domain_id = 0U;
    u32 timeslice_ns = 0U;

    nvgpu_gsp_dbg(g, " ");

    err = nvgpu_nvs_get_gsp_domain_info(g, nvgpu_domain_id,
            &domain_id, &timeslice_ns);
    if (err != 0) {
        nvgpu_err(g, "gsp domain info is NULL");
		goto exit;
    }

    gsp_domain->domain_id = domain_id;
    gsp_domain->priority = 0;
    gsp_domain->time_slicing = timeslice_ns;
    err = gsp_nvs_get_runlist_info(g, gsp_domain, nvgpu_domain_id);
    if (err != 0) {
        nvgpu_err(g, "copy of gsp runlist info failed");
        goto exit;
    }
exit:
    return err;
}
#endif

#ifdef CONFIG_NVS_PRESENT
/* this function adds nvs domain info to the gsp domain info containers */
int nvgpu_gsp_nvs_add_domain(struct gk20a *g, u64 nvgpu_domain_id)
{
    struct nvgpu_gsp_domain_info gsp_domain = { };
    int err = 0;

    nvgpu_gsp_dbg(g, " ");

    err = gsp_nvs_get_domain_info(g, nvgpu_domain_id, &gsp_domain);
    if (err != 0) {
        nvgpu_err(g, " gsp domain data copy to cmd buffer failed");
        goto exit;
    }

    err = nvgpu_gsp_sched_domain_add(g, &gsp_domain);
    if (err != 0) {
        nvgpu_err(g, "gsp add domain failed");
        goto exit;
    }

exit:
    return err;
}
#endif

/* function to request delete the domain by id */
int nvgpu_gsp_nvs_delete_domain(struct gk20a *g, u64 nvgpu_domain_id)
{
    int err = 0;
    nvgpu_gsp_dbg(g, " ");
    // request for deletion of the domain with id
    err = nvgpu_gsp_sched_domain_delete(g, u64_lo32(nvgpu_domain_id));
    if (err != 0) {
        nvgpu_err(g, "domain delete failed");
    }
    return err;
}

/* funtion to update the runlist domain of gsp */
int nvgpu_gps_sched_update_runlist(struct gk20a *g,
    struct nvgpu_runlist_domain *domain, struct nvgpu_runlist *rl)
{
    struct nvgpu_gsp_runlist_info gsp_runlist = { };
    int err = 0;

    nvgpu_gsp_dbg(g, " ");
    /* copy runlist data to cmd buffer */
    err = gsp_nvs_update_runlist_info(g, &gsp_runlist, rl, domain);
    if (err != 0){
        nvgpu_err(g, "gsp runlist update to cmd failed");
        goto exit;
    }

    err = nvgpu_gsp_sched_runlist_update(g, &gsp_runlist);
    if (err != 0) {
        nvgpu_err(g, "command buffer for runlist sent failed");
        goto exit;
    }

exit:
    return err;
}
