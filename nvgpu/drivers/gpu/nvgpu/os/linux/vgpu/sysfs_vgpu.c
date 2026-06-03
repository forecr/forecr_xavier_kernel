// SPDX-License-Identifier: GPL-2.0-only
// SPDX-FileCopyrightText: Copyright (c) 2017-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.

#include <linux/device.h>
#include <nvgpu/vgpu/vgpu.h>

#include "os/linux/platform_gk20a.h"
#include "os/linux/os_linux.h"
#include "common/vgpu/ecc_vgpu.h"
#include "common/vgpu/ivc/comm_vgpu.h"
#include "common/vgpu/pmu_vgpu.h"
#include "vgpu_linux.h"

static ssize_t vgpu_get_gpu_load(struct device *dev,
			      struct device_attribute *attr,
			      char *buf, u32 perfmon_class)
{
	struct gk20a *g = get_gk20a(dev);
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_gpu_load_params *p = &msg.params.gpu_load;
	int err;

	p->perfmon_class = perfmon_class;
	msg.cmd = TEGRA_VGPU_CMD_GET_GPU_LOAD;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv(g, &msg);
	if (err)
		return err;

	return snprintf(buf, NVGPU_CPU_PAGE_SIZE, "%u\n", p->load);
}

static ssize_t vgpu_load_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_GR);
}
static DEVICE_ATTR(load, S_IRUGO, vgpu_load_show, NULL);

static ssize_t vgpu_nvenc0_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_NVENC0);
}
static DEVICE_ATTR(nvenc0_load, S_IRUGO, vgpu_nvenc0_load_show, NULL);

static ssize_t vgpu_nvenc1_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_NVENC1);
}
static DEVICE_ATTR(nvenc1_load, S_IRUGO, vgpu_nvenc1_load_show, NULL);

static ssize_t vgpu_nvdec0_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_NVDEC0);
}
static DEVICE_ATTR(nvdec0_load, S_IRUGO, vgpu_nvdec0_load_show, NULL);

static ssize_t vgpu_nvdec1_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_NVDEC1);
}
static DEVICE_ATTR(nvdec1_load, S_IRUGO, vgpu_nvdec1_load_show, NULL);

static ssize_t vgpu_ofa_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_OFA);
}
static DEVICE_ATTR(ofa_load, S_IRUGO, vgpu_ofa_load_show, NULL);

static ssize_t vgpu_nvjpg0_load_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	return vgpu_get_gpu_load(dev, attr, buf, NVGPU_PMU_PERFMON_CLASS_NVJPG0);
}
static DEVICE_ATTR(nvjpg0_load, S_IRUGO, vgpu_nvjpg0_load_show, NULL);

static ssize_t vgpu_get_gpu_temperature_sysfs(struct device *dev, char *buf, u32 temp_type)
{
	struct gk20a *g = get_gk20a(dev);
	s32 temp_raw = 0;
	int temp_celsius_int = 0;
	int temp_celsius_frac = 0;
	int remainder = 0;
	int err;

	err = vgpu_get_gpu_temperature(g, temp_type, &temp_raw);
	if (err) {
		return snprintf(buf, NVGPU_CPU_PAGE_SIZE,
			"Temperature unavailable: vgpu comm failed (%d)\n", err);
	}

	if (temp_raw == (s32)0xFFFFFFFF) {
		return snprintf(buf, NVGPU_CPU_PAGE_SIZE,
			"Temperature unavailable: not supported\n");
	}

	/* Convert from PMU format to show both integer and decimal precision
	 * PMU format: temp_celsius = temp_raw / 256 (signed arithmetic)
	 * Integer part: temp_raw / 256
	 * Fractional part: abs((temp_raw % 256) * 10) / 256
	 */
	temp_celsius_int = temp_raw / 256;

	/* Handle fractional part carefully for negative temperatures */
	remainder = temp_raw % 256;

	if (remainder < 0)
		remainder = -remainder;  /* Make positive for fractional calculation */
	temp_celsius_frac = (remainder * 10) / 256;

	return snprintf(buf, NVGPU_CPU_PAGE_SIZE, "%d.%d\n", temp_celsius_int, temp_celsius_frac);
}

static ssize_t vgpu_temperature_max_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	return vgpu_get_gpu_temperature_sysfs(dev, buf, TEGRA_VGPU_GPU_TEMPERATURE_TYPE_MAX);
}
static DEVICE_ATTR(gpu_temperature_max, S_IRUGO, vgpu_temperature_max_show, NULL);

static ssize_t vgpu_temperature_avg_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	return vgpu_get_gpu_temperature_sysfs(dev, buf, TEGRA_VGPU_GPU_TEMPERATURE_TYPE_AVG);
}
static DEVICE_ATTR(gpu_temperature_avg, S_IRUGO, vgpu_temperature_avg_show, NULL);

static ssize_t vgpu_temperature_min_show(struct device *dev,
					      struct device_attribute *attr,
					      char *buf)
{
	return vgpu_get_gpu_temperature_sysfs(dev, buf, TEGRA_VGPU_GPU_TEMPERATURE_TYPE_MIN);
}
static DEVICE_ATTR(gpu_temperature_min, S_IRUGO, vgpu_temperature_min_show, NULL);

static ssize_t vgpu_ecc_stat_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct gk20a *g = get_gk20a(dev);
	struct tegra_vgpu_cmd_msg msg = {0};
	struct tegra_vgpu_ecc_counter_params *p = &msg.params.ecc_counter;
	struct dev_ext_attribute *ext_attr = container_of(attr,
			struct dev_ext_attribute, attr);
	struct vgpu_ecc_stat *ecc_stat = ext_attr->var;
	int err;

	p->ecc_id = ecc_stat->ecc_id;

	msg.cmd = TEGRA_VGPU_CMD_GET_ECC_COUNTER_VALUE;
	msg.handle = vgpu_get_handle(g);
	err = vgpu_comm_sendrecv(g, &msg);
	err = err ? err : msg.ret;
	if (unlikely(err)) {
		nvgpu_err(g, "ecc: cannot get ECC counter value: %d", err);
		return err;
	}

	return snprintf(buf, NVGPU_CPU_PAGE_SIZE, "%u\n", p->value);
}

static int vgpu_create_ecc_sysfs(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	struct vgpu_ecc_stat *stats;
	struct dev_ext_attribute *attrs;
	int err, i, count;

	err = vgpu_ecc_get_info(g);
	if (unlikely(err)) {
		nvgpu_err(g, "ecc: cannot get ECC info: %d", err);
		return err;
	}

	stats = priv->ecc_stats;
	count = priv->ecc_stats_count;

	attrs = nvgpu_kzalloc(g, count * sizeof(*attrs));
	if (unlikely(!attrs)) {
		nvgpu_err(g, "ecc: no memory");
		vgpu_ecc_remove_info(g);
		return -ENOMEM;
	}

	for (i = 0; i < count; i++) {
		sysfs_attr_init(&attrs[i].attr.attr);
		attrs[i].attr.attr.name = stats[i].name;
		attrs[i].attr.attr.mode = VERIFY_OCTAL_PERMISSIONS(S_IRUGO);
		attrs[i].attr.show = vgpu_ecc_stat_show;
		attrs[i].attr.store = NULL;
		attrs[i].var = &stats[i];

		err = device_create_file(dev, &attrs[i].attr);
		if (unlikely(err)) {
			nvgpu_warn(g, "ecc: cannot create file \"%s\": %d",
				   stats[i].name, err);
		}
	}

	l->ecc_attrs = attrs;
	return 0;
}

static void vgpu_remove_ecc_sysfs(struct device *dev)
{
	struct gk20a *g = get_gk20a(dev);
	struct nvgpu_os_linux *l = nvgpu_os_linux_from_gk20a(g);
	struct vgpu_priv_data *priv = vgpu_get_priv_data(g);
	int i;

	if (l->ecc_attrs) {
		for (i = 0; i < priv->ecc_stats_count; i++)
			device_remove_file(dev, &l->ecc_attrs[i].attr);

		nvgpu_kfree(g, l->ecc_attrs);
		l->ecc_attrs = NULL;
	}

	vgpu_ecc_remove_info(g);
}

void vgpu_create_sysfs(struct device *dev)
{
	if (device_create_file(dev, &dev_attr_load))
		dev_err(dev, "Failed to create vgpu sysfs attributes!\n");

	if (device_create_file(dev, &dev_attr_nvenc0_load))
		dev_err(dev, "Failed to create vgpu nvenc0_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_nvenc1_load))
		dev_err(dev, "Failed to create vgpu nvenc1_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_nvdec0_load))
		dev_err(dev, "Failed to create vgpu nvdec0_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_nvdec1_load))
		dev_err(dev, "Failed to create vgpu nvdec1_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_ofa_load))
		dev_err(dev, "Failed to create vgpu ofa_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_nvjpg0_load))
		dev_err(dev, "Failed to create vgpu nvjpg0_load sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_gpu_temperature_max))
		dev_err(dev, "Failed to create vgpu gpu_temperature_max sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_gpu_temperature_avg))
		dev_err(dev, "Failed to create vgpu gpu_temperature_avg sysfs attribute!\n");

	if (device_create_file(dev, &dev_attr_gpu_temperature_min))
		dev_err(dev, "Failed to create vgpu gpu_temperature_min sysfs attribute!\n");

	vgpu_create_ecc_sysfs(dev);
}

void vgpu_remove_sysfs(struct device *dev)
{
	device_remove_file(dev, &dev_attr_load);
	device_remove_file(dev, &dev_attr_nvenc0_load);
	device_remove_file(dev, &dev_attr_nvenc1_load);
	device_remove_file(dev, &dev_attr_nvdec0_load);
	device_remove_file(dev, &dev_attr_nvdec1_load);
	device_remove_file(dev, &dev_attr_ofa_load);
	device_remove_file(dev, &dev_attr_nvjpg0_load);
	device_remove_file(dev, &dev_attr_gpu_temperature_max);
	device_remove_file(dev, &dev_attr_gpu_temperature_avg);
	device_remove_file(dev, &dev_attr_gpu_temperature_min);
	vgpu_remove_ecc_sysfs(dev);
}
