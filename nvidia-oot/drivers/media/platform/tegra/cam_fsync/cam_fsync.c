// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2022-2023, NVIDIA CORPORATION. All rights reserved.
 */

#include <nvidia/conftest.h>

#include <asm/types.h>
#include <linux/bitfield.h>
#include <linux/bits.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/debugfs.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/lcm.h>
#include <linux/list.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#include <uapi/media/cam_fsync.h>


#define TSC_TICKS_PER_HZ			(31250000ULL)
#define TSC_NS_PER_TICK				(32)
#define NS_PER_MS				(1000000U)

#define TSC_MTSCCNTCV0				(0x10)
#define TSC_MTSCCNTCV0_CV			GENMASK(31, 0)

#define TSC_MTSCCNTCV1				(0x14)
#define TSC_MTSCCNTCV1_CV			GENMASK(31, 0)

#define TSC_GENX_CTRL				(0x00)
#define TSC_GENX_CTRL_RST			(0x00)
#define TSC_GENX_CTRL_INITIAL_VAL		BIT(1)
#define TSC_GENX_CTRL_ENABLE			BIT(0)

#define TSC_GENX_START0				(0x04)
#define TSC_GENX_START0_LSB_VAL			GENMASK(31, 0)

#define TSC_GENX_START1				(0x08)
#define TSC_GENX_START1_MSB_VAL			GENMASK(23, 0)

#define TSC_GENX_STATUS				(0x0C)
#define TSC_GENX_STATUS_INTERRUPT_STATUS	BIT(6)
#define TSC_GENX_STATUS_VALUE			BIT(5)
#define TSC_GENX_STATUS_EDGE_ID			GENMASK(4, 2)
#define TSC_GENX_STATUS_RUNNING			BIT(1)
#define TSC_GENX_STATUS_WAITING			BIT(0)

#define TSC_GENX_EDGE0				(0x18)
#define TSC_GENX_EDGE1				(0x1C)
#define TSC_GENX_EDGE2				(0x20)
#define TSC_GENX_EDGE3				(0x24)
#define TSC_GENX_EDGE4				(0x28)
#define TSC_GENX_EDGE5				(0x2C)
#define TSC_GENX_EDGE6				(0x30)
#define TSC_GENX_EDGE7				(0x34)

#define TSC_GENX_EDGEX_INTERRUPT_EN		BIT(31)
#define TSC_GENX_EDGEX_STOP			BIT(30)
#define TSC_GENX_EDGEX_TOGGLE			BIT(29)
#define TSC_GENX_EDGEX_LOOP			BIT(28)
#define TSC_GENX_EDGEX_OFFSET			GENMASK(27, 0)

/* Time (ms) offset for the TSC signal generators */
#define TSC_GENX_START_OFFSET_MS		(100)

#define TSC_DEFAULT_GROUP_ID (0)
#define TSC_MAX_GENERATORS (4)

#define CAM_FSYNC_CLASS_NAME	"cam-fsync-groups"
#define CAM_FSYNC_GROUPS_NODE	"fsync-groups"

/**
 * struct cam_fsync_controller_features: TSC signal controller SW feature support.
 * @rational_locking:
 *   @enforced: Generator periods must be derived from a common base.
 *   @max_freq_hz_lcm: Maximum frequency (hz) of the common base generator period.
 * @offset:
 *   @enabled: Allow generators to offset their signal from the start of their period.
 */
struct cam_fsync_controller_features {
	struct {
		bool enforced;
		u32 max_freq_hz_lcm;
	} rational_locking;
	struct {
		bool enabled;
	} offset;
};

/**
 * struct cam_fsync_generator : Generator context.
 * @base: ioremapped register base.
 * @of: Generator device node.
 * @config:
 *   @freq_hz: Frequency (hz) of the generator.
 *   @duty_cycle: Duty cycle (%) of the generator.
 *   @offset_ms: Offset (ms) to shift the signal by.
 * @debugfs:
 *   @regset_ro: Debug FS read-only register set.
 * @list: List node
 */
struct cam_fsync_generator {
	void __iomem *base;
	struct device_node *of;
	struct {
		u32 freq_hz;
		u32 duty_cycle;
		u32 offset_ms;
	} config;
	struct {
		struct debugfs_regset32 regset_ro;
	} debugfs;
	struct list_head list;
};

/**
 * sturct fsync_generator_group : Generator group context
 * @id: Group id as specified in DT
 * @dev: device.
 * @features: Feature support for the group.
 * @abs_start_ticks: Start time in TSC ticks to start all generators in group
 * @active: Is group active
 * @generators: Linked list of child generators
 * @list: List node
 */
struct fsync_generator_group {
	u32 id;
	struct device *dev;
	const struct cam_fsync_controller_features *features;
	uint64_t abs_start_ticks;
	bool active;
	struct list_head generators;
	struct list_head list;
};

/**
 * struct cam_fsync_controller : Controller context
 * @dev: device.
 * @cdev: character device
 * @base: ioremapped register base.
 * @debugfs:
 *   @d: dentry to debugfs directory.
 * @features: Feature support for the controller.
 * @cam_fsync_class: Config class
 * @cam_fsync_major: Holds dynamically allocated major number
 * @groups: Linked list of child groups.
 * @cam_fsync_fops: Holds file ops function pointer
 */
struct cam_fsync_controller {
	struct device *dev;
	struct cdev cdev;
	void __iomem *base;
	struct {
		struct dentry *d;
	} debugfs;
	const struct cam_fsync_controller_features *features;
	struct class *cam_fsync_class;
	int cam_fsync_major;
	struct list_head groups;
	const struct file_operations cam_fsync_fops;
};

static const struct cam_fsync_controller_features tegra234_cam_fsync_features = {
	.rational_locking = {
		.enforced = true,
		.max_freq_hz_lcm = 120,
	},
	.offset = {
		.enabled = true,
	},
};

static const struct debugfs_reg32 cam_fsync_generator_debugfs_regset[] = {
	{
		.name = "status",
		.offset = TSC_GENX_STATUS,
	},
};

#define TSC_SIG_GEN_DEBUGFS_REGSET_SIZE ARRAY_SIZE(cam_fsync_generator_debugfs_regset)


static inline void
cam_fsync_generator_writel(struct cam_fsync_generator *generator, u32 reg, u32 val)
{
	writel(val, generator->base + reg);
}

static inline u32
cam_fsync_generator_readl(struct cam_fsync_generator *generator, u32 reg)
{
	return readl(generator->base + reg);
}

static inline u32
cam_fsync_controller_readl(struct cam_fsync_controller *controller, u32 reg)
{
	return readl(controller->base + reg);
}

static u32
cam_fsync_find_max_freq_hz_lcm(struct fsync_generator_group *group)
{
	struct cam_fsync_generator *generator;
	u32 running_lcm = 0;

	list_for_each_entry(generator, &group->generators, list) {
		running_lcm = lcm_not_zero(generator->config.freq_hz, running_lcm);
	}

	return running_lcm;
}

/**
 * @brief Check if generator is already part of any group
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	new_generator	pointer to struct cam_fsync_generator (non-null)
 *
 * @returns	true (Generator in some group), false (Generator not in group)
 */
static bool
cam_fsync_is_generator_in_group(struct cam_fsync_controller *controller,
					struct cam_fsync_generator *new_generator)
{
	struct cam_fsync_generator *generator;
	struct fsync_generator_group *group;

	list_for_each_entry(group, &controller->groups, list) {
		list_for_each_entry(generator, &group->generators, list) {
			if (!strcmp(generator->of->full_name, new_generator->of->full_name))
				return true;
		}
	}
	return false;
}

/**
 * @brief Add generators to fsync generator group struct
 * Allocate memory for generator, read and program details from DT
 * Add generator to list in group structure
 *
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 * @param[in]	np	pointer to struct device_node (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_add_generator(struct fsync_generator_group *group, struct device_node *np)
{
	struct cam_fsync_generator *generator;
	struct resource res;
	int err;
	struct cam_fsync_controller *controller = dev_get_drvdata(group->dev);

	generator = devm_kzalloc(group->dev, sizeof(*generator), GFP_KERNEL);
	if (!generator)
		return -ENOMEM;

	generator->of = np;
	INIT_LIST_HEAD(&generator->list);

	if (of_address_to_resource(np, 0, &res))
		return -EINVAL;

	if (cam_fsync_is_generator_in_group(controller, generator)) {
		dev_err(group->dev, "Generator is already part of a group\n");
		return -EALREADY;
	}

	generator->base = devm_ioremap_resource(group->dev, &res);
	if (IS_ERR(generator->base))
		return PTR_ERR(generator->base);

	err = of_property_read_u32(np, "freq_hz", &generator->config.freq_hz);
	if (err != 0) {
		dev_err(group->dev, "Failed to read generator frequency: %d\n", err);
		return err;
	}

	if (generator->config.freq_hz == 0) {
		dev_err(group->dev, "Frequency must be non-zero\n");
		return -EINVAL;
	}

	err = of_property_read_u32(np, "duty_cycle", &generator->config.duty_cycle);
	if (err != 0) {
		dev_err(group->dev, "Failed to read generator duty cycle: %d\n", err);
		return err;
	}
	if (generator->config.duty_cycle >= 100) {
		dev_err(group->dev, "Duty cycle must be < 100%%\n");
		return -EINVAL;
	}

	if (group->features->offset.enabled) {
		err = of_property_read_u32(np, "offset_ms", &generator->config.offset_ms);
		if (err != 0) {
			dev_err(group->dev, "Failed to read generator offset: %d\n", err);
			return err;
		}
	}
	list_add_tail(&generator->list, &group->generators);
	return err;
}

static int cam_fsync_program_group_generator_edges(struct fsync_generator_group *group)
{
	struct cam_fsync_generator *generator;
	u32 max_freq_hz_lcm = 0;

	/*
	 * If rational locking is enforced (e.g. a 30Hz & 60Hz signal must align every two periods
	 * w.r.t. the 60Hz signal) edges will be derived from whole-number multiples of the LCM of
	 * all generator frequencies belonging to this group.
	 *
	 * If rational locking is _not_ enforced then generator edges will be independently
	 * derived based on their configured frequency.
	 */
	if (group->features->rational_locking.enforced) {
		max_freq_hz_lcm = cam_fsync_find_max_freq_hz_lcm(group);
		if (max_freq_hz_lcm > group->features->rational_locking.max_freq_hz_lcm) {
			dev_err(group->dev,
				"Highest common frequency of %u hz exceeds maximum allowed (%u hz)\n",
				max_freq_hz_lcm,
				group->features->rational_locking.max_freq_hz_lcm);
			return -EINVAL;
		}
	}

	list_for_each_entry(generator, &group->generators, list) {
		u32 ticks_in_period = 0;
		u32 ticks_active = 0;
		u32 ticks_inactive = 0;

		if (group->features->rational_locking.enforced) {
			ticks_in_period = DIV_ROUND_CLOSEST(TSC_TICKS_PER_HZ, max_freq_hz_lcm);
			ticks_in_period *= max_freq_hz_lcm / generator->config.freq_hz;
		} else {
			ticks_in_period = DIV_ROUND_CLOSEST(TSC_TICKS_PER_HZ,
				generator->config.freq_hz);
		}

		ticks_active = mult_frac(ticks_in_period, generator->config.duty_cycle, 100);
		ticks_inactive = ticks_in_period - ticks_active;

		cam_fsync_generator_writel(generator, TSC_GENX_EDGE0,
			TSC_GENX_EDGEX_TOGGLE |
			FIELD_PREP(TSC_GENX_EDGEX_OFFSET, ticks_active));

		cam_fsync_generator_writel(generator, TSC_GENX_EDGE1,
			TSC_GENX_EDGEX_TOGGLE |
			TSC_GENX_EDGEX_LOOP |
			FIELD_PREP(TSC_GENX_EDGEX_OFFSET, ticks_inactive));
	}

	return 0;
}

/**
 * @brief Program start value to the generator
 *
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 */
static void cam_fsync_program_group_generator_start_values(struct fsync_generator_group *group)
{
	struct cam_fsync_generator *generator;
	uint64_t abs_start_tsc_ticks = 0;

	list_for_each_entry(generator, &group->generators, list) {
		abs_start_tsc_ticks = group->abs_start_ticks;
		if (group->features->offset.enabled && (generator->config.offset_ms != 0))
			abs_start_tsc_ticks += mult_frac(generator->config.offset_ms,
				NS_PER_MS, TSC_NS_PER_TICK);

		cam_fsync_generator_writel(generator, TSC_GENX_START0,
			FIELD_PREP(TSC_GENX_START0_LSB_VAL, lower_32_bits(abs_start_tsc_ticks)));

		cam_fsync_generator_writel(generator, TSC_GENX_START1,
			FIELD_PREP(TSC_GENX_START1_MSB_VAL, upper_32_bits(abs_start_tsc_ticks)));
	}
}

/**
 * @brief Get current tsc ticks
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 *
 * @returns Current ticks
 */
static u64 cam_fsync_get_current_tsc_ticks(struct cam_fsync_controller *controller)
{
	const u32 current_ticks_lo = FIELD_GET(TSC_MTSCCNTCV0_CV,
		cam_fsync_controller_readl(controller, TSC_MTSCCNTCV0));
	const u32 current_ticks_hi = FIELD_GET(TSC_MTSCCNTCV1_CV,
		cam_fsync_controller_readl(controller, TSC_MTSCCNTCV1));
	const u64 current_ticks = ((u64)current_ticks_hi << 32) | current_ticks_lo;
	return current_ticks;
}

/**
 * @brief Get default start time
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 *
 * @returns default start ticks
 */
static u64 cam_fsync_get_default_start_ticks(struct cam_fsync_controller *controller)
{
	u64 default_start_ticks = mult_frac(
		TSC_GENX_START_OFFSET_MS, NS_PER_MS, TSC_NS_PER_TICK);
	default_start_ticks += cam_fsync_get_current_tsc_ticks(controller);
	return default_start_ticks;
}

/**
 * @brief Check if generator is running
 *
 * @param[in]	generator	pointer to struct cam_fsync_generator (non-null)
 *
 * @returns	True (running), False (Not running)
 */
static bool cam_fsync_generator_is_running(struct cam_fsync_generator *generator)
{
	const u32 status = cam_fsync_generator_readl(generator, TSC_GENX_STATUS);

	return FIELD_GET(TSC_GENX_STATUS_RUNNING, status) == 1;
}

/**
 * @brief Check if generator is waiting to start
 *
 * @param[in]	generator	pointer to struct cam_fsync_generator (non-null)
 *
 * @returns	True (Waiting), False (Not Waiting)
 */
static bool cam_fsync_generator_is_waiting(struct cam_fsync_generator *generator)
{
	const u32 status = cam_fsync_generator_readl(generator, TSC_GENX_STATUS);

	return FIELD_GET(TSC_GENX_STATUS_WAITING, status) == 1;
}

/**
 * @brief Check if generator is idle
 *
 * @param[in]	generator	pointer to struct cam_fsync_generator (non-null)
 *
 * @returns	True (Idle), False (Not Idle)
 */
static inline bool cam_fsync_generator_is_idle(struct cam_fsync_generator *generator)
{
	return !cam_fsync_generator_is_running(generator) &&
		!cam_fsync_generator_is_waiting(generator);
}

/**
 * @brief Start all generators in group
 * Program and start generator in group
 *
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_start_group_generators(struct fsync_generator_group *group)
{
	struct cam_fsync_generator *generator;
	int err;

	/* A generator must be idle (e.g. neither running nor waiting) before starting */
	list_for_each_entry(generator, &group->generators, list) {
		if (!cam_fsync_generator_is_idle(generator)) {
			dev_err(group->dev, "Generator %s in group %d is not idle\n",
				generator->of->full_name, group->id);
			return -EBUSY;
		}
	}

	err = cam_fsync_program_group_generator_edges(group);
	if (err != 0)
		return err;

	cam_fsync_program_group_generator_start_values(group);

	/* Start the generators */
	list_for_each_entry(generator, &group->generators, list) {
		cam_fsync_generator_writel(generator, TSC_GENX_CTRL,
			TSC_GENX_CTRL_INITIAL_VAL | TSC_GENX_CTRL_ENABLE);

			if (cam_fsync_generator_is_idle(generator)) {
				dev_err(group->dev, "Generator start failed %s\n",
					generator->of->full_name);
				return -EFAULT;
			}
	}
	group->active = true;

	return 0;
}

/**
 * @brief Stop all generators in group
 *
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_stop_group_generators(struct fsync_generator_group *group)
{
	struct cam_fsync_generator *generator;

	list_for_each_entry(generator, &group->generators, list) {
		cam_fsync_generator_writel(generator, TSC_GENX_CTRL, TSC_GENX_CTRL_RST);

		/* Ensure the generator has stopped */
		if (!cam_fsync_generator_is_idle(generator)) {
			dev_err(group->dev, "Generator %s failed to stop\n",
				generator->of->full_name);
			return -EIO;
		}
	}

	return 0;
}

/**
 * @brief Init debugfs
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_debugfs_init(struct cam_fsync_controller *controller)
{
	struct cam_fsync_generator *generator;
	struct fsync_generator_group *group;

	controller->debugfs.d =
		debugfs_create_dir(controller->dev->of_node->full_name, NULL);
	if (IS_ERR(controller->debugfs.d))
		return PTR_ERR(controller->debugfs.d);

	list_for_each_entry(group, &controller->groups, list) {
		list_for_each_entry(generator, &group->generators, list) {
			generator->debugfs.regset_ro.regs = cam_fsync_generator_debugfs_regset;
			generator->debugfs.regset_ro.nregs = TSC_SIG_GEN_DEBUGFS_REGSET_SIZE;
			generator->debugfs.regset_ro.base = generator->base;

			debugfs_create_regset32(
				generator->of->full_name,
				0400,
				controller->debugfs.d,
				&generator->debugfs.regset_ro);
		}
	}

	return 0;
}


/**
 * @brief Remove debugfs
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 */
static void cam_fsync_debugfs_remove(struct cam_fsync_controller *controller)
{
	debugfs_remove_recursive(controller->debugfs.d);
	controller->debugfs.d = NULL;
}

/**
 * @brief Get correspoding group for specified id
 * Iterate through list of groups and return pointer to the group for which group_id match
 * otherwise return null
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	group_id	numerical value start time(0: MAX_GROUP_ID_DEFINED IN_DT)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static struct fsync_generator_group *
cam_fsync_get_group_by_id(struct cam_fsync_controller *controller, unsigned int group_id)
{
	struct fsync_generator_group *group;

	list_for_each_entry(group, &controller->groups, list) {
		if (group->id == group_id) {
			dev_info(controller->dev, "Group %u found\n", group_id);
			return group;
		}
	}

	dev_err(controller->dev, "Group %u not found\n", group_id);
	return NULL;
}

/**
 * @brief Open group node
 * @ref Get reference to corresponding group structure
 *
 * This is the a open file operation handler for a cam fsync generator group node.
 *
 * @param[in]	inode	Generator group character device inode struct (Non-NULL)
 * @param[in]	file	Generator group character device file struct (Non-NULL)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_open(struct inode *inode, struct file *file)
{
	unsigned int group_id = iminor(inode);
	struct cam_fsync_controller *controller;
	struct fsync_generator_group *group;

	controller = container_of(file->f_op, struct cam_fsync_controller, cam_fsync_fops);
	if (IS_ERR(controller))
		return PTR_ERR(controller);

	group = cam_fsync_get_group_by_id(controller, group_id);
	if (IS_ERR(group))
		return PTR_ERR(group);

	file->private_data = group;

	return 0;
}

/**
 * @brief Validate start time
 * Start time ticks must be greater than current ticks
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	start_time	numerical value start time(current_ticks < value > MAX_UINT64)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int
cam_fsync_validate_start_time(struct cam_fsync_controller *controller, u64 start_time_ticks)
{
	u64 current_ticks = cam_fsync_get_current_tsc_ticks(controller);

	if (start_time_ticks < current_ticks) {
		dev_err(controller->dev, "Start time is in past\n");
		return -EFAULT;
	}

	return 0;
}

/**
 * @brief Process an IOCTL call on a cam fsync group character device.
 *
 * Depending on the specific IOCTL, the argument may be a pointer to a
 * defined struct payload that is copied from user-space. This memory
 * is allocated and mapped from user-space and must be kept available until
 * after the IOCTL call completes.
 *
 * This is the a ioctl file operation handler for a cam fsync group.
 *
 * @param[in]	file	cam fsync group character device file struct (non-null)
 * @param[in]	cmd	cam fsync group IOCTL command (CAM_FSYNC_GRP_ABS_START_VAL)
 * @param[in]	startTimeInTSCTicks	numerical value start time
 *					(current_ticks < value > MAX_UINT64)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static long cam_fsync_ioctl(struct file *file, unsigned int cmd, unsigned long startTimeInTSCTicks)
{
	struct fsync_generator_group *group = file->private_data;
	struct cam_fsync_controller *controller = dev_get_drvdata(group->dev);
	long err = 0;

	switch (cmd) {
	case CAM_FSYNC_GRP_ABS_START_VAL:
			if (copy_from_user(&group->abs_start_ticks, (u64 *)startTimeInTSCTicks,
				sizeof(group->abs_start_ticks)))
				dev_err(group->dev, "Unable to read start value\n");
			err = cam_fsync_validate_start_time(controller, group->abs_start_ticks);
			if (err != 0) {
				dev_err(group->dev, "Invalid start value\n");
				return err;
			}
			err = cam_fsync_start_group_generators(group);
			break;
	default:
			dev_err(group->dev, "Invalid command\n");
			err = -EINVAL;
			break;
	}
	return err;
}

static const struct file_operations cam_fsync_fileops = {
	.owner = THIS_MODULE,
	.open = cam_fsync_open,
	.unlocked_ioctl = cam_fsync_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = cam_fsync_ioctl,
#endif
};

/**
 * @brief Create device node for group
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	pdev	pointer to struct platform_device (non-null)
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_group_node_init(struct cam_fsync_controller *controller,
							struct platform_device *pdev,
							struct fsync_generator_group *group)
{
	struct device *dev;
	dev_t devt;

	devt = MKDEV(controller->cam_fsync_major, group->id);

	dev = device_create(controller->cam_fsync_class, &pdev->dev, devt, NULL,
			"fsync-group%u", group->id);
	if (IS_ERR(dev)) {
		dev_err(controller->dev, "Error creating device for group %d\n", group->id);
		return PTR_ERR(dev);
	}

	return 0;
}

/**
 * @brief Destroy device node for group
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	group	pointer to struct fsync_generator_group (non-null)
 *
 * @returns	0 (success)
 */
static int cam_fsync_group_node_deinit(struct cam_fsync_controller *controller,
							struct fsync_generator_group *group)
{
	dev_t devt;

	devt = MKDEV(controller->cam_fsync_major, group->id);

	device_destroy(controller->cam_fsync_class, devt);

	return 0;
}

/**
 * @brief Create and initialize group structure
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	group_id	id corresponding to group read from DT (0: MAX_UINT32_t)
 *
 * @returns	struct fsync_generator_group *(pointer to newly create group struct or null)
 */
static struct fsync_generator_group *cam_fsync_group_init(
						struct cam_fsync_controller *controller,
						u32 group_id)
{
	struct fsync_generator_group *group = devm_kzalloc(controller->dev,
		sizeof(*group), GFP_KERNEL);

	if (IS_ERR(group))
		return group;

	INIT_LIST_HEAD(&group->generators);
	INIT_LIST_HEAD(&group->list);
	group->id = group_id;
	group->dev = controller->dev;
	group->features = controller->features;
	return group;
}

/**
 * @brief Create default group struct
 * If group node is not found in DT, create a group with default group id for all generators
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_create_default_group(struct cam_fsync_controller *controller)
{
	struct device_node *np;
	const char *node_status;
	int err = 0;
	struct fsync_generator_group *group = cam_fsync_group_init(controller,
		TSC_DEFAULT_GROUP_ID);

	if (IS_ERR(group))
		return PTR_ERR(group);

	dev_info(controller->dev, "Creating default group with id %d\n", TSC_DEFAULT_GROUP_ID);

	for_each_child_of_node(controller->dev->of_node, np) {
		err = of_property_read_string(np, "status", &node_status);
		if (err != 0) {
			dev_err(controller->dev, "Failed to read generator status: %d\n", err);
			return err;
		}
		if (strcmp("okay", node_status) != 0) {
			dev_dbg(controller->dev, "Generator %s disabled - skipping\n",
					np->full_name);
			continue;
		}
		err = cam_fsync_add_generator(group, np);
		if (err != 0) {
			dev_err(controller->dev, "Failed to add generator : %d\n", err);
			return err;
		}
	}
	list_add_tail(&group->list, &controller->groups);

	return err;
}

/**
 * @brief Find and create group
 * If group node is found in DT, create a group with generators defined in DT config
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 * @param[in]	pdev	pointer to struct platform_device (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_find_and_add_groups(struct cam_fsync_controller *controller,
							struct platform_device *pdev)
{
	struct device_node *np;
	struct device_node *group_node;
	const char *node_status;
	int err = 0;
	int num_generators, i;
	struct device_node *gen;
	struct fsync_generator_group *group;
	u32 group_id = 0;

	group_node = of_get_child_by_name(controller->dev->of_node, CAM_FSYNC_GROUPS_NODE);
	if (group_node == NULL) {
		dev_err(controller->dev, "Failed to read generator status: %d\n", err);
		return -EOPNOTSUPP;
	}
	err = of_property_read_string(group_node, "status", &node_status);
	if (err != 0) {
		dev_err(controller->dev, "Failed to read group status: %d\n", err);
		return err;
	}
	if (strcmp("okay", node_status) != 0) {
		dev_info(controller->dev, "Grouping disabled\n");
		return -EOPNOTSUPP;
	}

	for_each_child_of_node(group_node, np) {
		err = of_property_read_string(np, "status", &node_status);
		if (err != 0) {
			dev_err(controller->dev, "Failed to read group status: %d\n", err);
			return err;
		}
		if (strcmp("okay", node_status) != 0) {
			dev_dbg(controller->dev, "Group %s disabled - skipping\n", np->full_name);
			continue;
		}

		err = of_property_read_u32(np, "id", &group_id);
		if (err != 0) {
			dev_err(controller->dev, "Failed to read group id: %d\n", err);
			return err;
		}

		group = cam_fsync_group_init(controller, group_id);
		if (IS_ERR(group))
			return PTR_ERR(group);

		cam_fsync_group_node_init(controller, pdev, group);

		num_generators = of_property_count_elems_of_size(np, "generators",
			sizeof(u32));
		for (i = 0; i < num_generators; i++) {
			gen = of_parse_phandle(np, "generators", i);
			err = cam_fsync_add_generator(group, gen);
			if (err != 0) {
				dev_err(controller->dev, "Failed to add generator %s : %d\n",
						gen->full_name, err);
				cam_fsync_group_node_deinit(controller, group);
				return err;
			}
		}
		list_add_tail(&group->list, &controller->groups);
	}
	return err;
}

/**
 * @brief Character device init
 * Create class and register char device to get major number allocated dynamically
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_chrdev_init(struct cam_fsync_controller *controller)
{
#if defined(NV_CLASS_CREATE_HAS_NO_OWNER_ARG) /* Linux v6.4 */
	controller->cam_fsync_class = class_create(CAM_FSYNC_CLASS_NAME);
#else
	controller->cam_fsync_class = class_create(THIS_MODULE, CAM_FSYNC_CLASS_NAME);
#endif
	if (IS_ERR(controller->cam_fsync_class))
		return PTR_ERR(controller->cam_fsync_class);

	controller->cam_fsync_major = register_chrdev(0, CAM_FSYNC_CLASS_NAME,
						&controller->cam_fsync_fops);
	if (controller->cam_fsync_major < 0) {
		class_destroy(controller->cam_fsync_class);
		return controller->cam_fsync_major;
	}

	return 0;
}

/**
 * @brief Character device deinit
 * Delete class and deregister char device
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 */
static void cam_fsync_chrdev_deinit(struct cam_fsync_controller *controller)
{
	struct fsync_generator_group *group;

	list_for_each_entry(group, &controller->groups, list) {
		cam_fsync_group_node_deinit(controller, group);
	}
	unregister_chrdev(controller->cam_fsync_major, CAM_FSYNC_CLASS_NAME);
	class_destroy(controller->cam_fsync_class);
}

/**
 * @brief Create and start default group
 * If group is not specified in DT, a default group will be created and all
 * generators will be part of it
 *
 * @param[in]	controller	pointer to struct cam_fsync_controller (non-null)
 */
static int cam_fsync_default(struct cam_fsync_controller *controller)
{
	struct fsync_generator_group *group;
	int err = cam_fsync_create_default_group(controller);

	if (err != 0)
		return err;

	group = cam_fsync_get_group_by_id(controller, TSC_DEFAULT_GROUP_ID);
	group->abs_start_ticks = cam_fsync_get_default_start_ticks(controller);
	return cam_fsync_start_group_generators(group);
}

/**
 * @brief Driver Probe
 * Called when driver is loaded.
 *
 * @param[in]	pdev	pointer to struct platform_device (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_probe(struct platform_device *pdev)
{
	struct cam_fsync_controller *controller;
	struct resource *res;
	int err;

	dev_info(&pdev->dev, "CAM_FSYNC probing...\n");

	controller = devm_kzalloc(&pdev->dev, sizeof(*controller), GFP_KERNEL);
	if (!controller)
		return -ENOMEM;

	memcpy((void *)&controller->cam_fsync_fops, &cam_fsync_fileops,
		sizeof(cam_fsync_fileops));

	err = cam_fsync_chrdev_init(controller);
	if (err != 0)
		return err;

	controller->dev = &pdev->dev;
	controller->features = of_device_get_match_data(&pdev->dev);
	if (controller->features == NULL) {
		dev_err(controller->dev, "No controller feature table found\n");
		return -ENODEV;
	}

	INIT_LIST_HEAD(&controller->groups);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	controller->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(controller->base))
		return PTR_ERR(controller->base);

	platform_set_drvdata(pdev, controller);

	/*
	 * Check if fsync group node defined in DT, if yes read group and expose node for each group
	 * If not defined read all the generators and start them
	 */
	err = cam_fsync_find_and_add_groups(controller, pdev);
	if (err == -EOPNOTSUPP) {
		dev_info(controller->dev,
			"Failed to find frame sync group, creating default group\n");
		err = cam_fsync_default(controller);
	}

	if (err != 0) {
		dev_err(controller->dev,
			"Failed to create frame sync group\n");
		cam_fsync_chrdev_deinit(controller);
		return err;
	}

	if (debugfs_initialized()) {
		err = cam_fsync_debugfs_init(controller);
		if (err != 0) {
			cam_fsync_chrdev_deinit(controller);
			return err;
		}
	}
	return 0;
}

/**
 * @brief Driver remove
 * Called when driver is removed.
 *
 * @param[in]	pdev	pointer to struct platform_device (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int cam_fsync_remove(struct platform_device *pdev)
{
	struct cam_fsync_controller *controller = platform_get_drvdata(pdev);
	struct fsync_generator_group *group;
	int err = 0;

	cam_fsync_debugfs_remove(controller);
	cam_fsync_chrdev_deinit(controller);

	list_for_each_entry(group, &controller->groups, list) {
		if (group->active) {
			err = cam_fsync_stop_group_generators(group);
			if (err != 0)
				return err;
			group->active = false;
		}
	}

	return err;
}

/**
 * @brief SC7 Suspend
 * Called when driver is removed.
 *
 * @param[in]	dev	pointer to struct device (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int __maybe_unused cam_fsync_suspend(struct device *dev)
{
	struct cam_fsync_controller *controller = dev_get_drvdata(dev);
	struct fsync_generator_group *group;
	int err = 0;

	list_for_each_entry(group, &controller->groups, list) {
		if (group->active) {
			err = cam_fsync_stop_group_generators(group);
			if (err != 0)
				return err;
		}
	}

	return err;
}

/**
 * @brief SC7 Resume
 * Called when driver is removed.
 *
 * @param[in]	dev	pointer to struct device (non-null)
 *
 * @returns	0 (success), neg. errno (failure)
 */
static int __maybe_unused cam_fsync_resume(struct device *dev)
{
	struct cam_fsync_controller *controller = dev_get_drvdata(dev);
	struct fsync_generator_group *group;
	int err = 0;

	list_for_each_entry(group, &controller->groups, list) {
		if (group->active) {
			err = cam_fsync_start_group_generators(group);
			if (err != 0)
				return err;
		}
	}

	return err;
}

static const struct of_device_id cam_fsync_of_match[] = {
	{ .compatible = "nvidia,tegra234-cdi-tsc", .data = &tegra234_cam_fsync_features },
	{ },
};
MODULE_DEVICE_TABLE(of, cam_fsync_of_match);

static SIMPLE_DEV_PM_OPS(cam_fsync_pm, cam_fsync_suspend, cam_fsync_resume);

static struct platform_driver cam_fsync_driver = {
	.driver = {
		.name = "cam_fsync",
		.owner = THIS_MODULE,
		.of_match_table = cam_fsync_of_match,
		.pm = &cam_fsync_pm,
	},
	.probe = cam_fsync_probe,
	.remove = cam_fsync_remove,
};
module_platform_driver(cam_fsync_driver);

MODULE_AUTHOR("Mohit Ingale <mohiti@nvidia.com>");
MODULE_DESCRIPTION("Cam Fsync Driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:cam_fsync");
