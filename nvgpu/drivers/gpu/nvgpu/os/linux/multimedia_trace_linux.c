// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>
#include <nvgpu/trace.h>
#include <uapi/linux/nvgpu.h>
#include <nvgpu/log.h>
#include <nvgpu/atomic.h>
#include <nvgpu/barrier.h>
#include <nvgpu/gk20a.h>
#include <nvgpu/enabled.h>
#include <nvgpu/multimedia.h>
#include <nvgpu/multimedia_trace.h>
#include <nvgpu/multimedia_trace_ctrl.h>
#include <nvgpu/nvgpu_init.h>

#include "platform_gk20a.h"
#include "os_linux.h"
#include "multimedia_trace_linux.h"
#include "ioctl.h"

struct nvgpu_multimedia_dev {
	struct gk20a *g;
	struct nvgpu_multimedia_ring_header *hdr;
	struct nvgpu_multimedia_trace_entry_internal *ents;
	bool write_enabled;
	struct nvgpu_cond readout_wq;
	size_t size;
	u32 num_ents;
	nvgpu_atomic_t vma_ref;
	struct nvgpu_mutex write_lock;
	u32 multimedia_id;
};

struct nvgpu_multimedia_trace_devs {
	struct nvgpu_multimedia_dev devs[NVGPU_MULTIMEDIA_ENGINE_MAX];
};

struct nvgpu_multimedia_dev_ctx {
	struct gk20a *g;
	struct nvgpu_multimedia_dev *dev;
	u32 dev_type;
	u32 instance;
	u32 size;
};

static inline int ring_is_empty(struct nvgpu_multimedia_ring_header *hdr)
{
	return (hdr->write_idx == hdr->read_idx);
}

static inline int ring_is_full(struct nvgpu_multimedia_ring_header *hdr)
{
	return ((hdr->write_idx + 1) % hdr->num_ents) == hdr->read_idx;
}

static inline int ring_len(struct nvgpu_multimedia_ring_header *hdr)
{
	return (hdr->write_idx - hdr->read_idx) % hdr->num_ents;
}

static void nvgpu_set_multimedia_trace_entry(
	struct nvgpu_multimedia_trace_entry *entry_dst,
	struct nvgpu_multimedia_trace_entry_internal *entry_src)
{
	entry_dst->tag = entry_src->tag;
	entry_dst->vmid = entry_src->vmid;
	entry_dst->seqno = entry_src->seqno;
	entry_dst->context_id = entry_src->context_id;
	entry_dst->pid = entry_src->pid;
	entry_dst->timestamp = entry_src->timestamp;
}

ssize_t nvgpu_multimedia_dev_read(struct file *filp, char __user *buf, size_t size,
				loff_t *off)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = filp->private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;
	struct nvgpu_multimedia_ring_header *hdr;
	struct nvgpu_multimedia_trace_entry __user *entry =
		(struct nvgpu_multimedia_trace_entry *) buf;
	struct nvgpu_multimedia_trace_entry user_entry;
	size_t copied = 0;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme,
			"filp=%p buf=%p size=%zu", filp, buf, size);

	if (dev == NULL) {
		goto ret_size;
	}

	hdr = dev->hdr;

	nvgpu_mutex_acquire(&dev->write_lock);
	while (ring_is_empty(hdr)) {
		nvgpu_mutex_release(&dev->write_lock);
		if (filp->f_flags & O_NONBLOCK) {
			return -EAGAIN;
		}
		err = NVGPU_COND_WAIT_INTERRUPTIBLE(&dev->readout_wq,
						!ring_is_empty(hdr), 0U);
		if (err) {
			return err;
		}
		nvgpu_mutex_acquire(&dev->write_lock);
	}

	while (size >= sizeof(struct nvgpu_multimedia_trace_entry_internal)) {
		if (ring_is_empty(hdr)) {
			break;
		}

		nvgpu_set_multimedia_trace_entry(&user_entry,
						&dev->ents[hdr->read_idx]);
		if (copy_to_user(entry, &user_entry, sizeof(*entry))) {
			nvgpu_mutex_release(&dev->write_lock);
			return -EFAULT;
		}

		hdr->read_idx++;
		if (hdr->read_idx >= hdr->num_ents) {
			hdr->read_idx = 0;
		}

		entry++;
		copied += sizeof(*entry);
		size -= sizeof(*entry);
	}

	nvgpu_log(g, gpu_dbg_mme, "copied=%zu read_idx=%d", copied,
		hdr->read_idx);

	*off = hdr->read_idx;
	nvgpu_mutex_release(&dev->write_lock);

ret_size:
	return copied;
}

static int nvgpu_multimedia_dev_ioctl_trace_enable(struct nvgpu_multimedia_dev_ctx *dev_ctx)
{
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;

	if (dev == NULL) {
		return -ENODEV;
	}

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "trace enabled for multimedia %u",
			dev->multimedia_id);

	nvgpu_mutex_acquire(&dev->write_lock);
	dev->write_enabled = true;
	nvgpu_mutex_release(&dev->write_lock);

	nvgpu_multimedia_trace_ctrl_enable(g, dev->multimedia_id);

	return 0;
}

static int nvgpu_multimedia_dev_ioctl_trace_disable(struct nvgpu_multimedia_dev_ctx *dev_ctx)
{
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;

	if (dev == NULL) {
		return -ENODEV;
	}

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "trace disabled for multimedia %u",
			dev->multimedia_id);

	nvgpu_multimedia_trace_ctrl_disable(g, dev->multimedia_id);

	nvgpu_mutex_acquire(&dev->write_lock);
	dev->write_enabled = false;
	nvgpu_mutex_release(&dev->write_lock);

	return 0;
}

static int nvgpu_multimedia_trace_ring_alloc(struct gk20a *g,
					void **buf, size_t *size)
{
	struct nvgpu_multimedia_ring_header *hdr;
	u64 total_size = 0ULL;

	*size = round_up(*size, NVGPU_CPU_PAGE_SIZE);
	hdr = vmalloc_user(*size);
	if (!hdr) {
		return -ENOMEM;
	}

	hdr->magic = NVGPU_MULTIMEDIA_RING_HEADER_MAGIC;
	hdr->version = NVGPU_MULTIMEDIA_RING_HEADER_VERSION;
	if (!nvgpu_safe_sub_u64_return(*size, sizeof(struct nvgpu_multimedia_ring_header),
					&total_size)) {
		nvgpu_err(g, "total_size overflow");
		return -EOVERFLOW;
	}

	hdr->num_ents = total_size / sizeof(struct nvgpu_multimedia_trace_entry_internal);
	hdr->ent_size = sizeof(struct nvgpu_multimedia_trace_entry_internal);
	hdr->drop_count = 0;
	hdr->read_idx = 0;
	hdr->write_idx = 0;
	hdr->write_seqno = 0;

	*buf = hdr;
	return 0;
}

static void nvgpu_multimedia_trace_ring_free(struct nvgpu_multimedia_dev *dev)
{
	nvgpu_vfree(dev->g, dev->hdr);
	dev->hdr = NULL;
}

static int nvgpu_multimedia_dev_alloc_buffer(struct nvgpu_multimedia_dev *dev,
					size_t *size)
{
	struct gk20a *g = dev->g;
	void *buf;
	int err;

	if ((dev->write_enabled) || (nvgpu_atomic_read(&dev->vma_ref))) {
		return -EBUSY;
	}

	if (dev->hdr) {
		nvgpu_multimedia_trace_ring_free(dev);
	}

	err = nvgpu_multimedia_trace_ring_alloc(g, &buf, size);
	if (err) {
		return err;
	}

	dev->hdr = buf;
	dev->ents = (struct nvgpu_multimedia_trace_entry_internal *) (dev->hdr + 1);
	dev->size = *size;
	dev->num_ents = dev->hdr->num_ents;

	nvgpu_log(g, gpu_dbg_mme, "size=%zu hdr=%p ents=%p num_ents=%d ent_size=%u",
		dev->size, dev->hdr, dev->ents, dev->hdr->num_ents, dev->hdr->ent_size);
	return 0;
}

static int nvgpu_multimedia_dev_ioctl_ring_setup(struct nvgpu_multimedia_dev_ctx *dev_ctx,
	struct nvgpu_multimedia_ring_setup_args *args)
{
	struct nvgpu_multimedia_dev *dev;
	struct gk20a *g = dev_ctx->g;
	struct nvgpu_multimedia_trace_devs *trace_devs;
	size_t size = args->size;
	u32 multimedia_id, dev_type;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "size=%zu eng_id %u instance %u",
					size, args->engine_id, args->instance);

	if (size > NVGPU_MULTIMEDIA_TRACE_MAX_VM_RING_SIZE) {
		return -EINVAL;
	}

	if (dev_ctx->dev != NULL) {
		nvgpu_err(g, "ring_setup is already done! not supporting re-config");
		return -EBUSY;
	}

	trace_devs = g->multimedia_trace_devs;
	if (!trace_devs) {
		return -ENODEV;
	}

	dev_type = nvgpu_engine_id_to_dev_type(args->engine_id);
	if (dev_type == NVGPU_MAX_DEVTYPE) {
		return -EINVAL;
	}

	multimedia_id = nvgpu_multimedia_get_id_from_dev(dev_type, args->instance);
	if (multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX) {
		nvgpu_err(g, "Not supported, multimedia_id %u eng_id %u dev_type %u instance %u",
				multimedia_id, args->engine_id, dev_type, args->instance);
		return -EINVAL;
	}

	dev = &trace_devs->devs[multimedia_id];
	nvgpu_mutex_acquire(&dev->write_lock);

	dev->multimedia_id = multimedia_id;
	if (dev->hdr) {
		err = -EBUSY;
		goto done;
	}

	/* If size is not passed, use default size */
	if (size <= 0) {
		size = sizeof(struct nvgpu_multimedia_ring_header) +
					MULTIMEDIA_TRACE_NUM_RECORDS *
					sizeof(struct nvgpu_multimedia_trace_entry_internal);
	}

	err = nvgpu_multimedia_dev_alloc_buffer(dev, &size);
	if (!err) {
		nvgpu_log(g, gpu_dbg_mme, "dev=%p size=%zu", dev, size);
	}

	dev_ctx->dev = dev;

done:
	nvgpu_mutex_release(&dev->write_lock);

	args->size = size;
	return err;
}

static int nvgpu_multimedia_dev_ioctl_poll(struct nvgpu_multimedia_dev_ctx *dev_ctx)
{
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, " ");

	if (dev == NULL) {
		return -ENODEV;
	}

	err = gk20a_busy(g);
	if (err) {
		return err;
	}

	nvgpu_multimedia_trace_ctrl_poll(g, dev->multimedia_id);

	gk20a_idle(g);
	return err;
}

int nvgpu_multimedia_dev_open(struct inode *inode, struct file *filp)
{
	struct gk20a *g;
	struct nvgpu_multimedia_dev_ctx *dev_ctx;
	struct nvgpu_cdev *cdev;
	int err;

	cdev = container_of(inode->i_cdev, struct nvgpu_cdev, cdev);
	g = nvgpu_get_gk20a_from_cdev(cdev);

	g = nvgpu_get(g);
	if (!g) {
		return -ENODEV;
	}

	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE)) {
		nvgpu_put(g);
		return -ENODEV;
	}

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "g=%p", g);

	err = gk20a_busy(g);
	if (err) {
		goto free_ref;
	}

	dev_ctx = nvgpu_kzalloc(g, sizeof(*dev_ctx));
	if (unlikely(!dev_ctx)) {
		return -ENOMEM;
	}

	dev_ctx->g = g;
	filp->private_data = dev_ctx;

	nvgpu_log(g, gpu_dbg_mme, "filp=%p dev_ctx=%p", filp, dev_ctx);

	gk20a_idle(g);

free_ref:
	if (err) {
		nvgpu_put(g);
	}

	return err;
}

int nvgpu_multimedia_dev_release(struct inode *inode, struct file *filp)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = filp->private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "dev: %p", dev);

	if (dev == NULL) {
		goto free_dev_ctx;
	}

	nvgpu_mutex_acquire(&dev->write_lock);
	if (dev->write_enabled) {
		dev->write_enabled = false;
		nvgpu_multimedia_trace_ctrl_disable(g, dev->multimedia_id);
	}
	nvgpu_mutex_release(&dev->write_lock);

	if (dev->hdr) {
		nvgpu_multimedia_trace_ring_free(dev);
	}

free_dev_ctx:
	nvgpu_kfree(g, dev_ctx);
	nvgpu_put(g);

	return 0;
}

long nvgpu_multimedia_dev_ioctl(struct file *filp, unsigned int cmd,
	unsigned long arg)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = filp->private_data;
	struct gk20a *g = dev_ctx->g;
	void *buf = NULL;
	int err = 0;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "nr=%d", _IOC_NR(cmd));

	if ((_IOC_TYPE(cmd) != NVGPU_MULTIMEDIA_IOCTL_MAGIC) ||
		(_IOC_NR(cmd) == 0) ||
		(_IOC_NR(cmd) > NVGPU_MULTIMEDIA_IOCTL_LAST) ||
		(_IOC_SIZE(cmd) > NVGPU_MULTIMEDIA_IOCTL_MAX_ARG_SIZE)) {
		return -EINVAL;
	}

	buf = nvgpu_kzalloc(g, NVGPU_MULTIMEDIA_IOCTL_MAX_ARG_SIZE);
	if (buf == NULL) {
		return -ENOMEM;
	}

	if (_IOC_DIR(cmd) & _IOC_WRITE) {
		if (copy_from_user(buf, (void __user *) arg, _IOC_SIZE(cmd))) {
			err = -EFAULT;
			goto cleanup;
		}
	}

	switch (cmd) {
	case NVGPU_MULTIMEDIA_IOCTL_TRACE_ENABLE:
		err = nvgpu_multimedia_dev_ioctl_trace_enable(dev_ctx);
		break;
	case NVGPU_MULTIMEDIA_IOCTL_TRACE_DISABLE:
		err = nvgpu_multimedia_dev_ioctl_trace_disable(dev_ctx);
		break;
	case NVGPU_MULTIMEDIA_IOCTL_RING_SETUP:
		err = nvgpu_multimedia_dev_ioctl_ring_setup(dev_ctx,
			(struct nvgpu_multimedia_ring_setup_args *) buf);
		break;
	case NVGPU_MULTIMEDIA_IOCTL_POLL:
		err = nvgpu_multimedia_dev_ioctl_poll(dev_ctx);
		break;
	default:
		dev_dbg(dev_from_gk20a(g), "unrecognized gpu ioctl cmd: 0x%x",
			cmd);
		err = -ENOTTY;
	}

	if ((err == 0) && (_IOC_DIR(cmd) & _IOC_READ)) {
		err = copy_to_user((void __user *) arg, buf, _IOC_SIZE(cmd));
	}

cleanup:
	nvgpu_kfree(g, buf);
	return err;
}

unsigned int nvgpu_multimedia_dev_poll(struct file *filp, poll_table *wait)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = filp->private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct nvgpu_multimedia_ring_header *hdr = dev->hdr;
	unsigned int mask = 0;

	if (unlikely(!dev)) {
		goto return_mask;
	}

	nvgpu_mutex_acquire(&dev->write_lock);
	poll_wait(filp, &dev->readout_wq.wq, wait);
	if (!ring_is_empty(hdr)) {
		mask |= POLLIN | POLLRDNORM;
	}
	nvgpu_mutex_release(&dev->write_lock);

return_mask:
	return mask;
}

static void nvgpu_multimedia_dev_vma_open(struct vm_area_struct *vma)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = vma->vm_private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;

	nvgpu_atomic_inc(&dev->vma_ref);
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "vma_ref=%d",
		nvgpu_atomic_read(&dev->vma_ref));
}

static void nvgpu_multimedia_dev_vma_close(struct vm_area_struct *vma)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = vma->vm_private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;

	nvgpu_atomic_dec(&dev->vma_ref);
	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "vma_ref=%d",
		nvgpu_atomic_read(&dev->vma_ref));
}

static const struct vm_operations_struct nvgpu_multimedia_dev_vma_ops = {
	.open = nvgpu_multimedia_dev_vma_open,
	.close = nvgpu_multimedia_dev_vma_close,
};

int nvgpu_multimedia_dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct nvgpu_multimedia_dev_ctx *dev_ctx = filp->private_data;
	struct nvgpu_multimedia_dev *dev = dev_ctx->dev;
	struct gk20a *g = dev_ctx->g;
	size_t mmapsize = 0;
	void *mmapaddr;
	int ret;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "vm_start=%lx vm_end=%lx",
		vma->vm_start, vma->vm_end);

	if (dev == NULL) {
		return -ENODEV;
	}

	mmapaddr = dev->hdr;

	if (mmapsize) {
		unsigned long size = 0;
		unsigned long vsize = vma->vm_end - vma->vm_start;

		size = min(mmapsize, vsize);
		size = round_up(size, NVGPU_CPU_PAGE_SIZE);

		ret = remap_pfn_range(vma, vma->vm_start,
			(unsigned long) mmapaddr,
			size,
			vma->vm_page_prot);

	} else {
		ret = remap_vmalloc_range(vma, mmapaddr, 0);
	}

	if (likely(!ret)) {
		vma->vm_private_data = dev_ctx;
		vma->vm_ops = &nvgpu_multimedia_dev_vma_ops;
		vma->vm_ops->open(vma);
	}

	return ret;
}

static int nvgpu_multimedia_init_devs(struct gk20a *g)
{
	struct nvgpu_multimedia_trace_devs *trace_devs = g->multimedia_trace_devs;
	struct nvgpu_multimedia_dev *dev = trace_devs->devs;
	int i;

	for (i = 0; i < NVGPU_MULTIMEDIA_ENGINE_MAX; i++) {
		dev->g = g;
		dev->hdr = NULL;
		dev->write_enabled = false;
		nvgpu_cond_init(&dev->readout_wq);
		nvgpu_mutex_init(&dev->write_lock);
		nvgpu_atomic_set(&dev->vma_ref, 0);
		dev++;
	}
	return 0;
}

int nvgpu_multimedia_trace_init(struct gk20a *g)
{
	struct nvgpu_multimedia_trace_devs *trace_devs = g->multimedia_trace_devs;
	int err;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "g=%p trace_devs=%p", g, trace_devs);

	if (!g->support_gpu_tools) {
		nvgpu_log_info(g, "multimedia tracing initialization skipped. disabled via DT.");
		nvgpu_set_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE, false);
		return 0;
	}

	/* if tracing is not supported, skip this */
	if (!nvgpu_is_enabled(g, NVGPU_SUPPORT_MULTIMEDIA_TRACE)) {
		return 0;
	}

	if (likely(trace_devs)) {
		return 0;
	}

	trace_devs = nvgpu_kzalloc(g, sizeof(*trace_devs));
	if (unlikely(!trace_devs)) {
		return -ENOMEM;
	}
	g->multimedia_trace_devs = trace_devs;

	err = nvgpu_multimedia_init_devs(g);
	if (err) {
		goto fail;
	}

	err = nvgpu_multimedia_trace_ctrl_init(g);
	if (unlikely(err)) {
		goto fail;
	}

	return 0;

fail:
	nvgpu_kfree(g, trace_devs);
	g->multimedia_trace_devs = NULL;
	return err;
}

void nvgpu_multimedia_trace_cleanup(struct gk20a *g)
{
	struct nvgpu_multimedia_trace_devs *trace_devs;
	struct nvgpu_multimedia_dev *dev;
	int i;

	if (!g->multimedia_trace_devs) {
		return;
	}

	trace_devs = g->multimedia_trace_devs;
	dev = trace_devs->devs;

	for (i = 0; i < NVGPU_MULTIMEDIA_ENGINE_MAX; i++) {
		nvgpu_mutex_destroy(&dev->write_lock);
		dev++;
	}

	nvgpu_kfree(g, g->multimedia_trace_devs);
	g->multimedia_trace_devs = NULL;

	nvgpu_multimedia_trace_ctrl_deinit(g);
}

int nvgpu_multimedia_trace_write_entry(struct gk20a *g, u32 multimedia_id,
		struct nvgpu_multimedia_trace_entry_internal *entry)
{
	struct nvgpu_multimedia_ring_header *hdr;
	struct nvgpu_multimedia_dev *dev;
	int ret = 0;
	const char *reason;
	u32 write_idx;

	if (unlikely(!g->multimedia_trace_devs)) {
		return 0;
	}

	if (unlikely(multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX)) {
		return -EINVAL;
	}

	dev = &g->multimedia_trace_devs->devs[multimedia_id];
	hdr = dev->hdr;

	nvgpu_log(g, gpu_dbg_fn | gpu_dbg_mme, "dev=%p hdr=%p", dev, hdr);

	nvgpu_mutex_acquire(&dev->write_lock);

	if (unlikely(!hdr)) {
		/* device has been released */
		ret = -ENODEV;
		goto done;
	}

	write_idx = hdr->write_idx;
	if (write_idx >= dev->num_ents) {
		nvgpu_err(dev->g, "write_idx=%u out of range [0..%u]",
			write_idx, dev->num_ents);
		ret = -ENOSPC;
		reason = "write_idx out of range";
		goto disable;
	}

	entry->seqno = hdr->write_seqno++;
	entry->vmid = gk20a_get_vmid(g);

	if (!dev->write_enabled) {
		ret = -EBUSY;
		reason = "write disabled";
		goto drop;
	}

	if (unlikely(ring_is_full(hdr))) {
		ret = -ENOSPC;
		reason = "user fifo full";
		goto drop;
	}

	nvgpu_log(g, gpu_dbg_mme,
		"seqno=%d context_id=%08x pid=%lld tag=%x timestamp=%llx",
		entry->seqno, entry->context_id, entry->pid,
		entry->tag, entry->timestamp);

	dev->ents[write_idx] = *entry;

	/* ensure record is written before updating write index */
	nvgpu_smp_wmb();

	write_idx++;
	if (unlikely(write_idx >= hdr->num_ents)) {
		write_idx = 0;
	}
	hdr->write_idx = write_idx;
	nvgpu_log(g, gpu_dbg_mme, "added: read=%d write=%d len=%d",
		hdr->read_idx, hdr->write_idx, ring_len(hdr));

	nvgpu_mutex_release(&dev->write_lock);
	return ret;

disable:
	nvgpu_multimedia_trace_ctrl_disable(g, dev->multimedia_id);

drop:
	hdr->drop_count++;
	nvgpu_log(g, gpu_dbg_mme,
			"dropping seqno=%d context_id=%08x pid=%lld "
			"tag=%x time=%llx (%s)",
			entry->seqno, entry->context_id, entry->pid,
			entry->tag, entry->timestamp, reason);

done:
	nvgpu_mutex_release(&dev->write_lock);
	return ret;
}

void nvgpu_multimedia_trace_wake_up(struct gk20a *g, u32 multimedia_id)
{
	struct nvgpu_multimedia_dev *dev;

	if (unlikely(!g->multimedia_trace_devs)) {
		return;
	}

	if (unlikely(multimedia_id >= NVGPU_MULTIMEDIA_ENGINE_MAX)) {
		return;
	}

	dev = &g->multimedia_trace_devs->devs[multimedia_id];
	nvgpu_cond_signal_interruptible(&dev->readout_wq);
}

void nvgpu_multimedia_trace_add_tsg_reset(struct gk20a *g, struct nvgpu_tsg *tsg,
					u32 multimedia_id)
{
	int vmid = gk20a_get_vmid(g);
	struct nvgpu_multimedia_trace_entry_internal entry = {
		.vmid = vmid,
		.tag = NVGPU_MULTIMEDIA_TAG_ENGINE_RESET,
		.context_id = 0,
		.pid = tsg->tgid,
	};

	if (!g->multimedia_trace_devs) {
		return;
	}

	g->ops.ptimer.read_ptimer(g, &entry.timestamp);
	nvgpu_multimedia_trace_write_entry(g, multimedia_id, &entry);
	nvgpu_multimedia_trace_wake_up(g, multimedia_id);
}

/**
 * Convert multimedia trace event id of type MULTIMEDIA_EVENT_ID_* to
 * linux nvgpu multimedia tag of the type NVGPU_MULTIMEDIA_TAG_*.
 */
u8 nvgpu_multimedia_event_id_to_tag(u8 event_id)
{
	switch (event_id) {
	case MULTIMEDIA_EVENT_ID_CTXSW_REQ_BY_HOST:
		return NVGPU_MULTIMEDIA_TAG_CTXSW_REQ_BY_HOST;
	case MULTIMEDIA_EVENT_ID_RESTORE_START:
		return NVGPU_MULTIMEDIA_TAG_RESTORE_START;
	case MULTIMEDIA_EVENT_ID_CONTEXT_START:
		return NVGPU_MULTIMEDIA_TAG_CONTEXT_START;
	case MULTIMEDIA_EVENT_ID_SAVE_END:
		return NVGPU_MULTIMEDIA_TAG_SAVE_END;
	case MULTIMEDIA_EVENT_ID_SESSION_START:
		return NVGPU_MULTIMEDIA_TAG_SESSION_START;
	case MULTIMEDIA_EVENT_ID_SESSION_END:
		return NVGPU_MULTIMEDIA_TAG_SESSION_END;
	case MULTIMEDIA_EVENT_ID_POWER_STATE_CHANGE:
		return NVGPU_MULTIMEDIA_TAG_ENGINE_RESET;
	}

	return NVGPU_MULTIMEDIA_TAG_INVALID_TIMESTAMP;
}
