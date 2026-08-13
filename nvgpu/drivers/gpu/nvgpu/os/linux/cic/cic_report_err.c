/*
 * Copyright (c) 2021-2022, NVIDIA Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <nvgpu/cic_mon.h>
#include <nvgpu/types.h>

#ifdef CONFIG_NVGPU_ENABLE_MISC_EC
#include <linux/tegra-epl.h>
#include <nvgpu/timers.h>
#include "os/linux/os_linux.h"
#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
#include <linux/tegra-hsierrrptinj.h>
#define NVGPU_FSI_REPORTER_ID 0x8016
#endif
#endif


struct gk20a;

int nvgpu_cic_mon_report_err_safety_services(struct gk20a *g, u32 err_id)
{
	int ret = 0;

#ifdef CONFIG_NVGPU_ENABLE_MISC_EC
	u32 ss_retries = 0U;
	bool ss_status;
	struct device *dev = dev_from_gk20a(g);

	if (g->enable_polling == true) {
		for (ss_retries = MAX_SS_RETRIES; ss_retries > 0U;
					ss_retries--) {
			ret = epl_get_misc_ec_err_status(dev,
						MISC_EC_SW_ERR_CODE_0,
						&ss_status);
			if (ret == 0) {
				if (ss_status == true) {
					/**
					 * Previously reported error is cleared.
					 */
					break;
				} else {
					nvgpu_info(g, "Polling is in progress");
					nvgpu_udelay(SS_WAIT_DURATION_US);
					continue;
				}
			} else if (ret == -ENODEV) {
				nvgpu_err(g, "Error reporting is not "
						"supported in this platform");
				ret = 0;
				return ret;
			} else {
				nvgpu_err(g, "Error reporting failed");
				return ret;
			}
		}
		if (ss_retries == 0U) {
			nvgpu_err(g, "Error reporting failed: previous"
					"error is not cleared after retries");
			ret = -1;
			return ret;
		}
	}

	/**
	 * Enable polling immediately after reporting of first error from boot.
	 */
	if (g->enable_polling == false) {
		g->enable_polling = true;
	}

	/**
	 * MISC_EC_SW_ERR_CODE_0 register has been allocated for NvGPU
	 * to report GPU HW errors to Safety_Services via MISC_EC interface.
	 */
	ret = epl_report_misc_ec_error(dev, MISC_EC_SW_ERR_CODE_0, err_id);
	if (ret == 0) {
		nvgpu_err(g, "Reported err_id(0x%x) to Safety_Services",
				err_id);
	} else if (ret == -ENODEV) {
		/**
		 * MISC_EC is not supported in L4T.
		 */
		ret = 0;
		nvgpu_err(g, "Error reporting is not supported in "
				"this platform");
	} else {
		nvgpu_err(g, "Error reporting to Safety_Services failed");
		nvgpu_err(g, "ret(%d). err_id(0x%x)", ret, err_id);
	}
#endif

	return ret;
}

#ifdef CONFIG_NVGPU_FSI_ERR_INJECTION
static int nvgpu_cic_mon_inject_err_fsi(uint32_t inst_id,
		struct epl_error_report_frame err_rpt_frame, void *data)
{
	struct gk20a *g = (struct gk20a *)data;
	int err = 0;

	/* Sanity check reporter_id */
	if (err_rpt_frame.reporter_id != NVGPU_FSI_REPORTER_ID) {
		nvgpu_err(g, "Invalid Input -> Reporter ID = %u",
					err_rpt_frame.reporter_id);
		return -EINVAL;
	}

	/* Sanity check inst_id */
	if (inst_id != 0U) {
		nvgpu_err(g, "Invalid Input -> instance ID = %u", inst_id);
		return -EINVAL;
	}

	err = nvgpu_cic_mon_report_err_safety_services(g,
					err_rpt_frame.error_code);
	if (err != 0) {
		nvgpu_err(g, "Error injection failed for err_id: %u",
					err_rpt_frame.error_code);
		return -EFAULT;
	}

	return err;
}

int nvgpu_cic_mon_reg_errinj_cb(struct gk20a *g)
{
	hsierrrpt_ipid_t ip_id = IP_GPU;
	unsigned int inst_id = 0U;

	/* Send NvGPU context which can be used during error injection */
	return  hsierrrpt_reg_cb(ip_id, inst_id, nvgpu_cic_mon_inject_err_fsi, g);
}

int nvgpu_cic_mon_dereg_errinj_cb(void)
{
	hsierrrpt_ipid_t ip_id = IP_GPU;
	unsigned int inst_id = 0U;

	return  hsierrrpt_dereg_cb(ip_id, inst_id);
}
#endif
