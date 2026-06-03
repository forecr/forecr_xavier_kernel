/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVGPU_NVS_H
#define NVGPU_NVS_H

/**
 * @file
 * @page nvs_sched_common nvs_scheduler
 *
 *
 * Introduction
 * ============
 * GPU domain scheduling provides a mechanism to implement temporal isolation
 * between sets of GPU time-slice groups. This is achieved by assigning every
 * GPU time-slice group to a scheduling domain and providing a mechanism to
 * control which domain is active at any given time. Domains and control fifo
 * are created/destroyed during init/deinit state on user request via the
 * devctls commands - @ref devctl_asil_b
 *
 * igpu_safety_scheduler also responsible for runlist submits on user's request.
 * nvgpu-rm loads igpu_safety_scheduler firmware onto the GSP microcontroller.
 * Refer @ref igpu_safety_scheduler for more details.
 *
 * Design
 * ======
 * Refer detail design at GIZA:NvGPU/SWAD/gpu-domain-scheduling
 *
 * External APIs
 * ===============================================
 *    + nvgpu_nvs_init()
 *    + nvgpu_nvs_remove_support()
 *    + nvgpu_nvs_open()
 *    + nvgpu_nvs_add_domain()
 *    + nvgpu_nvs_domain_by_id_locked()
 *    + nvgpu_nvs_get_shadow_domain_locked()
 *    + nvgpu_nvs_domain_by_id()
 *    + nvgpu_nvs_domain_by_name()
 *    + nvgpu_nvs_domain_get()
 *    + nvgpu_nvs_domain_put()
 *    + nvgpu_nvs_del_domain()
 *    + nvgpu_nvs_domain_count()
 *    + nvgpu_nvs_domain_get_name()
 *    + nvgpu_nvs_get_log()
 *    + nvgpu_nvs_print_domain()
 *    + nvgpu_nvs_gsp_get_runlist_domain_info()
 *    + nvgpu_nvs_get_gsp_domain_info()
 *    + nvgpu_nvs_gsp_usr_domain_present()
 *    + nvgpu_nvs_ctrl_fifo_reset_exclusive_user()
 *    + nvgpu_nvs_ctrl_fifo_reserve_exclusive_user()
 *    + nvgpu_nvs_ctrl_fifo_user_exists()
 *    + nvgpu_nvs_ctrl_fifo_is_exclusive_user()
 *    + nvgpu_nvs_ctrl_fifo_add_user()
 *    + nvgpu_nvs_ctrl_fifo_user_is_active()
 *    + nvgpu_nvs_ctrl_fifo_remove_user()
 *    + nvgpu_nvs_ctrl_fifo_create()
 *    + nvgpu_nvs_ctrl_fifo_destroy()
 *    + nvgpu_nvs_ctrl_fifo_idle()
 *    + nvgpu_nvs_ctrl_fifo_unidle()
 *    + nvgpu_nvs_ctrl_fifo_is_busy()
 *    + nvgpu_nvs_ctrl_fifo_get_queue()
 *    + nvgpu_nvs_ctrl_fifo_get_capabilities()
 *    + nvgpu_nvs_buffer_is_sendq_valid()
 *    + nvgpu_nvs_buffer_is_receiveq_valid()
 *    + nvgpu_nvs_buffer_is_eventq_valid()
 *    + nvgpu_nvs_buffer_alloc()
 *    + nvgpu_nvs_buffer_free()
 *    + nvgpu_nvs_ctrl_fifo_lock_queues()
 *    + nvgpu_nvs_ctrl_fifo_unlock_queues()
 *    + nvgpu_nvs_ctrl_fifo_queue_has_subscribed_users()
 *    + nvgpu_nvs_ctrl_fifo_user_subscribe_queue()
 *    + nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue()
 *    + nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue()
 *    + nvgpu_nvs_ctrl_fifo_erase_all_queues()
 *    + nvgpu_nvs_ctrl_fifo_erase_queue_locked()
 */

#ifdef CONFIG_NVS_PRESENT
#include <nvs/domain.h>
#endif

#include <nvgpu/atomic.h>
#include <nvgpu/lock.h>
#include <nvgpu/worker.h>
#include <nvgpu/timers.h>
#include <nvgpu/nvgpu_mem.h>
#include <nvgpu/nvs-control-interface-parser.h>

/*
 * Max size we'll parse from an NVS log entry.
 */
#define NVS_LOG_BUF_SIZE	128
/*
 * Keep it to page size for now. Can be updated later.
 */
#define NVS_QUEUE_DEFAULT_SIZE (64U * 1024U)

struct gk20a;
struct nvgpu_nvs_domain_ioctl;
struct nvgpu_runlist;
struct nvgpu_runlist_domain;
struct nvgpu_nvs_ctrl_queue;
struct nvgpu_nvs_domain_ctrl_fifo;
struct nvgpu_nvs_domain;

/**
 * @brief Manage Control-Fifo Capabilities
 *
 * This is a placeholder for storing different capabilities of the control-fifo.
 *
 * Current implementation includes the backend implementation of the scheduler.
 * NVGPU_NVS_DOMAIN_SCHED_KMD, NVGPU_NVS_DOMAIN_SCHED_GSP.
 */
struct nvs_domain_ctrl_fifo_capabilities {
	/* Store type of scheduler backend */
	uint8_t scheduler_implementation_hw;
};

/**
 * @brief A structure to characterise users of the control-fifo device node.
 *
 * Users refer to Usermode processes/threads that open the control-fifo device
 * node. Users of the control-fifo device nodes mainly fall into the following
 * two categories.
 *
 * Exclusive R/W user: A user who has exclusive access to the control-fifo
 * scheduling queues namely Send, Receive and Event Queues. Exclusive users
 * can open/close a combination of the above queues for scheduling purposes.
 *
 * Exclusive user controls the scheduling control-flow by writing commands into
 * the shared queues. An example of a typical flow is where a user writes
 * the Domain ID into the 'Send' queue and awaits for the scheduler to schedule the
 * domain and then write the response into the 'Receive' queue.
 *
 * Non-Exclusive R/O user: A user who acts as an observer and has access to
 * the Event queue only.
 *
 * Other fields exist that store other necessary metadata for each of the user.
 */
struct nvs_domain_ctrl_fifo_user {
	/**
	 * Flag to determine whether the user has write access.
	 * Only users having write access can be marked as exclusive user
	 */
	bool has_write_access;
	/**
	 * PID of the user. Used to prevent a given user from opening
	 * multiple instances of control-fifo device node.
	 */
	int pid;
	/** Mask of actively used queues */
	u32 active_used_queues;

	/**
	 * Used to hold the scheduler capabilities.
	 */
	struct nvs_domain_ctrl_fifo_capabilities capabilities;
	/*
	 * Listnode used for keeping references to the user in
	 * the master struct nvgpu_nvs_domain_ctrl_fifo
	 */
	struct nvgpu_list_node sched_ctrl_list;
};

static inline struct nvs_domain_ctrl_fifo_user *
nvs_domain_ctrl_fifo_user_from_sched_ctrl_list(struct nvgpu_list_node *node)
{
	return (struct nvs_domain_ctrl_fifo_user *)
		((uintptr_t)node - offsetof(struct nvs_domain_ctrl_fifo_user, sched_ctrl_list));
};

/**
 * @brief A wrapper for nvs_domain. This is the front facing object for NVGPU-KMD's
 * domain management code. This manages the lifecycle of one instance of
 * struct nvgpu_runlist_domain per engine and maintains its links with each engine's
 * struct nvgpu_runlist.
 *
 * This structure also maintains a link with nvs_domain via a parent node.
 */
struct nvgpu_nvs_domain {
	u64 id;

	/**
	 * Subscheduler ID to define the scheduling within a domain. These will
	 * be implemented by the kernel as needed. There'll always be at least
	 * one, which is the host HW built in round-robin scheduler.
	 */
	u32 subscheduler;

	/**
	 * Convenience pointer for linking back to the parent object.
	 */
	struct nvs_domain *parent;

	/**
	 * Domains are dynamically used by their participant TSGs and the
	 * runlist HW. A refcount prevents them from getting prematurely freed.
	 *
	 * This is not the usual refcount. The primary owner is userspace via the
	 * ioctl layer and a TSG putting a ref does not result in domain deletion.
	 */
	u32 ref;
	/**
	 * Userspace API on the device nodes.
	 */
	struct nvgpu_nvs_domain_ioctl *ioctl;

	/**
	 * One corresponding to every runlist
	 */
	struct nvgpu_runlist_domain **rl_domains;
};

#define NVS_WORKER_STATE_STOPPED 0
#define NVS_WORKER_STATE_RUNNING 1
#define NVS_WORKER_STATE_SHOULD_PAUSE 2
#define NVS_WORKER_STATE_PAUSED 3
#define NVS_WORKER_STATE_SHOULD_RESUME 4

/**
 * @brief A central structure to manage the CPU based
 * worker thread for KMD scheduling.
 */
struct nvgpu_nvs_worker {
	nvgpu_atomic_t nvs_sched_state;
	struct nvgpu_cond wq_request;
	struct nvgpu_worker worker;
	struct nvgpu_timeout timeout;
	u32 current_timeout;
};

/**
 * @brief A central structure one each for a given GPU instance.
 * This structure contains instances corresponding to worker, top
 * level scheduler object, active domain and shadow_domain.
 *
 * This object stores a global 64 bit id_counter per GPU instance for
 * allocating IDs to domains incrementally.
 */
struct nvgpu_nvs_scheduler {
	/**
	 * @brief Instance of the top level scheduler object.
	 *
	 */
	struct nvs_sched *sched;

	/**
	 * @brief 64 bit based atomic counter for managing domain IDs for userspace.
	 * Upon a userspace request, a new domain is created and one 64bit integer
	 * is allocated for the domain ID.
	 */
	nvgpu_atomic64_t id_counter;
	struct nvgpu_nvs_worker worker;
	struct nvgpu_nvs_domain *active_domain;

	/**
	 * @brief An instance of shadow domain object maintained by NVGPU.
	 *
	 */
	struct nvgpu_nvs_domain *shadow_domain;
};

/**
 * @brief Describes the type of queue.
 *
 * NVGPU_NVS_NUM_CONTROL indicates Send/Receive queue.
 * These queues are meant for Control-Fifo operations such
 * as Handshaking, Scheduling, Control-Flow etc.
 *
 * NVGPU_NVS_NUM_EVENT indicates Event queue. Event queues
 * are meant for generating events such as Recovery, Preemption
 * etc.
 */
enum nvgpu_nvs_ctrl_queue_num {
	NVGPU_NVS_NUM_CONTROL = 0,
	NVGPU_NVS_NUM_EVENT,
	NVGPU_NVS_INVALID,
};

/**
 * @brief Describes the direction of a queue.
 *
 * The direction is described in terms of CLIENT
 * and SCHEDULER.
 *
 * NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER indicates that client is a producer
 * and scheduler is a consumer e.g. Send Queue
 *
 * NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT indicates that client is a consumer
 * and scheduler is a producer e.g. Receive Queue/Event Queue
 */
enum nvgpu_nvs_ctrl_queue_direction {
	NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER = 0,
	NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT,
	NVGPU_NVS_DIR_INVALID,
};

/*
 * The below definitions mirror the nvgpu-nvs(UAPI)
 * headers.
 */

/*
 * Invalid domain scheduler.
 * The value of 'domain_scheduler_implementation'
 * when 'has_domain_scheduler_control_fifo' is 0.
 */
#define NVGPU_NVS_DOMAIN_SCHED_INVALID 0U
/*
 * CPU based scheduler implementation. Intended use is mainly
 * for debug and testing purposes. Doesn't meet latency requirements.
 * Implementation will be supported in the initial versions and eventually
 * discarded.
 */
#define NVGPU_NVS_DOMAIN_SCHED_KMD 1U
/*
 * GSP based scheduler implementation that meets latency requirements.
 * This implementation will eventually replace NVGPU_NVS_DOMAIN_SCHED_KMD.
 */
#define NVGPU_NVS_DOMAIN_SCHED_GSP 2U

/* Queue meant for exclusive client write access. This shared queue will be
 * used for communicating the scheduling metadata between the client(producer)
 * and scheduler(consumer).
 */
#define NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_WRITE 1U

/* Queue meant for exclusive client read access. This shared queue will be
 * used for communicating the scheduling metadata between the scheduler(producer)
 * and client(consumer).
 */
#define NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_READ 2U

/* Queue meant for generic read access. Clients can subscribe to this read-only
 * queue for processing events such as recovery, preemption etc.
 */
#define NVGPU_NVS_CTRL_FIFO_QUEUE_CLIENT_EVENTS_READ 4U

/* Structure to hold control_queues. This can be then passed to GSP or Rm based subscheduler. */
struct nvgpu_nvs_ctrl_queue {
	struct nvgpu_mem	mem;
	struct gk20a		*g;
	/*
	 * Filled in by each OS - this holds the necessary data to export this
	 * buffer to userspace.
	 */
	void			*priv;
	bool			valid;
	u8				mask;
	u8				ref;
	enum nvgpu_nvs_ctrl_queue_direction		queue_direction;
	void (*free_queue)(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue,
			   bool is_abrupt_close);
};

#ifdef CONFIG_NVS_PRESENT
/**
 * @brief This API is used to initialize NVS services
 *
 * Initializes \a g->sched_mutex and invokes \a nvgpu_nvs_open
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 *
 * @retval 0 on success.
 * @retval failure codes of \a nvgpu_nvs_open
 */
int nvgpu_nvs_init(struct gk20a *g);

/**
 * @brief Initialize the NVS for the GPU.
 *
 * The function performs initialization of the NVS by allocating and setting up
 * the scheduler structure, creating a control FIFO, and generating a shadow
 * domain. It also handles the resumption from a low power state (railgate) by
 * re-adding the shadow domain for GSP (GPU Scheduler Processor) based
 * scheduling if enabled.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the scheduler mutex using nvgpu_mutex_acquire() to ensure exclusive access
 *    to the scheduler during initialization.
 * -# If the scheduler is already initialized, it indicates resuming from
 *    railgate. In this case, if GSP scheduling is supported and enabled, using
 *    nvgpu_gsp_nvs_add_domain() add the shadow domain for GSP scheduling and release
 *    the mutex using nvgpu_mutex_release().
 * -# If the scheduler is not initialized, allocate memory for the scheduler
 *    structure using nvgpu_kzalloc().
 * -# Initialize the domain ID counter to zero using nvgpu_atomic64_set() and using
 *    nvgpu_smp_wmb() write memory barrier to ensure the write is seen by other processors.
 * -# If NVS is supported and enabled, create the control FIFO for NVS using nvgpu_nvs_ctrl_fifo_create().
 * -# Using nvgpu_kzalloc() allocate memory for the NVS scheduler within the scheduler structure.
 * -# Using nvs_sched_create() create the NVS scheduler and generate the shadow domain
 *    using nvgpu_nvs_gen_shadow_domain().
 * -# Using nvgpu_mutex_release() release the scheduler mutex and return success
 *    if all steps are completed without errors.
 * -# If any step fails, perform cleanup by freeing allocated memory using nvgpu_kfree(),
 *    destroying the control FIFO using nvgpu_nvs_ctrl_fifo_destroy() if created,
 *    and releasing the scheduler mutex using nvgpu_mutex_release().
 *
 * @param [in] g - The GPU super structure.
 *
 * @return 0 on success, indicating NVS is initialized or resumed properly.
 * @return -ENOMEM if memory allocation fails at any step.
 * @return Error code from nvgpu_gsp_nvs_add_domain() or nvgpu_nvs_gen_shadow_domain()
 *         if those operations fail.
 */
int nvgpu_nvs_open(struct gk20a *g);

/**
 * @brief Remove support for NVS.
 *
 * 1) Erase all existing struct nvgpu_dom in NVS.
 * 2) Erase the Shadow Domain.
 * 3) Release the metadata required for NVS.
 * 4) Remove Control Fifo support if its enabled.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_remove_support(struct gk20a *g);


/**
 * @brief Retrieves the latest log event from the NVS scheduler.
 *
 * This function fetches the most recent log event from the NVS scheduler and
 * provides the timestamp and message associated with the event. If there is no
 * event, it sets the timestamp to 0 and the message to NULL.
 *
 * The steps performed by the function are as follows:
 * -# Call nvs_log_get() to retrieve the latest log event from the NVS scheduler.
 * -# If the retrieved event indicates that there is no event, set the timestamp
 *    to 0 and the message to NULL, then return from the function.
 * -# If there is an event, call nvs_log_event_string() to convert the event
 *    identifier to a human-readable message.
 * -# Set the output message pointer to the human-readable message.
 * -# Set the output timestamp to the timestamp of the event.
 *
 * @Param [in]  g          The GPU super structure which contains the NVS scheduler.
 * @Param [out] timestamp  Pointer to store the timestamp of the log event.
 * @Param [out] msg        Pointer to store the message string of the log event.
 */
void nvgpu_nvs_get_log(struct gk20a *g, s64 *timestamp, const char **msg);

/**
 * @brief Return the number of active domains in the global list
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @return u32 Count of active domains.
 */
u32 nvgpu_nvs_domain_count(struct gk20a *g);

/**
 * @brief Deletes an NVS domain by its ID.
 *
 * This function attempts to delete an NVS domain specified by its domain ID.
 * It ensures that the domain is not in use by checking its reference count.
 * If the domain is in use or does not exist, appropriate errors are returned.
 * Otherwise, the domain is unlinked from the runlists, its memory is freed,
 * and it is removed from the scheduler's domain list.
 *
 * The steps performed by the function are as follows:
 * -# Acquire the scheduler mutex using nvgpu_mutex_acquire() to ensure exclusive access to the scheduler.
 * -# Attempt to retrieve the NVS domain by its ID using nvgpu_nvs_domain_by_id_locked().
 * -# If the domain does not exist, log the error and set the return value to -ENOENT.
 * -# If the domain's reference count is not 1, log the error and set the return value to -EBUSY.
 * -# If GSP scheduling is supported and enabled, attempt to delete the domain using
 *    nvgpu_gsp_nvs_delete_domain().
 * -# Unlink the domain from the runlists using nvgpu_nvs_unlink_rl_domains().
 * -# Free the runlist domain memory using nvgpu_nvs_delete_rl_domain_mem().
 * -# Set the domain's reference count to 0.
 * -# If the domain being deleted is the active domain, find the next domain in the list or
 *    set the active domain to the shadow domain if it is the only domain.
 * -# Unlink and destroy the domain using nvs_domain_unlink_and_destroy().
 * -# Using nvgpu_kfree() free the memory allocated for the domain's runlist domains array
 *    and the domain itself.
 * -# Release the scheduler mutex using nvgpu_mutex_release().
 * -# Return the error code, if any.
 *
 * @Param [in] g       The GPU super structure which contains the scheduler.
 * @Param [in] dom_id  The ID of the domain to be deleted.
 *
 * @return 0 on successful deletion, -ENOENT if the domain does not exist, or -EBUSY if the
 *         domain is still in use.
 */
int nvgpu_nvs_del_domain(struct gk20a *g, u64 dom_id);

/**
 * @brief Adds a new domain to the NVS scheduler with specified parameters.
 *
 * This function creates a new NVS domain with the given name, timeslice, and
 * preempt grace period. It allocates memory for the domain, links it with
 * runlist domains, and attaches it to the NVS scheduler. If GSP scheduling is
 * supported and enabled, it also sends the domain information to GSP.
 *
 * The steps performed by the function are as follows:
 * -# Validate the input parameters 'name' and 'pdomain' for NULL pointers.
 * -# Validate the domain name string using nvgpu_string_validate().
 * -# Acquire the scheduler mutex using nvgpu_mutex_acquire() to ensure exclusive
 *    access to the scheduler during domain addition.
 * -# Check if a domain with the given name already exists using nvs_domain_by_name().
 * -# Generate a new NVS domain with a unique ID using nvgpu_nvs_gen_domain() and
 *    nvgpu_nvs_new_id().
 * -# Create runlist domain memory for the new NVS domain using
 *    nvgpu_nvs_create_rl_domain_mem().
 * -# Link the new NVS domain with its corresponding runlist domains using
 *    nvgpu_nvs_link_rl_domains().
 * -# Attach the new NVS domain to the NVS scheduler using nvs_domain_scheduler_attach().
 * -# If GSP scheduling is supported and enabled, send the domain information to GSP
 *    using nvgpu_gsp_nvs_add_domain().
 * -# Set the output parameter 'pdomain' to the address of the new NVS domain.
 * -# Release the scheduler mutex using nvgpu_mutex_release().
 * -# Return the error code if any step fails, otherwise return 0 indicating success.
 *
 * @Param [in] g - The GPU super structure.
 * @Param [in] name - The name of the domain to be added.
 * @Param [in] timeslice - The timeslice for the domain.
 * @Param [in] preempt_grace - The preempt grace period for the domain.
 * @Param [out] pdomain - Pointer to the location where the address of the new
 *                        NVS domain will be stored.
 *
 * @return 0 on success, indicating the domain was added successfully.
 * @return -EINVAL if 'name' or 'pdomain' is NULL.
 * @return -EEXIST if a domain with the given name already exists.
 * @return -ENOMEM if memory allocation for the domain fails.
 * @return Error code from nvgpu_gsp_nvs_add_domain() if sending domain info to GSP fails.
 */
int nvgpu_nvs_add_domain(struct gk20a *g, const char *name, u64 timeslice,
			 u64 preempt_grace, struct nvgpu_nvs_domain **pdomain);

/**
 * @brief Print domain attributes
 *
 * Print domain attributes such as name, timeslice, preempt_grace
 * and ID.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param dom Input Domain. Need null check
 */
void nvgpu_nvs_print_domain(struct gk20a *g, struct nvgpu_nvs_domain *domain);

/**
 * @brief Get a pointer to a corresponding domain metadata using ID
 *
 * Within a global scheduling lock, check if the corresponding domain ID mapping
 * exists and increment its reference counter.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param domain_id Domain Id required for input.
 *
 * @retval NULL If domain_id doesn't exist.
 * @retval correct pointer to struct nvgpu_nvs_domain
 */
struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_id(struct gk20a *g, u64 domain_id);

/**
 * @brief Search for instance of nvgpu_nvs_domain by name.
 *
 * Within a global scheduling lock, check if the corresponding domain name
 * exists and increment its reference counter and return the instance.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param name Name of the domain to search for. Must not be NULL.
 *
 * @retval NULL If domain is null or doesn't exist.
 * @retval correct pointer to instance of struct nvgpu_nvs_domain
 */
struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_name(struct gk20a *g, const char *name);

/**
 * @brief Increment the domain's reference counter
 *
 * Within a global scheduling lock, increment the corresponding domain's
 * reference counter. Warn if zero already before increment as the init
 * value is one.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param dom Input Domain. Need null check
 */
void nvgpu_nvs_domain_get(struct gk20a *g, struct nvgpu_nvs_domain *dom);

/**
 * @brief Decrement the domain's reference counter
 *
 * Within a global scheduling lock, decrement the corresponding domain's
 * reference counter. Assert that value after decrement stays greater than
 * zero.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param dom Input Domain. Need null check
 */
void nvgpu_nvs_domain_put(struct gk20a *g, struct nvgpu_nvs_domain *dom);

/**
 * @brief Return name corresponding to the domain.
 *
 * @param dom Input Domain, Need validation check
 *
 * @retval Return name of the domain.
 * @retval NULL If domain_id doesn't exist.
 */
const char *nvgpu_nvs_domain_get_name(struct nvgpu_nvs_domain *dom);
/*
 * Debug wrapper for NVS code.
 */
#define nvs_dbg(g, fmt, arg...)			\
	nvgpu_log(g, gpu_dbg_nvs, fmt, ##arg)

/**
 * @brief Acquire a lock for access-control of Control-Fifo queues.
 *
 * Doesn't support recursive calls.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_lock_queues(struct gk20a *g);

/**
 * @brief Release lock for access-control of Control-Fifo queues.
 *
 * A lock must have been held by the same thread using
 * nvgpu_nvs_ctrl_fifo_lock_queues().
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_unlock_queues(struct gk20a *g);

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
void nvgpu_nvs_worker_pause(struct gk20a *g);
void nvgpu_nvs_worker_resume(struct gk20a *g);
bool nvgpu_nvs_ctrl_fifo_is_enabled(struct gk20a *g);
#endif
/**
 * @brief Creates and initializes a control-fifo structure for NVS domain.
 *
 * This function allocates memory for a new control-fifo structure, initializes
 * its members, and returns a pointer to the newly created structure. It sets
 * up the necessary locks and lists to manage exclusive and non-exclusive users
 * of the control-fifo.
 *
 * The steps performed by the function are as follows:
 * -# Allocate memory for the control-fifo structure using nvgpu_kzalloc().
 * -# If memory allocation fails, return NULL.
 * -# Assign the GPU structure pointer to the control-fifo structure.
 * -# Set the scheduler implementation hardware to #NVGPU_NVS_DOMAIN_SCHED_GSP.
 * -# Initialize the spinlock for user access control using nvgpu_spinlock_init().
 * -# Initialize the mutex for queue access control using nvgpu_mutex_init().
 * -# Initialize the list node for exclusive users using nvgpu_init_list_node().
 * -# Initialize the list node for non-exclusive users using nvgpu_init_list_node().
 * -# Return a pointer to the newly created control-fifo structure.
 *
 * @param [in] g The GPU super structure.
 *
 * @return Pointer to the newly created control-fifo structure on success.
 * @return NULL if memory allocation fails.
 */
struct nvgpu_nvs_domain_ctrl_fifo *nvgpu_nvs_ctrl_fifo_create(struct gk20a *g);

/**
 * @brief Check whether a process having a given PID has already opened the Control-Fifo
 *        device node.
 *
 * A user here typically indicates a process that opens the control-fifo device node.
 * The PIDs are stored for each such process and is used for detecting duplicates.
 *
 * Iterate through the list of non-exclusive users as well as exclusive users and check
 * whether any user exists with the given PID. The check is entirely done within a user
 * lock.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL.
 * @param pid[In] This pid is compared with the existing users to identify duplicates.
 * @param rw[In] Permission of the user.
 * @retval true If some user exists having same pid as that of input pid.
 * @retval false No user exists having same pid as that of input pid or sched_ctrl is NULL.
 */
bool nvgpu_nvs_ctrl_fifo_user_exists(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
    int pid, bool rw);

/**
 * @brief Idle the GPU when using Control-Fifo nodes
 *
 * Nvgpu KMD increments a power lock everytime Control-Fifo device node is
 * opened by a User process.
 *
 * This method can be used to allow decrementing the power lock. This doesn't
 * guarantee that work submits would complete. User of this API needs to make
 * sure that no jobs would be submitted when this API is called. This API
 * should be balanced by a call to an equivalent unidle() version.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_idle(struct gk20a *g);

/**
 * @brief Unidle the GPU when using Control-Fifo nodes
 *
 * Invoke this function after a previous call to idle the GPU has already
 * been done.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_unidle(struct gk20a *g);

/**
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL.
 *
 * Atomically check whether the usage_counter in the users field is non-zero.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL.
 * @retval true Atleast one user is using the control-fifo
 * @retval false No user is using the control-fifo or sched_ctrl is NULL.
 */
bool nvgpu_nvs_ctrl_fifo_is_busy(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl);

/**
 * @brief Destroy the master control-fifo structure
 *
 * Assert that nvgpu_nvs_ctrl_fifo_is_busy() is not true.
 *
 * Erase all the Queues and clear other references to the memory.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_destroy(struct gk20a *g);

/**
 * @brief Check whether a user is actively in use.
 *
 * Verify whether user's active_used_queues count is non-zero.
 *
 * @param user A user for the control-fifo node. Must not be NULL.
 * @retval true If active_used_queues is set to non-zero.
 * @retval false If active_used_queues is set to zero.
 */
bool nvgpu_nvs_ctrl_fifo_user_is_active(struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Add a control-fifo user into the master structure.
 *
 * Atomically add a control-fifo user into the master structure as a non-exclusive user.
 * By default all users are added as non-exclusive users. nvgpu_nvs_ctrl_fifo_reserve_exclusive_user()
 * can be used to convert a non-exclusive user to an exclusive.
 *
 * Increment the control-fifo's user's usage_counter.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @param user User Pointer to a User to be attached to the master structure. Must not be NULL
 */
void nvgpu_nvs_ctrl_fifo_add_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
    struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Check whether a given user is marked as exclusive user.
 *
 *  Atomically check whether a given user is marked as exclusive.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @param user User Pointer to a User to be attached to the master structure. Must not be NULL
 * @retval true If the user is marked as exclusive.
 * @retval false If the user is not marked as exclusive or other inputs are NULL.
 */
bool nvgpu_nvs_ctrl_fifo_is_exclusive_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
    struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Reset exclusive users as non-exclusive users
 *
 * Atomically change the characterisation of exclusive users and non-exclusive users.
 * Separate the exclusive user from the Exclusive entry and move it to the list of
 * non-exclusive users.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @param user User Pointer to a User to be attached to the master structure. Must not be NULL
 */
void nvgpu_nvs_ctrl_fifo_reset_exclusive_user(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl, struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Convert a non-exclusive user to an exclusive user.
 *
 * Atomically change a non-exclusive user to an exclusive user provided the user
 * has a write access. Remove the corresponding entry of the user from the non-exclusive list
 * and add it to the exclusive entry.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @param user User Pointer to a User to be attached to the master structure. Must not be NULL
 *
 * @retval 0 If the user is marked as exclusive.
 * @retval ENODEV if sched_ctrl or user is NULL
 * @retval EPERM if the user doesn't have write access.
 * @retval EBUSY if an exclusive user already exists.
 */
int nvgpu_nvs_ctrl_fifo_reserve_exclusive_user(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl, struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Remove user from the master control-fifo structure.
 *
 * Atomically remove a user from the master control-fifo structure i.e. sched_ctrl.
 * Remove the entry from the sched_ctrl's user entry and decrement its usage counter.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @param user User Pointer to a User to be attached to the master structure. Must not be NULL
 */
void nvgpu_nvs_ctrl_fifo_remove_user(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		struct nvs_domain_ctrl_fifo_user *user);

/**
 * @brief Return an instance of the capabilities of the control-fifo.
 *
 * @param sched_ctrl Pointer to the master structure for control-fifo. Must not be NULL
 * @return struct nvs_domain_ctrl_fifo_capabilities*
 * @retval NULL if doesn't exist.
 */
struct nvs_domain_ctrl_fifo_capabilities *nvgpu_nvs_ctrl_fifo_get_capabilities(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl);

/**
 * @brief Obtain a reference to the Queue based on input parameters.
 *
 * Returns an appropriate queue based on the combination of queue_num
 * and queue_direction and also sets the mask value corresponding to the queue type.
 *
 * 1) If queue_num is set to NVGPU_NVS_NUM_CONTROL and queue_direction is set to
 *    NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER, return pointer to Send queue and set mask as
 *    NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_WRITE
 *
 * 2) else for NVGPU_NVS_NUM_CONTROL and queue_direction set to
 *    NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT, return pointer to Receive queue and set mask as
 *    NVGPU_NVS_CTRL_FIFO_QUEUE_EXCLUSIVE_CLIENT_READ
 *
 * 3) Otherwise, if queue_num is NVGPU_NVS_NUM_EVENT and queue_direction set to
 *    NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT return pointer to Event Queue and set mask as
 *    NVGPU_NVS_CTRL_FIFO_QUEUE_CLIENT_EVENTS_READ.
 *
 * 4) Return NULL otherwise
 *
 * @param sched_ctrl[In] Pointer to the master structure for control-fifo.
 *                       Must not be NULL.
 * @param queue_num[In] Indicates the type of Queue.
 * @param queue_direction[In] Indicates the direction of Queue
 * @param mask[Out] Pointer to a One byte variable to indicate the mask.
 *                  Must not be NULL.
 * @retval struct nvgpu_nvs_ctrl_queue* NULL if conditions are not met or input parameters
 *                                      are NULL.
 * @retval struct nvgpu_nvs_ctrl_queue* Pointer to a Queue based on the input parameters.
 */
struct nvgpu_nvs_ctrl_queue *nvgpu_nvs_ctrl_fifo_get_queue(
		struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		enum nvgpu_nvs_ctrl_queue_num queue_num,
		enum nvgpu_nvs_ctrl_queue_direction queue_direction,
		u8 *mask);

#ifdef CONFIG_KMD_SCHEDULING_WORKER_THREAD
struct nvs_control_fifo_receiver *nvgpu_nvs_domain_ctrl_fifo_get_receiver(struct gk20a *g);
struct nvs_control_fifo_sender *nvgpu_nvs_domain_ctrl_fifo_get_sender(struct gk20a *g);
void nvgpu_nvs_domain_ctrl_fifo_set_receiver(struct gk20a *g,
		struct nvs_control_fifo_receiver *receiver);
void nvgpu_nvs_domain_ctrl_fifo_set_sender(struct gk20a *g,
		struct nvs_control_fifo_sender *sender);
int nvgpu_nvs_ctrl_fifo_scheduler_handle_requests(struct gk20a *g);
#endif

/* Below methods require nvgpu_nvs_ctrl_fifo_lock_queues() to be held. */
/**
 * @brief Check if buffer's valid entry is marked to true.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param buf Input Queue buffer. Must not be NULL.
 * @retval true If buffer's valid is set to true, indicates buffer is valid
 * @retval false if buf is NULL or valid is set to false.
 */
bool nvgpu_nvs_buffer_is_valid(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *buf);

/**
 * @brief Check if the send queue of the NVS domain control FIFO is valid.
 *
 * The function 'nvgpu_nvs_buffer_is_sendq_valid' checks the validity of the send queue
 * within the NVS domain control FIFO structure associated with a given GPU instance.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the NVS domain control FIFO structure from the GPU instance.
 * -# Call 'nvgpu_nvs_buffer_is_valid()' to check the validity of the send queue.
 *
 * @param [in] g The GPU instance structure.
 *
 * @return True if the send queue is valid, false otherwise.
 */
bool nvgpu_nvs_buffer_is_sendq_valid(struct gk20a *g);

/**
 * @brief Check if the receive queue of the NVS domain control FIFO is valid.
 *
 * The function 'nvgpu_nvs_buffer_is_receiveq_valid' checks the validity of the receive queue
 * within the NVS domain control FIFO structure associated with a given GPU instance.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the NVS domain control FIFO structure from the GPU instance.
 * -# Call 'nvgpu_nvs_buffer_is_valid()' to check the validity of the receive queue.
 *
 * @param [in] g The GPU instance structure.
 *
 * @return True if the receive queue is valid, false otherwise.
 */
bool nvgpu_nvs_buffer_is_receiveq_valid(struct gk20a *g);

/**
 * @brief Check if the event queue of the NVS domain control FIFO is valid.
 *
 * The function 'nvgpu_nvs_buffer_is_eventq_valid' checks the validity of the event queue
 * within the NVS domain control FIFO structure associated with a given GPU instance.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the pointer to the NVS domain control FIFO structure from the GPU instance.
 * -# Call 'nvgpu_nvs_buffer_is_valid()' to check the validity of the event queue.
 *
 * @param [in] g The GPU instance structure.
 *
 * @return True if the event queue is valid, false otherwise.
 */
bool nvgpu_nvs_buffer_is_eventq_valid(struct gk20a *g);

/**
 * @brief Allocate DMA memory for shared queues.
 *
 * Use System VM to allocate memory for shared queues for Control-Fifo
 * operations. This memory is suitable for doing DMA and is mapped into the GMMU.
 * This memory will be placed in SYSMEM.
 *
 * Mark the queue's valid entry as true to indicate validity of the queue.
 * and set the mask to the corresponding input entry buf.
 *
 * @param sched_ctrl[In] Pointer to the master structure for control-fifo.
 *                       Must not be NULL.
 * @param bytes[In] Size of the Queue in bytes.
 * @param mask[In] Queue Mask.
 * @param buf[In] Input Queue buffer. Must not be NULL.
 *
 * @retval 0 If buffer's valid is set to true, indicates buffer is valid
 * @retval false if buf is NULL or valid is set to false.
 */
int nvgpu_nvs_buffer_alloc(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		size_t bytes, u8 mask, struct nvgpu_nvs_ctrl_queue *buf);

/**
 * @brief Free memory allocated for the shared queue.
 *
 * Free the memory corresponding to the queue if the queue's valid entry is
 * true.
 *
 * @param sched_ctrl[In] Pointer to the master structure for control-fifo.
 *                       Must not be NULL.
 * @param buf[In] Input Queue buffer. Must not be NULL.
 * @param is_abrupt_close[In] Flag to indicate abrupt closing
 */
void nvgpu_nvs_buffer_free(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		struct nvgpu_nvs_ctrl_queue *buf, bool is_abrupt_close);

/**
 * @brief Check whether queue has any active users.
 *
 * Check whether queue's reference counter is non-zero.
 *
 * @param queue[In] Input Queue buffer. Must not be NULL.
 * @retval true If queue's reference counter is non-zero.
 * @retval false If queue is NULL or reference counter is zero.
 */
bool nvgpu_nvs_ctrl_fifo_queue_has_subscribed_users(struct nvgpu_nvs_ctrl_queue *queue);

/**
 * @brief Mark user as subscribed to the queue.
 *
 * Bitwise 'OR' the queue's mask to the user's active_used_queues.
 * and increment the queue's reference counter. This is required to prevent
 * multiple accesses into the queue by the same user. This can be used to prevent
 * the same user from opening the device nodes multiple times.
 *
 * @param user[In] User that needs to subscribe to the queue. Must not be NULL.
 * @param queue[In] Input Queue buffer. Must not be NULL.
 */
void nvgpu_nvs_ctrl_fifo_user_subscribe_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue);

/**
 * @brief Mark user as unsubscribed to the queue.
 *
 * Decrement the queue's reference counter and remove the corresponding queue's
 * mask from user's active_used_queues.
 *
 * @param user[In] User that needs to subscribe to the queue. Must not be NULL.
 * @param queue[In] Input Queue buffer. Must not be NULL.
 */
void nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue);

/**
 * @brief Check whether a given user is subscribed to the queue.
 *
 * @param user[In] User that needs to subscribe to the queue. Must not be NULL.
 * @param queue[In] Input Queue buffer. Must not be NULL.
 * @retval true If user's active_used_queues field contains the queue's mask.
 * @retval false If either of Input values are empty or user's active_used_queues doesn't
 *         contain the queue's mask.
 */
bool nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_queue *queue);

/**
 * @brief Call queue's free method.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param queue[In] Input Queue buffer. Must not be NULL.
 * @param is_abrupt_close[In] Flag to indicate abrupt closing
 */
void nvgpu_nvs_ctrl_fifo_erase_queue_locked(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue,
	bool is_abrupt_close);

/**
 * @brief Erase all queues within the control-fifo structure.
 *
 * Erase the Send/Receive and Event Queues while the global queue lock is taken.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 */
void nvgpu_nvs_ctrl_fifo_erase_all_queues(struct gk20a *g);

/**
 * @brief Get the Shadow Domain.
 *
 * Get a pointer to the shadow domain. The shadow domain is always present.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @return struct nvgpu_nvs_domain* Shadow Domain.
 */
struct nvgpu_nvs_domain *
nvgpu_nvs_get_shadow_domain_locked(struct gk20a *g);

/**
 * @brief Retrieves the NVS domain associated with a given domain ID.
 *
 * This function searches for an NVS domain by its unique identifier within the
 * NVS scheduler's list of domains. It assumes that the caller has already
 * acquired the necessary locks to ensure thread-safe access to the scheduler.
 *
 * The steps performed by the function are as follows:
 * -# Retrieve the NVS scheduler from the GPU super structure.
 * -# Using nvs_domain_for_each() iterate over each domain in the NVS scheduler's list of domains.
 * -# For each domain, retrieve the NVS domain structure from the private data.
 * -# Compare the domain ID of the current NVS domain with the input domain ID.
 * -# If a match is found, return the pointer to the matching NVS domain.
 * -# If no match is found after iterating through all domains, return NULL.
 *
 * @Param [in] g - The GPU super structure.
 * @Param [in] domain_id - The unique identifier of the domain to find.
 *
 * @return Pointer to the NVS domain if found, otherwise NULL.
 */
struct nvgpu_nvs_domain *nvgpu_nvs_domain_by_id_locked(struct gk20a *g, u64 domain_id);

#else

static inline int nvgpu_nvs_init(struct gk20a *g)
{
	(void)g;
	return 0;
}

static inline void nvgpu_nvs_remove_support(struct gk20a *g)
{
	(void)g;
}

static inline struct nvgpu_nvs_domain *
nvgpu_nvs_domain_by_name(struct gk20a *g, const char *name)
{
	(void)g;
	(void)name;
	return NULL;
}

static inline void nvgpu_nvs_domain_put(struct gk20a *g, struct nvgpu_nvs_domain *dom)
{
	(void)g;
	(void)dom;
}

static inline const char *nvgpu_nvs_domain_get_name(struct nvgpu_nvs_domain *dom)
{
	(void)dom;
	return NULL;
}
static inline struct nvgpu_nvs_domain *
nvgpu_nvs_get_shadow_domain_locked(struct gk20a *g)
{
	(void)g;
	return NULL;
}
static inline struct nvgpu_nvs_domain *nvgpu_nvs_domain_by_id_locked(struct gk20a *g, u64 domain_id)
{
	(void)g;
	return NULL;
	(void)domain_id;
}
#endif

#ifdef CONFIG_NVGPU_GSP_SCHEDULER
/**
 * @brief Retrieves information about a specific runlist domain.
 *
 * This function provides the IOVA address, aperture, and number of entries for a
 * runlist domain identified by the NVS domain ID and index. It handles the special
 * case of the shadow domain separately.
 *
 * The steps performed by the function are as follows:
 * -# Check if the NVS domain ID corresponds to the shadow domain. If it does,
 *    retrieve the shadow domain using nvgpu_nvs_get_shadow_domain_locked() and
 *    runlist domain using nvgpu_runlist_get_shadow_domain().
 *    If the shadow domain is not found, log an error and set the return value to -ENXIO.
 * -# If the NVS domain ID does not correspond to the shadow domain, retrieve the
 *    NVS domain by its ID using nvgpu_nvs_domain_by_id_locked(). If the domain is
 *    not found, log an error and set the return value to -ENXIO.
 * -# Retrieve the runlist domain using the index if it's not the shadow domain.
 *    If the runlist domain is not found, log an error and set the return value to -ENXIO.
 * -# Retrieve the IOVA address of the runlist domain's memory using nvgpu_mem_get_addr().
 * -# Retrieve the aperture of the runlist domain's memory using gops_runlist.get_runlist_aperture().
 * -# Retrieve the count of entries in the runlist domain's memory.
 * -# Return the error code, if any.
 *
 * @Param [in]  g                The GPU super structure which contains the NVS scheduler.
 * @Param [in]  nvgpu_domain_id  The ID of the NVS domain to retrieve information for.
 * @Param [out] num_entries      Pointer to store the number of entries in the runlist domain.
 * @Param [out] runlist_iova     Pointer to store the IOVA address of the runlist domain's memory.
 * @Param [out] aperture         Pointer to store the aperture of the runlist domain's memory.
 * @Param [in]  index            The index of the runlist domain within the NVS domain.
 *
 * @return 0 on success, -ENXIO if the NVS domain or runlist domain is not found.
 */
s32 nvgpu_nvs_gsp_get_runlist_domain_info(struct gk20a *g, u64 nvgpu_domain_id, u32 *num_entries,
	u64 *runlist_iova, u32 *aperture, u32 index);

/**
 * @brief Retrieves domain ID and timeslice information for a given NVS domain.
 *
 * This function fetches the domain ID and timeslice in nanoseconds for the NVS
 * domain specified by the NVS domain ID. It handles the special case of the
 * shadow domain separately.
 *
 * The steps performed by the function are as follows:
 * -# Check if the NVS domain ID corresponds to the shadow domain. If it does,
 *    retrieve the shadow domain using nvgpu_nvs_get_shadow_domain_locked().
 * -# If the NVS domain ID does not correspond to the shadow domain, retrieve the
 *    NVS domain by its ID using nvgpu_nvs_domain_by_id_locked().
 * -# If the NVS domain is not found, log an error and set the return value to -ENXIO.
 * -# Retrieve the lower 32 bits of the NVS domain ID and store it in the output
 *    parameter for the domain ID.
 * -# Cast the timeslice from u64 to u32 safely using nvgpu_safe_cast_u64_to_u32()
 *    and store it in the output parameter for the timeslice in nanoseconds.
 * -# Return the error code, if any.
 *
 * @Param [in]  g                The GPU super structure which contains the NVS scheduler.
 * @Param [in]  nvgpu_domain_id  The ID of the NVS domain to retrieve information for.
 * @Param [out] domain_id        Pointer to store the domain ID.
 * @Param [out] timeslice_ns     Pointer to store the timeslice in nanoseconds.
 *
 * @return 0 on success, -ENXIO if the NVS domain is not found.
 */
s32 nvgpu_nvs_get_gsp_domain_info(struct gk20a *g, u64 nvgpu_domain_id,
		u32 *domain_id, u32 *timeslice_ns);
#endif
#ifdef CONFIG_NVS_PRESENT
/**
 * @brief Checks if any user NVS domains are present in the scheduler.
 *
 * This function determines whether there are any user NVS domains present
 * by checking the domain count in the NVS scheduler. If the count is greater
 * than zero, it indicates that user domains are present.
 *
 * The steps performed by the function are as follows:
 * -# Call nvs_domain_count() to get the count of NVS domains in the scheduler.
 * -# If the count is greater than zero, set the return value to true, indicating
 *    that user domains are present.
 * -# Return the boolean value indicating the presence of user domains.
 *
 * @Param [in] g  The GPU super structure which contains the NVS scheduler.
 *
 * @return true if user NVS domains are present, false otherwise.
 */
bool nvgpu_nvs_gsp_usr_domain_present(struct gk20a *g);
#endif
#endif
