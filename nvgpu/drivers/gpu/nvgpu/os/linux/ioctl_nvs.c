// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/uaccess.h>
#include <linux/dma-buf.h>

#include <uapi/linux/nvgpu-nvs.h>

#include <nvgpu/nvs.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/nvgpu_init.h>
#include <nvgpu/os_sched.h>
#include "os_linux.h"

#include <nvs/nvs_sched.h>
#include <nvs/domain.h>

#if defined (CONFIG_NVS_PRESENT) && defined (CONFIG_NVGPU_GSP_SCHEDULER)
#include <nvgpu/gsp_sched.h>
#endif

#include "ioctl.h"
#include "ioctl_nvs.h"
#include "dmabuf_nvs.h"

/*
 * OS-specific layer to hold device node mapping for a domain.
 */
struct nvgpu_nvs_domain_ioctl {
	struct gk20a *g;
	struct nvgpu_nvs_domain *domain;
	struct cdev *cdev;
	struct device *dev;
	struct nvgpu_class *class;
	struct list_head list; /* entry in cdev_lookup_list */
};

/*
 * This lock serializes domain removal and opening of domain device nodes.
 */
static DEFINE_MUTEX(cdev_lookup_mutex);
/*
 * A list of struct nvgpu_nvs_domain_ioctl objects.
 */
static LIST_HEAD(cdev_lookup_list);

/*
 * Priv data for an open domain device file.
 *
 * While a domain device is open, it holds a ref to the domain.
 */
struct nvgpu_nvs_domain_file_private {
	struct gk20a *g;
	struct nvgpu_nvs_domain *domain;
};

static struct nvgpu_nvs_domain_ioctl *nvgpu_nvs_lookup_cdev(dev_t dev)
{
	struct nvgpu_nvs_domain_ioctl *ioctl, *ret = NULL;

	mutex_lock(&cdev_lookup_mutex);

	list_for_each_entry(ioctl, &cdev_lookup_list, list) {
		if (ioctl->cdev->dev == dev) {
			/* put back in nvgpu_nvs_domain_dev_release */
			nvgpu_nvs_domain_get(ioctl->g, ioctl->domain);
			ret = ioctl;
			goto out;
		}
	}

out:
	mutex_unlock(&cdev_lookup_mutex);
	return ret;
}

int nvgpu_nvs_dev_open(struct inode *inode, struct file *filp)
{
	struct nvgpu_cdev *cdev;
	struct gk20a *g;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);

	filp->private_data = g;

	return 0;
}

int nvgpu_nvs_dev_release(struct inode *inode, struct file *filp)
{
	/*
	 * Since the scheduler persists through a close() call, there's nothing
	 * to do on device close (for now).
	 */
	return 0;
}

static int nvgpu_nvs_domain_dev_do_open(struct gk20a *g,
		struct nvgpu_nvs_domain *domain,
		struct file *filp)
{
	struct nvgpu_nvs_domain_file_private *priv;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, "opening domain %s",
			domain->parent->name);

	g = nvgpu_get(g);
	if (!g)
		return -ENODEV;

	priv = nvgpu_kzalloc(g, sizeof(*priv));
	if (!priv) {
		err = -ENOMEM;
		goto put_ref;
	}

	priv->g = g;
	priv->domain = domain;
	filp->private_data = priv;

	return 0;

put_ref:
	nvgpu_put(g);
	return err;
}

static int nvgpu_nvs_domain_dev_open(struct inode *inode, struct file *filp)
{
	struct nvgpu_nvs_domain_ioctl *ioctl;
	struct cdev *cdev = inode->i_cdev;
	struct nvgpu_nvs_domain *domain;
	struct gk20a *g;
	int err;

	ioctl = nvgpu_nvs_lookup_cdev(cdev->dev);
	if (ioctl == NULL) {
		return -ENXIO;
	}

	g = ioctl->g;
	domain = ioctl->domain;

	err = nvgpu_nvs_domain_dev_do_open(g, domain, filp);
	if (err) {
		nvgpu_nvs_domain_put(g, domain);
	}

	return err;

}

static int nvgpu_nvs_domain_dev_release(struct inode *inode, struct file *filp)
{
	struct nvgpu_nvs_domain_file_private *priv = filp->private_data;
	struct nvgpu_nvs_domain *domain;
	struct gk20a *g;

	if (!priv)
		return 0;

	g = priv->g;
	domain = priv->domain;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, "releasing domain %s",
			domain->parent->name);

	/* this was taken when the file was opened */
	nvgpu_nvs_domain_put(g, domain);

	nvgpu_kfree(g, priv);
	nvgpu_put(g);
	filp->private_data = NULL;

	return 0;
}

static const struct file_operations nvgpu_nvs_domain_ops = {
	.owner   = THIS_MODULE,
	.open    = nvgpu_nvs_domain_dev_open,
	.release = nvgpu_nvs_domain_dev_release,
};

struct nvgpu_nvs_domain *nvgpu_nvs_domain_get_from_file(int fd)
{
	struct nvgpu_nvs_domain_file_private *priv;
	struct nvgpu_nvs_domain *domain;
	struct file *f = fget(fd);

	if (!f)
		return NULL;

	if (f->f_op != &nvgpu_nvs_domain_ops) {
		fput(f);
		return NULL;
	}

	priv = (struct nvgpu_nvs_domain_file_private *)f->private_data;
	domain = priv->domain;

	nvgpu_log(priv->g, gpu_dbg_fn | gpu_dbg_nvs, "domain %s",
			domain->parent->name);
	nvgpu_nvs_domain_get(priv->g, domain);
	fput(f);

	return domain;
}

static int create_domain_dev(struct gk20a *g,
			     struct nvgpu_nvs_domain *domain)
{
	struct device *dev = dev_from_gk20a(g);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct nvs_domain *nvs_domain = domain->parent;
	struct nvgpu_nvs_domain_ioctl *ioctl = domain->ioctl;
	char name[sizeof("nvsched-") + ARRAY_SIZE(nvs_domain->name)];
	struct nvgpu_class *class;
	dev_t devno;
	unsigned int minor;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, " ");

	class = nvgpu_get_v2_user_class(g);
	if (class == NULL) {
		/* MIG? */
		dev_err(dev, "unsupported GPU for scheduling");
		return -ENOSYS;
	}

	minor = nvgpu_allocate_cdev_minor(g);
	devno = MKDEV(MAJOR(l->cdev_region), minor);
	err = register_chrdev_region(devno, 1, dev_name(dev));
	if (err) {
		dev_err(dev, "failed to allocate devno");
		return err;
	}

	(void) snprintf(name, sizeof(name), "nvsched-%s", nvs_domain->name);

	ioctl->g = g;
	ioctl->domain = domain;
	INIT_LIST_HEAD(&ioctl->list);
	ioctl->cdev = cdev_alloc();
	ioctl->cdev->ops = &nvgpu_nvs_domain_ops;
	ioctl->class = class;
	err = nvgpu_create_device(dev, devno, name,
			ioctl->cdev, &ioctl->dev, class);
	if (err) {
		unregister_chrdev_region(devno, 1);
		return err;
	}

	list_add_tail(&ioctl->list, &cdev_lookup_list);

	return 0;
}

static void delete_domain_dev(struct gk20a *g,
			     struct nvgpu_nvs_domain *domain)
{
	struct nvgpu_nvs_domain_ioctl *ioctl = domain->ioctl;
	dev_t dev = ioctl->cdev->dev;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, " ");
	/*
	 * note: we're under the lookup lock, so no new open would succeed after this.
	 *
	 * nvgpu_nvs_domain_dev_open() might be waiting for the lock now. Open
	 * cdevs remain accessible even after cdev deletion, but we won't get
	 * here until all successfully opened devices have been closed because
	 * they hold domain refs.
	 */
	list_del(&ioctl->list);

	device_destroy(nvgpu_class_get_class(ioctl->class), dev);
	cdev_del(ioctl->cdev);
	unregister_chrdev_region(dev, 1);
}

static int nvgpu_nvs_ioctl_create_domain(
	struct gk20a *g,
	struct nvgpu_nvs_ioctl_create_domain *dom_args)
{
	struct nvgpu_nvs_domain *domain = NULL;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, " ");

	if (dom_args->reserved1 != 0) {
		return -EINVAL;
	}

	if (dom_args->domain_params.reserved1 != 0) {
		return -EINVAL;
	}

	if (dom_args->domain_params.reserved2 != 0) {
		return -EINVAL;
	}

	if (dom_args->domain_params.dom_id != 0) {
		return -EINVAL;
	}

	if (g->scheduler == NULL) {
		return -ENOSYS;
	}

	err = nvgpu_nvs_add_domain(g,
				      dom_args->domain_params.name,
				      dom_args->domain_params.timeslice_ns,
				      dom_args->domain_params.preempt_grace_ns,
				      &domain);
	if (err != 0) {
		return err;
	}

	domain->subscheduler = dom_args->domain_params.subscheduler;

	dom_args->domain_params.dom_id = domain->id;

	domain->ioctl = nvgpu_kzalloc(g, sizeof(*domain->ioctl));
	if (domain->ioctl == NULL) {
		err = -ENOMEM;
		goto del_domain;
	}

	mutex_lock(&cdev_lookup_mutex);
	err = create_domain_dev(g, domain);
	mutex_unlock(&cdev_lookup_mutex);
	if (err != 0) {
		goto free_ioctl;
	}

	return 0;
free_ioctl:
	nvgpu_kfree(g, domain->ioctl);
del_domain:
	nvgpu_nvs_del_domain(g, domain->id);
	return err;
}

static int nvgpu_nvs_ioctl_remove_domain(struct gk20a *g,
	struct nvgpu_nvs_ioctl_remove_domain *args)
{
	struct nvgpu_nvs_domain_ioctl *ioctl;
	struct nvgpu_nvs_domain *domain;
	int ret;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, " ");

	if (args->reserved1 != 0) {
		return -EINVAL;
	}

	if (g->scheduler == NULL) {
		return -ENOSYS;
	}

	domain = nvgpu_nvs_domain_by_id(g, args->dom_id);
	if (domain == NULL) {
		nvgpu_err(g, "domain %llu does not exist!", args->dom_id);
		return -ENOENT;
	}

	ioctl = domain->ioctl;

	mutex_lock(&cdev_lookup_mutex);

	nvgpu_nvs_domain_put(g, domain);
	ret = nvgpu_nvs_del_domain(g, args->dom_id);

	/* note: the internal default domain lacks ->ioctl */
	if (ret == 0 && ioctl != NULL) {
		delete_domain_dev(g, domain);
		nvgpu_kfree(g, ioctl);
	}

	mutex_unlock(&cdev_lookup_mutex);

	return ret;
}

static int nvgpu_nvs_ioctl_query_domains_locked(
	struct gk20a *g,
	void __user *user_arg,
	struct nvgpu_nvs_ioctl_query_domains *args)
{
	struct nvgpu_nvs_domain *nvgpu_dom;
	struct nvs_domain *nvs_dom;
	u32 index;
	u32 user_capacity = args->nr;
	struct nvgpu_nvs_ioctl_domain *args_domains =
		(void __user *)(uintptr_t)args->domains;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_nvs, " ");

	if (args->reserved0 != 0) {
		return -EINVAL;
	}

	if (args->reserved1 != 0) {
		return -EINVAL;
	}

	if (g->scheduler == NULL) {
		return -ENOSYS;
	}

	/* First call variant: return number of domains. */
	args->nr = nvs_domain_count(g->scheduler->sched);
	if (copy_to_user(user_arg, args, sizeof(*args))) {
		return -EFAULT;
	}
	nvs_dbg(g, "Nr domains: %u", args->nr);

	if (args_domains != NULL) {
		/*
		 * Second call variant: populate the passed array with domain info.
		 */
		index = 0;
		nvs_domain_for_each(g->scheduler->sched, nvs_dom) {
			struct nvgpu_nvs_ioctl_domain dom;
			if (index == user_capacity) {
				break;
			}

			nvgpu_dom = nvs_dom->priv;

			nvs_dbg(g, "Copying dom #%u [%s] (%llu) (%u refs)",
				index, nvs_dom->name, nvgpu_dom->id, nvgpu_dom->ref);

			(void)memset(&dom, 0, sizeof(dom));

			strncpy(dom.name, nvs_dom->name, sizeof(dom.name) - 1);
			dom.timeslice_ns     = nvs_dom->timeslice_ns;
			dom.preempt_grace_ns = nvs_dom->preempt_grace_ns;
			dom.subscheduler     = nvgpu_dom->subscheduler;
			dom.dom_id           = nvgpu_dom->id;

			if (copy_to_user(&args_domains[index],
					 &dom, sizeof(dom))) {
				nvs_dbg(g, "Fault during copy of domain to userspace.");
				return -EFAULT;
			}

			index += 1;
		}
	}

	return 0;
}

static int nvgpu_nvs_ioctl_query_domains(
	struct gk20a *g,
	void __user *user_arg,
	struct nvgpu_nvs_ioctl_query_domains *args)
{
	int err;

	nvgpu_mutex_acquire(&g->sched_mutex);
	err = nvgpu_nvs_ioctl_query_domains_locked(g, user_arg, args);
	nvgpu_mutex_release(&g->sched_mutex);
	return err;
}

long nvgpu_nvs_dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	u8 buf[NVGPU_NVS_IOCTL_MAX_ARG_SIZE] = { 0 };
	bool writable = filp->f_mode & FMODE_WRITE;
	struct gk20a *g = filp->private_data;
	int err = 0;

	nvs_dbg(g, "IOC_TYPE: %c", _IOC_TYPE(cmd));
	nvs_dbg(g, "IOC_NR:   %u", _IOC_NR(cmd));
	nvs_dbg(g, "IOC_SIZE: %u", _IOC_SIZE(cmd));

	if ((_IOC_TYPE(cmd) != NVGPU_NVS_IOCTL_MAGIC) ||
	    (_IOC_NR(cmd) == 0) ||
	    (_IOC_NR(cmd) > NVGPU_NVS_IOCTL_LAST) ||
	    (_IOC_SIZE(cmd) > NVGPU_NVS_IOCTL_MAX_ARG_SIZE)) {
		nvs_dbg(g, "-> BAD!!");
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(buf, (void __user *)arg, _IOC_SIZE(cmd)))
			return -EFAULT;
	}

	err = gk20a_busy(g);
	if (err != 0) {
		return err;
	}

	switch (cmd) {
	case NVGPU_NVS_IOCTL_CREATE_DOMAIN:
	{
		struct nvgpu_nvs_ioctl_create_domain *args =
			(struct nvgpu_nvs_ioctl_create_domain *)buf;

		if (!writable) {
			err = -EPERM;
			goto done;
		}

		err = nvgpu_nvs_ioctl_create_domain(g, args);
		if (err)
			goto done;

		/*
		 * Remove the domain in case of fault when copying back to
		 * userspace to keep this ioctl atomic.
		 */
		if (copy_to_user((void __user *)arg, buf, _IOC_SIZE(cmd))) {
			nvgpu_nvs_del_domain(g, args->domain_params.dom_id);
			err = -EFAULT;
			goto done;
		}

		break;
	}
	case NVGPU_NVS_IOCTL_QUERY_DOMAINS:
	{
		struct nvgpu_nvs_ioctl_query_domains *args =
			(struct nvgpu_nvs_ioctl_query_domains *)buf;

		err = nvgpu_nvs_ioctl_query_domains(g,
						    (void __user *)arg,
						    args);
		break;
	}
	case NVGPU_NVS_IOCTL_REMOVE_DOMAIN:
	{
		struct nvgpu_nvs_ioctl_remove_domain *args =
			(struct nvgpu_nvs_ioctl_remove_domain *)buf;

		if (!writable) {
			err = -EPERM;
			goto done;
		}

		err = nvgpu_nvs_ioctl_remove_domain(g, args);
		break;
	}
	default:
		err = -ENOTTY;
		goto done;
	}

done:
	gk20a_idle(g);
	return err;
}

ssize_t nvgpu_nvs_dev_read(struct file *filp, char __user *buf,
			   size_t size, loff_t *off)
{
	struct gk20a *g = filp->private_data;
	char log_buf[NVS_LOG_BUF_SIZE];
	const char *log_msg;
	s64 timestamp;
	int bytes;

	/*
	 * We need at least NVS_LOG_BUF_SIZE to parse text into from the binary
	 * log format.
	 *
	 * TODO: If size is large enough, return multiple entries in one go.
	 */
	if (size < NVS_LOG_BUF_SIZE) {
		nvgpu_err(g, "Write buf size too small: %zu", size);
		return -EINVAL;
	}

	nvgpu_nvs_get_log(g, &timestamp, &log_msg);
	if (log_msg == NULL) {
		return 0;
	}

	bytes = snprintf(log_buf, NVS_LOG_BUF_SIZE, "[%16lld] %s\n",
			 timestamp, log_msg);

	if (copy_to_user(buf, log_buf, bytes)) {
		return -EFAULT;
	}

	return bytes;
}

struct nvgpu_nvs_domain_ctrl_fifo_user_linux {
	struct nvs_domain_ctrl_fifo_user user;
	struct nvgpu_cdev *cdev;
};

int nvgpu_nvs_ctrl_fifo_ops_open(struct inode *inode, struct file *filp)
{
	struct nvgpu_cdev *cdev;
	struct gk20a *g;
	int pid;
	int err = 0;
	struct nvgpu_nvs_domain_ctrl_fifo_user_linux *linux_user;
	bool writable = filp->f_mode & FMODE_WRITE;
	bool readable = filp->f_mode & FMODE_READ;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);

	if (!writable && !readable) {
		nvgpu_err(g, "User doesn't have adequate permission to open this device");
		return -EOPNOTSUPP;
	}

	err = gk20a_busy(g);
	if (err != 0) {
		nvgpu_err(g, "Unable to power on the device");
		return err;
	}

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_NVS)) {
		gk20a_idle(g);
		return -EOPNOTSUPP;
	}

	pid = nvgpu_current_pid(g);
	nvgpu_nvs_ctrl_fifo_lock_queues(g);
	if (nvgpu_nvs_ctrl_fifo_user_exists(g->sched_ctrl_fifo, pid, writable)) {
		nvgpu_err(g, "User already exists");
		nvgpu_nvs_ctrl_fifo_unlock_queues(g);
		gk20a_idle(g);
		return -EEXIST;
	}

	linux_user = nvgpu_kzalloc(g, sizeof(*linux_user));
	if (linux_user == NULL) {
		nvgpu_nvs_ctrl_fifo_unlock_queues(g);
		gk20a_idle(g);
		return -ENOMEM;
	}

	linux_user->cdev = cdev;
	linux_user->user.pid = pid;
	if (writable)
		linux_user->user.has_write_access = true;

	nvgpu_nvs_ctrl_fifo_add_user(g->sched_ctrl_fifo, &linux_user->user);
	nvgpu_nvs_ctrl_fifo_unlock_queues(g);

	filp->private_data = linux_user;

	return 0;
}

int nvgpu_nvs_ctrl_fifo_ops_release(struct inode *inode, struct file *filp)
{
	struct nvgpu_cdev *cdev;
	struct gk20a *g;
	struct nvgpu_nvs_ctrl_queue *sendq = NULL, *recvq = NULL, *eventq = NULL;
	struct nvgpu_nvs_domain_ctrl_fifo_user_linux *linux_user = NULL;
	enum nvgpu_nvs_ctrl_queue_num num_queue = NVGPU_NVS_INVALID;
	enum nvgpu_nvs_ctrl_queue_direction queue_direction = NVGPU_NVS_DIR_INVALID;
	bool is_sendq_remove = false;
	bool is_eventq_remove = false;
	bool is_recvq_remove = false;
	bool is_exclusive_user;
	u8 mask = 0;
	int err = 0;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);

	linux_user = filp->private_data;

	if (g->sched_ctrl_fifo == NULL) {
		return -EINVAL;
	}

	if (linux_user == NULL) {
		return -ENODEV;
	}

	nvgpu_nvs_ctrl_fifo_lock_queues(g);
	is_exclusive_user = nvgpu_nvs_ctrl_fifo_is_exclusive_user(
					g->sched_ctrl_fifo, &linux_user->user);
	if (is_exclusive_user) {
		num_queue = NVGPU_NVS_NUM_CONTROL;
		if (nvgpu_nvs_buffer_is_sendq_valid(g)) {
			queue_direction = NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER;
			sendq = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo,
					num_queue, queue_direction, &mask);
			if ((sendq != NULL) &&
				nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(
					&linux_user->user, sendq)) {
				nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(
					&linux_user->user, sendq);
				is_sendq_remove = true;
			}
		}
		if (nvgpu_nvs_buffer_is_receiveq_valid(g)) {
			queue_direction = NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT;
			recvq = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo,
					num_queue, queue_direction, &mask);
			if (recvq != NULL &&
				nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(
					&linux_user->user, recvq)) {
				nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(
					&linux_user->user, recvq);
				is_recvq_remove = true;
			}
		}
	} else {
		if (nvgpu_nvs_buffer_is_eventq_valid(g)) {
			queue_direction = NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT;
			num_queue = NVGPU_NVS_NUM_EVENT;
			eventq = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo,
					num_queue, queue_direction, &mask);
			if (eventq != NULL &&
				nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(
					&linux_user->user, eventq)) {
				nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(
					&linux_user->user, eventq);
				is_eventq_remove = true;
			}
		}
	}

	if (nvgpu_nvs_ctrl_fifo_user_is_active(&linux_user->user)) {
		nvgpu_err(g, "user is still active");
		nvgpu_nvs_ctrl_fifo_unlock_queues(g);
		err = -EBUSY;
		return err;
	}

	if (is_exclusive_user) {
		nvgpu_nvs_ctrl_fifo_reset_exclusive_user(g->sched_ctrl_fifo,
			&linux_user->user);
	}


	if (is_sendq_remove) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, sendq);
	}

	if (is_recvq_remove) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, recvq);
	}

	nvgpu_nvs_ctrl_fifo_remove_user(g->sched_ctrl_fifo, &linux_user->user);

	if (!nvgpu_nvs_ctrl_fifo_is_busy(g->sched_ctrl_fifo)) {
		if (is_eventq_remove) {
			nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, eventq);
		}
	}
	nvgpu_nvs_ctrl_fifo_unlock_queues(g);

	filp->private_data = NULL;

	nvgpu_kfree(g, linux_user);
	gk20a_idle(g);

	return err;
}

extern const struct file_operations nvgpu_nvs_ctrl_fifo_ops;

static int nvgpu_nvs_ctrl_fifo_create_queue_verify_flags(struct gk20a *g,
		struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_fifo_ioctl_create_queue_args *args)
{
	int err = 0;

	if (args->reserve0 != 0) {
		return -EINVAL;
	}

	if (args->dmabuf_fd != 0) {
		return -EINVAL;
	}

	if (args->queue_size != 0) {
		return -EINVAL;
	}

	if (args->access_type == NVGPU_NVS_CTRL_FIFO_QUEUE_ACCESS_TYPE_EXCLUSIVE) {
		if ((args->queue_num != NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_EVENT)
				&& (args->queue_num != NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_CONTROL))
			return -EINVAL;
		if ((args->direction != NVGPU_NVS_CTRL_FIFO_QUEUE_DIRECTION_CLIENT_TO_SCHEDULER)
				&& (args->direction != NVGPU_NVS_CTRL_FIFO_QUEUE_DIRECTION_SCHEDULER_TO_CLIENT))
			return -EINVAL;
		if (!nvgpu_nvs_ctrl_fifo_is_exclusive_user(g->sched_ctrl_fifo, user)) {
			err = nvgpu_nvs_ctrl_fifo_reserve_exclusive_user(g->sched_ctrl_fifo, user);
			if (err != 0) {
				return err;
			}
		}
	} else if (args->access_type == NVGPU_NVS_CTRL_FIFO_QUEUE_ACCESS_TYPE_NON_EXCLUSIVE) {
		if (args->queue_num != NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_EVENT)
			return -EINVAL;
		if (args->direction != NVGPU_NVS_CTRL_FIFO_QUEUE_DIRECTION_SCHEDULER_TO_CLIENT)
			return -EINVAL;
	} else {
		return -EINVAL;
	}

	return 0;
}

static enum nvgpu_nvs_ctrl_queue_num nvgpu_nvs_translate_queue_num(u32 queue_num_arg)
{
	enum nvgpu_nvs_ctrl_queue_num num_queue = NVGPU_NVS_INVALID;
	if (queue_num_arg == NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_CONTROL)
		num_queue = NVGPU_NVS_NUM_CONTROL;
	else if (queue_num_arg == NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_EVENT)
		num_queue = NVGPU_NVS_NUM_EVENT;

	return num_queue;
}

static enum nvgpu_nvs_ctrl_queue_direction
	nvgpu_nvs_translate_queue_direction(u32 queue_direction)
{
	enum nvgpu_nvs_ctrl_queue_direction direction = NVGPU_NVS_DIR_INVALID;
	if (queue_direction == NVGPU_NVS_CTRL_FIFO_QUEUE_DIRECTION_CLIENT_TO_SCHEDULER)
		direction = NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER;
	else if (queue_direction == NVGPU_NVS_CTRL_FIFO_QUEUE_DIRECTION_SCHEDULER_TO_CLIENT)
		direction = NVGPU_NVS_DIR_SCHEDULER_TO_CLIENT;

	return direction;
}

static int nvgpu_nvs_ctrl_fifo_create_queue(struct gk20a *g,
		struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_fifo_ioctl_create_queue_args *args)
{
	enum nvgpu_nvs_ctrl_queue_num num_queue;
	enum nvgpu_nvs_ctrl_queue_direction queue_direction;
	struct nvgpu_nvs_ctrl_queue *queue;
	struct nvgpu_nvs_linux_buf_priv *priv = NULL;
	int err = 0;
	int fd;
	int flag = O_CLOEXEC;
	bool read_only;
	size_t queue_size;
	u8 mask = 0;

	nvgpu_nvs_ctrl_fifo_lock_queues(g);
	err = nvgpu_nvs_ctrl_fifo_create_queue_verify_flags(g, user, args);
	if (err != 0) {
		args->dmabuf_fd = -1;
		goto fail;
	}

	num_queue = nvgpu_nvs_translate_queue_num(args->queue_num);
	queue_direction = nvgpu_nvs_translate_queue_direction(args->direction);

	queue = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo, num_queue, queue_direction, &mask);
	if (queue == NULL) {
		err = -EOPNOTSUPP;
		goto fail;
	}

	/* queue direction is needed by gsp scheduler */
	queue->queue_direction = queue_direction;

	read_only = (args->access_type == NVGPU_NVS_CTRL_FIFO_QUEUE_ACCESS_TYPE_EXCLUSIVE) ? false : true;
	if (read_only) {
		flag |= O_RDONLY;
	} else {
		flag |= O_RDWR;
	}

	if (args->access_type == NVGPU_NVS_CTRL_FIFO_QUEUE_ACCESS_TYPE_EXCLUSIVE) {
		/* Observers are not supported for Control Queues, So ensure, buffer is invalid */
		if (nvgpu_nvs_buffer_is_valid(g, queue) && (num_queue == NVGPU_NVS_NUM_CONTROL)) {
			err = -EBUSY;
			goto fail;
		}
	}

	/* Prevent multiple subscription by the same user. */
	if (nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(user, queue)) {
		err = -EEXIST;
		goto fail;
	}

	queue_size = NVS_QUEUE_DEFAULT_SIZE;

	/* Exclusive User or First Observer */
	if (!nvgpu_nvs_buffer_is_valid(g, queue)) {
		err = nvgpu_nvs_alloc_and_get_buf(g, queue, queue_size, mask, read_only);
	} else {
		/* User is not already subscribed.
		 * Other observers or (Exclusive User & Event Queue).
		 */
		err = nvgpu_nvs_get_buf(g, queue, read_only);
	}

	/*
	 * sending control fifo info to GSP scheduler
	 * currently only control and message queues
	 * are supported and not event queue
	 */
#if defined (CONFIG_NVS_PRESENT) && defined (CONFIG_NVGPU_GSP_SCHEDULER)
	if (nvgpu_is_enabled(g, (u32)(NVGPU_SUPPORT_GSP_SCHED))) {
		if ((nvgpu_gsp_is_ready(g) == true) &&
			(num_queue == NVGPU_NVS_NUM_CONTROL)) {
			err = nvgpu_gsp_sched_send_queue_info(g, queue, queue_direction);
		}
	}
#endif
	if (err != 0) {
		goto fail;
	}

	/* At this point
	 * 1) dma_mapping exists
	 * 2) An instance of struct dma_buf * exists in priv->dmabuf_temp
	 */

	priv = queue->priv;

	fd = dma_buf_fd(priv->dmabuf_temp, flag);
	if (fd < 0) {
		/* Release the dmabuf pointer here */
		dma_buf_put(priv->dmabuf_temp);
		priv->dmabuf_temp = NULL;

		/* Erase mapping for num_queues = NVGPU_NVS_NUM_CONTROL,
		 * For num_queues = NVGPU_NVS_NUM_EVENT, erase only if
		 * underlying backing buffer is not already being used.
		 */
		if ((num_queue == NVGPU_NVS_NUM_CONTROL) ||
				!nvgpu_nvs_ctrl_fifo_queue_has_subscribed_users(queue)) {
			nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, queue);
		}
		err = fd;
		goto fail;
	}

	nvgpu_nvs_ctrl_fifo_user_subscribe_queue(user, queue);

	args->dmabuf_fd = fd;
	/* update the queue size correctly */
	args->queue_size = queue_size;

	nvgpu_nvs_ctrl_fifo_unlock_queues(g);
	return 0;

fail:
	nvgpu_nvs_ctrl_fifo_unlock_queues(g);
	nvgpu_err(g, "failed");

	/* set dmabuf_fd to -1 for failure exclusively */
	args->dmabuf_fd = -1;

	return err;
}

static void nvgpu_nvs_ctrl_fifo_undo_create_queue(struct gk20a *g,
		struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_fifo_ioctl_create_queue_args *args)
{
	enum nvgpu_nvs_ctrl_queue_num num_queue;
	enum nvgpu_nvs_ctrl_queue_direction queue_direction;
	struct nvgpu_nvs_ctrl_queue *queue;
	struct nvgpu_nvs_linux_buf_priv *priv;
	u8 mask = 0;

	nvgpu_nvs_ctrl_fifo_lock_queues(g);

	num_queue = nvgpu_nvs_translate_queue_num(args->queue_num);
	queue_direction = nvgpu_nvs_translate_queue_direction(args->direction);

	queue = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo, num_queue, queue_direction, &mask);
	if (queue == NULL) {
		nvgpu_nvs_ctrl_fifo_unlock_queues(g);
		return;
	}

	priv = (struct nvgpu_nvs_linux_buf_priv *)queue->priv;

	nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(user, queue);

	/* put the dma_buf here */
	dma_buf_put(priv->dmabuf_temp);
	priv->dmabuf_temp = NULL;

	/* Control queues has no other subscribed users,
	 * Event queue might have other subscribed users.
	 */
	if (nvgpu_nvs_buffer_is_valid(g, queue) &&
			!nvgpu_nvs_ctrl_fifo_queue_has_subscribed_users(queue)) {
		nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, queue);
	}

	put_unused_fd(args->dmabuf_fd);
	args->dmabuf_fd = 0;

	nvgpu_nvs_ctrl_fifo_unlock_queues(g);
}

static int nvgpu_nvs_ctrl_fifo_destroy_queue(struct gk20a *g,
		struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_fifo_ioctl_release_queue_args *args)
{
	enum nvgpu_nvs_ctrl_queue_num num_queue;
	enum nvgpu_nvs_ctrl_queue_direction queue_direction;
	struct nvgpu_nvs_ctrl_queue *queue;
	bool is_exclusive_user;
	int err = 0;
	u8 mask = 0;

	if (args->reserve0 != 0) {
		return -EINVAL;
	}

	if (args->reserve1 != 0) {
		return -EINVAL;
	}

	if (args->reserve2 != 0) {
		return -EINVAL;
	}

	num_queue = nvgpu_nvs_translate_queue_num(args->queue_num);
	queue_direction = nvgpu_nvs_translate_queue_direction(args->direction);

	is_exclusive_user = nvgpu_nvs_ctrl_fifo_is_exclusive_user(g->sched_ctrl_fifo, user);
	if (!is_exclusive_user) {
		if ((num_queue == NVGPU_NVS_NUM_CONTROL) ||
			(queue_direction == NVGPU_NVS_DIR_CLIENT_TO_SCHEDULER)) {
			return -EPERM;
		}
	}

	nvgpu_nvs_ctrl_fifo_lock_queues(g);

	queue = nvgpu_nvs_ctrl_fifo_get_queue(g->sched_ctrl_fifo, num_queue, queue_direction, &mask);
	if (queue == NULL) {
		err = -EOPNOTSUPP;
		goto fail;
	}

	if (!nvgpu_nvs_ctrl_fifo_user_is_subscribed_to_queue(user, queue)) {
		err = -EPERM;
		goto fail;
	}

	nvgpu_nvs_ctrl_fifo_user_unsubscribe_queue(user, queue);

	/* For Event Queues, don't erase even if the buffer
	 * is currently not mapped. There might be some observers
	 * who has acquired the dma_bufs but hasn't mapped yet.
	 * Erase the queue only when the last user is removed.
	 *
	 * For Control Queues, no mappings should exist
	 */
	if (num_queue == NVGPU_NVS_NUM_CONTROL) {
		if (!nvgpu_nvs_buf_linux_is_mapped(g, queue)) {
			nvgpu_nvs_ctrl_fifo_erase_queue_locked(g, queue);
		} else {
			err = -EBUSY;
			goto fail;
		}
	}

	nvgpu_nvs_ctrl_fifo_unlock_queues(g);

	return 0;

fail:
	nvgpu_nvs_ctrl_fifo_unlock_queues(g);
	nvgpu_err(g, "failed to destroy queue");

	return err;
}

static u32 nvgpu_nvs_translate_hw_scheduler_impl(struct gk20a *g, uint8_t impl)
{
	if (impl == NVGPU_NVS_DOMAIN_SCHED_KMD) {
		return NVGPU_NVS_DOMAIN_SCHED_KMD;
	} else if (impl == NVGPU_NVS_DOMAIN_SCHED_GSP) {
		return NVGPU_NVS_DOMAIN_SCHED_GSP;
	}

	return NVGPU_NVS_DOMAIN_SCHED_INVALID;
}

static int nvgpu_nvs_query_scheduler_characteristics(struct gk20a *g,
		struct nvs_domain_ctrl_fifo_user *user,
		struct nvgpu_nvs_ctrl_fifo_ioctl_query_scheduler_characteristics_args *args)
{
	struct nvs_domain_ctrl_fifo_capabilities *capabilities;

	if (args->reserved0 != 0) {
		return -EINVAL;
	}

	if (args->reserved1 != 0) {
		return -EINVAL;
	}

	capabilities = nvgpu_nvs_ctrl_fifo_get_capabilities(g->sched_ctrl_fifo);
	args->domain_scheduler_implementation =
		nvgpu_nvs_translate_hw_scheduler_impl(g, capabilities->scheduler_implementation_hw);
	args->available_queues = NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_EVENT;

	if (user->has_write_access) {
		args->available_queues |= NVGPU_NVS_CTRL_FIFO_QUEUE_NUM_CONTROL;
	}

	return 0;
}

long nvgpu_nvs_ctrl_fifo_ops_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	u8 buf[NVGPU_NVS_CTRL_FIFO_IOCTL_MAX_ARG_SIZE] = { 0 };
	int err = 0;
	struct nvgpu_nvs_domain_ctrl_fifo_user_linux *linux_user;
	struct nvs_domain_ctrl_fifo_user *user;
	struct gk20a *g;

	linux_user = (struct nvgpu_nvs_domain_ctrl_fifo_user_linux *)filp->private_data;
	if (linux_user == NULL) {
		return -ENODEV;
	}

	user = &linux_user->user;
	g = nvgpu_get_gk20a_from_cdev(linux_user->cdev);

	nvs_dbg(g, "IOC_TYPE: %c", _IOC_TYPE(cmd));
	nvs_dbg(g, "IOC_NR:   %u", _IOC_NR(cmd));
	nvs_dbg(g, "IOC_SIZE: %u", _IOC_SIZE(cmd));

	if ((_IOC_TYPE(cmd) != NVGPU_NVS_CTRL_FIFO_IOCTL_MAGIC) ||
		(_IOC_NR(cmd) == 0) ||
		(_IOC_NR(cmd) > NVGPU_NVS_CTRL_FIFO_IOCTL_LAST) ||
		(_IOC_SIZE(cmd) > NVGPU_NVS_CTRL_FIFO_IOCTL_MAX_ARG_SIZE)) {
		nvs_dbg(g, "-> BAD!!");
		return -EINVAL;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(buf, (void __user *)arg, _IOC_SIZE(cmd)))
			return -EFAULT;
	}

	err = gk20a_busy(g);
	if (err != 0) {
		return err;
	}

	switch (cmd) {
	case NVGPU_NVS_CTRL_FIFO_IOCTL_CREATE_QUEUE:
	{
		struct nvgpu_nvs_ctrl_fifo_ioctl_create_queue_args *args =
			(struct nvgpu_nvs_ctrl_fifo_ioctl_create_queue_args *)buf;

		err = nvgpu_nvs_ctrl_fifo_create_queue(g, user, args);
		if (err)
			goto done;

		if (copy_to_user((void __user *)arg, buf, _IOC_SIZE(cmd))) {
			nvgpu_nvs_ctrl_fifo_undo_create_queue(g, user, args);
			err = -EFAULT;
			args->dmabuf_fd = -1;
			args->queue_size = 0;
			goto done;
		}

		break;
	}
	case NVGPU_NVS_CTRL_FIFO_IOCTL_RELEASE_QUEUE:
	{
		struct nvgpu_nvs_ctrl_fifo_ioctl_release_queue_args *args =
			(struct nvgpu_nvs_ctrl_fifo_ioctl_release_queue_args *)buf;

		err = nvgpu_nvs_ctrl_fifo_destroy_queue(g, user, args);
		if (err)
			goto done;

		break;
	}
	case NVGPU_NVS_CTRL_FIFO_IOCTL_ENABLE_EVENT:
	{
		err = -EOPNOTSUPP;
		goto done;
	}
	case NVGPU_NVS_CTRL_FIFO_IOCTL_QUERY_SCHEDULER_CHARACTERISTICS:
	{
		struct nvgpu_nvs_ctrl_fifo_ioctl_query_scheduler_characteristics_args *args =
			(struct nvgpu_nvs_ctrl_fifo_ioctl_query_scheduler_characteristics_args *)buf;

		if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_NVS)) {
			err = -EOPNOTSUPP;
			return err;
		}

		err = nvgpu_nvs_query_scheduler_characteristics(g, user, args);
		if (err != 0) {
			return err;
		}

		if (copy_to_user((void __user *)arg, buf, _IOC_SIZE(cmd))) {
			err = -EFAULT;
			goto done;
		}

		break;
	}
	default:
		err = -ENOTTY;
		goto done;
	}

done:
	gk20a_idle(g);
	return err;
}
