// SPDX-License-Identifier: GPL-2.0
/*
 * A V4L2 driver for Analog Devices ADV7280-M video decoder.
 * Copyright (C) 2026, Forecr
 * 
 * Author: Kaya Kaan Tuna <kayatuna@forecr.io>
 *
 * Based on OmniVision OV5647 driver.
 */

#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_graph.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/gpio/consumer.h>
#include <linux/videodev2.h>

#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-subdev.h>
#include <media/camera_common.h>

/* ---------------------------------------------------------------------
 * Registers (identical addresses/values to the proven adv7180.c driver)
 * ---------------------------------------------------------------------
 */
#define ADV7180_STD_AD_PAL_BG_NTSC_J_SECAM		0x0
#define ADV7180_STD_PAL_M				0xa

#define ADV7180_REG_INPUT_CONTROL			0x0000
#define ADV7180_INPUT_CONTROL_INSEL_MASK		0x0f

#define ADV7182_REG_INPUT_VIDSEL			0x0002
#define ADV7182_REG_INPUT_RESERVED			BIT(2)

#define ADV7180_REG_OUTPUT_CONTROL			0x0003
#define ADV7180_REG_EXTENDED_OUTPUT_CONTROL		0x0004

#define ADV7180_REG_CON		0x0008
#define ADV7180_CON_MIN		0
#define ADV7180_CON_DEF		128
#define ADV7180_CON_MAX		255

#define ADV7180_REG_BRI		0x000a
#define ADV7180_BRI_MIN		-128
#define ADV7180_BRI_DEF		0
#define ADV7180_BRI_MAX		127

#define ADV7180_REG_HUE		0x000b
#define ADV7180_HUE_MIN		-127
#define ADV7180_HUE_DEF		0
#define ADV7180_HUE_MAX		128

#define ADV7180_REG_DEF_VALUE_Y	0x000c
#define ADV7180_DEF_VAL_EN		0x1
#define ADV7180_DEF_VAL_AUTO_EN	0x2
#define ADV7180_REG_CTRL		0x000e

#define ADV7180_REG_PWR_MAN		0x0f
#define ADV7180_PWR_MAN_ON		0x04
#define ADV7180_PWR_MAN_OFF		0x24
#define ADV7180_PWR_MAN_RES		0x80

#define ADV7180_REG_STATUS1		0x0010
#define ADV7180_STATUS1_IN_LOCK		0x01
#define ADV7180_STATUS1_AUTOD_MASK	0x70
#define ADV7180_STATUS1_AUTOD_NTSM_M_J	0x00
#define ADV7180_STATUS1_AUTOD_NTSC_4_43 0x10
#define ADV7180_STATUS1_AUTOD_PAL_M	0x20
#define ADV7180_STATUS1_AUTOD_PAL_60	0x30
#define ADV7180_STATUS1_AUTOD_PAL_B_G	0x40
#define ADV7180_STATUS1_AUTOD_SECAM	0x50
#define ADV7180_STATUS1_AUTOD_PAL_COMB	0x60
#define ADV7180_STATUS1_AUTOD_SECAM_525	0x70

#define ADV7180_REG_IDENT 0x0011

#define ADV7180_REG_STATUS3		0x0013
#define ADV7180_REG_ANALOG_CLAMP_CTL	0x0014
#define ADV7180_REG_SHAP_FILTER_CTL_1	0x0017
#define ADV7180_REG_CTRL_2		0x001d
#define ADV7180_REG_VSYNC_FIELD_CTL_1	0x0031
#define ADV7180_VSYNC_FIELD_CTL_1_NEWAV 0x12
#define ADV7180_REG_LOCK_CNT		0x0051
#define ADV7180_REG_CVBS_TRIM		0x0052
#define ADV7180_REG_CLAMP_ADJ		0x005a
#define ADV7180_REG_RES_CIR		0x005f
#define ADV7180_REG_DIFF_MODE		0x0060

#define ADV7180_REG_ICONF1		0x2040
#define ADV7180_ICONF1_ACTIVE_LOW	0x01
#define ADV7180_ICONF1_PSYNC_ONLY	0x10

#define ADV7180_REG_SD_SAT_CB	0x00e3
#define ADV7180_REG_SD_SAT_CR	0x00e4
#define ADV7180_SAT_MIN		0
#define ADV7180_SAT_DEF		128
#define ADV7180_SAT_MAX		255

#define ADV7180_IRQ3_AD_CHANGE	0x08
#define ADV7180_REG_ISR3	0x204A
#define ADV7180_REG_ICR3	0x204B
#define ADV7180_REG_IMR1	0x2044
#define ADV7180_REG_IMR2	0x2048
#define ADV7180_REG_IMR3	0x204C
#define ADV7180_REG_IMR4	0x2050

#define ADV7180_REG_NTSC_V_BIT_END	0x00E6
#define ADV7180_NTSC_V_BIT_END_MANUAL_NVEND	0x4F

#define ADV7180_REG_VPP_SLAVE_ADDR	0xFD
#define ADV7180_REG_CSI_SLAVE_ADDR	0xFE

#define ADV7180_REG_RST_CLAMP	0x809c
#define ADV7180_REG_AGC_ADJ1	0x80b6
#define ADV7180_REG_AGC_ADJ2	0x80c0

#define ADV7180_DEFAULT_CSI_I2C_ADDR 0x44
#define ADV7180_DEFAULT_VPP_I2C_ADDR 0x42

#define V4L2_CID_ADV_FAST_SWITCH	(V4L2_CID_USER_ADV7180_BASE + 0x00)

#define ADV7280M_NUM_OF_SKIP_FRAMES       10

#define ADV7182_INPUT_CVBS_AIN1 0x00
#define ADV7182_INPUT_CVBS_AIN2 0x01
#define ADV7182_INPUT_CVBS_AIN3 0x02
#define ADV7182_INPUT_CVBS_AIN4 0x03
#define ADV7182_INPUT_CVBS_AIN5 0x04
#define ADV7182_INPUT_CVBS_AIN6 0x05
#define ADV7182_INPUT_CVBS_AIN7 0x06
#define ADV7182_INPUT_CVBS_AIN8 0x07
#define ADV7182_INPUT_SVIDEO_AIN1_AIN2 0x08
#define ADV7182_INPUT_SVIDEO_AIN3_AIN4 0x09
#define ADV7182_INPUT_SVIDEO_AIN5_AIN6 0x0a
#define ADV7182_INPUT_SVIDEO_AIN7_AIN8 0x0b
#define ADV7182_INPUT_YPRPB_AIN1_AIN2_AIN3 0x0c
#define ADV7182_INPUT_YPRPB_AIN4_AIN5_AIN6 0x0d
#define ADV7182_INPUT_DIFF_CVBS_AIN1_AIN2 0x0e
#define ADV7182_INPUT_DIFF_CVBS_AIN3_AIN4 0x0f
#define ADV7182_INPUT_DIFF_CVBS_AIN5_AIN6 0x10
#define ADV7182_INPUT_DIFF_CVBS_AIN7_AIN8 0x11

#define ADV7280M_VALID_INPUT_MASK ( \
	BIT(ADV7182_INPUT_CVBS_AIN1) | BIT(ADV7182_INPUT_CVBS_AIN2) | \
	BIT(ADV7182_INPUT_CVBS_AIN3) | BIT(ADV7182_INPUT_CVBS_AIN4) | \
	BIT(ADV7182_INPUT_CVBS_AIN5) | BIT(ADV7182_INPUT_CVBS_AIN6) | \
	BIT(ADV7182_INPUT_CVBS_AIN7) | BIT(ADV7182_INPUT_CVBS_AIN8) | \
	BIT(ADV7182_INPUT_SVIDEO_AIN1_AIN2) | BIT(ADV7182_INPUT_SVIDEO_AIN3_AIN4) | \
	BIT(ADV7182_INPUT_SVIDEO_AIN5_AIN6) | BIT(ADV7182_INPUT_SVIDEO_AIN7_AIN8) | \
	BIT(ADV7182_INPUT_YPRPB_AIN1_AIN2_AIN3) | BIT(ADV7182_INPUT_YPRPB_AIN4_AIN5_AIN6))

/*
 * CSI-2 link/pixel rate. Per the ADV7280-M datasheet's MIPI Video Output
 * Specifications: nominal 432 Mbps output data rate, link_freq = data
 * rate / 2. Pixel rate is the standard ITU-R BT.656 SD digitized-video
 * clock (27MHz); cross-check: 27e6 * 16 bits / 1 lane = 432 Mbps, matching
 * the datasheet exactly.
 */
#define ADV7280M_PIXEL_RATE		27000000U
#define ADV7280M_LINK_FREQ		216000000ULL

static const s64 adv7280m_link_freq_menu[] = {
	ADV7280M_LINK_FREQ,
};

static const char * const test_pattern_menu[] = {
	"Single color",
	"Color bars",
	"Luma ramp",
	"Boundary box",
	"Disable",
};

static const int adv7280m_frmrates[] = { 25, 30 };

enum {
	ADV7280M_MODE_720X576,
};

static const struct camera_common_frmfmt adv7280m_frmfmt[] = {
	{{720, 576}, adv7280m_frmrates, ARRAY_SIZE(adv7280m_frmrates), 0,
	 ADV7280M_MODE_720X576},
};

#define ADV7280M_DEFAULT_MODE		ADV7280M_MODE_720X576
#define ADV7280M_DEFAULT_WIDTH		720U
#define ADV7280M_DEFAULT_HEIGHT		576U
#define ADV7280M_DEFAULT_DATAFMT	MEDIA_BUS_FMT_UYVY8_2X8
#define ADV7280M_DEFAULT_CLK_FREQ	ADV7280M_PIXEL_RATE

struct adv7280m {
	struct camera_common_power_rail	power;
	struct v4l2_ctrl_handler		ctrl_handler;
	struct i2c_client			*i2c_client;

	/* points at common_data->subdev; keep as pointer */
	struct v4l2_subdev			*subdev;
	struct media_pad			pad;
	struct mutex				mutex;

	struct camera_common_data		*s_data;
	struct camera_common_pdata		*pdata;

	struct v4l2_ctrl			*ctrls[8];
	struct v4l2_ctrl			*link_freq;
	struct v4l2_ctrl			*pixel_rate;
	struct v4l2_ctrl			*brightness;
	struct v4l2_ctrl			*contrast;
	struct v4l2_ctrl			*saturation;
	struct v4l2_ctrl			*hue;
	struct v4l2_ctrl			*fast_switch;
	struct v4l2_ctrl			*test_pattern;

	struct i2c_client			*csi_client;
	struct i2c_client			*vpp_client;

	struct gpio_desc			*pwdn_gpio;
	struct gpio_desc			*rst_gpio;

	unsigned int				register_page;
	v4l2_std_id				curr_norm;
	bool					powered;
	bool					streaming;
	u8					input;
	u32					analog_input;
	bool					force_bt656_4;
	enum v4l2_field				field;
	int					irq;
};

static inline struct adv7280m *to_adv7280m(struct v4l2_subdev *sd)
{
	struct camera_common_data *s_data = container_of(sd, struct camera_common_data, subdev);

	return (struct adv7280m *)s_data->priv;
}


static int adv7280m_client_write_reg(struct i2c_client *client, u8 reg, u8 val)
{
	u8 buf[2] = { reg, val };
	int ret;

	dev_info(&client->dev, "I2C: %s addr=0x%02X reg=0x%02X val=0x%02X\n",
		__func__, client->addr, reg, val);

	ret = i2c_master_send(client, buf, sizeof(buf));
	if (ret != (int)sizeof(buf)) {
		dev_err(&client->dev, "%s, error.\n", __func__);
		return -EIO;
	}

	return 0;
}

static int adv7280m_client_read_reg(struct i2c_client *client, u8 reg, u8 *val)
{
	struct i2c_msg msgs[2];
	u8 addr_buf[1] = { reg };
	u8 data_buf[1] = { 0 };
	int ret;

	msgs[0].addr = client->addr;
	msgs[0].flags = 0;
	msgs[0].len = sizeof(addr_buf);
	msgs[0].buf = addr_buf;

	msgs[1].addr = client->addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = sizeof(data_buf);
	msgs[1].buf = data_buf;

	ret = i2c_transfer(client->adapter, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs)) {
		dev_err(&client->dev, "%s, error.\n", __func__);
		return -EIO;
	}

	*val = data_buf[0];
	return 0;
}

static int adv7280m_select_page(struct adv7280m *priv, unsigned int page)
{
	if (priv->register_page != page) {
		adv7280m_client_write_reg(priv->i2c_client, ADV7180_REG_CTRL, page);
		priv->register_page = page;
	}
	return 0;
}

static int adv7280m_write(struct adv7280m *priv, unsigned int reg, unsigned int value)
{
	lockdep_assert_held(&priv->mutex);
	adv7280m_select_page(priv, reg >> 8);
	return adv7280m_client_write_reg(priv->i2c_client, reg & 0xff, value);
}

static int adv7280m_read(struct adv7280m *priv, unsigned int reg)
{
	u8 val = 0;
	int ret;

	lockdep_assert_held(&priv->mutex);
	adv7280m_select_page(priv, reg >> 8);
	ret = adv7280m_client_read_reg(priv->i2c_client, reg & 0xff, &val);
	if (ret)
		return ret;

	return val;
}

static int adv7280m_csi_write(struct adv7280m *priv, unsigned int reg, unsigned int value)
{
	return adv7280m_client_write_reg(priv->csi_client, reg, value);
}

static int adv7280m_vpp_write(struct adv7280m *priv, unsigned int reg, unsigned int value)
{
	return adv7280m_client_write_reg(priv->vpp_client, reg, value);
}

/* ---------------------------------------------------------------------
 * Video standard helpers
 * ---------------------------------------------------------------------
 */
static v4l2_std_id adv7280m_std_to_v4l2(u8 status1)
{
	if (!(status1 & ADV7180_STATUS1_IN_LOCK))
		return V4L2_STD_UNKNOWN;

	switch (status1 & ADV7180_STATUS1_AUTOD_MASK) {
	case ADV7180_STATUS1_AUTOD_NTSM_M_J:
		return V4L2_STD_NTSC;
	case ADV7180_STATUS1_AUTOD_NTSC_4_43:
		return V4L2_STD_NTSC_443;
	case ADV7180_STATUS1_AUTOD_PAL_M:
		return V4L2_STD_PAL_M;
	case ADV7180_STATUS1_AUTOD_PAL_60:
		return V4L2_STD_PAL_60;
	case ADV7180_STATUS1_AUTOD_PAL_B_G:
		return V4L2_STD_PAL;
	case ADV7180_STATUS1_AUTOD_SECAM:
		return V4L2_STD_SECAM;
	case ADV7180_STATUS1_AUTOD_PAL_COMB:
		return V4L2_STD_PAL_Nc | V4L2_STD_PAL_N;
	case ADV7180_STATUS1_AUTOD_SECAM_525:
		return V4L2_STD_SECAM;
	default:
		return V4L2_STD_UNKNOWN;
	}
}

static int v4l2_std_to_adv7280m(v4l2_std_id std)
{
	if (std == V4L2_STD_PAL_N)
		return ADV7180_STD_PAL_M;
	if (std & V4L2_STD_PAL)
		return 0x8; /* ADV7180_STD_PAL_BG */
	if (std & V4L2_STD_NTSC)
		return 0x5; /* ADV7180_STD_NTSC_M */
	if (std & V4L2_STD_SECAM)
		return 0xe; /* ADV7180_STD_PAL_SECAM */

	return -EINVAL;
}

static u32 adv7280m_status_to_v4l2(u8 status1)
{
	if (!(status1 & ADV7180_STATUS1_IN_LOCK))
		return V4L2_IN_ST_NO_SIGNAL;
	return 0;
}

static int __adv7280m_status(struct adv7280m *priv, u32 *status, v4l2_std_id *std)
{
	int status1;

	msleep(100);
	status1 = adv7280m_read(priv, ADV7180_REG_STATUS1);
	if (status1 < 0)
		return status1;

	if (status)
		*status = adv7280m_status_to_v4l2(status1);
	if (std)
		*std = adv7280m_std_to_v4l2(status1);

	return 0;
}

static int adv7280m_set_std(struct adv7280m *priv, unsigned int std)
{
	/* Failing to set the reserved bit can result in increased video noise */
	return adv7280m_write(priv, ADV7182_REG_INPUT_VIDSEL,
			      (std << 4) | ADV7182_REG_INPUT_RESERVED);
}

static int adv7280m_program_std(struct adv7280m *priv)
{
	int ret = v4l2_std_to_adv7280m(priv->curr_norm);

	if (ret < 0)
		return ret;
	return adv7280m_set_std(priv, ret);
}

static int adv7280m_querystd(struct v4l2_subdev *sd, v4l2_std_id *std)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int err = mutex_lock_interruptible(&priv->mutex);

	if (err)
		return err;

	if (priv->streaming) {
		err = -EBUSY;
		goto unlock;
	}

	err = adv7280m_set_std(priv, ADV7180_STD_AD_PAL_BG_NTSC_J_SECAM);
	if (err)
		goto unlock;

	msleep(100);
	__adv7280m_status(priv, NULL, std);

	err = v4l2_std_to_adv7280m(priv->curr_norm);
	if (err < 0)
		goto unlock;

	err = adv7280m_set_std(priv, err);

unlock:
	mutex_unlock(&priv->mutex);
	return err;
}

static int adv7280m_g_input_status(struct v4l2_subdev *sd, u32 *status)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int ret = mutex_lock_interruptible(&priv->mutex);

	if (ret)
		return ret;
	ret = __adv7280m_status(priv, status, NULL);
	mutex_unlock(&priv->mutex);
	return ret;
}

static int adv7280m_g_std(struct v4l2_subdev *sd, v4l2_std_id *norm)
{
	struct adv7280m *priv = to_adv7280m(sd);

	*norm = priv->curr_norm;
	return 0;
}

static int adv7280m_s_std(struct v4l2_subdev *sd, v4l2_std_id std)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int ret = mutex_lock_interruptible(&priv->mutex);

	if (ret)
		return ret;

	ret = v4l2_std_to_adv7280m(std);
	if (ret < 0)
		goto out;

	priv->curr_norm = std;
	ret = adv7280m_program_std(priv);
out:
	mutex_unlock(&priv->mutex);
	return ret;
}

static int adv7280m_g_pixelaspect(struct v4l2_subdev *sd, struct v4l2_fract *aspect)
{
	struct adv7280m *priv = to_adv7280m(sd);

	if (priv->curr_norm & V4L2_STD_525_60) {
		aspect->numerator = 11;
		aspect->denominator = 10;
	} else {
		aspect->numerator = 54;
		aspect->denominator = 59;
	}
	return 0;
}

static int adv7280m_g_tvnorms(struct v4l2_subdev *sd, v4l2_std_id *norm)
{
	*norm = V4L2_STD_ALL;
	return 0;
}

static unsigned int adv7280m_lbias_settings[][3] = {
	[0] = { 0xCD, 0x4E, 0x80 }, /* CVBS */
	[1] = { 0xC0, 0x4E, 0x80 }, /* DIFF_CVBS */
	[2] = { 0x0B, 0xCE, 0x80 }, /* S-Video */
	[3] = { 0x0B, 0x4E, 0xC0 }, /* YPbPr */
};

static unsigned int adv7280m_input_type(unsigned int input)
{
	switch (input) {
	case ADV7182_INPUT_SVIDEO_AIN1_AIN2:
	case ADV7182_INPUT_SVIDEO_AIN3_AIN4:
	case ADV7182_INPUT_SVIDEO_AIN5_AIN6:
	case ADV7182_INPUT_SVIDEO_AIN7_AIN8:
		return 2;
	case ADV7182_INPUT_YPRPB_AIN1_AIN2_AIN3:
	case ADV7182_INPUT_YPRPB_AIN4_AIN5_AIN6:
		return 3;
	case ADV7182_INPUT_DIFF_CVBS_AIN1_AIN2:
	case ADV7182_INPUT_DIFF_CVBS_AIN3_AIN4:
	case ADV7182_INPUT_DIFF_CVBS_AIN5_AIN6:
	case ADV7182_INPUT_DIFF_CVBS_AIN7_AIN8:
		return 1;
	default:
		return 0;
	}
}

static int adv7280m_select_input(struct adv7280m *priv, unsigned int input)
{
	unsigned int input_type, i;
	unsigned int *lbias;
	int ret;

	ret = adv7280m_write(priv, ADV7180_REG_INPUT_CONTROL, input);
	if (ret)
		return ret;

	/* Reset clamp circuitry - ADI recommended writes */
	adv7280m_write(priv, ADV7180_REG_RST_CLAMP, 0x00);
	adv7280m_write(priv, ADV7180_REG_RST_CLAMP, 0xff);

	input_type = adv7280m_input_type(input);

	if (input_type == 0 || input_type == 1)
		adv7280m_write(priv, ADV7180_REG_SHAP_FILTER_CTL_1, 0x41);
	else
		adv7280m_write(priv, ADV7180_REG_SHAP_FILTER_CTL_1, 0x01);

	lbias = adv7280m_lbias_settings[input_type];
	for (i = 0; i < ARRAY_SIZE(adv7280m_lbias_settings[0]); i++)
		adv7280m_write(priv, ADV7180_REG_CVBS_TRIM + i, lbias[i]);

	if (input_type == 1) {
		/* differential CVBS */
		adv7280m_write(priv, ADV7180_REG_RES_CIR, 0xa8);
		adv7280m_write(priv, ADV7180_REG_CLAMP_ADJ, 0x90);
		adv7280m_write(priv, ADV7180_REG_DIFF_MODE, 0xb0);
		adv7280m_write(priv, ADV7180_REG_AGC_ADJ1, 0x08);
		adv7280m_write(priv, ADV7180_REG_AGC_ADJ2, 0xa0);
	} else {
		adv7280m_write(priv, ADV7180_REG_RES_CIR, 0xf0);
		adv7280m_write(priv, ADV7180_REG_CLAMP_ADJ, 0xd0);
		adv7280m_write(priv, ADV7180_REG_DIFF_MODE, 0x10);
		adv7280m_write(priv, ADV7180_REG_AGC_ADJ1, 0x9c);
		adv7280m_write(priv, ADV7180_REG_AGC_ADJ2, 0x00);
	}

	return 0;
}

static int adv7280m_s_routing(struct v4l2_subdev *sd, u32 input, u32 output, u32 config)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int ret = mutex_lock_interruptible(&priv->mutex);

	if (ret)
		return ret;

	if (input > 31 || !(BIT(input) & ADV7280M_VALID_INPUT_MASK)) {
		ret = -EINVAL;
		goto out;
	}

	ret = adv7280m_select_input(priv, input);
	if (ret == 0)
		priv->input = input;
out:
	mutex_unlock(&priv->mutex);
	return ret;
}

static void adv7280m_set_power_pin(struct adv7280m *priv, bool on)
{
	if (!priv->pwdn_gpio)
		return;

	if (on) {
		gpiod_set_value_cansleep(priv->pwdn_gpio, 0);
		usleep_range(5000, 10000);
	} else {
		gpiod_set_value_cansleep(priv->pwdn_gpio, 1);
	}
}

static void adv7280m_set_reset_pin(struct adv7280m *priv, bool on)
{
	if (!priv->rst_gpio)
		return;

	if (on) {
		gpiod_set_value_cansleep(priv->rst_gpio, 1);
	} else {
		gpiod_set_value_cansleep(priv->rst_gpio, 0);
		usleep_range(5000, 10000);
	}
}

static int adv7280m_set_power(struct adv7280m *priv, bool on)
{
	int ret;

	ret = adv7280m_write(priv, ADV7180_REG_PWR_MAN,
			     on ? ADV7180_PWR_MAN_ON : ADV7180_PWR_MAN_OFF);
	if (ret)
		return ret;

	if (on) {
		adv7280m_csi_write(priv, 0xDE, 0x02);
		adv7280m_csi_write(priv, 0xD2, 0xF7);
		adv7280m_csi_write(priv, 0xD8, 0x65);
		adv7280m_csi_write(priv, 0xE0, 0x09);
		adv7280m_csi_write(priv, 0x2C, 0x00);
		if (priv->field == V4L2_FIELD_NONE)
			adv7280m_csi_write(priv, 0x1D, 0x80);
		adv7280m_csi_write(priv, 0x00, 0x00);
	} else {
		adv7280m_csi_write(priv, 0x00, 0x80);
	}

	return 0;
}

static int adv7280m_s_power(struct v4l2_subdev *sd, int on)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int ret = mutex_lock_interruptible(&priv->mutex);

	if (ret)
		return ret;

	ret = adv7280m_set_power(priv, on);
	if (ret == 0)
		priv->powered = on;

	mutex_unlock(&priv->mutex);
	return ret;
}

static int adv7280m_set_field_mode(struct adv7280m *priv)
{
	if (priv->field == V4L2_FIELD_NONE) {
		adv7280m_csi_write(priv, 0x01, 0x20);
		adv7280m_csi_write(priv, 0x02, 0x28);
		adv7280m_csi_write(priv, 0x03, 0x38);
		adv7280m_csi_write(priv, 0x04, 0x30);
		adv7280m_csi_write(priv, 0x05, 0x30);
		adv7280m_csi_write(priv, 0x06, 0x80);
		adv7280m_csi_write(priv, 0x07, 0x70);
		adv7280m_csi_write(priv, 0x08, 0x50);
		adv7280m_vpp_write(priv, 0xa3, 0x00);
		adv7280m_vpp_write(priv, 0x5b, 0x00);
		adv7280m_vpp_write(priv, 0x55, 0x80);
	} else {
		adv7280m_csi_write(priv, 0x01, 0x18);
		adv7280m_csi_write(priv, 0x02, 0x18);
		adv7280m_csi_write(priv, 0x03, 0x30);
		adv7280m_csi_write(priv, 0x04, 0x20);
		adv7280m_csi_write(priv, 0x05, 0x28);
		adv7280m_csi_write(priv, 0x06, 0x40);
		adv7280m_csi_write(priv, 0x07, 0x58);
		adv7280m_csi_write(priv, 0x08, 0x30);
		adv7280m_vpp_write(priv, 0xa3, 0x70);
		adv7280m_vpp_write(priv, 0x5b, 0x80);
		adv7280m_vpp_write(priv, 0x55, 0x00);
	}

	return 0;
}

/* ---------------------------------------------------------------------
 * Controls
 * ---------------------------------------------------------------------
 */
static int adv7280m_test_pattern(struct adv7280m *priv, int value)
{
	unsigned int reg = 0;

	if (value < 3)
		reg = value;
	if (value == 3)
		reg = 5;

	adv7280m_write(priv, ADV7180_REG_ANALOG_CLAMP_CTL, reg);

	if (value == ARRAY_SIZE(test_pattern_menu) - 1) {
		reg = adv7280m_read(priv, ADV7180_REG_DEF_VALUE_Y);
		reg &= ~ADV7180_DEF_VAL_EN;
		adv7280m_write(priv, ADV7180_REG_DEF_VALUE_Y, reg);
		return 0;
	}

	reg = adv7280m_read(priv, ADV7180_REG_DEF_VALUE_Y);
	reg |= ADV7180_DEF_VAL_EN | ADV7180_DEF_VAL_AUTO_EN;
	adv7280m_write(priv, ADV7180_REG_DEF_VALUE_Y, reg);

	return 0;
}

static int adv7280m_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct adv7280m *priv = container_of(ctrl->handler, struct adv7280m, ctrl_handler);
	int ret = mutex_lock_interruptible(&priv->mutex);
	int val;

	if (ret)
		return ret;
	val = ctrl->val;

	switch (ctrl->id) {
	case V4L2_CID_BRIGHTNESS:
		ret = adv7280m_write(priv, ADV7180_REG_BRI, val);
		break;
	case V4L2_CID_HUE:
		ret = adv7280m_write(priv, ADV7180_REG_HUE, -val);
		break;
	case V4L2_CID_CONTRAST:
		ret = adv7280m_write(priv, ADV7180_REG_CON, val);
		break;
	case V4L2_CID_SATURATION:
		ret = adv7280m_write(priv, ADV7180_REG_SD_SAT_CB, val);
		if (ret < 0)
			break;
		ret = adv7280m_write(priv, ADV7180_REG_SD_SAT_CR, val);
		break;
	case V4L2_CID_ADV_FAST_SWITCH:
		if (ctrl->val) {
			adv7280m_write(priv, 0x80d9, 0x44);
			adv7280m_write(priv, 0x40e0, 0x1);
		} else {
			adv7280m_write(priv, 0x80d9, 0xc4);
			adv7280m_write(priv, 0x40e0, 0x00);
		}
		break;
	case V4L2_CID_TEST_PATTERN:
		ret = adv7280m_test_pattern(priv, val);
		break;
	default:
		ret = -EINVAL;
	}

	mutex_unlock(&priv->mutex);
	return ret;
}

static const struct v4l2_ctrl_ops adv7280m_ctrl_ops = {
	.s_ctrl = adv7280m_s_ctrl,
};

static const struct v4l2_ctrl_config adv7280m_ctrl_fast_switch = {
	.ops = &adv7280m_ctrl_ops,
	.id = V4L2_CID_ADV_FAST_SWITCH,
	.name = "Fast Switching",
	.type = V4L2_CTRL_TYPE_BOOLEAN,
	.min = 0,
	.max = 1,
	.step = 1,
};

static int adv7280m_ctrls_init(struct adv7280m *priv)
{
	struct i2c_client *client = priv->i2c_client;
	int i = 0;

	dev_info(&client->dev, "%s++\n", __func__);

	v4l2_ctrl_handler_init(&priv->ctrl_handler, 8);

	priv->link_freq = v4l2_ctrl_new_int_menu(&priv->ctrl_handler, NULL,
						 V4L2_CID_LINK_FREQ,
						 ARRAY_SIZE(adv7280m_link_freq_menu) - 1,
						 0, adv7280m_link_freq_menu);
	if (priv->link_freq)
		priv->link_freq->flags |= V4L2_CTRL_FLAG_READ_ONLY;
	priv->ctrls[i++] = priv->link_freq;

	priv->pixel_rate = v4l2_ctrl_new_std(&priv->ctrl_handler, NULL,
					     V4L2_CID_PIXEL_RATE,
					     0, ADV7280M_PIXEL_RATE, 1,
					     ADV7280M_PIXEL_RATE);
	if (priv->pixel_rate)
		priv->pixel_rate->flags |= V4L2_CTRL_FLAG_READ_ONLY;
	priv->ctrls[i++] = priv->pixel_rate;

	priv->brightness = v4l2_ctrl_new_std(&priv->ctrl_handler, &adv7280m_ctrl_ops,
					     V4L2_CID_BRIGHTNESS, ADV7180_BRI_MIN,
					     ADV7180_BRI_MAX, 1, ADV7180_BRI_DEF);
	priv->ctrls[i++] = priv->brightness;

	priv->contrast = v4l2_ctrl_new_std(&priv->ctrl_handler, &adv7280m_ctrl_ops,
					   V4L2_CID_CONTRAST, ADV7180_CON_MIN,
					   ADV7180_CON_MAX, 1, ADV7180_CON_DEF);
	priv->ctrls[i++] = priv->contrast;

	priv->saturation = v4l2_ctrl_new_std(&priv->ctrl_handler, &adv7280m_ctrl_ops,
					     V4L2_CID_SATURATION, ADV7180_SAT_MIN,
					     ADV7180_SAT_MAX, 1, ADV7180_SAT_DEF);
	priv->ctrls[i++] = priv->saturation;

	priv->hue = v4l2_ctrl_new_std(&priv->ctrl_handler, &adv7280m_ctrl_ops,
				      V4L2_CID_HUE, ADV7180_HUE_MIN,
				      ADV7180_HUE_MAX, 1, ADV7180_HUE_DEF);
	priv->ctrls[i++] = priv->hue;

	priv->fast_switch = v4l2_ctrl_new_custom(&priv->ctrl_handler,
						 &adv7280m_ctrl_fast_switch, NULL);
	priv->ctrls[i++] = priv->fast_switch;

	priv->test_pattern = v4l2_ctrl_new_std_menu_items(&priv->ctrl_handler,
					&adv7280m_ctrl_ops, V4L2_CID_TEST_PATTERN,
					ARRAY_SIZE(test_pattern_menu) - 1,
					0, ARRAY_SIZE(test_pattern_menu) - 1,
					test_pattern_menu);
	priv->ctrls[i++] = priv->test_pattern;

	if (priv->ctrl_handler.error) {
		int err = priv->ctrl_handler.error;

		v4l2_ctrl_handler_free(&priv->ctrl_handler);
		return err;
	}

	priv->subdev->ctrl_handler = priv->s_data->ctrl_handler = &priv->ctrl_handler;
	v4l2_ctrl_handler_setup(&priv->ctrl_handler);

	return 0;
}


static int adv7280m_enum_mbus_code(struct v4l2_subdev *sd,
				   struct v4l2_subdev_state *sd_state,
				   struct v4l2_subdev_mbus_code_enum *code)
{
	if (code->index != 0)
		return -EINVAL;

	code->code = ADV7280M_DEFAULT_DATAFMT;
	return 0;
}

static void adv7280m_mbus_fmt(struct adv7280m *priv, struct v4l2_mbus_framefmt *fmt)
{
	fmt->code = ADV7280M_DEFAULT_DATAFMT;
	fmt->colorspace = V4L2_COLORSPACE_SMPTE170M;
	fmt->width = 720;
	fmt->height = priv->curr_norm & V4L2_STD_525_60 ? 480 : 576;

	if (priv->field == V4L2_FIELD_ALTERNATE)
		fmt->height /= 2;
}

static struct v4l2_mbus_framefmt *
adv7280m_state_get_format(struct v4l2_subdev_state *sd_state, unsigned int pad)
{
	if (!sd_state || !sd_state->pads)
		return NULL;
	return &sd_state->pads[pad].format;
}

static int adv7280m_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state,
			    struct v4l2_subdev_format *format)
{
	struct adv7280m *priv = to_adv7280m(sd);

	if (format->which == V4L2_SUBDEV_FORMAT_TRY) {
		struct v4l2_mbus_framefmt *fmt = adv7280m_state_get_format(sd_state, 0);

		if (!fmt)
			return -EINVAL;
		format->format = *fmt;
	} else {
		adv7280m_mbus_fmt(priv, &format->format);
		format->format.field = priv->field;
	}

	return 0;
}

static int adv7280m_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state,
			    struct v4l2_subdev_format *format)
{
	struct adv7280m *priv = to_adv7280m(sd);
	struct v4l2_mbus_framefmt *framefmt;

	switch (format->format.field) {
	case V4L2_FIELD_NONE:
		break;
	default:
		format->format.field = V4L2_FIELD_ALTERNATE;
		break;
	}

	adv7280m_mbus_fmt(priv, &format->format);

	if (format->which == V4L2_SUBDEV_FORMAT_ACTIVE) {
		if (priv->field != format->format.field) {
			priv->field = format->format.field;
			adv7280m_set_power(priv, false);
			adv7280m_set_field_mode(priv);
			adv7280m_set_power(priv, true);
		}
	} else {
		framefmt = adv7280m_state_get_format(sd_state, 0);
		if (!framefmt)
			return -EINVAL;
		*framefmt = format->format;
	}

	return 0;
}

static int adv7280m_init_state(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state)
{
	struct v4l2_subdev_format fmt = {
		.which = sd_state ? V4L2_SUBDEV_FORMAT_TRY : V4L2_SUBDEV_FORMAT_ACTIVE,
	};

	return adv7280m_set_fmt(sd, sd_state, &fmt);
}

static int adv7280m_get_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				    struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2_DPHY;
	cfg->bus.mipi_csi2.num_data_lanes = 1;
	cfg->bus.mipi_csi2.flags = 0;
	return 0;
}

static int adv7280m_get_skip_frames(struct v4l2_subdev *sd, u32 *frames)
{
	*frames = ADV7280M_NUM_OF_SKIP_FRAMES;
	return 0;
}

static int adv7280m_get_frame_interval(struct v4l2_subdev *sd,
				       struct v4l2_subdev_state *sd_state,
				       struct v4l2_subdev_frame_interval *fi)
{
	struct adv7280m *priv = to_adv7280m(sd);

	if (fi->which != V4L2_SUBDEV_FORMAT_ACTIVE)
		return -EINVAL;

	if (priv->curr_norm & V4L2_STD_525_60) {
		fi->interval.numerator = 1001;
		fi->interval.denominator = 30000;
	} else {
		fi->interval.numerator = 1;
		fi->interval.denominator = 25;
	}

	return 0;
}

static int adv7280m_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct adv7280m *priv = to_adv7280m(sd);
	int ret;

	if (!enable) {
		priv->streaming = enable;
		return 0;
	}

	ret = mutex_lock_interruptible(&priv->mutex);
	if (ret)
		return ret;
	priv->streaming = enable;
	mutex_unlock(&priv->mutex);
	msleep(300);
	return 0;
}

static int adv7280m_subscribe_event(struct v4l2_subdev *sd, struct v4l2_fh *fh,
				    struct v4l2_event_subscription *sub)
{
	switch (sub->type) {
	case V4L2_EVENT_SOURCE_CHANGE:
		return v4l2_src_change_event_subdev_subscribe(sd, fh, sub);
	case V4L2_EVENT_CTRL:
		return v4l2_ctrl_subdev_subscribe_event(sd, fh, sub);
	default:
		return -EINVAL;
	}
}

static const struct v4l2_subdev_video_ops adv7280m_video_ops = {
	.s_std = adv7280m_s_std,
	.g_std = adv7280m_g_std,
	.querystd = adv7280m_querystd,
	.g_input_status = adv7280m_g_input_status,
	.s_routing = adv7280m_s_routing,
	.g_pixelaspect = adv7280m_g_pixelaspect,
	.g_tvnorms = adv7280m_g_tvnorms,
	.s_stream = adv7280m_s_stream,
};

static const struct v4l2_subdev_core_ops adv7280m_core_ops = {
	.s_power = adv7280m_s_power,
	.subscribe_event = adv7280m_subscribe_event,
	.unsubscribe_event = v4l2_event_subdev_unsubscribe,
};

static const struct v4l2_subdev_pad_ops adv7280m_pad_ops = {
	.enum_mbus_code = adv7280m_enum_mbus_code,
	.set_fmt = adv7280m_set_fmt,
	.get_fmt = adv7280m_get_fmt,
	.get_frame_interval = adv7280m_get_frame_interval,
	.get_mbus_config = adv7280m_get_mbus_config,
};

static const struct v4l2_subdev_sensor_ops adv7280m_sensor_ops = {
	.g_skip_frames = adv7280m_get_skip_frames,
};

static const struct v4l2_subdev_ops adv7280m_subdev_ops = {
	.core = &adv7280m_core_ops,
	.video = &adv7280m_video_ops,
	.pad = &adv7280m_pad_ops,
	.sensor = &adv7280m_sensor_ops,
};

static int adv7280m_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);

	dev_info(&client->dev, "%s\n", __func__);
	return 0;
}

static const struct v4l2_subdev_internal_ops adv7280m_internal_ops = {
	.init_state = adv7280m_init_state,
	.open = adv7280m_open,
};

static const struct media_entity_operations adv7280m_media_ops = {
	.link_validate = v4l2_subdev_link_validate,
};


static irqreturn_t adv7280m_irq(int irq, void *devid)
{
	struct adv7280m *priv = devid;
	u8 isr3;

	mutex_lock(&priv->mutex);
	isr3 = adv7280m_read(priv, ADV7180_REG_ISR3);
	adv7280m_write(priv, ADV7180_REG_ICR3, isr3);

	if (isr3 & ADV7180_IRQ3_AD_CHANGE) {
		static const struct v4l2_event src_ch = {
			.type = V4L2_EVENT_SOURCE_CHANGE,
			.u.src_change.changes = V4L2_EVENT_SRC_CH_RESOLUTION,
		};

		v4l2_subdev_notify_event(priv->subdev, &src_ch);
	}
	mutex_unlock(&priv->mutex);

	return IRQ_HANDLED;
}


static int adv7280m_chip_init(struct adv7280m *priv)
{
	adv7280m_write(priv, ADV7180_REG_CSI_SLAVE_ADDR, ADV7180_DEFAULT_CSI_I2C_ADDR << 1);
	adv7280m_write(priv, ADV7180_REG_VPP_SLAVE_ADDR, ADV7180_DEFAULT_VPP_I2C_ADDR << 1);

	/* ADI recommended writes for improved video quality */
	adv7280m_write(priv, 0x0080, 0x51);
	adv7280m_write(priv, 0x0081, 0x51);
	adv7280m_write(priv, 0x0082, 0x68);

	/* ADI required writes */
	adv7280m_write(priv, ADV7180_REG_OUTPUT_CONTROL, 0x4e);
	adv7280m_write(priv, ADV7180_REG_EXTENDED_OUTPUT_CONTROL, 0x57);
	adv7280m_write(priv, ADV7180_REG_CTRL_2, 0xc0);

	adv7280m_write(priv, ADV7180_REG_STATUS3, 0x00);

	/* color bars test pattern by default, matches adv7180.c behavior */
	adv7280m_write(priv, ADV7180_REG_ANALOG_CLAMP_CTL, 0x11);

	return 0;
}

static int adv7280m_init_device(struct adv7280m *priv)
{
	int ret;

	mutex_lock(&priv->mutex);

	adv7280m_set_power_pin(priv, true);
	adv7280m_set_reset_pin(priv, false);

	adv7280m_write(priv, ADV7180_REG_PWR_MAN, ADV7180_PWR_MAN_RES);
	usleep_range(5000, 10000);

	ret = adv7280m_chip_init(priv);
	if (ret)
		goto out_unlock;

	ret = adv7280m_program_std(priv);
	if (ret)
		goto out_unlock;

	adv7280m_set_field_mode(priv);

	if (priv->irq > 0) {
		ret = adv7280m_write(priv, ADV7180_REG_ICONF1,
				     ADV7180_ICONF1_ACTIVE_LOW | ADV7180_ICONF1_PSYNC_ONLY);
		if (ret < 0)
			goto out_unlock;

		ret = adv7280m_write(priv, ADV7180_REG_IMR1, 0);
		if (ret < 0)
			goto out_unlock;
		ret = adv7280m_write(priv, ADV7180_REG_IMR2, 0);
		if (ret < 0)
			goto out_unlock;
		ret = adv7280m_write(priv, ADV7180_REG_IMR3, ADV7180_IRQ3_AD_CHANGE);
		if (ret < 0)
			goto out_unlock;
		ret = adv7280m_write(priv, ADV7180_REG_IMR4, 0);
		if (ret < 0)
			goto out_unlock;
	}

out_unlock:
	mutex_unlock(&priv->mutex);
	return ret;
}

static const struct of_device_id adv7280m_dt_ids[] = {
	{ .compatible = "forecr,adv7280-m" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, adv7280m_dt_ids);

static struct camera_common_pdata *adv7280m_parse_dt(struct i2c_client *client)
{
	struct device_node *np = client->dev.of_node;
	struct device *dev = &client->dev;
	struct camera_common_pdata *board_priv_pdata;

	if (!np)
		return NULL;

	if (!of_match_device(adv7280m_dt_ids, dev)) {
		dev_err(dev, "failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata = devm_kzalloc(dev, sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;

	return board_priv_pdata;
}

static int adv7280m_probe(struct i2c_client *client)
{
	struct device_node *np = client->dev.of_node;
	struct camera_common_data *common_data;
	struct device *dev = &client->dev;
	struct adv7280m *priv;
	int ret, data;

	dev_info(dev, "Entering %s probe\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -EIO;

	common_data = devm_kzalloc(dev, sizeof(*common_data), GFP_KERNEL);
	if (!common_data)
		return -ENOMEM;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->i2c_client = client;
	priv->field = V4L2_FIELD_NONE;
	priv->curr_norm = V4L2_STD_PAL;
	priv->input = 0;
	priv->irq = client->irq;

	common_data->ops = NULL;
	common_data->ctrl_handler = &priv->ctrl_handler;
	common_data->dev = dev;
	common_data->power = &priv->power;
	common_data->ctrls = priv->ctrls;
	common_data->priv = (void *)priv;

	common_data->frmfmt = adv7280m_frmfmt;
	common_data->colorfmt = camera_common_find_datafmt(ADV7280M_DEFAULT_DATAFMT);
	common_data->numfmts = ARRAY_SIZE(adv7280m_frmfmt);
	common_data->numctrls = 8;

	common_data->def_mode = ADV7280M_DEFAULT_MODE;
	common_data->def_width = ADV7280M_DEFAULT_WIDTH;
	common_data->def_height = ADV7280M_DEFAULT_HEIGHT;
	common_data->def_clk_freq = ADV7280M_DEFAULT_CLK_FREQ;
	common_data->fmt_width = common_data->def_width;
	common_data->fmt_height = common_data->def_height;

	priv->s_data = common_data;
	priv->subdev = &common_data->subdev;

	mutex_init(&priv->mutex);

	v4l2_i2c_subdev_init(priv->subdev, client, &adv7280m_subdev_ops);

	priv->pdata = adv7280m_parse_dt(client);
	if (!priv->pdata) {
		dev_err(dev, "unable to parse dt / get platform data\n");
		ret = -EINVAL;
		goto error_mutex;
	}

	if (of_property_read_bool(np, "adv,force-bt656-4"))
		priv->force_bt656_4 = true;

	priv->pwdn_gpio = devm_gpiod_get_optional(dev, "powerdown", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->pwdn_gpio)) {
		ret = PTR_ERR(priv->pwdn_gpio);
		dev_err(dev, "request for power pin failed: %d\n", ret);
		goto error_mutex;
	}

	priv->rst_gpio = devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(priv->rst_gpio)) {
		ret = PTR_ERR(priv->rst_gpio);
		dev_err(dev, "request for reset pin failed: %d\n", ret);
		goto error_mutex;
	}

	ret = camera_common_initialize(common_data, "adv7280-m");
	if (ret) {
		dev_err(dev, "Failed to initialize adv7280-m (%d)\n", ret);
		goto error_mutex;
	}

	priv->csi_client = i2c_new_dummy_device(client->adapter, ADV7180_DEFAULT_CSI_I2C_ADDR);
	if (IS_ERR(priv->csi_client)) {
		ret = PTR_ERR(priv->csi_client);
		goto error_common;
	}

	priv->vpp_client = i2c_new_dummy_device(client->adapter, ADV7180_DEFAULT_VPP_I2C_ADDR);
	if (IS_ERR(priv->vpp_client)) {
		ret = PTR_ERR(priv->vpp_client);
		goto error_i2c_csi;
	}

	ret = adv7280m_ctrls_init(priv);
	if (ret)
		goto error_i2c_vpp;

	priv->subdev->internal_ops = &adv7280m_internal_ops;
	priv->subdev->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;

#if defined(CONFIG_MEDIA_CONTROLLER)
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->subdev->entity.function = MEDIA_ENT_F_ATV_DECODER;
	priv->subdev->entity.ops = &adv7280m_media_ops;
	ret = tegra_media_entity_init(&priv->subdev->entity, 1, &priv->pad, true, true);
	if (ret < 0) {
		dev_err(dev, "unable to init media entity\n");
		goto error_handler_free;
	}
#endif

	ret = adv7280m_init_device(priv);
	if (ret)
		goto error_media_entity;

	if (priv->irq) {
		ret = request_threaded_irq(client->irq, NULL, adv7280m_irq,
					   IRQF_ONESHOT | IRQF_TRIGGER_FALLING,
					   KBUILD_MODNAME, priv);
		if (ret)
			goto error_media_entity;
	}

	ret = v4l2_async_register_subdev(priv->subdev);
	if (ret) {
		dev_err(dev, "failed to register sensor sub-device: %d\n", ret);
		goto error_irq;
	}

	mutex_lock(&priv->mutex);
	ret = adv7280m_read(priv, ADV7180_REG_IDENT);
	mutex_unlock(&priv->mutex);
	if (ret < 0)
		goto error_async_unregister;

	v4l_info(client, "chip id 0x%x found @ 0x%02x (%s)\n",
		 ret, client->addr, client->adapter->name);

	dev_info(dev, "avdcdr bt656_4 support: %d\n", priv->force_bt656_4);
	if (of_get_property(np, "analog_input_select", NULL)) {
		of_property_read_u32(np, "analog_input_select", &priv->analog_input);
		ret = adv7280m_write(priv, ADV7180_REG_INPUT_CONTROL, priv->analog_input);
		if (ret < 0)
			goto error_async_unregister;
		msleep(100);
		data = adv7280m_read(priv, ADV7180_REG_INPUT_CONTROL);
		v4l_info(client, "successfully initialized on AIN%d", data + 1);
	}

	dev_info(dev, "adv7280-m probe successful\n");
	return 0;

error_async_unregister:
	v4l2_async_unregister_subdev(priv->subdev);
error_irq:
	if (priv->irq > 0)
		free_irq(client->irq, priv);
error_media_entity:
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&priv->subdev->entity);
error_handler_free:
#endif
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
error_i2c_vpp:
	i2c_unregister_device(priv->vpp_client);
error_i2c_csi:
	i2c_unregister_device(priv->csi_client);
error_common:
	camera_common_cleanup(common_data);
error_mutex:
	mutex_destroy(&priv->mutex);
	return ret;
}

static void adv7280m_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd = i2c_get_clientdata(client);
	struct camera_common_data *s_data = container_of(sd, struct camera_common_data, subdev);
	struct adv7280m *priv = (struct adv7280m *)s_data->priv;

	v4l2_async_unregister_subdev(priv->subdev);

	if (priv->irq > 0)
		free_irq(client->irq, priv);

#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&priv->subdev->entity);
#endif
	v4l2_ctrl_handler_free(&priv->ctrl_handler);

	i2c_unregister_device(priv->vpp_client);
	i2c_unregister_device(priv->csi_client);

	adv7280m_set_reset_pin(priv, true);
	adv7280m_set_power_pin(priv, false);

	camera_common_cleanup(s_data);
	mutex_destroy(&priv->mutex);
}

static const struct i2c_device_id adv7280m_id[] = {
	{ "adv7280-m", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(i2c, adv7280m_id);

static struct i2c_driver adv7280m_i2c_driver = {
	.driver = {
		.name = "adv7280-m",
		.owner = THIS_MODULE,
		.of_match_table = adv7280m_dt_ids,
	},
	.probe = adv7280m_probe,
	.remove = adv7280m_remove,
	.id_table = adv7280m_id,
};

module_i2c_driver(adv7280m_i2c_driver);

MODULE_DESCRIPTION("Analog Devices ADV7280-M video decoder driver");
MODULE_AUTHOR("Kaya Kaan Tuna <kayatuna@forecr.io>");
MODULE_LICENSE("GPL v2");
