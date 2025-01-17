// SPDX-License-Identifier: MIT
/*
 * Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
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

#include <tegra_hwpm_static_analysis.h>
#include <tegra_hwpm_mem_mgmt.h>
#include <tegra_hwpm_aperture.h>
#include <tegra_hwpm_common.h>
#include <tegra_hwpm_kmem.h>
#include <tegra_hwpm_log.h>
#include <tegra_hwpm_io.h>
#include <tegra_hwpm.h>

static int tegra_hwpm_element_reserve(struct tegra_soc_hwpm *hwpm,
	struct hwpm_ip_inst *ip_inst, struct hwpm_ip_aperture *element)
{
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	switch (element->element_type) {
	case HWPM_ELEMENT_PERFMON:
	case HWPM_ELEMENT_PERFMUX:
		err = tegra_hwpm_perfmon_reserve(hwpm, ip_inst, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x reserve failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case IP_ELEMENT_PERFMUX:
	case IP_ELEMENT_BROADCAST:
		err = tegra_hwpm_perfmux_reserve(hwpm, ip_inst, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x reserve failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case HWPM_ELEMENT_INVALID:
	default:
		tegra_hwpm_err(hwpm, "Invalid element type %d",
			element->element_type);
		return -EINVAL;
	}

fail:
	return err;
}

static int tegra_hwpm_element_enable(struct tegra_soc_hwpm *hwpm,
	struct hwpm_ip_aperture *element)
{
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	switch (element->element_type) {
	case HWPM_ELEMENT_PERFMON:
		err = hwpm->active_chip->perfmon_enable(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x disable failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case HWPM_ELEMENT_PERFMUX:
	case IP_ELEMENT_PERFMUX:
	case IP_ELEMENT_BROADCAST:
		/* Nothing to do here */
		break;
	case HWPM_ELEMENT_INVALID:
	default:
		tegra_hwpm_err(hwpm, "Invalid element type %d",
			element->element_type);
		return -EINVAL;
	}

fail:
	return err;
}

static int tegra_hwpm_element_disable(struct tegra_soc_hwpm *hwpm,
	struct hwpm_ip_aperture *element)
{
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	switch (element->element_type) {
	case HWPM_ELEMENT_PERFMON:
	case HWPM_ELEMENT_PERFMUX:
		err = hwpm->active_chip->perfmon_disable(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x disable failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case IP_ELEMENT_PERFMUX:
	case IP_ELEMENT_BROADCAST:
		err = hwpm->active_chip->perfmux_disable(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x disable failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case HWPM_ELEMENT_INVALID:
	default:
		tegra_hwpm_err(hwpm, "Invalid element type %d",
			element->element_type);
		return -EINVAL;
	}

fail:
	return err;
}

static int tegra_hwpm_element_release(struct tegra_soc_hwpm *hwpm,
	struct hwpm_ip_aperture *element)
{
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	switch (element->element_type) {
	case HWPM_ELEMENT_PERFMON:
	case HWPM_ELEMENT_PERFMUX:
		err = tegra_hwpm_perfmon_release(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x release failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case IP_ELEMENT_PERFMUX:
	case IP_ELEMENT_BROADCAST:
		err = tegra_hwpm_perfmux_release(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "Element mask 0x%x release failed",
				element->element_index_mask);
			goto fail;
		}
		break;
	case HWPM_ELEMENT_INVALID:
	default:
		tegra_hwpm_err(hwpm, "Invalid element type %d",
			element->element_type);
		return -EINVAL;
	}

fail:
	return err;
}

static void tegra_hwpm_free_dynamic_inst_array(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	u32 ip_idx, struct hwpm_ip *chip_ip)
{
	u32 a_type;

	tegra_hwpm_fn(hwpm, " ");

	/* Do this for all aperture types */
	for (a_type = 0U; a_type < TEGRA_HWPM_APERTURE_TYPE_MAX; a_type++) {
		struct hwpm_ip_inst_per_aperture_info *inst_a_info =
			&chip_ip->inst_aperture_info[a_type];

		if (inst_a_info->inst_arr == NULL) {
			tegra_hwpm_dbg(hwpm, hwpm_verbose,
				"No a_type = %d elements in IP %d",
				a_type, ip_idx);
			continue;
		}

		tegra_hwpm_kfree(hwpm, inst_a_info->inst_arr);
	}
}

static int tegra_hwpm_alloc_dynamic_inst_element_array(
	struct tegra_soc_hwpm *hwpm, struct hwpm_ip *chip_ip, u32 a_type)
{
	struct hwpm_ip_inst_per_aperture_info *inst_a_info =
			&chip_ip->inst_aperture_info[a_type];
	u64 ip_element_range = 0ULL;
	u32 idx;

	tegra_hwpm_fn(hwpm, " ");

	if (inst_a_info->range_end == 0ULL) {
		tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
			"No a_type = %d elements in IP", a_type);
		return 0;
	}

	ip_element_range = tegra_hwpm_safe_add_u64(
	tegra_hwpm_safe_sub_u64(inst_a_info->range_end,
		inst_a_info->range_start), 1ULL);
	inst_a_info->inst_slots = tegra_hwpm_safe_cast_u64_to_u32(
		ip_element_range / inst_a_info->inst_stride);

	inst_a_info->inst_arr = tegra_hwpm_kcalloc(
		hwpm, inst_a_info->inst_slots, sizeof(struct hwpm_ip_inst *));
	if (inst_a_info->inst_arr == NULL) {
		tegra_hwpm_err(hwpm, "a_type %d instance array alloc failed",
			a_type);
		return -ENOMEM;
	}

	for (idx = 0U; idx < inst_a_info->inst_slots; idx++) {
		inst_a_info->inst_arr[idx] = NULL;
	}

	tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
		"IP inst range(0x%llx-0x%llx) a_type = %d inst_slots %d",
		(unsigned long long)inst_a_info->range_start,
		(unsigned long long)inst_a_info->range_end,
		a_type, inst_a_info->inst_slots);

	return 0;
}

static int tegra_hwpm_alloc_dynamic_inst_array(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args, enum tegra_hwpm_funcs iia_func,
	u32 ip_idx, struct hwpm_ip *chip_ip)
{
	int err = 0;
	u32 a_type;

	tegra_hwpm_fn(hwpm, " ");

	/* Do this for all aperture types */
	for (a_type = 0U; a_type < TEGRA_HWPM_APERTURE_TYPE_MAX; a_type++) {
		err = tegra_hwpm_alloc_dynamic_inst_element_array(
			hwpm, chip_ip, a_type);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d a_type %d"
				"dynamic_inst_perfmon_array alloc failed",
				ip_idx, a_type);
			goto fail;
		}
	}

fail:
	return err;
}

static int tegra_hwpm_func_single_element(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx, struct hwpm_ip *chip_ip,
	u32 static_inst_idx, u32 a_type, u32 static_aperture_idx)
{
	int err = 0, ret = 0;
	struct hwpm_ip_inst *ip_inst =
		&chip_ip->ip_inst_static_array[static_inst_idx];
	struct hwpm_ip_element_info *e_info = &ip_inst->element_info[a_type];
	struct hwpm_ip_aperture *element =
		&e_info->element_static_array[static_aperture_idx];
	u64 element_offset = 0ULL;
	u32 idx = 0U;
	u32 reg_val = 0U;

	tegra_hwpm_fn(hwpm, " ");

	switch (iia_func) {
	case TEGRA_HWPM_INIT_IP_STRUCTURES:
		/* Compute element offset from element range start */
		element_offset = tegra_hwpm_safe_sub_u64(
			element->start_abs_pa, e_info->range_start);

		/* Compute element's slot index */
		idx = tegra_hwpm_safe_cast_u64_to_u32(
			element_offset / e_info->element_stride);

		tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
			"IP %d inst %d a_type %d element type %d"
			" start_addr 0x%llx static idx %d == dynamic idx %d",
			ip_idx, static_inst_idx, a_type,
			element->element_type, (unsigned long long)element->start_abs_pa,
			static_aperture_idx, idx);

		/* Set element slot pointer */
		e_info->element_arr[idx] = element;
		break;
	case TEGRA_HWPM_UPDATE_IP_INST_MASK:
		/* HWPM perfmuxes (PMA,RTR) can be assumed to be available */
		if (element->element_type == HWPM_ELEMENT_PERFMUX) {
			break;
		}

		/* Handle IPs with some value of fuse_fs_mask */
		if (ip_inst->fuse_fs_mask != 0U) {
			if ((element->element_index_mask &
				ip_inst->fuse_fs_mask) == 0U) {
				/* This element is floorswept */
				tegra_hwpm_dbg(hwpm, hwpm_dbg_floorsweep_info,
					"skip floorswept element 0x%llx",
					(unsigned long long)element->start_abs_pa);
				break;
			}
		}

		/* Validate perfmux availability by reading 1st alist offset */
		ret = tegra_hwpm_regops_readl(hwpm, ip_inst, element,
			tegra_hwpm_safe_add_u64(element->start_abs_pa,
				element->alist[0U].reg_offset), &reg_val);
		if (ret != 0) {
			/*
			 * If an IP element is unavailable, perfmux register
			 * read will return with failure.
			 * Mark corresponding element as unavailable.
			 * NOTE: This is possible for floorswept IP elements.
			 * Hence, failure should not be propagated.
			 */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_floorsweep_info,
				"perfmux start_abs_pa 0x%llx unavailable",
				(unsigned long long)element->start_abs_pa);

			ip_inst->element_fs_mask &=
				~(element->element_index_mask);
		} else {
			/* Update element mask in the instance */
			ip_inst->element_fs_mask |= element->element_index_mask;
		}
		break;
	case TEGRA_HWPM_GET_ALIST_SIZE:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_allowlist,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		if (element->alist) {
			hwpm->alist_map->full_alist_size =
				tegra_hwpm_safe_add_u64(
					hwpm->alist_map->full_alist_size,
					element->alist_size);
		} else {
			tegra_hwpm_err(hwpm, "IP %d"
				" element type %d static_idx %d NULL alist",
				ip_idx, a_type, static_aperture_idx);
		}
		break;
	case TEGRA_HWPM_COMBINE_ALIST:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_allowlist,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		err = hwpm->active_chip->copy_alist(hwpm,
			element, func_args->alist, &func_args->full_alist_idx);
		if (err != 0) {
			tegra_hwpm_err(hwpm,
				"IP %d element type %d static_idx %d"
				" alist copy failed",
				ip_idx, a_type, static_aperture_idx);
			return err;
		}
		break;
	case TEGRA_HWPM_RESERVE_GIVEN_RESOURCE:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_reserve_resource,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reservable",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		err = tegra_hwpm_element_reserve(hwpm, ip_inst, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d static_idx %d reserve failed",
				ip_idx, a_type, static_aperture_idx);
			goto fail;
		}
		break;
	case TEGRA_HWPM_RELEASE_RESOURCES:
	case TEGRA_HWPM_RELEASE_ROUTER:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		ret = tegra_hwpm_element_release(hwpm, element);
		if (ret != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d idx %d release failed",
				ip_idx, a_type, static_aperture_idx);
		}
		break;
	case TEGRA_HWPM_BIND_RESOURCES:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_bind,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		err = hwpm->active_chip->zero_alist_regs(
			hwpm, ip_inst, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d idx %d zero regs failed",
				ip_idx, a_type, static_aperture_idx);
			goto fail;
		}

		err = tegra_hwpm_element_enable(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d idx %d enable failed",
				ip_idx, a_type, static_aperture_idx);
			goto fail;
		}
		break;
	case TEGRA_HWPM_UNBIND_RESOURCES:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_bind,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}

		err = tegra_hwpm_element_disable(hwpm, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d idx %d enable failed",
				ip_idx, a_type, static_aperture_idx);
			goto fail;
		}

		err = hwpm->active_chip->zero_alist_regs(
			hwpm, ip_inst, element);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d element"
				" type %d idx %d zero regs failed",
				ip_idx, a_type, static_aperture_idx);
			goto fail;
		}
		break;
	case TEGRA_HWPM_RELEASE_IP_STRUCTURES:
		if ((element->element_index_mask &
			ip_inst->element_fs_mask) == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_release,
				"IP %d inst %d a_type %d element type %d"
				" start_addr 0x%llx not reserved",
				ip_idx, static_inst_idx, a_type,
				element->element_type,
				(unsigned long long)element->start_abs_pa);
			return 0;
		}
		break;
	default:
		tegra_hwpm_err(hwpm, "func 0x%x unknown", iia_func);
		return -EINVAL;
		break;
	}

	return 0;
fail:
	return err;
}

static int tegra_hwpm_func_all_elements_of_type(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx, struct hwpm_ip *chip_ip,
	u32 static_inst_idx, u32 a_type)
{
	u32 static_idx = 0U, idx = 0U;
	u64 inst_element_range = 0ULL;
	int err = 0;
	struct hwpm_ip_inst *ip_inst =
		&chip_ip->ip_inst_static_array[static_inst_idx];
	struct hwpm_ip_element_info *e_info = &ip_inst->element_info[a_type];

	tegra_hwpm_fn(hwpm, " ");

	if (iia_func == TEGRA_HWPM_INIT_IP_STRUCTURES) {
		if (e_info->num_element_per_inst == 0U) {
			/* no a_type elements in this IP */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
				"No a_type = %d elements in IP %d stat inst %d",
				a_type, ip_idx, static_inst_idx);
			return 0;
		}

		inst_element_range = tegra_hwpm_safe_add_u64(
			tegra_hwpm_safe_sub_u64(e_info->range_end,
				e_info->range_start), 1ULL);
		e_info->element_slots = tegra_hwpm_safe_cast_u64_to_u32(
			inst_element_range / e_info->element_stride);

		e_info->element_arr = tegra_hwpm_kcalloc(
			hwpm, e_info->element_slots,
			sizeof(struct hwpm_ip_aperture *));
		if (e_info->element_arr == NULL) {
			tegra_hwpm_err(hwpm,
				"a_type %d element array alloc failed", a_type);
			return -ENOMEM;
		}

		for (idx = 0U; idx < e_info->element_slots; idx++) {
			e_info->element_arr[idx] = NULL;
		}

		tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
			"iia_func %d IP %d static inst %d a_type %d"
			" element range(0x%llx-0x%llx) element_slots %d "
			"num_element_per_inst %d",
			iia_func, ip_idx, static_inst_idx, a_type,
			(unsigned long long)e_info->range_start,
			(unsigned long long)e_info->range_end,
			e_info->element_slots, e_info->num_element_per_inst);
	}

	if (iia_func == TEGRA_HWPM_UPDATE_IP_INST_MASK) {
		if (a_type != TEGRA_HWPM_APERTURE_TYPE_PERFMUX) {
			/* Only perfmuxes are essential for element_fs_mask */
			return 0;
		}
	}

	for (static_idx = 0U; static_idx < e_info->num_element_per_inst;
		static_idx++) {
		err = tegra_hwpm_func_single_element(
			hwpm, func_args, iia_func, ip_idx,
			chip_ip, static_inst_idx, a_type, static_idx);
		if (err != 0) {
			tegra_hwpm_err(hwpm,
				"IP %d inst %d a_type %d idx %d func %d failed",
				ip_idx, static_inst_idx, a_type,
				static_idx, iia_func);
			goto fail;
		}
	}

	if (iia_func == TEGRA_HWPM_RELEASE_IP_STRUCTURES) {
		if (e_info->element_arr != NULL) {
			tegra_hwpm_kfree(hwpm, e_info->element_arr);
		}
	}

	return 0;
fail:
	return err;
}

static int tegra_hwpm_func_all_elements(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx, struct hwpm_ip *chip_ip,
	u32 static_inst_idx)
{
	u32 a_type;
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	for (a_type = 0U; a_type < TEGRA_HWPM_APERTURE_TYPE_MAX; a_type++) {
		err = tegra_hwpm_func_all_elements_of_type(hwpm, func_args,
			iia_func, ip_idx, chip_ip, static_inst_idx, a_type);
		if (err != 0) {
			tegra_hwpm_err(hwpm,
				"IP %d inst %d a_type %d func %d failed",
				ip_idx, static_inst_idx, a_type, iia_func);
			goto fail;
		}
	}

	return 0;
fail:
	return err;
}

static int tegra_hwpm_func_single_inst(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx, struct hwpm_ip *chip_ip,
	u32 static_inst_idx)
{
	int err = 0;
	u32 a_type, idx = 0U;
	u64 inst_offset = 0ULL;
	struct hwpm_ip_inst *ip_inst =
		&chip_ip->ip_inst_static_array[static_inst_idx];
	struct hwpm_ip_inst_per_aperture_info *inst_a_info = NULL;
	struct hwpm_ip_element_info *e_info = NULL;

	tegra_hwpm_fn(hwpm, " ");

	if (iia_func == TEGRA_HWPM_INIT_IP_STRUCTURES) {
		for (a_type = 0U; a_type < TEGRA_HWPM_APERTURE_TYPE_MAX;
			a_type++) {
			inst_a_info = &chip_ip->inst_aperture_info[a_type];
			e_info = &ip_inst->element_info[a_type];

			if (inst_a_info->range_end == 0ULL) {
				tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
				"No a_type = %d elements in IP %d",
				a_type, ip_idx);
				continue;
			}

			/* Compute element offset from element range start */
			inst_offset = tegra_hwpm_safe_sub_u64(
				e_info->range_start, inst_a_info->range_start);

			/* Compute element's slot index */
			idx = tegra_hwpm_safe_cast_u64_to_u32(
				inst_offset / inst_a_info->inst_stride);

			tegra_hwpm_dbg(hwpm, hwpm_dbg_driver_init,
				"IP %d a_type %d inst range start 0x%llx"
				"element range start 0x%llx"
				" static inst idx %d == dynamic idx %d",
				ip_idx, a_type, (unsigned long long)inst_a_info->range_start,
				(unsigned long long)e_info->range_start, static_inst_idx, idx);

			/* Set perfmux slot pointer */
			inst_a_info->inst_arr[idx] = ip_inst;
		}
	}

	if ((iia_func == TEGRA_HWPM_RESERVE_GIVEN_RESOURCE) ||
		(iia_func == TEGRA_HWPM_UPDATE_IP_INST_MASK)) {
		if ((chip_ip->inst_fs_mask & ip_inst->hw_inst_mask) == 0U) {
			/* This instance is unavailable */
			return 0;
		}

		/* Disable IP power management */
		err = tegra_hwpm_ip_handle_power_mgmt(hwpm, ip_inst, true);
		if (err != 0) {
			tegra_hwpm_err(hwpm,
				"IP %d inst %d power mgmt disable failed",
				ip_idx, static_inst_idx);
			goto fail;
		}
	}

	/* Continue functionality for all apertures */
	err = tegra_hwpm_func_all_elements(hwpm, func_args, iia_func,
		ip_idx, chip_ip, static_inst_idx);
	if (err != 0) {
		tegra_hwpm_err(hwpm, "IP %d inst %d func 0x%x failed",
			ip_idx, static_inst_idx, iia_func);
		goto fail;
	}

	if (iia_func == TEGRA_HWPM_UPDATE_IP_INST_MASK) {
		if (ip_inst->element_fs_mask == 0U) {
			/* No element available in this inst */
			chip_ip->inst_fs_mask &= ~(ip_inst->hw_inst_mask);
		}
		if (chip_ip->inst_fs_mask == 0U) {
			/* No instance is available */
			chip_ip->resource_status =
				TEGRA_HWPM_RESOURCE_STATUS_INVALID;
		}
	}

	if ((iia_func == TEGRA_HWPM_RELEASE_RESOURCES) ||
		(iia_func == TEGRA_HWPM_UPDATE_IP_INST_MASK)) {
		/* Enable IP power management */
		err = tegra_hwpm_ip_handle_power_mgmt(hwpm, ip_inst, false);
		if (err != 0) {
			tegra_hwpm_err(hwpm,
				"IP %d inst %d power mgmt enable failed",
				ip_idx, static_inst_idx);
			goto fail;
		}
	}

fail:
	return err;
}

static int tegra_hwpm_func_all_inst(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx, struct hwpm_ip *chip_ip)
{
	int err = 0, ret = 0;
	u32 inst_idx = 0U;
	unsigned long reserved_insts = 0UL, idx = 0UL;

	tegra_hwpm_fn(hwpm, " ");

	for (inst_idx = 0U; inst_idx < chip_ip->num_instances; inst_idx++) {
		err = tegra_hwpm_func_single_inst(hwpm, func_args, iia_func,
			ip_idx, chip_ip, inst_idx);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d inst %d func 0x%x failed",
				ip_idx, inst_idx, iia_func);
			goto fail;
		}

		if (iia_func == TEGRA_HWPM_RESERVE_GIVEN_RESOURCE) {
			reserved_insts |= BIT(inst_idx);
		}
	}

	return 0;

fail:
	if (iia_func == TEGRA_HWPM_RESERVE_GIVEN_RESOURCE) {
		/* Revert previously reserved instances of this IP */
		for_each_set_bit(idx, &reserved_insts, 32U) {
			/* Release all apertures belonging to this instance */
			ret = tegra_hwpm_func_single_inst(hwpm, func_args,
				TEGRA_HWPM_RELEASE_RESOURCES,
				ip_idx, chip_ip, idx);
			if (ret != 0) {
				tegra_hwpm_err(hwpm,
					"IP %d inst %ld func 0x%x failed",
					ip_idx, idx,
					TEGRA_HWPM_RELEASE_RESOURCES);
			}
		}
	}
	/* Return previous error */
	return err;
}

int tegra_hwpm_func_single_ip(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func, u32 ip_idx)
{
	struct tegra_soc_hwpm_chip *active_chip = hwpm->active_chip;
	struct hwpm_ip *chip_ip = active_chip->chip_ips[ip_idx];
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	if (chip_ip == NULL) {
		tegra_hwpm_err(hwpm, "IP %d not populated", ip_idx);
		return -ENODEV;
	}

	switch (iia_func) {
	case TEGRA_HWPM_UPDATE_IP_INST_MASK:
		if (chip_ip->inst_fs_mask == 0U) {
			/* No available IP instances */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_floorsweep_info,
				"Chip IP %d not available", ip_idx);
			return 0;
		}
		break;
	case TEGRA_HWPM_GET_ALIST_SIZE:
	case TEGRA_HWPM_COMBINE_ALIST:
	case TEGRA_HWPM_BIND_RESOURCES:
	case TEGRA_HWPM_UNBIND_RESOURCES:
		/* Skip unavailable IPs */
		if (!chip_ip->reserved) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_allowlist | hwpm_dbg_bind,
				"Chip IP %d not reserved", ip_idx);
			return 0;
		}

		if (chip_ip->inst_fs_mask == 0U) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_allowlist | hwpm_dbg_bind,
				"Chip IP %d not available", ip_idx);
			return 0;
		}
		break;
	case TEGRA_HWPM_RESERVE_GIVEN_RESOURCE:
		/* Skip IPs which are already reserved */
		if (chip_ip->reserved) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_reserve_resource,
				"Chip IP %d already reserved", ip_idx);
			return 0;
		}

		/* Make sure IP override is not enabled */
		if (chip_ip->override_enable) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_reserve_resource,
				"Chip IP %d not available", ip_idx);
			return 0;
		}

		if (chip_ip->resource_status ==
			TEGRA_HWPM_RESOURCE_STATUS_INVALID) {
			/* No IP instance is available to reserve */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_reserve_resource,
				"Chip IP %d not available", ip_idx);
			return -EINVAL;
		}
		break;
	case TEGRA_HWPM_RELEASE_RESOURCES:
		if (ip_idx == active_chip->get_rtr_int_idx(hwpm)) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"Router will be released later");
			return 0;
		}
		/* Skip unavailable IPs */
		if (!chip_ip->reserved) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"Chip IP %d not reserved", ip_idx);
			return 0;
		}

		if (chip_ip->inst_fs_mask == 0U) {
			/* No IP instance is available to release */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"Chip IP %d not available", ip_idx);
			return 0;
		}
		break;
	case TEGRA_HWPM_RELEASE_ROUTER:
		/* Skip unavailable IPs */
		if (!chip_ip->reserved) {
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"Router not reserved");
			return 0;
		}

		if (chip_ip->inst_fs_mask == 0U) {
			/* No IP instance is available to release */
			tegra_hwpm_dbg(hwpm, hwpm_dbg_release_resource,
				"Router not available");
			return 0;
		}
		break;
	case TEGRA_HWPM_INIT_IP_STRUCTURES:
		err = tegra_hwpm_alloc_dynamic_inst_array(
			hwpm, func_args, iia_func, ip_idx, chip_ip);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d func 0x%x failed",
				ip_idx, iia_func);
			goto fail;
		}
		break;
	case TEGRA_HWPM_RELEASE_IP_STRUCTURES:
		break;
	default:
		tegra_hwpm_err(hwpm, "func 0x%x unknown", iia_func);
		goto fail;
		break;
	}

	/* Continue functionality for all instances in this IP */
	err = tegra_hwpm_func_all_inst(hwpm, func_args, iia_func,
		ip_idx, chip_ip);
	if (err != 0) {
		tegra_hwpm_err(hwpm, "IP %d func 0x%x failed",
			ip_idx, iia_func);
		goto fail;
	}

	/* Post execute functionality */
	if (iia_func == TEGRA_HWPM_RESERVE_GIVEN_RESOURCE) {
		chip_ip->reserved = true;
	}
	if ((iia_func == TEGRA_HWPM_RELEASE_RESOURCES) ||
		(iia_func == TEGRA_HWPM_RELEASE_ROUTER)) {
		chip_ip->reserved = false;
	}
	if (iia_func == TEGRA_HWPM_RELEASE_IP_STRUCTURES) {
		tegra_hwpm_free_dynamic_inst_array(hwpm, NULL, ip_idx, chip_ip);
	}

	return 0;
fail:
	return err;
}

int tegra_hwpm_func_all_ip(struct tegra_soc_hwpm *hwpm,
	struct tegra_hwpm_func_args *func_args,
	enum tegra_hwpm_funcs iia_func)
{
	struct tegra_soc_hwpm_chip *active_chip = hwpm->active_chip;
	u32 ip_idx;
	int err = 0;

	tegra_hwpm_fn(hwpm, " ");

	if (iia_func == TEGRA_HWPM_COMBINE_ALIST) {
		/* Start from zeroth index */
		func_args->full_alist_idx = 0ULL;
	}

	for (ip_idx = 0U; ip_idx < active_chip->get_ip_max_idx(hwpm);
		ip_idx++) {

		err = tegra_hwpm_func_single_ip(
			hwpm, func_args, iia_func, ip_idx);
		if (err != 0) {
			tegra_hwpm_err(hwpm, "IP %d func 0x%x failed",
				ip_idx, iia_func);
			return err;
		}
	}

	return 0;
}
