/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright (c) 2022-2023, NVIDIA CORPORATION & AFFILIATES. All rights reserved. */

/*
 * Internal to nvscic2c-pcie module. This file is not supposed to be included
 * by any other external modules.
 */
#ifndef __MODULE_H__
#define __MODULE_H__

#include <linux/completion.h>
#include <linux/device.h>
#include <linux/types.h>
#include <linux/pci-epc.h>
#include <linux/pci-epf.h>

#include "common.h"

/* forward declaration.*/
struct device_node;
struct platform_device;

/*
 * Parameters for the nvscic2c-pcie module and it's endpoints.
 *
 * These are read-only for the rest of the nvscic2c-pcie module.
 */
struct driver_param_t {
	/* This is (nvscic2c-pcie) device tree node as found in device tree.*/
	struct device_node *of_node;

	/* driver mode as parsed from compatible string in device-tree.*/
	enum drv_mode_t drv_mode;

	/* nvscic2c-pcie device tree node.*/
	struct platform_device *pdev;

	/* host1x device node and pdev via phandle.*/
	struct device_node *host1x_np;
	struct platform_device *host1x_pdev;

	/* device node of pcie controller via phandle.*/
	struct device_node *edma_np;

	/* pci endpoint device id.*/
	u32 pci_dev_id;

	/* bar window size. - applicable only for epf.*/
	u32 bar_win_size;

	/* node information, Board+SoC Id.*/
	struct node_info_t local_node;
	struct node_info_t peer_node;

	/*
	 * Properties that each endpoint shall be configured with.
	 * These properties are populated from device tree node.
	 */
	u8 nr_endpoint;
	struct endpoint_prop_t {
		/* Endpoint ID as populated from device tree node. */
		u8 id;

		/*
		 * Human readable name of the endpoint - char dev node shall be
		 * instanced using this name.
		 */
		char name[NAME_MAX];

		/* Frames and per frame size.*/
		u8 nframes;
		u32 frame_sz;
	} endpoint_props[MAX_ENDPOINTS];
};

/* nvscic2c-pcie epf specific context. */
struct epf_context_t {
	bool notifier_registered;
	struct pci_epf_header header;
	struct pci_epf *epf;
	void *drv_ctx;
	struct work_struct initialization_work;
	struct work_struct deinitialization_work;
	atomic_t core_initialized;
	atomic_t epf_initialized;
	atomic_t shutdown_msg_received;
	wait_queue_head_t core_initialized_waitq;
};

/* nvscic2c-pcie epc specific context. */
struct epc_context_t {
	struct completion epf_ready_cmpl;
	struct completion epf_shutdown_cmpl;
	atomic_t aer_received;
};

/*
 * nvscic2c-pcie module context.
 * Contains all the information for all
 *  - Configuration parameters per device-tree.
 */
struct driver_ctx_t {
	/* driver mode as parsed from compatible string in device-tree.*/
	enum drv_mode_t drv_mode;
	char *drv_name;

	/* the configuration for module and it's endpoints.*/
	struct driver_param_t drv_param;

	/*
	 * Visible region to peer SoC for PCIe writes. In nvscic2c-pcie
	 * use-cases, it is backed by physical memory allocated for
	 * comm-channel, endpoints and stream-objs(mem and sync), etc.
	 *
	 * Peer's write lands here to be read by local/self.
	 */
	struct dma_buff_t self_mem;

	/*
	 * Point to peer's visible region for data-writes. This is a PCIe
	 * aperture which allows local/self to write into peer's memory.
	 */
	struct pci_aper_t peer_mem;

	/* pci-client abstraction handle.*/
	void *pci_client_h;

	/* comm-channel abstraction. */
	void *comm_channel_h;

	/* vmap abstraction, this can be moved within endpoints.*/
	void *vmap_h;

	/* tegra-pcie-edma module handle.*/
	void *edma_h;

	/* endpoint absraction handle.*/
	void *endpoints_h;

	/* DRV_MODE specific.*/
	struct epf_context_t *epf_ctx;
	struct epc_context_t *epc_ctx;

	/* peer cpu */
	enum peer_cpu_t  peer_cpu;

	/* IOVA alloc abstraction.*/
	struct iova_alloc_domain_t *ivd_h;
};

/*
 * Look-up device tree node for the compatible string. Check for the
 * pci-dev-id within the compatible node, if more than one such node found also
 * return error.
 */
int
dt_parse(u32 pci_dev_id, enum drv_mode_t drv_mode,
	 struct driver_param_t *drv_param);

/* Release any memory allocated during dt_parse().*/
int
dt_release(struct driver_param_t *drv_param);
#endif //__MODULE_H__
