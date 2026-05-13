// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Würth Elektronik WSEN-ISDS (2536030320001)
 * 6-Axis IMU — I2C Bus Driver
 *
 */

#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/regmap.h>

#include "wsen_isds.h"

/* -----------------------------------------------------------------------
 * I2C probe / remove
 * ----------------------------------------------------------------------- */
static int isds_i2c_probe(struct i2c_client *client)
{
	struct regmap *regmap;

	/* Verify the I2C adapter can do what we need */
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		dev_err(&client->dev, "I2C_FUNC_I2C not supported\n");
		return -EOPNOTSUPP;
	}

	regmap = devm_regmap_init_i2c(client, &isds_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "Failed to initialise regmap: %ld\n",
			PTR_ERR(regmap));
		return PTR_ERR(regmap);
	}

	return isds_core_probe(&client->dev, regmap, "wsen_isds");
}

/* -----------------------------------------------------------------------
 * Device tree / ACPI / legacy I2C ID tables
 * ----------------------------------------------------------------------- */
static const struct of_device_id isds_of_match[] = {
	{
		.compatible = "we,wsen-isds-2536030320001",
	},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, isds_of_match);

static const struct i2c_device_id isds_i2c_id[] = {
	{ "wsen-isds",         0 },
	{ "2536030320001",     0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, isds_i2c_id);

static struct i2c_driver isds_i2c_driver = {
	.driver = {
		.name           = "wsen_isds",
		.of_match_table = isds_of_match,
	},
	.probe    = isds_i2c_probe,
	.id_table = isds_i2c_id,
};

module_i2c_driver(isds_i2c_driver);

MODULE_DESCRIPTION("Wurth Elektronik WSEN-ISDS 6-Axis IMU I2C driver");
MODULE_LICENSE("GPL v2");
