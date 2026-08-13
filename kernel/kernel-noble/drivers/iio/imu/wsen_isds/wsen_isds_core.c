// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Würth Elektronik WSEN-ISDS (2536030320001)
 * 6-Axis IMU — 3-axis Accelerometer + 3-axis Gyroscope + Temperature
 * Linux IIO Kernel Driver — Core
 *
 * Datasheet rev 001.005 (2025-10-30):
 *   https://www.we-online.com/components/products/datasheet/2536030320001.pdf
 *
 * IIO channels exposed:
 *   - IIO_ACCEL  X / Y / Z  (m/s²)
 *   - IIO_ANGL_VEL  X / Y / Z  (rad/s)
 *   - IIO_TEMP  (millidegrees Celsius)
 *
 */

#include <linux/delay.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/module.h>
#include <linux/regmap.h>

#include "wsen_isds.h"

/* -----------------------------------------------------------------------
 * regmap configuration
 * ----------------------------------------------------------------------- */
const struct regmap_config isds_regmap_config = {
	.reg_bits  = 8,
	.val_bits  = 8,
	.max_register = 0x7F,
};
EXPORT_SYMBOL_GPL(isds_regmap_config);

/* -----------------------------------------------------------------------
 * ODR table:  { ODR register value,  ODR in milli-Hz }
 * ----------------------------------------------------------------------- */
struct isds_odr_entry {
	u8  reg_val;
	int odr_mhz; /* milli-Hz */
};

static const struct isds_odr_entry isds_odr_table[] = {
	{ ISDS_ODR_POWER_DOWN,  0       },
	{ ISDS_ODR_12_5_HZ,     12500   },
	{ ISDS_ODR_26_HZ,       26000   },
	{ ISDS_ODR_52_HZ,       52000   },
	{ ISDS_ODR_104_HZ,      104000  },
	{ ISDS_ODR_208_HZ,      208000  },
	{ ISDS_ODR_416_HZ,      416000  },
	{ ISDS_ODR_833_HZ,      833000  },
	{ ISDS_ODR_1666_HZ,     1666000 },
	{ ISDS_ODR_3332_HZ,     3332000 },
	{ ISDS_ODR_6664_HZ,     6664000 },
};

/* -----------------------------------------------------------------------
 * Accelerometer full-scale table
 * Each entry: { FS register value, range in g (±), sensitivity in ng/LSB }
 * IIO reports in m/s² : val_int = g * 9.80665 / 1000 (micro-g -> m/s²)
 * ----------------------------------------------------------------------- */
struct isds_xl_fs_entry {
	u8  reg_val;
	int range_g;
	int sens_ng_lsb; /* nano-g per LSB */
};

static const struct isds_xl_fs_entry isds_xl_fs_table[] = {
	{ ISDS_XL_FS_2G,  2,  ISDS_XL_SENS_2G_NG_LSB  },
	{ ISDS_XL_FS_4G,  4,  ISDS_XL_SENS_4G_NG_LSB  },
	{ ISDS_XL_FS_8G,  8,  ISDS_XL_SENS_8G_NG_LSB  },
	{ ISDS_XL_FS_16G, 16, ISDS_XL_SENS_16G_NG_LSB },
};

/* -----------------------------------------------------------------------
 * Gyroscope full-scale table
 * sensitivity in micro-dps per LSB
 * ----------------------------------------------------------------------- */
struct isds_g_fs_entry {
	u8   reg_val;
	bool fs125;
	int  range_dps;
	int  sens_udps_lsb;
};

static const struct isds_g_fs_entry isds_g_fs_table[] = {
	{ ISDS_G_FS_250_DPS,  false, 250,  ISDS_G_SENS_250_UDPS_LSB  },
	{ ISDS_G_FS_500_DPS,  false, 500,  ISDS_G_SENS_500_UDPS_LSB  },
	{ ISDS_G_FS_1000_DPS, false, 1000, ISDS_G_SENS_1000_UDPS_LSB },
	{ ISDS_G_FS_2000_DPS, false, 2000, ISDS_G_SENS_2000_UDPS_LSB },
	{ 0,                  true,  125,  ISDS_G_SENS_125_UDPS_LSB  },
};

/* -----------------------------------------------------------------------
 * Helper: read a 16-bit signed value from two consecutive registers
 * (little-endian, 2's complement — standard for all output registers)
 * ----------------------------------------------------------------------- */
static int isds_read_raw16(struct isds_data *data, u8 reg_l, s16 *out)
{
	u8 buf[2];
	int ret;

	ret = regmap_bulk_read(data->regmap, reg_l, buf, sizeof(buf));
	if (ret)
		return ret;

	*out = (s16)((buf[1] << 8) | buf[0]);
	return 0;
}

/* -----------------------------------------------------------------------
 * Software reset & init sequence
 * ----------------------------------------------------------------------- */
static int isds_reset(struct isds_data *data)
{
	int ret;
	unsigned int who_am_i;

	/* Issue SW_RESET */
	ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL3_C,
				 ISDS_CTRL3_SW_RESET, ISDS_CTRL3_SW_RESET);
	if (ret)
		return ret;

	/* Wait for reset to complete (typ. < 50 µs) */
	usleep_range(1000, 2000);

	/* Verify WHO_AM_I */
	ret = regmap_read(data->regmap, ISDS_REG_WHO_AM_I, &who_am_i);
	if (ret)
		return ret;

	if (who_am_i != ISDS_WHO_AM_I_VALUE) {
		dev_err(data->dev,
			"WHO_AM_I mismatch: expected 0x%02X, got 0x%02X\n",
			ISDS_WHO_AM_I_VALUE, who_am_i);
		return -ENODEV;
	}

	return 0;
}

/* -----------------------------------------------------------------------
 * Apply sensible power-on defaults:
 *  - BDU enabled (output registers not updated until both bytes read)
 *  - Auto register address increment (IF_INC)
 *  - Accel: 104 Hz, ±4 g
 *  - Gyro:  104 Hz, ±250 dps
 * ----------------------------------------------------------------------- */
static int isds_configure_defaults(struct isds_data *data)
{
	int ret;

	/* CTRL3_C: BDU=1, IF_INC=1 */
	ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL3_C,
				 ISDS_CTRL3_BDU | ISDS_CTRL3_IF_INC,
				 ISDS_CTRL3_BDU | ISDS_CTRL3_IF_INC);
	if (ret)
		return ret;

	/*
	 * CTRL1_XL: ODR_XL = 104 Hz (0x04), FS_XL = ±4 g (0x02)
	 * bits[7:4]=0100, bits[3:2]=10 => 0x48
	 */
	ret = regmap_write(data->regmap, ISDS_REG_CTRL1_XL,
			   FIELD_PREP(ISDS_CTRL1_XL_ODR_XL_MASK, ISDS_ODR_104_HZ) |
			   FIELD_PREP(ISDS_CTRL1_XL_FS_XL_MASK,  ISDS_XL_FS_4G));
	if (ret)
		return ret;

	/*
	 * CTRL2_G: ODR_G = 104 Hz (0x04), FS_G = ±250 dps (0x00)
	 * bits[7:4]=0100, bits[3:2]=00 => 0x40
	 */
	ret = regmap_write(data->regmap, ISDS_REG_CTRL2_G,
			   FIELD_PREP(ISDS_CTRL2_G_ODR_G_MASK, ISDS_ODR_104_HZ) |
			   FIELD_PREP(ISDS_CTRL2_G_FS_G_MASK,  ISDS_G_FS_250_DPS));
	if (ret)
		return ret;

	/* Cache defaults */
	data->xl_fs    = ISDS_XL_FS_4G;
	data->g_fs     = ISDS_G_FS_250_DPS;
	data->g_fs_125 = false;
	data->odr      = ISDS_ODR_104_HZ;

	return 0;
}

/* -----------------------------------------------------------------------
 * IIO channel definitions
 * ----------------------------------------------------------------------- */
static const struct iio_chan_spec isds_channels[] = {
	/* Accelerometer channels */
	{
		.type      = IIO_ACCEL,
		.modified  = 1,
		.channel2  = IIO_MOD_X,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTX_L_XL,
		.scan_index = 0,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},
	{
		.type      = IIO_ACCEL,
		.modified  = 1,
		.channel2  = IIO_MOD_Y,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTY_L_XL,
		.scan_index = 1,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},
	{
		.type      = IIO_ACCEL,
		.modified  = 1,
		.channel2  = IIO_MOD_Z,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTZ_L_XL,
		.scan_index = 2,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},

	/* Gyroscope channels */
	{
		.type      = IIO_ANGL_VEL,
		.modified  = 1,
		.channel2  = IIO_MOD_X,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTX_L_G,
		.scan_index = 3,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},
	{
		.type      = IIO_ANGL_VEL,
		.modified  = 1,
		.channel2  = IIO_MOD_Y,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTY_L_G,
		.scan_index = 4,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},
	{
		.type      = IIO_ANGL_VEL,
		.modified  = 1,
		.channel2  = IIO_MOD_Z,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW),
		.info_mask_shared_by_type =
			BIT(IIO_CHAN_INFO_SCALE) | BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.address   = ISDS_REG_OUTZ_L_G,
		.scan_index = 5,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},

	/* Temperature channel */
	{
		.type      = IIO_TEMP,
		.info_mask_separate =
			BIT(IIO_CHAN_INFO_RAW)   |
			BIT(IIO_CHAN_INFO_SCALE) |
			BIT(IIO_CHAN_INFO_OFFSET),
		.address   = ISDS_REG_OUT_TEMP_L,
		.scan_index = 6,
		.scan_type = {
			.sign      = 's',
			.realbits  = 16,
			.storagebits = 16,
			.endianness  = IIO_LE,
		},
	},
};

/* -----------------------------------------------------------------------
 * read_raw — called by IIO core for sysfs / triggered reads
 * ----------------------------------------------------------------------- */
static int isds_read_raw(struct iio_dev *indio_dev,
			 const struct iio_chan_spec *chan,
			 int *val, int *val2, long mask)
{
	struct isds_data *data = iio_priv(indio_dev);
	s16 raw;
	int ret;

	switch (mask) {
	case IIO_CHAN_INFO_RAW:
		mutex_lock(&data->lock);
		ret = isds_read_raw16(data, chan->address, &raw);
		mutex_unlock(&data->lock);
		if (ret)
			return ret;
		*val = raw;
		return IIO_VAL_INT;

	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_ACCEL: {
			/*
			 * Scale: sensitivity in ng/LSB → m/s² per LSB
			 * IIO convention: value = raw * scale
			 * sens_ng_lsb ng/LSB * (1e-9 g/ng) * 9.80665 m/s²/g
			 *            = sens_ng_lsb * 9.80665e-9 m/s² per LSB
			 *
			 * Return as IIO_VAL_INT_PLUS_NANO:
			 *   val  = 0
			 *   val2 = sens_ng_lsb * 9806 (nano-m/s² per LSB)
			 *        ≈ sens_ng_lsb * 9.80665 (rounding to integer nano)
			 */
			int sens;
			int i;

			for (i = 0; i < ARRAY_SIZE(isds_xl_fs_table); i++) {
				if (isds_xl_fs_table[i].reg_val == data->xl_fs) {
					sens = isds_xl_fs_table[i].sens_ng_lsb;
					break;
				}
			}
			if (i == ARRAY_SIZE(isds_xl_fs_table))
				return -EINVAL;

			/*
			 * 1 ng = 1e-9 g = 9.80665e-9 m/s²
			 * sens [ng/LSB] * 9.80665 [nm/s² / (ng/LSB)] ~ integer nm/s²/LSB
			 * Multiply by 9807 / 1000 for micro precision or just use nano:
			 * val2 = sens * 9807 (nano m/s² per LSB)  [good enough for IIO]
			 */
			*val  = 0;
			*val2 = sens * 9807; /* nano m/s² per LSB */
			return IIO_VAL_INT_PLUS_NANO;
		}

		case IIO_ANGL_VEL: {
			/*
			 * Scale: micro-dps/LSB → rad/s per LSB
			 * 1 dps = π/180 rad/s ≈ 0.01745329 rad/s
			 * 1 µdps = 1.745329e-8 rad/s
			 * val2 = sens_udps * 17453 (pico rad/s per LSB — return nano)
			 *
			 * IIO_VAL_INT_PLUS_NANO:  val=0, val2 in nano rad/s per LSB
			 * val2 = sens_udps_lsb * 17453 / 1000
			 *      = sens_udps_lsb * 17  (approximation; use IIO_VAL_FRACTIONAL
			 *        for full precision in production drivers)
			 *
			 * Full precision: sens [µdps/LSB] * π/180 * 1e-6 [rad/s/µdps]
			 *   = sens * 1.745329e-8 rad/s per LSB
			 * Return IIO_VAL_INT_PLUS_NANO, val2 = sens * 17 (nano rad/s/LSB)
			 * (error < 0.3% — sufficient for most applications)
			 */
			int sens;
			int i;
			bool found = false;

			for (i = 0; i < ARRAY_SIZE(isds_g_fs_table); i++) {
				if (data->g_fs_125 && isds_g_fs_table[i].fs125) {
					sens = isds_g_fs_table[i].sens_udps_lsb;
					found = true;
					break;
				} else if (!data->g_fs_125 &&
					   !isds_g_fs_table[i].fs125 &&
					   isds_g_fs_table[i].reg_val == data->g_fs) {
					sens = isds_g_fs_table[i].sens_udps_lsb;
					found = true;
					break;
				}
			}
			if (!found)
				return -EINVAL;

			*val  = 0;
			/* sens [µdps/LSB] * π/180 * 1e-6 → nano rad/s/LSB */
			*val2 = DIV_ROUND_CLOSEST(sens * 17453, 1000);
			return IIO_VAL_INT_PLUS_NANO;
		}

		case IIO_TEMP:
			/*
			 * Sensitivity = 1/256 °C per digit = ~3906.25 µ°C per digit
			 * IIO_CHAN_INFO_SCALE for temperature in millidegrees:
			 * val = 0, val2 = 3906 µ°C  => return IIO_VAL_INT_PLUS_MICRO
			 * (IIO temperature: processed = (raw + offset) * scale [m°C])
			 *
			 * Here we use:  scale = 1000/256 m°C per LSB = 3.90625 m°C/LSB
			 * Return IIO_VAL_FRACTIONAL: val=1000, val2=256
			 */
			*val  = 1000;
			*val2 = 256;
			return IIO_VAL_FRACTIONAL;

		default:
			return -EINVAL;
		}

	case IIO_CHAN_INFO_OFFSET:
		if (chan->type == IIO_TEMP) {
			/*
			 * Output 0 LSB corresponds to 25 °C.
			 * processed [m°C] = (raw + offset) * scale
			 * offset = 25°C / scale = 25000 m°C / (1000/256 m°C/LSB)
			 *        = 25000 * 256 / 1000 = 6400 LSB
			 */
			*val  = 6400;
			*val2 = 0;
			return IIO_VAL_INT;
		}
		return -EINVAL;

	case IIO_CHAN_INFO_SAMP_FREQ: {
		/* Return current ODR in Hz (integer + fractional milli) */
		int i;

		for (i = 0; i < ARRAY_SIZE(isds_odr_table); i++) {
			if (isds_odr_table[i].reg_val == data->odr) {
				*val  = isds_odr_table[i].odr_mhz / 1000;
				*val2 = (isds_odr_table[i].odr_mhz % 1000) * 1000;
				return IIO_VAL_INT_PLUS_MICRO;
			}
		}
		return -EINVAL;
	}

	default:
		return -EINVAL;
	}
}

/* -----------------------------------------------------------------------
 * write_raw — handle writes from userspace (ODR, full-scale)
 * ----------------------------------------------------------------------- */
static int isds_write_raw(struct iio_dev *indio_dev,
			  const struct iio_chan_spec *chan,
			  int val, int val2, long mask)
{
	struct isds_data *data = iio_priv(indio_dev);
	int odr_mhz = val * 1000 + val2 / 1000;
	int i, ret;

	switch (mask) {
	case IIO_CHAN_INFO_SAMP_FREQ:
		/* Find matching ODR */
		for (i = 0; i < ARRAY_SIZE(isds_odr_table); i++) {
			if (isds_odr_table[i].odr_mhz == odr_mhz)
				break;
		}
		if (i == ARRAY_SIZE(isds_odr_table))
			return -EINVAL;

		mutex_lock(&data->lock);

		/* Update accelerometer ODR */
		ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL1_XL,
					 ISDS_CTRL1_XL_ODR_XL_MASK,
					 FIELD_PREP(ISDS_CTRL1_XL_ODR_XL_MASK,
						    isds_odr_table[i].reg_val));
		if (ret)
			goto out_unlock;

		/* Update gyroscope ODR */
		ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL2_G,
					 ISDS_CTRL2_G_ODR_G_MASK,
					 FIELD_PREP(ISDS_CTRL2_G_ODR_G_MASK,
						    isds_odr_table[i].reg_val));
		if (ret)
			goto out_unlock;

		data->odr = isds_odr_table[i].reg_val;

out_unlock:
		mutex_unlock(&data->lock);
		return ret;

	case IIO_CHAN_INFO_SCALE:
		switch (chan->type) {
		case IIO_ACCEL:
			/*
			 * User writes scale in m/s² per LSB (nano).
			 * Find matching full-scale range.
			 */
			for (i = 0; i < ARRAY_SIZE(isds_xl_fs_table); i++) {
				int expected_nano = isds_xl_fs_table[i].sens_ng_lsb * 9807;
				if (val2 == expected_nano && val == 0)
					break;
			}
			if (i == ARRAY_SIZE(isds_xl_fs_table))
				return -EINVAL;

			mutex_lock(&data->lock);
			ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL1_XL,
						 ISDS_CTRL1_XL_FS_XL_MASK,
						 FIELD_PREP(ISDS_CTRL1_XL_FS_XL_MASK,
							    isds_xl_fs_table[i].reg_val));
			if (!ret)
				data->xl_fs = isds_xl_fs_table[i].reg_val;
			mutex_unlock(&data->lock);
			return ret;

		case IIO_ANGL_VEL:
			for (i = 0; i < ARRAY_SIZE(isds_g_fs_table); i++) {
				int expected_nano = DIV_ROUND_CLOSEST(
					isds_g_fs_table[i].sens_udps_lsb * 17453, 1000);
				if (val2 == expected_nano && val == 0)
					break;
			}
			if (i == ARRAY_SIZE(isds_g_fs_table))
				return -EINVAL;

			mutex_lock(&data->lock);
			if (isds_g_fs_table[i].fs125) {
				/* Enable ±125 dps mode via FS_125 bit */
				ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL2_G,
							 ISDS_CTRL2_G_FS_125_MASK,
							 ISDS_CTRL2_G_FS_125_MASK);
				if (!ret)
					data->g_fs_125 = true;
			} else {
				ret = regmap_update_bits(data->regmap, ISDS_REG_CTRL2_G,
							 ISDS_CTRL2_G_FS_125_MASK |
							 ISDS_CTRL2_G_FS_G_MASK,
							 FIELD_PREP(ISDS_CTRL2_G_FS_G_MASK,
								    isds_g_fs_table[i].reg_val));
				if (!ret) {
					data->g_fs     = isds_g_fs_table[i].reg_val;
					data->g_fs_125 = false;
				}
			}
			mutex_unlock(&data->lock);
			return ret;

		default:
			return -EINVAL;
		}

	default:
		return -EINVAL;
	}
}

/* -----------------------------------------------------------------------
 * Available ODR / scale sysfs attributes
 * ----------------------------------------------------------------------- */
static IIO_CONST_ATTR(in_accel_sampling_frequency_available,
	"0 12.5 26 52 104 208 416 833 1666 3332 6664");

static IIO_CONST_ATTR(in_anglvel_sampling_frequency_available,
	"0 12.5 26 52 104 208 416 833 1666 3332 6664");

static IIO_CONST_ATTR(in_accel_scale_available,
	"0.000598107 0.001196214 0.002392428 0.004784856");
	/* ±2g, ±4g, ±8g, ±16g: 61*9807 = 598227 nano → 0.000598 m/s² */

static IIO_CONST_ATTR(in_anglvel_scale_available,
	"0.000153398 0.000306796 0.000613593 0.001227186 0.002454372");
	/* ±125, ±250, ±500, ±1000, ±2000 dps */

static struct attribute *isds_attributes[] = {
	&iio_const_attr_in_accel_sampling_frequency_available.dev_attr.attr,
	&iio_const_attr_in_anglvel_sampling_frequency_available.dev_attr.attr,
	&iio_const_attr_in_accel_scale_available.dev_attr.attr,
	&iio_const_attr_in_anglvel_scale_available.dev_attr.attr,
	NULL,
};

static const struct attribute_group isds_attrs_group = {
	.attrs = isds_attributes,
};

/* -----------------------------------------------------------------------
 * IIO ops
 * ----------------------------------------------------------------------- */
static const struct iio_info isds_iio_info = {
	.read_raw        = isds_read_raw,
	.write_raw       = isds_write_raw,
	.attrs           = &isds_attrs_group,
};

/* -----------------------------------------------------------------------
 * Core probe — called by the bus-specific (I2C) probe function
 * ----------------------------------------------------------------------- */
int isds_core_probe(struct device *dev, struct regmap *regmap, const char *name)
{
	struct iio_dev    *indio_dev;
	struct isds_data  *data;
	int ret;

	indio_dev = devm_iio_device_alloc(dev, sizeof(*data));
	if (!indio_dev)
		return -ENOMEM;

	data           = iio_priv(indio_dev);
	data->regmap   = regmap;
	data->dev      = dev;
	mutex_init(&data->lock);

	indio_dev->name      = name;
	indio_dev->channels  = isds_channels;
	indio_dev->num_channels = ARRAY_SIZE(isds_channels);
	indio_dev->info      = &isds_iio_info;
	indio_dev->modes     = INDIO_DIRECT_MODE;

	/* Hardware initialisation */
	ret = isds_reset(data);
	if (ret) {
		dev_err(dev, "Reset / WHO_AM_I check failed: %d\n", ret);
		return ret;
	}

	ret = isds_configure_defaults(data);
	if (ret) {
		dev_err(dev, "Default configuration failed: %d\n", ret);
		return ret;
	}

	ret = devm_iio_device_register(dev, indio_dev);
	if (ret) {
		dev_err(dev, "IIO device registration failed: %d\n", ret);
		return ret;
	}

	dev_info(dev, "WSEN-ISDS IMU registered (I2C, 104 Hz, ±4g / ±250 dps)\n");
	return 0;
}
EXPORT_SYMBOL_GPL(isds_core_probe);

MODULE_DESCRIPTION("Wurth Elektronik WSEN-ISDS 6-Axis IMU IIO Driver");
MODULE_LICENSE("GPL v2");
