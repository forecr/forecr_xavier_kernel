/* SPDX-License-Identifier: GPL-2.0-only OR MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 */

#ifndef NVS_DOMAIN_H
#define NVS_DOMAIN_H

#include <nvs/types-internal.h>

struct nvs_sched;
struct nvs_domain;

/* Max supported domains by GSP are 16. Other than 1 shadow domain 15 user domains
 * can be created. This count should be in sync with GSP mMax supported domains
 * limit.
 */
#define MAX_SUPPORTED_USER_DOMAINS 15U

/**
 * nvsched provides a simple, singly linked list for keeping track of
 * available domains. If algorithms need something more complex, like a
 * table of priorities and domains therein, then it will need to build
 * these data structures during its init().
 */
struct nvs_domain_list {
	u32			 nr;
	struct nvs_domain	*domains;

	/**
	 * Convenience for adding a domain quickly.
	 */
	struct nvs_domain	*last;
};

/**
 * @brief This struct is an underlying abstract data type that
 * represents node of a linkedlist. This struct contains some core
 * data about the domain such as Timeslice, preempt_grace etc.
 *
 * Instances of this struct are bound to each other in the form of a
 * global linkedlist.
 */
struct nvs_domain {
	/**
	 * Name of the domain, limited to 32 characters.
	 */
	char			 name[32];

	struct nvs_context_list	*ctx_list;

	/**
	 * Internal, singly linked list pointer.
	 */
	struct nvs_domain	*next;

	/**
	 * Scheduling parameters: specify how long this domain should be scheduled
	 * for and what the grace period the scheduler should give this domain when
	 * preempting. A value of zero is treated as an infinite timeslice or an
	 * infinite grace period.
	 */
	u64			 timeslice_ns;
	u64			 preempt_grace_ns;

	/**
	 * Priv pointer for downstream use.
	 */
	void			*priv;
};

/**
 * @brief Iterate over the list of domains present in the sched.
 */
#define nvs_domain_for_each(sched, domain_ptr)			\
	for ((domain_ptr) = (sched)->domain_list->domains;	\
	     (domain_ptr) != NULL;				\
	     (domain_ptr) = (domain_ptr)->next)

/**
 * @brief Construct an instance of struct nvs_domain
 *
 * Allocate memory for a struct nvs_domain instance and populate it with
 * the details such as name, timeslice, preempt_grace and a priv pointer.
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param name Name of the domain
 * @param timeslice A timeslice refers to continuous period for a given domain in
 *                  Nanosecs
 * @param preempt_grace Grace time for preemption of the domain in NanoSecs
 * @param priv Pointer to store a reference to other structures
 *
 * @retval An instance of the constructed struct nvs_domain
 * @retval NULL If memory allocation failure.
 */
struct nvs_domain *nvs_domain_create(struct nvs_sched *sched,
		  const char *name, u64 timeslice, u64 preempt_grace,
		  void *priv);

/**
 * @brief Get an instance of next struct nvs_domain in the global list
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param dom Input domain from which the next domain in the list is retrieved.
 * @return struct nvs_domain* Instance of the next nvs_domain.
 */
struct nvs_domain *nvs_domain_get_next_domain(struct nvs_sched *sched, struct nvs_domain *dom);

/**
 * @brief Attach a given nvs_domain to the global domains list.
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param dom Input domain which is attached to the global list.
 */
void nvs_domain_scheduler_attach(struct nvs_sched *sched, struct nvs_domain *dom);

/**
 * @brief Delete a domain
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param dom Input domain that needs to be deleted.
 */
void nvs_domain_destroy(struct nvs_sched *sched, struct nvs_domain *dom);

/**
 * @brief Unlink a domain from the global list and destroy the domain.
 *
 * Remove the domain node from the global list and decrement the count of
 * active nodes.
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param dom Input domain that needs to be unlinked and destroyed.
 */
void nvs_domain_unlink_and_destroy(struct nvs_sched *sched,
			struct nvs_domain *dom);

/**
 * @brief Erase all nodes within the global list
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 */
void nvs_domain_clear_all(struct nvs_sched *sched);

/**
 * @brief Return the number of active domains in the global list.
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @return u32 count of number of domains.
 */
u32 nvs_domain_count(struct nvs_sched *sched);

/**
 * @brief Return the appropriate domain instance based on the name.
 *
 * @param sched Context or placeholder for storing pointer to a struct gk20a
 * @param name Input name of the domain. Must not be NULL
 * @return struct nvs_domain*
 */
struct nvs_domain *nvs_domain_by_name(struct nvs_sched *sched, const char *name);

#endif
