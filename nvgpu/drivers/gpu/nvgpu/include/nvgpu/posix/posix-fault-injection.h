/* SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2018-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_POSIX_FAULT_INJECTION_H
#define NVGPU_POSIX_FAULT_INJECTION_H

/**
 * State for fault injection instance.
 */
struct nvgpu_posix_fault_inj {
	bool enabled;
	unsigned int counter;
	unsigned long bitmask;
};

/**
 * Container for all fault injection instances.
 */
struct nvgpu_posix_fault_inj_container {
	/* nvgpu-core */
	struct nvgpu_posix_fault_inj thread_fi;
	struct nvgpu_posix_fault_inj thread_running_true_fi;
	struct nvgpu_posix_fault_inj thread_serial_fi;
	struct nvgpu_posix_fault_inj cond_fi;
	struct nvgpu_posix_fault_inj cond_broadcast_fi;
	struct nvgpu_posix_fault_inj map_return_fi;
	struct nvgpu_posix_fault_inj fstat_op;
	struct nvgpu_posix_fault_inj fread_op;
	struct nvgpu_posix_fault_inj kmem_fi;
	struct nvgpu_posix_fault_inj nvgpu_fi;
	struct nvgpu_posix_fault_inj golden_ctx_verif_fi;
	struct nvgpu_posix_fault_inj local_golden_image_fi;
	struct nvgpu_posix_fault_inj dma_fi;
	struct nvgpu_posix_fault_inj queue_out_fi;
	struct nvgpu_posix_fault_inj timers_fi;
	struct nvgpu_posix_fault_inj falcon_memcpy_fi;

	/* qnx */
	struct nvgpu_posix_fault_inj gk20a_get_comptags_fi;
	struct nvgpu_posix_fault_inj gk20a_get_comptags_2_fi;
	struct nvgpu_posix_fault_inj gk20a_get_comptags_3_fi;
	struct nvgpu_posix_fault_inj gk20a_comptags_start_clear_fi;
	struct nvgpu_posix_fault_inj NvEplInit_fi;
	struct nvgpu_posix_fault_inj nvdvms_set_process_state_fi;
	struct nvgpu_posix_fault_inj procmgr_ability_create_fi;
	struct nvgpu_posix_fault_inj NvTegraSysGetFuseOPT_info_fi;
	struct nvgpu_posix_fault_inj pthread_mutex_init_fi;
	struct nvgpu_posix_fault_inj mmap_fi;
	struct nvgpu_posix_fault_inj nvdvms_device_fi;
	struct nvgpu_posix_fault_inj nvdvms_device_reg_fail_fi;
	struct nvgpu_posix_fault_inj nvdvms_device_register_fi;
	struct nvgpu_posix_fault_inj NvRmMemMgrInit_fi;
	struct nvgpu_posix_fault_inj sem_wait_with_einval_errno_fi;
	struct nvgpu_posix_fault_inj sem_wait_with_eintr_errno_fi;
	struct nvgpu_posix_fault_inj usleep_fi;
	struct nvgpu_posix_fault_inj sdl_nvguard_fi;
	struct nvgpu_posix_fault_inj clock_gettime_fi;
	struct nvgpu_posix_fault_inj chip_id;
	struct nvgpu_posix_fault_inj chip_id_tegra14;
	struct nvgpu_posix_fault_inj nvdt_node_compatible;
	struct nvgpu_posix_fault_inj nvdt_read_prop_array_by_name_fi;
	struct nvgpu_posix_fault_inj nvdt_read_t194_prop_array_fi;
	struct nvgpu_posix_fault_inj nvdt_read_prop_array_by_index_var;
	struct nvgpu_posix_fault_inj nvdt_node_get_prop_fi;
	struct nvgpu_posix_fault_inj nvdt_node_prop_fi;
	struct nvgpu_posix_fault_inj node_get_prop_fi;
	struct nvgpu_posix_fault_inj nvdt_read_prop_array_by_address_fi;
	struct nvgpu_posix_fault_inj nvdt_prop_get_fi;
	struct nvgpu_posix_fault_inj nvdt_prop_fi;
	struct nvgpu_posix_fault_inj tegra_platform_null_fi;
	struct nvgpu_posix_fault_inj NvPowergatePowergatePartition_fi;
	struct nvgpu_posix_fault_inj NvPowergateUnpowergatePartition_fi;
	struct nvgpu_posix_fault_inj tegra_platform_fpga_fi;
	struct nvgpu_posix_fault_inj nvgpu_platform_simulation_var;
	struct nvgpu_posix_fault_inj nvdt_open_fi;
	struct nvgpu_posix_fault_inj check_os_native_var;
	struct nvgpu_posix_fault_inj get_chip_revision_var;
	struct nvgpu_posix_fault_inj nvdt_reg_prop_fi;
	struct nvgpu_posix_fault_inj nvdt_reg_prop_zero_fi;
	struct nvgpu_posix_fault_inj nvdt_reg_prop_size_zero_fi;
	struct nvgpu_posix_fault_inj nvmap_alloc_fi;
	struct nvgpu_posix_fault_inj nvgpu_invlaid_subctx_id_fi;
	struct nvgpu_posix_fault_inj nvgpu_nvhost_syncpt_read_ext_check_fi;
	struct nvgpu_posix_fault_inj shm_open_fi;
	struct nvgpu_posix_fault_inj shm_unlink_fi;
	struct nvgpu_posix_fault_inj shm_ctl_special_fi;
	struct nvgpu_posix_fault_inj shm_create_handle_fi;
	struct nvgpu_posix_fault_inj nvmap_physinfo_fi;
	struct nvgpu_posix_fault_inj nvmap_mmap_fi;
	struct nvgpu_posix_fault_inj nvmap_handleparams_fi;
	struct nvgpu_posix_fault_inj nvmap_enableevent_fi;
	struct nvgpu_posix_fault_inj nvmap_enableevent_free_fi;
	struct nvgpu_posix_fault_inj nvmap_useddesc_fi;
	struct nvgpu_posix_fault_inj nvmap_munmap_fi;
	struct nvgpu_posix_fault_inj nvmap_importid_fi;
	struct nvgpu_posix_fault_inj nvrm_reply_msg_fi;
	struct nvgpu_posix_fault_inj nvmap_unmap_user_mode_fi;
	struct nvgpu_posix_fault_inj nvmap_user_mode_fi;
	struct nvgpu_posix_fault_inj nvmap_user_mode_addr_fi;
	struct nvgpu_posix_fault_inj nvmap_user_mode_size_fi;
	struct nvgpu_posix_fault_inj nvgpu_hv_ipa_pa_var;
	struct nvgpu_posix_fault_inj nvgpu_hv_ipa_null_pa_var;
	struct nvgpu_posix_fault_inj nvgpu_io_map_var;
	struct nvgpu_posix_fault_inj nvgpu_readl_fi;
	struct nvgpu_posix_fault_inj nvgpu_readl_impl_fi;
	struct nvgpu_posix_fault_inj report_error_fi;
	struct nvgpu_posix_fault_inj nvhost1x_open_fi;
	struct nvgpu_posix_fault_inj nvhost1x_syncpointallocate_fi;
	struct nvgpu_posix_fault_inj nvhost1x_syncpointfree_fi;
	struct nvgpu_posix_fault_inj nvhost1x_getid_fi;
	struct nvgpu_posix_fault_inj nvhost1x_waiterallocate_fi;
	struct nvgpu_posix_fault_inj nvhost1x_syncpointread_fi;
	struct nvgpu_posix_fault_inj nvhost1x_creatememhandle_fi;
	struct nvgpu_posix_fault_inj nvhost1x_createfullhandle_fi;
	struct nvgpu_posix_fault_inj iofunc_devctl_default;
	struct nvgpu_posix_fault_inj iofunc_devctl_verify;
	struct nvgpu_posix_fault_inj iofunc_attr_lock;
	struct nvgpu_posix_fault_inj iofunc_attr_unlock;
	struct nvgpu_posix_fault_inj nv_iofunc_devctl_verify;
	struct nvgpu_posix_fault_inj resmgr_msgread_fi;
	struct nvgpu_posix_fault_inj resmgr_msgwrite_fi;
	struct nvgpu_posix_fault_inj resmgr_msgreply_fi;
	struct nvgpu_posix_fault_inj resmgr_attach_fi;
	struct nvgpu_posix_fault_inj dispatch_create_fi;
	struct nvgpu_posix_fault_inj thread_pool_create_fi;
	struct nvgpu_posix_fault_inj thread_pool_start_fi;
	struct nvgpu_posix_fault_inj resmgr_detach_fi;
	struct nvgpu_posix_fault_inj nvclock_device_clock_control_fi;
	struct nvgpu_posix_fault_inj nvclock_set_clock_freq_fi;
	struct nvgpu_posix_fault_inj nvclock_get_clock_freq_fi;
	struct nvgpu_posix_fault_inj nvclock_get_max_clock_freq_fi;
	struct nvgpu_posix_fault_inj nvdt_node_get_prop_clock_rates_fi;
	struct nvgpu_posix_fault_inj qnx_intattach_event_fi;
	struct nvgpu_posix_fault_inj qnx_int_wait_fi;
	struct nvgpu_posix_fault_inj guest_vm_id_fi;
	struct nvgpu_posix_fault_inj nvgpu_readl_loop_fi;
	struct nvgpu_posix_fault_inj tegra_sys_init_fi;
	struct nvgpu_posix_fault_inj waitfor_fi;
	struct nvgpu_posix_fault_inj procmgr_daemon_fi;
	struct nvgpu_posix_fault_inj procmgr_ability_fi;
	struct nvgpu_posix_fault_inj sem_init_fi;
	struct nvgpu_posix_fault_inj sem_timedwait_m_fi;
	struct nvgpu_posix_fault_inj sem_wait_fi;
	struct nvgpu_posix_fault_inj sem_post_fi;
	struct nvgpu_posix_fault_inj devctl_fi;
	struct nvgpu_posix_fault_inj devctl_ret_val_fi;
	struct nvgpu_posix_fault_inj nanospin_ns_fi;
	struct nvgpu_posix_fault_inj open_fi;
	struct nvgpu_posix_fault_inj close_fi;
	struct nvgpu_posix_fault_inj mprotect_fi;
	struct nvgpu_posix_fault_inj mprotect_text_fi;
	struct nvgpu_posix_fault_inj procmgr_ability_lookup_fi;
	struct nvgpu_posix_fault_inj pthread_sigmask_fi;
	struct nvgpu_posix_fault_inj pthread_sigaddset_fi;
	struct nvgpu_posix_fault_inj pthread_sigemptyset_fi;
	struct nvgpu_posix_fault_inj sigaction_fi;
	struct nvgpu_posix_fault_inj sigaction_execute_handler_fi;
	struct nvgpu_posix_fault_inj nvclock_get_device_clock_status_fi;
	struct nvgpu_posix_fault_inj nvclk_get_device_clock_status_fi;
	struct nvgpu_posix_fault_inj syncpt_alloc_buf_fi;
	struct nvgpu_posix_fault_inj nvgpu_dev_node_init_fi;
	struct nvgpu_posix_fault_inj nvgpu_tsg_invalid_chid;
	struct nvgpu_posix_fault_inj devctl_tsg_ioctl_qm_fi;
	struct nvgpu_posix_fault_inj nvgpu_devctl_verify_custom_ability_fi;
	struct nvgpu_posix_fault_inj nvgpu_devctl_verify_fi;
	struct nvgpu_posix_fault_inj nvgpu_vm_state_operational_check_fi;
	struct nvgpu_posix_fault_inj nvgpu_get_fi;
	struct nvgpu_posix_fault_inj nvepl_ss_status_fi;
	struct nvgpu_posix_fault_inj nvepl_status_fi;
	struct nvgpu_posix_fault_inj nvepl_error_fi;
	struct nvgpu_posix_fault_inj gk20a_as_release_share_fi;
	struct nvgpu_posix_fault_inj nvgpu_vm_unmap_fi;
	struct nvgpu_posix_fault_inj gk20a_as_alloc_share_fi;
	struct nvgpu_posix_fault_inj devctl_tsg_qm_fi;
	struct nvgpu_posix_fault_inj nvgpu_tsg_open_fi;
	struct nvgpu_posix_fault_inj create_sync_subcontext_fi;
	struct nvgpu_posix_fault_inj validate_ch_subctx_fi;
	struct nvgpu_posix_fault_inj bind_channel_fi;
	struct nvgpu_posix_fault_inj engine_get_fast_ce_fi;
	struct nvgpu_posix_fault_inj nvgpu_gk20a_busy_fi;
	struct nvgpu_posix_fault_inj nvgpu_is_enabled_fi;
	struct nvgpu_posix_fault_inj nvgpu_mdesc_length_fi;
	struct nvgpu_posix_fault_inj gk20a_ioctl_ctrl_fi;
	struct nvgpu_posix_fault_inj nvgpu_ptimer_scale_fi;
	struct nvgpu_posix_fault_inj nvgpu_checker_run_pass_fi;
	struct nvgpu_posix_fault_inj nvgpu_checker_run_fail_fi;
	struct nvgpu_posix_fault_inj nvgpu_checker_tsg_fi;
	struct nvgpu_posix_fault_inj dispatch_block_fi;
	struct nvgpu_posix_fault_inj pthread_setname_np_fi;
	struct nvgpu_posix_fault_inj iofunc_client_info_able_fi;
	struct nvgpu_posix_fault_inj iofunc_client_info_ability_fi;
	struct nvgpu_posix_fault_inj iofunc_client_info_pp_fi;
	struct nvgpu_posix_fault_inj nvdvms_resmgr_get_vm_state_fi;
	struct nvgpu_posix_fault_inj resmgr_get_vm_state_fi;
	struct nvgpu_posix_fault_inj nvgpu_channel_open;
	struct nvgpu_posix_fault_inj nvgpu_channel_open_chid_fi;
	struct nvgpu_posix_fault_inj nvgpu_gpfifo_params;
	struct nvgpu_posix_fault_inj nvgpu_gpfifo_alloc;
	struct nvgpu_posix_fault_inj nvgpu_runist_nv_domain_null;
	struct nvgpu_posix_fault_inj nvgpu_runlist_id_max;
	struct nvgpu_posix_fault_inj nvgpu_runlist_store_fi;
	struct nvgpu_posix_fault_inj nvgpu_tsg_from_ch;
	struct nvgpu_posix_fault_inj pthread_attr_setschedpolicy_fi;
	struct nvgpu_posix_fault_inj nvgpu_readl_mc_boot_fi;
	struct nvgpu_posix_fault_inj pthread_create_fi;
	struct nvgpu_posix_fault_inj pthread_cancel_fi;
	struct nvgpu_posix_fault_inj pthread_join_fi;
	struct nvgpu_posix_fault_inj pthread_join_err_fi;
	struct nvgpu_posix_fault_inj pthread_attr_init_fi;
	struct nvgpu_posix_fault_inj pthread_attr_setinheritsched_fi;
	struct nvgpu_posix_fault_inj pthread_attr_setschedparam_fi;
	struct nvgpu_posix_fault_inj pthread_attr_destroy_fi;
	struct nvgpu_posix_fault_inj dispatch_handler_fi;
	struct nvgpu_posix_fault_inj nvgpu_tsg_null;
	struct nvgpu_posix_fault_inj nvgpu_gr_ctx_mappings_get_subctx_mappings_fi;
	struct nvgpu_posix_fault_inj shm_revoke_retinvalid_errvalid_fi;
	struct nvgpu_posix_fault_inj shm_revoke_retinvalid_errinvalid_fi;
	struct nvgpu_posix_fault_inj nvgpu_sync_point_invalid_size;
	struct nvgpu_posix_fault_inj nvgpu_sync_point_get;
	struct nvgpu_posix_fault_inj nvgpu_sync_point_get_max;
	struct nvgpu_posix_fault_inj pthread_create_func_run_fi;
	struct nvgpu_posix_fault_inj nvgpu_mon_kzalloc_fi;
	struct nvgpu_posix_fault_inj nvgpu_checker_flag;
	struct nvgpu_posix_fault_inj calloc_fi;
	struct nvgpu_posix_fault_inj nvgpu_addr_table_deinit_fi;
	struct nvgpu_posix_fault_inj nvgpu_store_kernel_mapping_info_fi;
	struct nvgpu_posix_fault_inj nvgpu_pd_cache_get_nvgpu_mems_fi;
	struct nvgpu_posix_fault_inj nvgpu_deinit_page_table_checker_fi;
	struct nvgpu_posix_fault_inj get_attr_of_pte_entry_valevel_fi;
	struct nvgpu_posix_fault_inj get_attr_of_pte_entry_cpu_va_pde_fi;
	struct nvgpu_posix_fault_inj get_pa_of_pde_entry_valevel16_fi;
	struct nvgpu_posix_fault_inj get_pa_of_pde_entry_valevel8_fi;
	struct nvgpu_posix_fault_inj nvgpu_traverse_all_pg_level_next_lev_gpu_va_fi;
	struct nvgpu_posix_fault_inj nvgpu_traverse_all_pg_level_startbit_fi;
	struct nvgpu_posix_fault_inj nvgpu_verify_page_table_for_vm_fi;
	struct nvgpu_posix_fault_inj nvgpu_invalid_kernel_address_fi;
	struct nvgpu_posix_fault_inj nvgpu_deinit_page_table_checker_fault_fi;
	struct nvgpu_posix_fault_inj nvgpu_buffer_uniqueness_fault_fi;
	struct nvgpu_posix_fault_inj nvgpu_buffer_uniqueness_negative_fault_fi;
	struct nvgpu_posix_fault_inj nvgpu_verify_page_table_for_vm_curr_node_fi;
	struct nvgpu_posix_fault_inj nvgpu_page_table_pass_checker_fi;
	struct nvgpu_posix_fault_inj nvgpu_page_table_fail_checker_fi;
	struct nvgpu_posix_fault_inj nvgpu_hw_checker_fail_checker_fi;
	struct nvgpu_posix_fault_inj munmap_device_io_fi;
	struct nvgpu_posix_fault_inj nvgpu_hw_checker_fail_report_checker_fi;
	struct nvgpu_posix_fault_inj nvgpu_hw_checker_fail_overflow_checker_fi;
	struct nvgpu_posix_fault_inj nvgpu_dt_read_fi;
	struct nvgpu_posix_fault_inj nvgpu_sys_get_platform_quickturn_fi;
	struct nvgpu_posix_fault_inj nvgpu_platform_get_cpu_type_vdk_fi;
	struct nvgpu_posix_fault_inj nvgpu_ivm_reserve_fi;
	struct nvgpu_posix_fault_inj nvgpu_ivm_map_fi;
	struct nvgpu_posix_fault_inj ctag_fi;
	struct nvgpu_posix_fault_inj nvgpu_ivm_unmap_fi;
	struct nvgpu_posix_fault_inj nvgpu_nvlink_prot_fi;
	struct nvgpu_posix_fault_inj nvgpu_ivm_unreserve_fi;
	struct nvgpu_posix_fault_inj nvlinkSync_fi;
};

/**
 * @brief Initialize fault injection container for this thread.
 *
 * c [in]	Pointer to fault injection container to use in this thread.
 *
 * The fault injection container stores the fault injection state for all
 * the fault injection users. This container is in thread-local-storage and
 * must be initialized for a unit test thread and any child threads that need
 * to have the same fault injection state.
 */
void nvgpu_posix_init_fault_injection
				(struct nvgpu_posix_fault_inj_container *c);

/**
 * @brief Get the fault injection container for this thread.
 *
 * @return A pointer to the fault injection container in use for this thread.
 */
struct nvgpu_posix_fault_inj_container
	*nvgpu_posix_fault_injection_get_container(void);

/**
 * nvgpu_posix_enable_fault_injection - Enable/Disable fault injection for the
 *                                      object after @number calls to the
 *                                      module. This depends on
 *                                      nvgpu_posix_fault_injection_handle_call
 *                                      being called by each function in the
 *                                      module that can fault. Only routines
 *                                      that can fault will decrement the
 *                                      delay count.
 *
 * @fi - pointer to the fault_inj object.
 * @enable - true to enable.  false to disable.
 * @number - After <number> of calls to kmem alloc or cache routines, enable or
 *           disable fault injection. Use 0 to enable/disable immediately.
 */
void nvgpu_posix_enable_fault_injection(struct nvgpu_posix_fault_inj *fi,
					bool enable, unsigned int number);


/**
 * nvgpu_posix_is_fault_injection_triggered - Query if fault injection is
 *                                            currently enabled for the @fi
 *                                            object.
 *
 * @fi - pointer to the fault_inj object
 *
 * Returns true if fault injections are enabled.
 */
bool nvgpu_posix_is_fault_injection_triggered(struct nvgpu_posix_fault_inj *fi);

/**
 * nvgpu_posix_is_fault_injection_cntr_set - Query if fault injection is
 *                                           set to trigger in future for the
 *                                           @fi object.
 *
 * @fi - pointer to the fault_inj object
 *
 * Returns true if fault injection counter is non-zero.
 */
bool nvgpu_posix_is_fault_injection_cntr_set(struct nvgpu_posix_fault_inj *fi);

/**
 * nvgpu_posix_fault_injection_handle_call - Called by module functions to
 *                                           track enabling or disabling fault
 *                                           injections. Returns true if the
 *                                           module should return an error.
 *
 * @fi - pointer to the fault_inj object
 *
 * Returns true if the module should return an error.
 */
bool nvgpu_posix_fault_injection_handle_call(struct nvgpu_posix_fault_inj *fi);

/**
 * nvgpu_posix_set_fault_injection - Set fault injection bitmask for the given
 *				     object @fi with @bitmask and @number of
 * 				     times fi is needed. For example a bitmask
 * 				     0x12 and number as 6 will inject fault at
 * 				     2nd and 5th iteration. Currently it only
 * 				     supports upto 64 counter with bitmask. In
 *                                   future an array of bitmask can be passed
 *                                   and only implementation of this function
 *                                   need to be changed.
 *
 * @fi - pointer to the fault_inj object.
 * @bitmask - Call Interation to be faulted in bitmask format.
 * @number - Fault injection supported upto <number> count.
 */
void nvgpu_posix_set_fault_injection_bitmask(struct nvgpu_posix_fault_inj *fi,
	unsigned long *bitmask, unsigned int number);

/**
 * nvgpu_posix_fault_injection_handle_call - Reset the bitmask fi.
 *
 * @fi - pointer to the fault_inj object
 *
 */
void nvgpu_posix_reset_fault_injection_bitmask(struct nvgpu_posix_fault_inj *fi);

#endif /* NVGPU_POSIX_FAULT_INJECTION_H */
