// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2022-2023, NVIDIA CORPORATION.  All rights reserved.

#include <nvidia/conftest.h>

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/thermal.h>
#include <linux/delay.h>
#include <linux/version.h>

#define TS_ENABLE           BIT(7)
#define TS_DATA_VALID       BIT(31)
#define TS_DATA_SHIFT       (16)
#define TS_DATA_MASK        GENMASK(25, TS_DATA_SHIFT)
#define TS_DATA(x)          ((x & TS_DATA_MASK) >> TS_DATA_SHIFT)

#define PCI_CONF_ID         (0x0)
#define TEMP_SENSOR_CTRL    (0xBAC)

#define CMD_WRITE           (0x3)
#define CMD_READ            (0x4)
#define BYTE_EN             GENMASK(13, 10)

#define BUILD_CMD(rw, reg)  swab32(rw << 24 | BYTE_EN | reg >> 2)

static const struct of_device_id pex9749_of_match[] = {
	{ .compatible = "pex9749", },
	{},
};

enum chips { PEX9749 };

static const struct i2c_device_id pex9749_id[] = {
	{ "pex9749", PEX9749 },
	{},
};
MODULE_DEVICE_TABLE(i2c, pex9749_id);

struct pex9749_priv {
	struct i2c_client *client;
	struct thermal_zone_device *tzd;
};

union pex9749_i2c_write_cmd {
	struct {
		u32 cmd;
		u32 data;
	} pkt;
	u8	byte[8];
};

union pex9749_i2c_read_cmd {
	struct {
		u32 cmd;
	} pkt;
	u8	byte[4];
};

union pex9749_i2c_data {
	u32	dword;
	u8	byte[4];
};

static int pex9749_i2c_write_reg(struct i2c_client *client, u32 addr, u32 val)
{
	union pex9749_i2c_write_cmd wr_cmd;
	int len = sizeof(wr_cmd);

	wr_cmd.pkt.cmd = BUILD_CMD(CMD_WRITE, addr);
	wr_cmd.pkt.data = swab32(val);

	return len != i2c_master_send(client, wr_cmd.byte, len) ? -EIO : 0;
}

static int pex9749_i2c_read_reg(struct i2c_client *client, u32 addr, u32 *val)
{
	union pex9749_i2c_read_cmd rd_cmd;
	union pex9749_i2c_data resp = { 0 };
	int len;

	rd_cmd.pkt.cmd = BUILD_CMD(CMD_READ, addr);
	len = sizeof(rd_cmd);

	if (len != i2c_master_send(client, rd_cmd.byte, len))
		return -EIO;

	len = sizeof(resp);
	if (len != i2c_master_recv(client, resp.byte, len))
		return -EIO;

	*val = swab32(resp.dword);

	return 0;
}

static int pex9749_calc_temp(u16 v)
{
	long long temp;

	/*
	 * T(C) = (-4.5636e^-11) * v^4 + (1.4331e^-7) * v^3 +
	 * (-2.3557e^-4) * v^2 + (0.32597) * v - (53.509)
	 */
	temp = (-45636) * int_pow(v, 4) + (143310000) * int_pow(v, 3) +
		(-235570000000) * int_pow(v, 2) + (325970000000000) * v +
		(-53509000000000000);
	temp /= 1000000000000;

	return temp;
}

#if defined(NV_DEVM_THERMAL_OF_ZONE_REGISTER_PRESENT) /* Linux v6.1 */
static int pex9749_get_temp(struct thermal_zone_device *tz, int *temp)
{
#if defined(NV_THERMAL_ZONE_DEVICE_PRIV_PRESENT) /* Linux v6.4 */
	struct pex9749_priv *priv = thermal_zone_device_priv(tz);
#else
	struct pex9749_priv *priv = tz->devdata;
#endif
#else
static int pex9749_get_temp(void *data, int *temp)
{
	struct pex9749_priv *priv = data;
#endif
	struct i2c_client *client = priv->client;
	u32 reg;
	int ret, retry = 3;

	/*
	 * ->get_temp is serialized by tz->lock in thermal core, no lock
	 * required here. To start a new thermal sense operation, write 0
	 * then 1 to sensor enable bit.
	 */
	ret = pex9749_i2c_write_reg(client, TEMP_SENSOR_CTRL, 0);
	if (ret < 0)
		goto out;

	ret = pex9749_i2c_write_reg(client, TEMP_SENSOR_CTRL, TS_ENABLE);
	if (ret < 0)
		goto out;

retry:
	/* sensing operation takes 700~800 us */
	usleep_range(800, 1000);
	ret = pex9749_i2c_read_reg(client, TEMP_SENSOR_CTRL, &reg);
	if (ret < 0)
		goto out;

	if ((reg & TS_DATA_VALID) == 0) {
		if (retry-- > 0) {
			goto retry;
		} else {
			pr_warn("Invalid temperature data: 0x%08x\n", reg);
			ret = -EIO;
			goto out;
		}
	}

	*temp = pex9749_calc_temp(TS_DATA(reg));
out:
	return ret;
}

#if defined(NV_DEVM_THERMAL_OF_ZONE_REGISTER_PRESENT)
static struct thermal_zone_device_ops pex9749_ops = {
#else
static struct thermal_zone_of_device_ops pex9749_ops = {
#endif
	.get_temp = pex9749_get_temp,
};

static bool is_pex9749(struct i2c_client *client)
{
	u32 id;
	int ret;

	ret = pex9749_i2c_read_reg(client, PCI_CONF_ID, &id);
	if (ret < 0)
		return false;

	switch (id) {
	case 0x973310b5:
	case 0x974910b5:
		return true;

	default:
		return false;
	}
}

#if defined(NV_I2C_DRIVER_STRUCT_PROBE_WITHOUT_I2C_DEVICE_ID_ARG) /* Linux 6.3 */
static int pex9749_probe(struct i2c_client *client)
#else
static int pex9749_probe(struct i2c_client *client, const struct i2c_device_id *id)
#endif
{
	struct device *dev = &client->dev;
	struct pex9749_priv *priv;
	static bool deferred_probe;

	if (!is_pex9749(client)) {
		/* The AIC may not be up yet, request to probe later once */
		if (false == xchg(&deferred_probe, true))
			return -EPROBE_DEFER;
		return -ENODEV;
	}

	priv = devm_kzalloc(dev, sizeof(struct pex9749_priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->client = client;
#if defined(NV_DEVM_THERMAL_OF_ZONE_REGISTER_PRESENT)
	priv->tzd = devm_thermal_of_zone_register(dev, PEX9749, priv, &pex9749_ops);
#else
	priv->tzd = devm_thermal_zone_of_sensor_register(dev, PEX9749, priv, &pex9749_ops);
#endif
	if (IS_ERR(priv->tzd))
		return PTR_ERR(priv->tzd);

	i2c_set_clientdata(client, priv);

	return 0;
}

static struct i2c_driver pex9749_thermal_sensor = {
	.driver = {
		.name   = "PEX9749 thermal sensor",
		.of_match_table = of_match_ptr(pex9749_of_match),
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe      = pex9749_probe,
	.id_table   = pex9749_id,
};

module_i2c_driver(pex9749_thermal_sensor);
MODULE_AUTHOR("Leon Yu <leoyu@nvidia.com>");
MODULE_AUTHOR("Laxman Dewangan <ldewangan@nvidia.com>");
MODULE_DESCRIPTION("Temperature sensor driver for PEX9749");
MODULE_LICENSE("GPL v2");
