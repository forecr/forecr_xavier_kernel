/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Würth Elektronik WSEN-ISDS (2536030320001)
 * 6-Axis IMU (3-axis Accelerometer + 3-axis Gyroscope + Temperature)
 * Linux IIO Kernel Driver - Register Definitions
 *
 * Datasheet: https://www.we-online.com/components/products/datasheet/2536030320001.pdf
 * User Manual: https://www.we-online.com/en/components/products/WSEN-ISDS
 *
 */

#ifndef _WSEN_ISDS_H_
#define _WSEN_ISDS_H_

#include <linux/bitfield.h>
#include <linux/bits.h>
#include <linux/iio/iio.h>
#include <linux/regmap.h>

/* -----------------------------------------------------------------------
 * I2C Device Addresses
 * SAO pin low  => 0x6A
 * SAO pin high => 0x6B
 * ----------------------------------------------------------------------- */
#define ISDS_I2C_ADDR_SAO_LOW        0x6A
#define ISDS_I2C_ADDR_SAO_HIGH       0x6B

/* -----------------------------------------------------------------------
 * Register Map
 * ----------------------------------------------------------------------- */

/* Identification */
#define ISDS_REG_WHO_AM_I            0x0F
#define ISDS_WHO_AM_I_VALUE          0x6A  /* Fixed device ID */

/* Embedded functions / FIFO configuration (not used in basic driver) */
#define ISDS_REG_FUNC_CFG_ACCESS     0x01

/* FIFO */
#define ISDS_REG_FIFO_CTRL1          0x06
#define ISDS_REG_FIFO_CTRL2          0x07
#define ISDS_REG_FIFO_CTRL3          0x08
#define ISDS_REG_FIFO_CTRL4          0x09
#define ISDS_REG_FIFO_CTRL5          0x0A

/* Timestamp */
#define ISDS_REG_DRDY_PULSE_CFG      0x0B

/* Interrupt pins */
#define ISDS_REG_INT1_CTRL           0x0D
#define ISDS_REG_INT2_CTRL           0x0E

/* Control registers */
#define ISDS_REG_CTRL1_XL            0x10  /* Accel ODR, FS, AA filter BW */
#define ISDS_REG_CTRL2_G             0x11  /* Gyro ODR, FS */
#define ISDS_REG_CTRL3_C             0x12  /* Boot, BDU, H_LACTIVE, etc. */
#define ISDS_REG_CTRL4_C             0x13  /* I2C disable, LPF1 sel, etc. */
#define ISDS_REG_CTRL5_C             0x14  /* Self-test */
#define ISDS_REG_CTRL6_C             0x15  /* Gyro LPF1 BW, XL trig mode */
#define ISDS_REG_CTRL7_G             0x16  /* Gyro HPF */
#define ISDS_REG_CTRL8_XL            0x17  /* XL composite filter */
#define ISDS_REG_CTRL9_XL            0x18  /* XL axes enable */
#define ISDS_REG_CTRL10_C            0x19  /* Gyro axes enable, timestamps */

/* Wake-up / Activity / Tap */
#define ISDS_REG_WAKE_UP_SRC         0x1B
#define ISDS_REG_TAP_SRC             0x1C
#define ISDS_REG_D6D_SRC             0x1D  /* 6D orientation source */

/* Status */
#define ISDS_REG_STATUS              0x1E
#define ISDS_STATUS_TDA              BIT(2) /* Temperature data available */
#define ISDS_STATUS_GDA              BIT(1) /* Gyro data available */
#define ISDS_STATUS_XLDA             BIT(0) /* Accel data available */

/* Output data registers (all 16-bit, little-endian, 2's complement) */
#define ISDS_REG_OUT_TEMP_L          0x20  /* Temperature low byte  */
#define ISDS_REG_OUT_TEMP_H          0x21  /* Temperature high byte */

#define ISDS_REG_OUTX_L_G            0x22  /* Gyro X-axis low  */
#define ISDS_REG_OUTX_H_G            0x23
#define ISDS_REG_OUTY_L_G            0x24
#define ISDS_REG_OUTY_H_G            0x25
#define ISDS_REG_OUTZ_L_G            0x26
#define ISDS_REG_OUTZ_H_G            0x27

#define ISDS_REG_OUTX_L_XL           0x28  /* Accel X-axis low  */
#define ISDS_REG_OUTX_H_XL           0x29
#define ISDS_REG_OUTY_L_XL           0x2A
#define ISDS_REG_OUTY_H_XL           0x2B
#define ISDS_REG_OUTZ_L_XL           0x2C
#define ISDS_REG_OUTZ_H_XL           0x2D

/* Timestamp output */
#define ISDS_REG_TIMESTAMP0_REG      0x40
#define ISDS_REG_TIMESTAMP1_REG      0x41
#define ISDS_REG_TIMESTAMP2_REG      0x42

/* FIFO status */
#define ISDS_REG_FIFO_STATUS1        0x3A
#define ISDS_REG_FIFO_STATUS2        0x3B
#define ISDS_REG_FIFO_STATUS3        0x3C
#define ISDS_REG_FIFO_STATUS4        0x3D

/* FIFO data output */
#define ISDS_REG_FIFO_DATA_OUT_L     0x3E
#define ISDS_REG_FIFO_DATA_OUT_H     0x3F

/* Step counter */
#define ISDS_REG_STEP_COUNTER_L      0x4B
#define ISDS_REG_STEP_COUNTER_H      0x4C

/* -----------------------------------------------------------------------
 * CTRL1_XL (0x10) — Accelerometer control
 * ----------------------------------------------------------------------- */
#define ISDS_CTRL1_XL_ODR_XL_MASK   GENMASK(7, 4)
#define ISDS_CTRL1_XL_FS_XL_MASK    GENMASK(3, 2)
#define ISDS_CTRL1_XL_LPF1_BW_SEL   BIT(1)

/* ODR values for accelerometer (and gyroscope CTRL2_G uses the same encoding) */
#define ISDS_ODR_POWER_DOWN          0x00
#define ISDS_ODR_1_6_HZ              0x0B  /* Low-power only, XL only */
#define ISDS_ODR_12_5_HZ             0x01
#define ISDS_ODR_26_HZ               0x02
#define ISDS_ODR_52_HZ               0x03
#define ISDS_ODR_104_HZ              0x04
#define ISDS_ODR_208_HZ              0x05
#define ISDS_ODR_416_HZ              0x06
#define ISDS_ODR_833_HZ              0x07
#define ISDS_ODR_1666_HZ             0x08
#define ISDS_ODR_3332_HZ             0x09
#define ISDS_ODR_6664_HZ             0x0A

/* Accelerometer full-scale (FS_XL bits in CTRL1_XL) */
#define ISDS_XL_FS_2G                0x00  /* ±2 g  — 0.061 mg/LSB */
#define ISDS_XL_FS_16G               0x01  /* ±16 g — 0.488 mg/LSB */
#define ISDS_XL_FS_4G                0x02  /* ±4 g  — 0.122 mg/LSB */
#define ISDS_XL_FS_8G                0x03  /* ±8 g  — 0.244 mg/LSB */

/* Sensitivity in nano-g per LSB (multiply raw by these / 1000 to get mg) */
#define ISDS_XL_SENS_2G_NG_LSB       61    /*  0.061 mg/LSB */
#define ISDS_XL_SENS_4G_NG_LSB       122
#define ISDS_XL_SENS_8G_NG_LSB       244
#define ISDS_XL_SENS_16G_NG_LSB      488

/* -----------------------------------------------------------------------
 * CTRL2_G (0x11) — Gyroscope control
 * ----------------------------------------------------------------------- */
#define ISDS_CTRL2_G_ODR_G_MASK      GENMASK(7, 4)
#define ISDS_CTRL2_G_FS_G_MASK       GENMASK(3, 2)
#define ISDS_CTRL2_G_FS_125_MASK     BIT(1)  /* Enable ±125 dps range */

/* Gyroscope full-scale (FS_G bits) */
#define ISDS_G_FS_250_DPS            0x00  /* ±250  dps — 8.75 mdps/LSB */
#define ISDS_G_FS_500_DPS            0x01  /* ±500  dps — 17.5 mdps/LSB */
#define ISDS_G_FS_1000_DPS           0x02  /* ±1000 dps — 35   mdps/LSB */
#define ISDS_G_FS_2000_DPS           0x03  /* ±2000 dps — 70   mdps/LSB */
/* ±125 dps is enabled via FS_125 bit (FS_G ignored): 4.375 mdps/LSB */

/* Sensitivity in micro-dps per LSB */
#define ISDS_G_SENS_125_UDPS_LSB     4375
#define ISDS_G_SENS_250_UDPS_LSB     8750
#define ISDS_G_SENS_500_UDPS_LSB     17500
#define ISDS_G_SENS_1000_UDPS_LSB    35000
#define ISDS_G_SENS_2000_UDPS_LSB    70000

/* -----------------------------------------------------------------------
 * CTRL3_C (0x12) — General configuration
 * ----------------------------------------------------------------------- */
#define ISDS_CTRL3_BOOT              BIT(7)  /* Reboot memory */
#define ISDS_CTRL3_BDU               BIT(6)  /* Block data update */
#define ISDS_CTRL3_H_LACTIVE         BIT(5)  /* Interrupt active low */
#define ISDS_CTRL3_PP_OD             BIT(4)  /* Push-pull / open-drain INT */
#define ISDS_CTRL3_SIM               BIT(3)  /* SPI mode (0=4-wire) */
#define ISDS_CTRL3_IF_INC            BIT(2)  /* Auto address increment */
#define ISDS_CTRL3_BLE               BIT(1)  /* Big/little endian */
#define ISDS_CTRL3_SW_RESET          BIT(0)  /* Software reset */

/* -----------------------------------------------------------------------
 * Temperature conversion
 * Sensitivity: 0.00390625 °C/digit (1/256 °C per LSB)
 * Output 0 LSB = 25 °C (typ.)
 * Formula: T(°C) = 25 + (raw_s16 / 256.0)
 *
 * For IIO: report in millidegrees Celsius
 *   val  = 25000 + (raw_s16 * 1000) / 256
 * ----------------------------------------------------------------------- */
#define ISDS_TEMP_OFFSET_MDEGC       25000  /* 25 °C in milli-°C */
#define ISDS_TEMP_SCALE_NUM          1000
#define ISDS_TEMP_SCALE_DEN          256

/* -----------------------------------------------------------------------
 * Driver private state
 * ----------------------------------------------------------------------- */
struct isds_data {
	struct regmap      *regmap;
	struct device      *dev;

	/* Current full-scale settings (index into table) */
	int                xl_fs;   /* ISDS_XL_FS_* */
	int                g_fs;    /* ISDS_G_FS_* (or 125dps) */
	bool               g_fs_125;

	/* Cached ODR (common for both sensors in normal use) */
	int                odr;     /* ISDS_ODR_* */

	/* Mutual exclusion for multi-step read/write sequences */
	struct mutex       lock;
};

/* Forward declarations */
extern const struct regmap_config isds_regmap_config;
int isds_core_probe(struct device *dev, struct regmap *regmap, const char *name);

#endif /* _WSEN_ISDS_H_ */
