/*
 * Copyright (c) 2021-2023, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef NVGPU_NVS_H
#define NVGPU_NVS_H

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
	void (*free)(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue);
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
 * @brief Initialize NVS metadata and setup shadow domain.
 *
 * The initialization is done under a critical section defined by a global
 * scheduling lock.
 *
 * 1) Initialize the metadata required for NVS subject to whether
 *    the flag NVGPU_SUPPORT_NVS_CTRL_FIFO is supported. Return early,
 *    if the metadata is already initialized.
 * 2) Initialize NVS's internal ID counter used for tracking Domain IDs.
 * 3) Construct the Control-Fifo master structure and store it as part
 *    of \a g->sched_ctrl_fifo.
 * 4) Construct a global list for storing domains and initialize the counters
 *    associated with it.
 * 5) Generate the global Shadow Domain. The ID of the shadow domain is set to U64_MAX
 *    as well as timeslice set to 100U * NSEC_PER_MSEC. Ensure the Shadow Domain
 *    is linked with the corresponding Shadow Runlist Domains.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 *
 * @retval 0 on success.
 * @retval ENOMEM Failed to allocate enough memory.
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
 * @brief Erase a domain metadata corresponding to a given domain id.
 *
 * The removal of the metedata is done under a critical section defined by a global
 * scheduling lock.
 *
 * 1) Check if dom_id is valid i.e. a valid domain metadata(struct nvgpu_nvs_domain) exists.
 * 2) Check if the domain's reference counter is not one to ensure no existing
 *    user exists.
 * 3) Unlink the RL domain metadata corresponding to this domain metadata.
 * 4) Free RL domain metadata specific memory.
 * 5) Set NVS's active domain to the next domain, if no other domain exists,
 *    set the shadow domain as the active domain.
 * 6) Unlink strut nvgpu_nvs_domain and its corresponding nvs_domain
 * 7) Free domain metadata(struct nvgpu_nvs_domain).
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param dom_id Domain Id for which the domain needs to be erased.
 *
 * @retval 0 on success.
 * @retval ENOENT if domain doesn't exist.
 * @retval EBUSY If domain is already being used i.e domain's reference
 *         counter is not 1.
 */
int nvgpu_nvs_del_domain(struct gk20a *g, u64 dom_id);

/**
 * @brief Create Domain Metadata and Link with RL domain
 *
 * The initialization is done under a critical section defined by a global
 * scheduling lock.
 *
 * 1) Verify if name already doesn't exist, otherwise return failure.
 * 2) Generate a struct nvgpu_nvs_domain, an internal struct nvs_domain,
 *    add their corresponding linkages. i.e. associate nvgpu_nvs_domain
 *    as a priv of nvs_domain and set nvs_domain as the parent of nvgpu_nvs_domain.
 * 3) Increment the global domain ID counter and set the domain's ID to the same.
 * 4) Set the corresponding timeslice and preempt_grace values.
 * 5) Create a struct nvgpu_runlist_domain corresponding to each engines and associate
 *    them with the above struct nvgpu_nvs_domain.
 * 6) Link the struct nvs_domain in a global list.
 * 7) Set the struct nvgpu_nvs_domain's address to pdomain.
 *
 * @param g [in] The GPU super structure. Function does not perform any
 *	validation of the parameter.
 * @param name [in] Name of the domain. Must not be NULL and must not already exist.
 * @param timeslice [in] The default timeslice of the Domain. Function does not perform any
 *	validation of the parameter.
 * @param preempt_grace [in] The default preempt_grace of the Domain. Function does not perform any
 *	validation of the parameter.
 * @param pdomain [out] Placeholder for returning the constructed Domain pointer. Must be non NULL.
 *
 * @retval 0 on success.
 * @retval EINVAL name is NULL or pdomain is NULL.
 * @retval EEXIST If name already exists
 * @retval ENOMEM Memory allocation failure
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
#endif

bool nvgpu_nvs_ctrl_fifo_is_enabled(struct gk20a *g);
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

bool nvgpu_nvs_buffer_is_sendq_valid(struct gk20a *g);
bool nvgpu_nvs_buffer_is_receiveq_valid(struct gk20a *g);
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
 */
void nvgpu_nvs_buffer_free(struct nvgpu_nvs_domain_ctrl_fifo *sched_ctrl,
		struct nvgpu_nvs_ctrl_queue *buf);

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
 */
void nvgpu_nvs_ctrl_fifo_erase_queue_locked(struct gk20a *g, struct nvgpu_nvs_ctrl_queue *queue);

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
s32 nvgpu_nvs_gsp_get_runlist_domain_info(struct gk20a *g, u64 nvgpu_domain_id, u32 *num_entries,
	u64 *runlist_iova, u32 *aperture, u32 index);
s32 nvgpu_nvs_get_gsp_domain_info(struct gk20a *g, u64 nvgpu_domain_id,
		u32 *domain_id, u32 *timeslice_ns);
#endif
#ifdef CONFIG_NVS_PRESENT
/* function to chech if user domain are present or only shadow domain
 * exists if domain_list::nr = 0 if only shadow domain is present */
bool nvgpu_nvs_gsp_usr_domain_present(struct gk20a *g);
#endif
#endif
