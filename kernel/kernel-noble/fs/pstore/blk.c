// SPDX-License-Identifier: GPL-2.0
/*
 * Implements pstore backend driver that write to block (or non-block) storage
 * devices, using the pstore/zone API.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/blk_types.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/pstore_blk.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/init_syscalls.h>
#include <linux/mount.h>
#include <linux/vmalloc.h>

static long kmsg_size = CONFIG_PSTORE_BLK_KMSG_SIZE;
module_param(kmsg_size, long, 0400);
MODULE_PARM_DESC(kmsg_size, "kmsg dump record size in kbytes");

static int max_reason = CONFIG_PSTORE_BLK_MAX_REASON;
module_param(max_reason, int, 0400);
MODULE_PARM_DESC(max_reason,
		 "maximum reason for kmsg dump (default 2: Oops and Panic)");

#if IS_ENABLED(CONFIG_PSTORE_PMSG)
static long pmsg_size = CONFIG_PSTORE_BLK_PMSG_SIZE;
#else
static long pmsg_size = -1;
#endif
module_param(pmsg_size, long, 0400);
MODULE_PARM_DESC(pmsg_size, "pmsg size in kbytes");

#if IS_ENABLED(CONFIG_PSTORE_CONSOLE)
static long console_size = CONFIG_PSTORE_BLK_CONSOLE_SIZE;
#else
static long console_size = -1;
#endif
module_param(console_size, long, 0400);
MODULE_PARM_DESC(console_size, "console size in kbytes");

#if IS_ENABLED(CONFIG_PSTORE_FTRACE)
static long ftrace_size = CONFIG_PSTORE_BLK_FTRACE_SIZE;
#else
static long ftrace_size = -1;
#endif
module_param(ftrace_size, long, 0400);
MODULE_PARM_DESC(ftrace_size, "ftrace size in kbytes");

static bool best_effort;
module_param(best_effort, bool, 0400);
MODULE_PARM_DESC(best_effort, "use best effort to write (i.e. do not require storage driver pstore support, default: off)");

/*
 * blkdev - the block device to use for pstore storage
 * See Documentation/admin-guide/pstore-blk.rst for details.
 */
static char blkdev[80] = CONFIG_PSTORE_BLK_BLKDEV;
module_param_string(blkdev, blkdev, 80, 0400);
MODULE_PARM_DESC(blkdev, "block device for pstore storage");

/*
 * All globals must only be accessed under the pstore_blk_lock
 * during the register/unregister functions.
 */
static DEFINE_MUTEX(pstore_blk_lock);
static struct file *psblk_file;
static struct pstore_device_info *pstore_device_info;

#define check_size(name, alignsize) ({				\
	long _##name_ = (name);					\
	_##name_ = _##name_ <= 0 ? 0 : (_##name_ * 1024);	\
	if (_##name_ & ((alignsize) - 1)) {			\
		pr_info(#name " must align to %d\n",		\
				(alignsize));			\
		_##name_ = ALIGN(name, (alignsize));		\
	}							\
	_##name_;						\
})

#define verify_size(name, alignsize, enabled) {			\
	long _##name_;						\
	if (enabled)						\
		_##name_ = check_size(name, alignsize);		\
	else							\
		_##name_ = 0;					\
	/* Synchronize module parameters with results. */	\
	name = _##name_ / 1024;					\
	dev->zone.name = _##name_;				\
}

static int __register_pstore_device(struct pstore_device_info *dev)
{
	int ret;

	lockdep_assert_held(&pstore_blk_lock);

	if (!dev) {
		pr_err("NULL device info\n");
		return -EINVAL;
	}
	if (!dev->zone.total_size) {
		pr_err("zero sized device\n");
		return -EINVAL;
	}
	if (!dev->zone.read) {
		pr_err("no read handler for device\n");
		return -EINVAL;
	}
	if (!dev->zone.write) {
		pr_err("no write handler for device\n");
		return -EINVAL;
	}

	/* someone already registered before */
	if (pstore_device_info)
		return -EBUSY;

	/* zero means no limit on which backends attempt to store. */
	if (!dev->flags)
		dev->flags = UINT_MAX;

	/* Copy in module parameters. */
	verify_size(kmsg_size, 4096, dev->flags & PSTORE_FLAGS_DMESG);
	verify_size(pmsg_size, 4096, dev->flags & PSTORE_FLAGS_PMSG);
	verify_size(console_size, 4096, dev->flags & PSTORE_FLAGS_CONSOLE);
	verify_size(ftrace_size, 4096, dev->flags & PSTORE_FLAGS_FTRACE);
	dev->zone.max_reason = max_reason;

	/* Initialize required zone ownership details. */
	dev->zone.name = KBUILD_MODNAME;
	dev->zone.owner = THIS_MODULE;

	ret = register_pstore_zone(&dev->zone);
	if (ret == 0)
		pstore_device_info = dev;

	return ret;
}
/**
 * register_pstore_device() - register non-block device to pstore/blk
 *
 * @dev: non-block device information
 *
 * Return:
 * * 0		- OK
 * * Others	- something error.
 */
int register_pstore_device(struct pstore_device_info *dev)
{
	int ret;

	mutex_lock(&pstore_blk_lock);
	ret = __register_pstore_device(dev);
	mutex_unlock(&pstore_blk_lock);

	return ret;
}
EXPORT_SYMBOL_GPL(register_pstore_device);

static void __unregister_pstore_device(struct pstore_device_info *dev)
{
	lockdep_assert_held(&pstore_blk_lock);
	if (pstore_device_info && pstore_device_info == dev) {
		unregister_pstore_zone(&dev->zone);
		pstore_device_info = NULL;
	}
}

/**
 * unregister_pstore_device() - unregister non-block device from pstore/blk
 *
 * @dev: non-block device information
 */
void unregister_pstore_device(struct pstore_device_info *dev)
{
	mutex_lock(&pstore_blk_lock);
	__unregister_pstore_device(dev);
	mutex_unlock(&pstore_blk_lock);
}
EXPORT_SYMBOL_GPL(unregister_pstore_device);

static ssize_t psblk_generic_blk_read(char *buf, size_t bytes, loff_t pos)
{
	return kernel_read(psblk_file, buf, bytes, &pos);
}

#define PSBLK_PANIC_MAX_PAGES		16
#define PSBLK_POLL_DELAY_US		100
#define PSBLK_POLL_MAX_ITERATIONS	30000

struct psblk_panic_completion {
	int done;
	blk_status_t status;
};

/*
 * panic_write is only used for panic dmesg records. pstore/zone serializes
 * those writes, so a single statically reserved bio is enough and avoids
 * bio allocation from the panic path.
 */
static struct bio psblk_panic_bio;
static struct bio_vec psblk_panic_bvecs[PSBLK_PANIC_MAX_PAGES];
static struct psblk_panic_completion psblk_panic_comp;

/*
 * Set when a submitted bio was abandoned without completing, either because it
 * timed out or because it came back without a poll cookie. The block layer
 * still owns psblk_panic_bio in that case, so it cannot be reinitialised: a
 * late completion would then run psblk_panic_bio_endio() against a bio that has
 * been rebuilt for a different request, corrupting the new one or releasing the
 * wrong tags. There is no way to prove the abandoned request is gone short of a
 * controller reset, so the panic writer is disabled for the rest of this boot
 * rather than reusing shared state of unknown ownership. This is one-way and is
 * only reachable once the machine is already panicking.
 */
static bool psblk_panic_poisoned;

static void psblk_panic_bio_endio(struct bio *bio)
{
	struct psblk_panic_completion *comp = bio->bi_private;

	WRITE_ONCE(comp->status, bio->bi_status);
	/* Pairs with psblk_panic_done() before reading comp->status. */
	smp_store_release(&comp->done, 1);
}

static bool psblk_panic_done(struct psblk_panic_completion *comp)
{
	/* Pairs with psblk_panic_bio_endio() after writing comp->status. */
	return smp_load_acquire(&comp->done);
}

static bool psblk_panic_check_capable(struct block_device *bdev)
{
	struct request_queue *q = bdev_get_queue(bdev);
	unsigned int max_pages;
	unsigned int logical;
	size_t worst_case;

	if (!q) {
		dev_dbg(&bdev->bd_device,
			"cannot use polled panic writes: missing request queue\n");
		return false;
	}

	if (!test_bit(QUEUE_FLAG_POLL, &q->queue_flags) || !q->tag_set ||
	    !q->tag_set->map[HCTX_TYPE_POLL].nr_queues) {
		dev_dbg(&bdev->bd_device,
			"cannot use polled panic writes: missing polled hardware queues\n");
		return false;
	}

	if (!bdev_nowait(bdev)) {
		dev_dbg(&bdev->bd_device,
			"cannot use polled panic writes: missing nowait support\n");
		return false;
	}

	if (bdev_write_cache(bdev) && !bdev_fua(bdev)) {
		dev_dbg(&bdev->bd_device,
			"cannot use polled panic writes: write cache enabled without FUA\n");
		return false;
	}

	/*
	 * psblk_panic_max_chunk() rounds the usable chunk down to the logical
	 * block size, after losing offset_in_page(buf) bytes of the first page.
	 * The pstore buffer is not guaranteed to be page aligned, so check the
	 * worst case here: if a maximally misaligned buffer cannot yield even a
	 * single logical block, every panic write would round down to zero and
	 * the record would be dropped. Refuse the device instead.
	 */
	max_pages = min_t(unsigned int, PSBLK_PANIC_MAX_PAGES,
			  queue_max_segments(q));
	max_pages = min_t(unsigned int, max_pages, BIO_MAX_VECS);
	worst_case = max_pages ?
		     ((size_t)max_pages << PAGE_SHIFT) - (PAGE_SIZE - 1) : 0;
	logical = queue_logical_block_size(q);
	if (worst_case < logical || queue_max_bytes(q) < logical) {
		dev_dbg(&bdev->bd_device,
			"cannot use polled panic writes: %u segment(s) cannot hold a %u byte block\n",
			max_pages, logical);
		return false;
	}

	return true;
}

static size_t psblk_panic_max_chunk(struct request_queue *q, const char *buf,
				    size_t bytes)
{
	unsigned int max_pages;
	unsigned int logical;
	size_t page_room;
	size_t max_bytes;

	max_pages = min_t(unsigned int, PSBLK_PANIC_MAX_PAGES,
			  queue_max_segments(q));
	max_pages = min_t(unsigned int, max_pages, BIO_MAX_VECS);
	if (!max_pages)
		return 0;

	page_room = ((size_t)max_pages << PAGE_SHIFT) - offset_in_page(buf);
	max_bytes = min_t(size_t, queue_max_bytes(q),
			  (size_t)max_pages << PAGE_SHIFT);
	max_bytes = min(max_bytes, page_room);

	logical = queue_logical_block_size(q);
	max_bytes = round_down(max_bytes, logical);

	return min(bytes, max_bytes);
}

static ssize_t psblk_panic_fill_bio(struct bio *bio, const char *src,
				    size_t bytes)
{
	size_t added = 0;

	while (added < bytes) {
		const void *addr = src + added;
		struct page *page;
		size_t page_off;
		size_t page_len;
		int ret;

		page_off = offset_in_page(addr);
		page_len = min_t(size_t, PAGE_SIZE - page_off,
				 bytes - added);

		if (is_vmalloc_addr(addr))
			page = vmalloc_to_page(addr);
		else
			page = virt_to_page(addr);

		if (!page) {
			pr_emerg("failed to get page for buffer\n");
			return -EFAULT;
		}

		ret = bio_add_page(bio, page, page_len, page_off);
		if (ret != page_len) {
			pr_emerg("failed to add page to panic bio\n");
			return -EIO;
		}

		added += ret;
	}

	return added;
}

static int psblk_panic_bio_status(struct psblk_panic_completion *comp)
{
	blk_status_t status = READ_ONCE(comp->status);
	int err;

	if (status == BLK_STS_OK)
		return 0;

	err = blk_status_to_errno(status);
	pr_emerg("write failed with block status %u, error %d\n",
		 (__force unsigned int)status, err);
	return err;
}

static int psblk_panic_submit_and_wait(struct bio *bio,
				       struct psblk_panic_completion *comp,
				       bool *completed)
{
	unsigned long iters = 0;
	bool can_poll;

	WRITE_ONCE(comp->status, BLK_STS_OK);
	WRITE_ONCE(comp->done, 0);
	*completed = false;

	submit_bio(bio);

	can_poll = READ_ONCE(bio->bi_cookie) != BLK_QC_T_NONE;
	if (!can_poll && !psblk_panic_done(comp)) {
		/*
		 * Without a cookie there is nothing to poll, and the panic path
		 * runs with interrupts disabled so the completion interrupt can
		 * never arrive either. blk-mq may also have queued the request
		 * to a worker that cannot be scheduled from here. Spinning to
		 * the timeout would only delay the panic by
		 * PSBLK_POLL_MAX_ITERATIONS * PSBLK_POLL_DELAY_US and still
		 * lose the record, so give up immediately and let the caller
		 * report the failure. The bio is still owned by the block
		 * layer, so leave *completed false and do not uninit it.
		 */
		pr_emerg("REQ_POLLED write did not return a poll cookie\n");
		return -EIO;
	}

	while (!psblk_panic_done(comp)) {
		if (iters++ >= PSBLK_POLL_MAX_ITERATIONS) {
			pr_emerg("write timed out after %lu poll attempts\n",
				 iters);
			return -ETIMEDOUT;
		}

		if (can_poll)
			bio_poll(bio, NULL, BLK_POLL_ONESHOT);
		cpu_relax();

		if (!psblk_panic_done(comp))
			udelay(PSBLK_POLL_DELAY_US);
	}

	*completed = true;
	return psblk_panic_bio_status(comp);
}

static ssize_t psblk_panic_blk_write(const char *buf, size_t bytes, loff_t pos)
{
	struct file *file = READ_ONCE(psblk_file);
	struct psblk_panic_completion *comp = &psblk_panic_comp;
	struct block_device *bdev;
	struct request_queue *q;
	size_t remaining = bytes;
	size_t written = 0;
	loff_t offset = pos;
	unsigned int logical;
	blk_opf_t opf;
	int ret;

	if (!bytes)
		return 0;

	if (psblk_panic_poisoned) {
		pr_emerg("panic writer disabled: an earlier write left a bio in flight\n");
		return -EIO;
	}

	if (!file) {
		pr_emerg("block device file is not available\n");
		return -ENODEV;
	}

	if (!buf) {
		pr_emerg("missing write buffer\n");
		return -EINVAL;
	}

	if (pos < 0) {
		pr_emerg("invalid negative write offset %lld\n", pos);
		return -EINVAL;
	}

	if ((pos | bytes) & (SECTOR_SIZE - 1)) {
		pr_emerg("unaligned sector write: offset %lld, size %zu\n",
			 pos, bytes);
		return -EINVAL;
	}

	bdev = I_BDEV(file->f_mapping->host);
	if (!bdev) {
		pr_emerg("failed to get block device\n");
		return -EINVAL;
	}

	q = bdev_get_queue(bdev);
	if (!q) {
		pr_emerg("failed to get request queue\n");
		return -EINVAL;
	}

	if (bdev_read_only(bdev)) {
		pr_emerg("block device is read-only\n");
		return -EROFS;
	}

	if (blk_queue_dying(q)) {
		pr_emerg("queue is dying, device unavailable\n");
		return -ENODEV;
	}

	logical = queue_logical_block_size(q);
	if ((pos | bytes) & (logical - 1)) {
		pr_emerg("unaligned logical block write: offset %lld, size %zu, logical block %u\n",
			 pos, bytes, logical);
		return -EINVAL;
	}

	opf = REQ_OP_WRITE | REQ_SYNC | REQ_META | REQ_PRIO |
	      REQ_IDLE | REQ_POLLED | REQ_NOWAIT | REQ_FUA;

	while (remaining) {
		struct bio *bio = &psblk_panic_bio;
		bool completed = false;
		unsigned int nr_vecs;
		size_t to_write;
		ssize_t added;

		to_write = psblk_panic_max_chunk(q, buf + written, remaining);
		if (!to_write) {
			pr_emerg("failed to build queue-limited write chunk\n");
			ret = -EIO;
			break;
		}

		nr_vecs = DIV_ROUND_UP(offset_in_page(buf + written) +
				       to_write, PAGE_SIZE);
		if (WARN_ON_ONCE(nr_vecs > PSBLK_PANIC_MAX_PAGES)) {
			pr_emerg("write chunk needs %u bvecs, max is %u\n",
				 nr_vecs, PSBLK_PANIC_MAX_PAGES);
			ret = -EIO;
			break;
		}

		bio_init(bio, bdev, psblk_panic_bvecs, nr_vecs, opf);
		bio->bi_iter.bi_sector = offset >> SECTOR_SHIFT;
		bio->bi_private = comp;
		bio->bi_end_io = psblk_panic_bio_endio;

		added = psblk_panic_fill_bio(bio, buf + written, to_write);
		if (added != to_write) {
			bio_uninit(bio);
			ret = added < 0 ? added : -EIO;
			break;
		}

		ret = psblk_panic_submit_and_wait(bio, comp, &completed);
		if (completed)
			bio_uninit(bio);
		else
			psblk_panic_poisoned = true;
		if (ret)
			break;

		written += to_write;
		remaining -= to_write;
		offset += to_write;
	}

	if (written) {
		if (ret < 0)
			pr_emerg("panic write stopped after %zu of %zu bytes: error %d\n",
				 written, bytes, ret);
		return written;
	}

	if (ret < 0)
		pr_emerg("panic write failed with error %d\n", ret);

	return ret;
}

static ssize_t psblk_generic_blk_write(const char *buf, size_t bytes,
		loff_t pos)
{
	/* Console/Ftrace backend may handle buffer until flush dirty zones */
	if (in_interrupt() || irqs_disabled())
		return -EBUSY;
	return kernel_write(psblk_file, buf, bytes, &pos);
}

/*
 * This takes its configuration only from the module parameters now.
 */
static int __register_pstore_blk(struct pstore_device_info *dev,
				 const char *devpath)
{
	int ret = -ENODEV;

	lockdep_assert_held(&pstore_blk_lock);

	psblk_file = filp_open(devpath, O_RDWR | O_DSYNC | O_NOATIME | O_EXCL, 0);
	if (IS_ERR(psblk_file)) {
		ret = PTR_ERR(psblk_file);
		pr_err("failed to open '%s': %d!\n", devpath, ret);
		goto err;
	}

	if (!S_ISBLK(file_inode(psblk_file)->i_mode)) {
		pr_err("'%s' is not block device!\n", devpath);
		goto err_fput;
	}

	dev->zone.total_size =
		bdev_nr_bytes(I_BDEV(psblk_file->f_mapping->host));

	if (psblk_panic_check_capable(I_BDEV(psblk_file->f_mapping->host)))
		dev->zone.panic_write = psblk_panic_blk_write;

	ret = __register_pstore_device(dev);
	if (ret)
		goto err_fput;

	return 0;

err_fput:
	fput(psblk_file);
err:
	psblk_file = NULL;

	return ret;
}

/* get information of pstore/blk */
int pstore_blk_get_config(struct pstore_blk_config *info)
{
	strncpy(info->device, blkdev, 80);
	info->max_reason = max_reason;
	info->kmsg_size = check_size(kmsg_size, 4096);
	info->pmsg_size = check_size(pmsg_size, 4096);
	info->ftrace_size = check_size(ftrace_size, 4096);
	info->console_size = check_size(console_size, 4096);

	return 0;
}
EXPORT_SYMBOL_GPL(pstore_blk_get_config);


#ifndef MODULE
static const char devname[] = "/dev/pstore-blk";
static __init const char *early_boot_devpath(const char *initial_devname)
{
	/*
	 * During early boot the real root file system hasn't been
	 * mounted yet, and no device nodes are present yet. Use the
	 * same scheme to find the device that we use for mounting
	 * the root file system.
	 */
	dev_t dev;

	if (early_lookup_bdev(initial_devname, &dev)) {
		pr_err("failed to resolve '%s'!\n", initial_devname);
		return initial_devname;
	}

	init_unlink(devname);
	init_mknod(devname, S_IFBLK | 0600, new_encode_dev(dev));

	return devname;
}
#else
static inline const char *early_boot_devpath(const char *initial_devname)
{
	return initial_devname;
}
#endif

static int __init __best_effort_init(void)
{
	struct pstore_device_info *best_effort_dev;
	int ret;

	/* No best-effort mode requested. */
	if (!best_effort)
		return 0;

	/* Reject an empty blkdev. */
	if (!blkdev[0]) {
		pr_err("blkdev empty with best_effort=Y\n");
		return -EINVAL;
	}

	best_effort_dev = kzalloc(sizeof(*best_effort_dev), GFP_KERNEL);
	if (!best_effort_dev)
		return -ENOMEM;

	best_effort_dev->zone.read = psblk_generic_blk_read;
	best_effort_dev->zone.write = psblk_generic_blk_write;

	ret = __register_pstore_blk(best_effort_dev,
				    early_boot_devpath(blkdev));
	if (ret)
		kfree(best_effort_dev);
	else
		pr_info("attached %s (%lu)%s\n",
			blkdev, best_effort_dev->zone.total_size,
			best_effort_dev->zone.panic_write ?
			" (panic_write enabled)" :
			" (no dedicated panic_write!)");

	return ret;
}

static void __exit __best_effort_exit(void)
{
	/*
	 * Currently, the only user of psblk_file is best_effort, so
	 * we can assume that pstore_device_info is associated with it.
	 * Once there are "real" blk devices, there will need to be a
	 * dedicated pstore_blk_info, etc.
	 */
	if (psblk_file) {
		struct pstore_device_info *dev = pstore_device_info;

		__unregister_pstore_device(dev);
		kfree(dev);
		fput(psblk_file);
		psblk_file = NULL;
	}
}

static int __init pstore_blk_init(void)
{
	int ret;

	mutex_lock(&pstore_blk_lock);
	ret = __best_effort_init();
	mutex_unlock(&pstore_blk_lock);

	return ret;
}
late_initcall(pstore_blk_init);

static void __exit pstore_blk_exit(void)
{
	mutex_lock(&pstore_blk_lock);
	__best_effort_exit();
	/* If we've been asked to unload, unregister any remaining device. */
	__unregister_pstore_device(pstore_device_info);
	mutex_unlock(&pstore_blk_lock);
}
module_exit(pstore_blk_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("WeiXiong Liao <liaoweixiong@allwinnertech.com>");
MODULE_AUTHOR("Kees Cook <keescook@chromium.org>");
MODULE_DESCRIPTION("pstore backend for block devices");
