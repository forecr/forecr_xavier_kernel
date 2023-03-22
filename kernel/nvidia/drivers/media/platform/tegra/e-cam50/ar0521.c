/*
 * ar0521.c - AR0521 sensor driver
 * Copyright (c) 2017-2018, e-con Systems.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>

#include <media/camera_common.h>
#include "camera/camera_gpio.h"

/* For Thread */
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/sched.h>

#include "ar0521.h"

#include "cam_firmware.h"
#include "stream_mon_thrd.h"

#define DEBUG_PRINTK
#ifndef DEBUG_PRINTK
#define debug_printk(s , ... )
#else
#define debug_printk printk
#endif

#define MAX_NUM_CAM 6

static uint8_t num_cam = 0;
int init_err_hand_q=0;

/*For Thread*/
static struct task_struct *strm_mon_thrd;
static uint8_t is_stream_monitor_thrd = 0, stop_thread=0;
struct stream_monitor strm_mon[MAX_NUM_CAM];

/*For Queue*/
extern wait_queue_head_t econ_err_hand_q;
extern int econ_frame_err_track;
extern int econ_num_uncorr_err;
extern char econ_dev_name[32];

static const struct v4l2_ctrl_ops ar0521_ctrl_ops = {
	.g_volatile_ctrl = ar0521_g_volatile_ctrl,
	.s_ctrl = ar0521_s_ctrl,
};

int mcu_err_handle(struct i2c_client *err_client, struct ar0521 *err_priv)
{
	int err = 0;

	err = ar0521_reset_cam(err_client);
	if(err < 0) {
		dev_err(&err_client->dev,"%s failed.\n", __func__);
		return err;
	}

	return 0;
}

int find_err_cam(char *err_cam_name)
{
	int loop = 0;
	int cam_name_len = 0;
	cam_name_len = strlen(err_cam_name);
	if(cam_name_len <= 0) {
#if DEBUG
		printk("\nInvalid name found");
#endif
		return -1;
	}

	for(loop =0; loop < MAX_NUM_CAM; loop++) {
		if(!(strcmp(err_cam_name, strm_mon[loop].camera_name))){
#if DEBUG
			printk("Facing streaming issue in CAM %d\n",loop);
#endif
			goto cam_find_exit;
		}
	}
cam_find_exit:

	return loop;

}

/**
 * Econ's Error Handling Thread function
 */

int stream_monitor_thread(void *strm_dev)
{
	int count = 0;
	uint8_t err_cam = 0 ;
	while(!kthread_should_stop())
	{
		printk("Inside stream monitor thread and waiting for the event %d\n",count++);
		msleep(100);
		wait_event_interruptible(econ_err_hand_q, econ_frame_err_track == 1);
		if(econ_frame_err_track == 1 && stop_thread != 1 ) {
			econ_num_uncorr_err = 0;
			econ_frame_err_track = 0;
			printk("Got the uncorr_err event for the camera, %s\n",econ_dev_name);
			err_cam = find_err_cam(econ_dev_name);
			if(err_cam >=0 && err_cam < MAX_NUM_CAM) {
				mcu_err_handle(strm_mon[err_cam].strm_client_mon,strm_mon[err_cam].strm_priv_mon);
			}
		}
		else{
			msleep(100);
#if DEBUG
			printk("No Un_Corr ERROR\n");
#endif
		}
	}
#ifdef DEBUG
	printk("Stream monitor thread is stopped\n");
#endif
	return 0;
}

static int ar0521_power_on(struct camera_common_data *s_data)
{
	int err = 0;
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	struct camera_common_power_rail *pw = &priv->power;

	if (!priv || !priv->pdata)
		return -EINVAL;
	dev_dbg(&priv->i2c_client->dev, "%s: power on\n", __func__);

	if (priv->pdata && priv->pdata->power_on) {
		err = priv->pdata->power_on(pw);
		if (err)
			dev_err(&priv->i2c_client->dev,"%s failed.\n", __func__);
		else
			pw->state = SWITCH_ON;
		return err;
	}

	if (unlikely(!(pw->avdd || pw->iovdd )))
		goto skip_power_seqn;

	if (pw->avdd)
		err = regulator_enable(pw->avdd);
	if (err)
		goto ar0521_avdd_fail;

	if (pw->iovdd)
		err = regulator_enable(pw->iovdd);
	if (err)
		goto ar0521_iovdd_fail;

skip_power_seqn:
	usleep_range(1350, 1360);

	pw->state = SWITCH_ON;
	return 0;

 ar0521_iovdd_fail:
	regulator_disable(pw->avdd);

 ar0521_avdd_fail:
	dev_err(&priv->i2c_client->dev,"%s failed.\n", __func__);
	return -ENODEV;
}

#if 0
static int ar0521_power_put(struct ar0521 *priv)
{
	struct camera_common_power_rail *pw = &priv->power;
	if (!priv || !priv->pdata)
		return -EINVAL;

	if (unlikely(!pw))
		return -EFAULT;

	if (likely(pw->avdd))
		regulator_put(pw->avdd);

	if (likely(pw->iovdd))
		regulator_put(pw->iovdd);

	pw->avdd = NULL;
	pw->iovdd = NULL;

	if (priv->pdata->use_cam_gpio)
		cam_gpio_deregister(&priv->i2c_client->dev, pw->pwdn_gpio);
	else {
		gpio_free(pw->pwdn_gpio);
		gpio_free(pw->reset_gpio);
	}

	return 0;
}
#endif

static int ar0521_power_get(struct ar0521 *priv)
{
	struct camera_common_power_rail *pw = &priv->power;
	struct camera_common_pdata *pdata = priv->pdata;
	const char *mclk_name;
	const char *parentclk_name;
	struct clk *parent;
	int err = 0;

	if (!priv || !priv->pdata)
		return -EINVAL;

        if(priv->pdata->mclk_name) {
		pw->mclk = devm_clk_get(&priv->i2c_client->dev, mclk_name);
		if (IS_ERR(pw->mclk)) {
			dev_err(&priv->i2c_client->dev, "unable to get clock %s\n",
				mclk_name);
			return PTR_ERR(pw->mclk);
	        }

		parentclk_name = priv->pdata->parentclk_name;
	        if (parentclk_name) {
			parent = devm_clk_get(&priv->i2c_client->dev, parentclk_name);
			if (IS_ERR(parent))
				dev_err(&priv->i2c_client->dev,
					"unable to get parent clock %s",
					parentclk_name);
			else
				clk_set_parent(pw->mclk, parent);
		}
	}

	if(pdata->regulators.avdd)
		err |= camera_common_regulator_get(&priv->i2c_client->dev, &pw->avdd,
				pdata->regulators.avdd);

	if(pdata->regulators.iovdd)
		err |= camera_common_regulator_get(&priv->i2c_client->dev, &pw->iovdd,
				pdata->regulators.iovdd);

	if (err) {
		dev_err(&priv->i2c_client->dev, "%s: unable to get regulator(s)\n", __func__);
		goto done;
	}

done :
	pw->state = SWITCH_OFF;
	return err;
}

static int ar0521_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	//int err = 0;

	if (!priv || !priv->pdata)
		return -EINVAL;

	// Increment the refs count when streaming and decrement when streaming is disabled
	if (enable) {
		if (!try_module_get(s_data->owner))
			return -ENODEV;
	} else {
		module_put(s_data->owner);
	}
	
	
#if 0
	if (!enable) {
		//msleep(50);
		//return 0;
		/* Perform Stream Off Sequence - if any */
#if 0	/* 	Avoided stream_on/off for green frame issue occurrence */
		err = cam_stream_off(client);
		return err;
#endif
	}

	/* Perform Stream On Sequence - if any  */
#if 0
	err = cam_stream_on(client);
	if(err!= 0){
		dev_err(&client->dev,"%s (%d) Stream_On \n", __func__, __LINE__);
		return err;
	}

	mdelay(10);
#endif
#endif
	return 0;
}

static int ar0521_g_input_status(struct v4l2_subdev *sd, u32 * status)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	struct camera_common_power_rail *pw = &priv->power;

	if (!priv || !priv->pdata)
		return -EINVAL;

	*status = pw->state == SWITCH_ON;
	return 0;
}

static int ar0521_g_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;

	if (!priv || !priv->pdata) {
		return -ENOTTY;
	}

	param->parm.capture.capability |= V4L2_CAP_TIMEPERFRAME;

	param->parm.capture.timeperframe.denominator =
	    priv->cam_frmfmt[priv->frmfmt_mode].framerates[priv->frate_index];
	param->parm.capture.timeperframe.numerator = 1;

	return 0;
}

static int ar0521_s_parm(struct v4l2_subdev *sd, struct v4l2_streamparm *param)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	int ret = 0, err = 0;

	if (!priv || !priv->pdata) {
		return -EINVAL;
	}

	for (ret = 0; ret < priv->cam_frmfmt[priv->frmfmt_mode].num_framerates;
	     ret++) {
		if ((priv->cam_frmfmt[priv->frmfmt_mode].framerates[ret] ==
		     param->parm.capture.timeperframe.denominator)) {
			priv->frate_index = ret;

			param->parm.capture.capability |= V4L2_CAP_TIMEPERFRAME;
			param->parm.capture.timeperframe.denominator =
			priv->cam_frmfmt[priv->frmfmt_mode].framerates[priv->frate_index];
			param->parm.capture.timeperframe.numerator = 1;

			/* call stream config with width, height, frame rate */
			err =
				cam_stream_config(client, priv->format_fourcc, priv->frmfmt_mode,
						priv->frate_index);
			if (err < 0) {
				dev_err(&client->dev, "%s: Failed stream_config \n", __func__);
				return err;
			}
			mdelay(10);

			return 0;
		}
	}

	param->parm.capture.capability |= V4L2_CAP_TIMEPERFRAME;
	param->parm.capture.timeperframe.denominator =
	priv->cam_frmfmt[priv->frmfmt_mode].framerates[priv->frate_index];
	param->parm.capture.timeperframe.numerator = 1;

	return 0;
}

#if ISP_PWDN_WKUP
static int ar0521_s_power(struct v4l2_subdev *sd, int on)
{
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	int err;

	if(on) {
		if(priv->power_on == 0) {
			/* Perform power wakeup sequence */
			err = cam_isp_power_wakeup(client);
			if(err < 0)
			{
				dev_err(&client->dev, "%s: Failed Power_wakeup\n", __func__);
				return err;
			}
			mdelay(80);

			pr_info("Sensor Hardware Power Up Sequence\n");
		}
		priv->power_on++;
	}
	else {
		if(priv->power_on == 1) {
			/* Perform power down Sequence */
			err = cam_isp_power_down(client);
			if (err < 0)
			{
				dev_err(&client->dev, "%s: Failed power_down\n", __func__);
				return err;
			}
			pr_info("Sensor Hardware Power Down Sequence\n");
		}
		priv->power_on--;
	}
	return 0;
}
#endif

static struct v4l2_subdev_video_ops ar0521_subdev_video_ops = {
	.s_stream = ar0521_s_stream,
	.g_mbus_config = camera_common_g_mbus_config,
	.g_input_status = ar0521_g_input_status,
	.g_parm = ar0521_g_parm,
	.s_parm = ar0521_s_parm,
};

static struct v4l2_subdev_core_ops ar0521_subdev_core_ops = {
#if ISP_PWDN_WKUP
	.s_power = ar0521_s_power,
#else
	.s_power = camera_common_s_power,
#endif
};

static int ar0521_get_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	return camera_common_g_fmt(sd, &format->format);
}

static int ar0521_set_fmt(struct v4l2_subdev *sd, struct v4l2_subdev_pad_config *cfg,
			  struct v4l2_subdev_format *format)
{
	int ret;
	struct i2c_client *client = v4l2_get_subdevdata(sd);
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	int flag = 0, err = 0;
#ifdef FRAMESYNC_ENABLE
	int mode = 0;

	mode = s_data->mode;
#endif

	if (!priv || !priv->pdata)
		return -EINVAL;

	switch (format->format.code) {
	case MEDIA_BUS_FMT_UYVY8_1X16:
		priv->format_fourcc = V4L2_PIX_FMT_UYVY;
		break;

	default:
		/* Not Implemented */
		if (format->which != V4L2_SUBDEV_FORMAT_TRY) {
			return -EINVAL;
		}
	}

	/* Delay added for green frame issue */
	//msleep(50);
	//mode = s_data->mode;
	if (format->which == V4L2_SUBDEV_FORMAT_TRY) {
		ret = camera_common_try_fmt(sd, &format->format);
	} else {

		for (ret = 0; ret < s_data->numfmts ; ret++) {
			if ((priv->cam_frmfmt[ret].size.width == format->format.width)
					&& (priv->cam_frmfmt[ret].size.height ==
						format->format.height)) {
				priv->frmfmt_mode = priv->cam_frmfmt[ret].mode;
				flag = 1;
				break;
			}
		}

		if(flag == 0) {
			return -EINVAL;
		}

		/* call stream config with width, height, frame rate */
		err =
			cam_stream_config(client, priv->format_fourcc, priv->frmfmt_mode,
					priv->frate_index);
		if (err < 0) {
			dev_err(&client->dev, "%s: Failed stream_config \n", __func__);
			return err;
		}

		mdelay(10);

		ret = camera_common_s_fmt(sd, &format->format);
	}

#ifdef FRAMESYNC_ENABLE
	/*
	 * Recalibrate in case of Resolution switch from other resolution to MODE_2304x1296 or MODE_2304x1536
	 * when frame sync is calibrated at 60Hz Frequency then ReCalibrate to
	 * 30Hz frequency for MODE_2592x1944 or MODE_2560x1440 resolution
	 */
	if((mode == MODE_2592x1944 && priv->last_sync_mode == 2) ||
			(mode == MODE_2560x1440 && priv->last_sync_mode == 2 &&
			 priv->mipi_lane_config == NUM_LANES_2)) {
		calibration_init(0);
		priv->last_sync_mode = 1;
	}
#endif

	return ret;
}

static struct v4l2_subdev_pad_ops ar0521_subdev_pad_ops = {
	.enum_mbus_code = camera_common_enum_mbus_code,
	.set_fmt = ar0521_set_fmt,
	.get_fmt = ar0521_get_fmt,
	.enum_frame_size = camera_common_enum_framesizes,
	.enum_frame_interval = camera_common_enum_frameintervals,
};

static struct v4l2_subdev_ops ar0521_subdev_ops = {
	.core = &ar0521_subdev_core_ops,
	.video = &ar0521_subdev_video_ops,
	.pad = &ar0521_subdev_pad_ops,
};

static struct of_device_id ar0521_of_match[] = {
	{.compatible = "nvidia,ar0521",},
	{},
};

static int ar0521_g_volatile_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ar0521 *priv =
	    container_of(ctrl->handler, struct ar0521, ctrl_handler);
	struct i2c_client *client = priv->i2c_client;
	int err = 0;

	uint8_t ctrl_type = 0;
	int ctrl_val = 0;
	if (!priv || !priv->pdata)
		return -EINVAL;

	if (priv->power.state == SWITCH_OFF)
		return 0;

	if ((err = cam_get_ctrl(client, ctrl->id, &ctrl_type, &ctrl_val)) < 0) {
		return err;
	}

	if (ctrl_type == CTRL_STANDARD) {
		ctrl->val = ctrl_val;
	} else {
		/* Not Implemented */
		return -EINVAL;
	}

	return err;
}

static int ar0521_reset_cam(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	int err;

	/* De-initialize the ISP*/
	err = cam_isp_deinit(client);
	if(err < 0) {
		dev_err(&client->dev, "Unable to De-Init ISP \n");
		return -EFAULT;
	}

	/* Initialize the ISP */
	err = cam_isp_init(client);
        if(err < 0) {
		dev_err(&client->dev, "Unable to Init ISP \n");
		return -EFAULT;
	}

	/* Configure the stream again */
	priv->force_config = true;
	err = cam_stream_config(client, priv->format_fourcc, priv->prev_index,
						priv->prev_frateindex);
	if (err < 0) {
		dev_err(&client->dev, "%s: Failed stream_config \n", __func__);
		return err;
	}

	return err;
}

static int ar0521_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct ar0521 *priv =
	    container_of(ctrl->handler, struct ar0521, ctrl_handler);
	struct i2c_client *client = priv->i2c_client;
        int err = 0;
#ifdef FRAMESYNC_ENABLE
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	int mode = 0;

        mode = s_data->mode;
#endif

	if (!priv || !priv->pdata)
		return -EINVAL;

	if (priv->power.state == SWITCH_OFF)
		return 0;

	if(ctrl->id == V4L2_CID_RESET_CAM) {
		err = ar0521_reset_cam(client);
		if(err < 0) {
			dev_err(&client->dev," %s (%d ) Reset cam failed \n", __func__, __LINE__);
		}
		return err;
	}

	if ((err =
	     cam_set_ctrl(client, ctrl->id, CTRL_STANDARD, ctrl->val)) < 0) {
		dev_err(&client->dev," %s (%d ) \n", __func__, __LINE__);
		return -EINVAL;
	}

#ifdef FRAMESYNC_ENABLE
	if(ctrl->id == V4L2_CID_FRAME_SYNC) {
		if(ctrl->val == 1){
			calibration_init(0);
			priv->last_sync_mode = 1;
		}else if(ctrl->val == 2 &&(  mode < MODE_2592x1944  ||
				( mode == MODE_2560x1440 &&  priv->mipi_lane_config == NUM_LANES_4 ))) {
			calibration_init(1);
			priv->last_sync_mode = 2;
		}
	}
#endif

	return err;
}

static int ar0521_try_add_ctrls(struct ar0521 *priv, int index,
				ISP_CTRL_INFO * cam_ctrl)
{
	struct i2c_client *client = priv->i2c_client;
	struct v4l2_ctrl_config custom_ctrl_config;
#ifdef FRAMESYNC_ENABLE
	enum tegra_chipid chip_id;
#endif
	if (!priv || !priv->pdata)
		return -EINVAL;

	priv->ctrl_handler.error = 0;

	if (!priv->framesync_enabled && cam_ctrl->ctrl_id == V4L2_CID_FRAME_SYNC)
		return 0;

#ifdef FRAMESYNC_ENABLE
	chip_id= tegra_get_chipid();
	if(chip_id == JETSON_TX2 || chip_id == JETSON_TX1){
		if((cam_ctrl->ctrl_id == V4L2_CID_FRAME_SYNC) &&
				priv->mipi_lane_config == NUM_LANES_2){
			cam_ctrl->ctrl_data.std.ctrl_max=1;
		}
	}
#endif

	/* Try Enumerating in standard controls */
	priv->ctrls[index] =
	    v4l2_ctrl_new_std(&priv->ctrl_handler,
			      &ar0521_ctrl_ops,
			      cam_ctrl->ctrl_id,
			      cam_ctrl->ctrl_data.std.ctrl_min,
			      cam_ctrl->ctrl_data.std.ctrl_max,
			      cam_ctrl->ctrl_data.std.ctrl_step,
			      cam_ctrl->ctrl_data.std.ctrl_def);
	if (priv->ctrls[index] != NULL) {
		debug_printk("%d. Initialized Control 0x%08x - %s \n",
			     index, cam_ctrl->ctrl_id,
			     priv->ctrls[index]->name);
		return 0;
	}

	if(cam_ctrl->ctrl_id == V4L2_CID_EXPOSURE_AUTO)
		goto custom;


	/* Try Enumerating in standard menu */
	priv->ctrl_handler.error = 0;
	priv->ctrls[index] =
	    v4l2_ctrl_new_std_menu(&priv->ctrl_handler,
				   &ar0521_ctrl_ops,
				   cam_ctrl->ctrl_id,
				   cam_ctrl->ctrl_data.std.ctrl_max,
				   0, cam_ctrl->ctrl_data.std.ctrl_def);
	if (priv->ctrls[index] != NULL) {
		debug_printk("%d. Initialized Control Menu 0x%08x - %s \n",
			     index, cam_ctrl->ctrl_id,
			     priv->ctrls[index]->name);
		return 0;
	}


custom:
	priv->ctrl_handler.error = 0;
	memset(&custom_ctrl_config, 0x0, sizeof(struct v4l2_ctrl_config));

	if (cam_get_ctrl_ui(client, cam_ctrl, index)!= ERRCODE_SUCCESS) {
		dev_err(&client->dev, "Error Enumerating Control 0x%08x !! \n",
			cam_ctrl->ctrl_id);
		return -EIO;
	}

	/* Fill in Values for Custom Ctrls */
	custom_ctrl_config.ops = &ar0521_ctrl_ops;
	custom_ctrl_config.id = cam_ctrl->ctrl_id;
	/* Do not change the name field for the control */
	custom_ctrl_config.name = cam_ctrl->ctrl_ui_data.ctrl_ui_info.ctrl_name;

	/* Sample Control Type and Flags */
	custom_ctrl_config.type = cam_ctrl->ctrl_ui_data.ctrl_ui_info.ctrl_ui_type;
	custom_ctrl_config.flags = cam_ctrl->ctrl_ui_data.ctrl_ui_info.ctrl_ui_flags;

	custom_ctrl_config.min = cam_ctrl->ctrl_data.std.ctrl_min;
	custom_ctrl_config.max = cam_ctrl->ctrl_data.std.ctrl_max;
	custom_ctrl_config.step = cam_ctrl->ctrl_data.std.ctrl_step;
	custom_ctrl_config.def = cam_ctrl->ctrl_data.std.ctrl_def;

	if (custom_ctrl_config.type == V4L2_CTRL_TYPE_MENU) {
		custom_ctrl_config.step = 0;
		custom_ctrl_config.type_ops = NULL;

		custom_ctrl_config.qmenu =
			(const char *const *)(cam_ctrl->ctrl_ui_data.ctrl_menu_info.menu);
	}

	priv->ctrls[index] =
	    v4l2_ctrl_new_custom(&priv->ctrl_handler,
				 &custom_ctrl_config, NULL);
	if (priv->ctrls[index] != NULL) {
		debug_printk("%d. Initialized Custom Ctrl 0x%08x - %s \n",
			     index, cam_ctrl->ctrl_id,
			     priv->ctrls[index]->name);
		return 0;
	}

	dev_err(&client->dev,
		"%d.  default: Failed to init 0x%08x ctrl Error - %d \n",
		index, cam_ctrl->ctrl_id, priv->ctrl_handler.error);
	return -EINVAL;
}

static int ar0521_ctrls_init(struct ar0521 *priv, ISP_CTRL_INFO *cam_ctrls)
{
	struct i2c_client *client = priv->i2c_client;
	int err = 0, i = 0;

	/* Array of Ctrls */

	/* Custom Ctrl */
	if (!priv || !priv->pdata)
		return -EINVAL;

	if (cam_list_ctrls(client, cam_ctrls, priv) < 0) {
		dev_err(&client->dev, "Failed to init ctrls\n");
		goto error;
	}

	v4l2_ctrl_handler_init(&priv->ctrl_handler, priv->num_ctrls+1);
	priv->subdev->ctrl_handler = &priv->ctrl_handler;
	for (i = 0; i < priv->num_ctrls; i++) {

		if (cam_ctrls[i].ctrl_type == CTRL_STANDARD) {
				ar0521_try_add_ctrls(priv, i,
						     &cam_ctrls[i]);
		} else {
			/* Not Implemented */
		}
	}

	return 0;

 error:
	v4l2_ctrl_handler_free(&priv->ctrl_handler);
	return err;
}

MODULE_DEVICE_TABLE(of, ar0521_of_match);

static struct camera_common_pdata *ar0521_parse_dt(struct i2c_client *client)
{
	struct device_node *node = client->dev.of_node;
	struct camera_common_pdata *board_priv_pdata;
	const struct of_device_id *match;
	int gpio;
	int err;

	if (!node)
		return NULL;

	match = of_match_device(ar0521_of_match, &client->dev);
	if (!match) {
		dev_err(&client->dev, "Failed to find matching dt id\n");
		return NULL;
	}

	board_priv_pdata =
	    devm_kzalloc(&client->dev, sizeof(*board_priv_pdata), GFP_KERNEL);
	if (!board_priv_pdata)
		return NULL;

	gpio = of_get_named_gpio(node, "pwdn-gpios", 0);
	if (gpio < 0) {
		dev_err(&client->dev, "pwdn gpios not in DT\n");
		goto error;
	}
	board_priv_pdata->pwdn_gpio = (unsigned int)gpio;

	gpio = of_get_named_gpio(node, "reset-gpios", 0);
	if (gpio < 0) {
		/* reset-gpio is not absoluctly needed */
		dev_dbg(&client->dev, "reset gpios not in DT\n");
		gpio = 0;
	}
	board_priv_pdata->reset_gpio = (unsigned int)gpio;

	board_priv_pdata->use_cam_gpio =
	    of_property_read_bool(node, "cam,use-cam-gpio");

	err =
	    of_property_read_string(node, "avdd-reg",
				    &board_priv_pdata->regulators.avdd);
	err |=
	    of_property_read_string(node, "iovdd-reg",
				    &board_priv_pdata->regulators.iovdd);
	if (err) {
		dev_dbg(&client->dev, "avdd, iovdd-reg not in DT, assume sensor powered independently\n");
	}

	board_priv_pdata->has_eeprom =
	    of_property_read_bool(node, "has-eeprom");

	return board_priv_pdata;

error:
	devm_kfree(&client->dev, board_priv_pdata);
	return NULL;
}

static int ar0521_open(struct v4l2_subdev *sd, struct v4l2_subdev_fh *fh)
{
	return 0;
}

static const struct v4l2_subdev_internal_ops ar0521_subdev_internal_ops = {
	.open = ar0521_open,
};

static const struct media_entity_operations ar0521_media_ops = {
	.link_validate = v4l2_subdev_link_validate,
};

static int ar0521_read(struct i2c_client *client, u8 * val, u32 count)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.buf = val,
	};

	msg.flags = I2C_M_RD;
	msg.len = count;
	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0)
		goto err;

	return 0;

 err:
	dev_err(&client->dev, "Failed reading register ret = %d!\n", ret);
	return ret;
}

static int ar0521_write(struct i2c_client *client, u8 * val, u32 count)
{
	int ret;
	struct i2c_msg msg = {
		.addr = client->addr,
		.flags = 0,
		.len = count,
		.buf = val,
	};

	ret = i2c_transfer(client->adapter, &msg, 1);
	if (ret < 0) {
		dev_err(&client->dev, "Failed writing register ret = %d!\n",
			ret);
		return ret;
	}

	return 0;
}

int cam_bload_ascii2hex(unsigned char ascii)
{
	if (ascii <= '9') {
		return (ascii - '0');
	} else if ((ascii >= 'a') && (ascii <= 'f')) {
		return (0xA + (ascii - 'a'));
	} else if ((ascii >= 'A') && (ascii <= 'F')) {
		return (0xA + (ascii - 'A'));
	}
	return -1;
}

static void toggle_gpio(unsigned int gpio, int val)
{
	if (gpio_cansleep(gpio)){
		gpio_direction_output(gpio,val);
		gpio_set_value_cansleep(gpio, val);
	} else{
		gpio_direction_output(gpio,val);
		gpio_set_value(gpio, val);
	}
}

unsigned char errorcheck(char *data, unsigned int len)
{
	unsigned int i = 0;
	unsigned char crc = 0x00;

	for (i = 0; i < len; i++) {
		crc ^= data[i];
	}

	return crc;
}

static int cam_stream_config(struct i2c_client *client, uint32_t format,
			     int mode, int frate_index)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;

	uint32_t payload_len = 0;

	uint16_t cmd_status = 0, index = 0xFFFF;
	uint8_t retcode = 0, cmd_id = 0;
	int loop = 0, ret = 0, err = 0, retry = 1000;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	cmd_id = CMD_ID_STREAM_CONFIG;
	if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) < 0) {
		dev_err(&client->dev," %s(%d) Error \n", __func__, __LINE__);
		ret = -EIO;
		goto exit;
	}

#if 0 /* Disable debug prints */
	debug_printk
	    (" %s(%d) ISP Status = 0x%04x , Ret code = 0x%02x \n",
	     __func__, __LINE__, cmd_status, retcode);
#endif

	if ((cmd_status != CAM_CMD_STATUS_SUCCESS) ||
	    (retcode != ERRCODE_SUCCESS)) {
		debug_printk
		    (" ISP is Unintialized or Busy STATUS = 0x%04x Errcode = 0x%02x !! \n",
		     cmd_status, retcode);
		ret = -EBUSY;
		goto exit;
	}

	for (loop = 0;(&priv->streamdb[loop]) != NULL; loop++) {
		if (priv->streamdb[loop] == mode) {
			index = loop + frate_index;
			break;
		}
	}

	debug_printk(" Index = 0x%04x , format = 0x%08x, width = %hu,"
		     " height = %hu, frate num = %hu \n", index, format,
		     priv->cam_frmfmt[mode].size.width,
		     priv->cam_frmfmt[mode].size.height,
		     priv->cam_frmfmt[mode].framerates[frate_index]);

	if (index == 0xFFFF) {
		ret = -EINVAL;
		goto exit;
	}

	if(priv->prev_index == index && priv->force_config != true) {
		debug_printk("Skipping Previous mode set ... \n");
		ret = 0;
		goto exit;
	}

issue_cmd:
	/* First Txn Payload length = 0 */
	payload_len = 14;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_STREAM_CONFIG;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_STREAM_CONFIG;
	mc_data[2] = index >> 8;
	mc_data[3] = index & 0xFF;

	/* Format Fourcc - currently only UYVY */
	mc_data[4] = format >> 24;
	mc_data[5] = format >> 16;
	mc_data[6] = format >> 8;
	mc_data[7] = format & 0xFF;

	/* width */
	mc_data[8] = priv->cam_frmfmt[mode].size.width >> 8;
	mc_data[9] = priv->cam_frmfmt[mode].size.width & 0xFF;

	/* height */
	mc_data[10] = priv->cam_frmfmt[mode].size.height >> 8;
	mc_data[11] = priv->cam_frmfmt[mode].size.height & 0xFF;

	/* frame rate num */
	mc_data[12] = priv->cam_frmfmt[mode].framerates[frate_index] >> 8;
	mc_data[13] = priv->cam_frmfmt[mode].framerates[frate_index] & 0xFF;

	/* frame rate denom */
	mc_data[14] = 0x00;
	mc_data[15] = 0x01;

	mc_data[16] = errorcheck(&mc_data[2], 14);
	err = ar0521_write(client, mc_data, 17);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	while (--retry > 0) {
		cmd_id = CMD_ID_STREAM_CONFIG;
		if (cam_get_cmd_status
		    (client, &cmd_id, &cmd_status, &retcode) < 0) {
			dev_err(&client->dev,
				" %s(%d) CAM GET CMD Status Error : loop : %d \n",
				__func__, __LINE__, loop);
			ret = -EIO;
			goto exit;
		}

		if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
		    (retcode == ERRCODE_SUCCESS)) {
			ret = 0;
			goto exit;
		}

		if(retcode == ERRCODE_AGAIN) {
			/* Issue Command Again if Set */
			retry = 1000;
			goto issue_cmd;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			dev_err(&client->dev,
				"(%s) %d Error STATUS = 0x%04x RET = 0x%02x\n",
				__func__, __LINE__, cmd_status, retcode);
			ret = -EIO;
			goto exit;
		}

		/* Delay after retry */
		mdelay(10);
	}

	dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
			__LINE__, err);
	ret = -ETIMEDOUT;

exit:
	if(!ret) {
		priv->prev_index = index;
		priv->prev_frateindex = frate_index;
	        priv->force_config = false;	
	}

	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

#if ISP_PWDN_WKUP
static int cam_isp_power_down(struct i2c_client *client)
{
	uint32_t payload_len = 0;

	uint16_t cmd_status = 0;
	uint8_t retcode = 0, cmd_id = 0;
	int retry = 1000, err = 0;

	/*lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* First Txn Payload length = 0 */
	payload_len = 0;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_ISP_PDOWN;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_ISP_PDOWN;
	err = ar0521_write(client, mc_data, 2);
	if (err != 0) {
		dev_err(&client->dev, " %s(%d) Error - %d \n",
			__func__, __LINE__, err);
		goto exit;
	}

	while (--retry > 0) {
		msleep(20);
		cmd_id = CMD_ID_ISP_PDOWN;
		if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) <
		    0) {
			dev_err(&client->dev, " %s(%d) Get Status Error \n",
				__func__, __LINE__);
			err = -EINVAL;
			goto exit;
		}

		if ((cmd_status == CAM_CMD_STATUS_ISP_PWDN) &&
		    ((retcode == ERRCODE_SUCCESS) || retcode == ERRCODE_ALREADY)) {
			err = 0;
			goto exit;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			dev_err(&client->dev,
				"(%s) %d Error STATUS = 0x%04x RET = 0x%02x\n",
				__func__, __LINE__, cmd_status,
				retcode);
			err = -EIO;
			goto exit;
		}

	}
	err = -ETIMEDOUT;
 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);
	return err;
}

static int cam_isp_power_wakeup(struct i2c_client *client)
{
	uint32_t payload_len = 0;

	uint16_t cmd_status = 0;
	uint8_t retcode = 0, cmd_id = 0;
	int retry = 1000, err = 0;

	/*lock semaphore */
	mutex_lock(&cam_i2c_mutex);
	/* First Txn Payload length = 0 */
	payload_len = 0;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_ISP_PUP;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_ISP_PUP;
	err = ar0521_write(client, mc_data, 2);
	if (err != 0) {
		dev_err(&client->dev, " %s(%d) Error - %d \n",
			__func__, __LINE__, err);
		goto exit;
	}

	while (--retry > 0) {
		msleep(20);
		cmd_id = CMD_ID_ISP_PUP;
		if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) <
		    0) {
			dev_err(&client->dev, " %s(%d) Error \n",
				__func__, __LINE__);
			err = -EIO;
			goto exit;
		}

		if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
		    ((retcode == ERRCODE_SUCCESS) || retcode == ERRCODE_ALREADY)) {
			err = 0;
			goto exit;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			dev_err(&client->dev,
				"(%s) %d Error STATUS = 0x%04x RET = 0x%02x\n",
				__func__, __LINE__, cmd_status, retcode);
			err = -EIO;
			goto exit;
		}

	}

	err = -ETIMEDOUT;
 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);
	return err;
}
#endif

static int cam_get_ctrl(struct i2c_client *client, uint32_t arg_ctrl_id,
			uint8_t * ctrl_type, int32_t * curr_val)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;

	uint32_t payload_len = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0;
	uint16_t index = 0xFFFF;
	int loop = 0, ret = 0, err = 0;

	uint32_t ctrl_id = 0;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	ctrl_id = arg_ctrl_id;

	/* Read the Ctrl Value from Micro controller */

	for (loop = 0; loop < priv->num_ctrls; loop++) {
		if (priv->ctrldb[loop] == ctrl_id) {
			index = loop;
			break;
		}
	}

	if (index == 0xFFFF) {
		ret = -EINVAL;
		goto exit;
	}

	/* First Txn Payload length = 2 */
	payload_len = 2;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_CTRL;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_CTRL;
	mc_data[2] = index >> 8;
	mc_data[3] = index & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);
	err = ar0521_write(client, mc_data, 5);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[4];
	calc_crc = errorcheck(&mc_ret_data[2], 2);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -1;
		goto exit;
	}

	if (((mc_ret_data[2] << 8) | mc_ret_data[3]) == 0) {
		ret = -EIO;
		goto exit;
	}

	errcode = mc_ret_data[5];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EIO;
		goto exit;
	}

	payload_len =
	    ((mc_ret_data[2] << 8) | mc_ret_data[3]) + HEADER_FOOTER_SIZE;
	memset(mc_ret_data, 0x00, payload_len);
	err = ar0521_read(client, mc_ret_data, payload_len);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[payload_len - 2];
	calc_crc =
	    errorcheck(&mc_ret_data[2], payload_len - HEADER_FOOTER_SIZE);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -EINVAL;
		goto exit;
	}

	/* Verify Errcode */
	errcode = mc_ret_data[payload_len - 1];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EINVAL;
		goto exit;
	}

	/* Ctrl type starts from index 6 */

	*ctrl_type = mc_ret_data[6];

	switch (*ctrl_type) {
	case CTRL_STANDARD:
		*curr_val =
		    mc_ret_data[7] << 24 | mc_ret_data[8] << 16 | mc_ret_data[9]
		    << 8 | mc_ret_data[10];
		break;

	case CTRL_EXTENDED:
		/* Not Implemented */
		break;
	}

 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

static int cam_set_ctrl(struct i2c_client *client, uint32_t arg_ctrl_id,
			uint8_t ctrl_type, int32_t curr_val)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	uint32_t payload_len = 0;

	uint16_t cmd_status = 0, index = 0xFFFF;
	uint8_t retcode = 0, cmd_id = 0;
	int loop = 0, ret = 0, err = 0;
	uint32_t ctrl_id = 0;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	ctrl_id = arg_ctrl_id;

	/* call ISP Ctrl config command */

	for (loop = 0; loop < priv->num_ctrls; loop++) {
		if (priv->ctrldb[loop] == ctrl_id) {
			index = loop;
			break;
		}
	}

	if (index == 0xFFFF) {
		ret = -EINVAL;
		goto exit;
	}

	/* First Txn Payload length = 0 */
	payload_len = 11;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_SET_CTRL;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	/* Second Txn */
	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_SET_CTRL;

	/* Index */
	mc_data[2] = index >> 8;
	mc_data[3] = index & 0xFF;

	/* Control ID */
	mc_data[4] = ctrl_id >> 24;
	mc_data[5] = ctrl_id >> 16;
	mc_data[6] = ctrl_id >> 8;
	mc_data[7] = ctrl_id & 0xFF;

	/* Ctrl Type */
	mc_data[8] = ctrl_type;

	/* Ctrl Value */
	mc_data[9] = curr_val >> 24;
	mc_data[10] = curr_val >> 16;
	mc_data[11] = curr_val >> 8;
	mc_data[12] = curr_val & 0xFF;

	/* CRC */
	mc_data[13] = errorcheck(&mc_data[2], 11);

	err = ar0521_write(client, mc_data, 14);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	while (1) {
		cmd_id = CMD_ID_SET_CTRL;
		if (cam_get_cmd_status
		    (client, &cmd_id, &cmd_status, &retcode) < 0) {
			dev_err(&client->dev," %s(%d) Error \n",
			       __func__, __LINE__);
			ret = -EINVAL;
			goto exit;
		}

		if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
		    (retcode == ERRCODE_SUCCESS)) {
			ret = 0;
			goto exit;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			pr_err
			    ("(%s) %d ISP Error STATUS = 0x%04x RET = 0x%02x\n",
			     __func__, __LINE__, cmd_status, retcode);
			ret = -EIO;
			goto exit;
		}
	}

 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

static int cam_list_fmts(struct i2c_client *client, ISP_STREAM_INFO *stream_info, int *frm_fmt_size, struct ar0521 *priv)
{
	uint32_t payload_len = 0, err = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0, skip = 0;
	uint16_t index = 0, mode = 0;

	int loop = 0, num_frates = 0, ret = 0;

	/* Stream Info Variables */

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* List all formats from CAM and append to cam_ar0521_frmfmt array */

	for (index = 0;; index++) {
		/* First Txn Payload length = 0 */
		payload_len = 2;

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_GET_STREAM_INFO;
		mc_data[2] = payload_len >> 8;
		mc_data[3] = payload_len & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);

		err = ar0521_write(client, mc_data, TX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			ret = -EIO;
			goto exit;
		}

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_GET_STREAM_INFO;
		mc_data[2] = index >> 8;
		mc_data[3] = index & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);
		err = ar0521_write(client, mc_data, 5);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			ret = -EIO;
			goto exit;
		}

		err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			ret = -EIO;
			goto exit;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[4];
		calc_crc = errorcheck(&mc_ret_data[2], 2);
		if (orig_crc != calc_crc) {
			pr_err
			    (" %s(%d) CRC 0x%02x != 0x%02x \n",
			     __func__, __LINE__, orig_crc, calc_crc);
			ret = -EINVAL;
			goto exit;
		}

		if (((mc_ret_data[2] << 8) | mc_ret_data[3]) == 0) {
			if(stream_info == NULL) {
				*frm_fmt_size = index;
			} else {
				*frm_fmt_size = mode;
			}
			break;
		}

		payload_len =
		    ((mc_ret_data[2] << 8) | mc_ret_data[3]) +
		    HEADER_FOOTER_SIZE;
		errcode = mc_ret_data[5];
		if (errcode != ERRCODE_SUCCESS) {
			pr_err
			    (" %s(%d) Errcode - 0x%02x \n",
			     __func__, __LINE__, errcode);
			ret = -EIO;
			goto exit;
		}

		memset(mc_ret_data, 0x00, payload_len);
		err = ar0521_read(client, mc_ret_data, payload_len);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			ret = -1;
			goto exit;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[payload_len - 2];
		calc_crc =
		    errorcheck(&mc_ret_data[2],
				 payload_len - HEADER_FOOTER_SIZE);
		if (orig_crc != calc_crc) {
			pr_err
			    (" %s(%d) CRC 0x%02x != 0x%02x \n",
			     __func__, __LINE__, orig_crc, calc_crc);
			ret = -EINVAL;
			goto exit;
		}

		/* Verify Errcode */
		errcode = mc_ret_data[payload_len - 1];
		if (errcode != ERRCODE_SUCCESS) {
			pr_err
			    (" %s(%d) Errcode - 0x%02x \n",
			     __func__, __LINE__, errcode);
			ret = -EIO;
			goto exit;
		}
		if(stream_info != NULL) {
		/* check if any other format than UYVY is queried - do not append in array */
		stream_info->fmt_fourcc =
		    mc_ret_data[2] << 24 | mc_ret_data[3] << 16 | mc_ret_data[4]
		    << 8 | mc_ret_data[5];
		stream_info->width = mc_ret_data[6] << 8 | mc_ret_data[7];
		stream_info->height = mc_ret_data[8] << 8 | mc_ret_data[9];
		stream_info->frame_rate_type = mc_ret_data[10];

		switch (stream_info->frame_rate_type) {
		case FRAME_RATE_DISCRETE:
			stream_info->frame_rate.disc.frame_rate_num =
			    mc_ret_data[11] << 8 | mc_ret_data[12];

			stream_info->frame_rate.disc.frame_rate_denom =
			    mc_ret_data[13] << 8 | mc_ret_data[14];

			break;

		case FRAME_RATE_CONTINOUS:
			debug_printk
			    (" The Stream format at index 0x%04x has FRAME_RATE_CONTINOUS,"
			     "which is unsupported !! \n", index);

#if 0
			stream_info.frame_rate.cont.frame_rate_min_num =
			    mc_ret_data[11] << 8 | mc_ret_data[12];
			stream_info.frame_rate.cont.frame_rate_min_denom =
			    mc_ret_data[13] << 8 | mc_ret_data[14];

			stream_info.frame_rate.cont.frame_rate_max_num =
			    mc_ret_data[15] << 8 | mc_ret_data[16];
			stream_info.frame_rate.cont.frame_rate_max_denom =
			    mc_ret_data[17] << 8 | mc_ret_data[18];

			stream_info.frame_rate.cont.frame_rate_step_num =
			    mc_ret_data[19] << 8 | mc_ret_data[20];
			stream_info.frame_rate.cont.frame_rate_step_denom =
			    mc_ret_data[21] << 8 | mc_ret_data[22];
			break;
#endif
			continue;
		}

		switch (stream_info->fmt_fourcc) {
		case V4L2_PIX_FMT_UYVY:
			/* ar0521_codes is already populated with V4L2_MBUS_FMT_UYVY8_1X16 */
			/* check if width and height are already in array - update frame rate only */
			for (loop = 0; loop < (mode); loop++) {
				if ((priv->cam_frmfmt[loop].size.width ==
				     stream_info->width)
				    && (priv->cam_frmfmt[loop].size.height ==
					stream_info->height)) {

					num_frates =
					    priv->cam_frmfmt
					    [loop].num_framerates;
					*((int *)(priv->cam_frmfmt[loop].framerates) + num_frates)
					    = (int)(stream_info->frame_rate.
						    disc.frame_rate_num /
						    stream_info->frame_rate.
						    disc.frame_rate_denom);

					priv->cam_frmfmt
					    [loop].num_framerates++;

					priv->streamdb[index] = loop;
					skip = 1;
					break;
				}
			}

			if (skip) {
				skip = 0;
				continue;
			}

			/* Add Width, Height, Frame Rate array, Mode into cam_ar0521_frmfmt array */
			priv->cam_frmfmt[mode].size.width = stream_info->width;
			priv->cam_frmfmt[mode].size.height =
			    stream_info->height;
			num_frates = priv->cam_frmfmt[mode].num_framerates;

			*((int *)(priv->cam_frmfmt[mode].framerates) + num_frates) =
			    (int)(stream_info->frame_rate.disc.frame_rate_num /
				  stream_info->frame_rate.disc.frame_rate_denom);

			priv->cam_frmfmt[mode].num_framerates++;

			priv->cam_frmfmt[mode].mode = mode;
			priv->streamdb[index] = mode;
			mode++;
			break;

		default:
			debug_printk
			    (" The Stream format at index 0x%04x has format 0x%08x ,"
			     "which is unsupported !! \n", index,
			     stream_info->fmt_fourcc);
		}
		}
	}

 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

static int cam_get_ctrl_ui(struct i2c_client *client,
			   ISP_CTRL_INFO * cam_ui_info, int index)
{
	uint32_t payload_len = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0;
	int ret = 0, i = 0, err = 0;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* First Txn Payload length = 0 */
	payload_len = 2;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_CTRL_UI_INFO;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_CTRL_UI_INFO;
	mc_data[2] = index >> 8;
	mc_data[3] = index & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);
	err = ar0521_write(client, mc_data, 5);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[4];
	calc_crc = errorcheck(&mc_ret_data[2], 2);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -EINVAL;
		goto exit;
	}

	payload_len =
	    ((mc_ret_data[2] << 8) | mc_ret_data[3]) + HEADER_FOOTER_SIZE;
	errcode = mc_ret_data[5];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EINVAL;
		goto exit;
	}

	memset(mc_ret_data, 0x00, payload_len);
	err = ar0521_read(client, mc_ret_data, payload_len);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[payload_len - 2];
	calc_crc =
	    errorcheck(&mc_ret_data[2], payload_len - HEADER_FOOTER_SIZE);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -EINVAL;
		goto exit;
	}

	/* Verify Errcode */
	errcode = mc_ret_data[payload_len - 1];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EIO;
		goto exit;
	}

	strncpy((char *)cam_ui_info->ctrl_ui_data.ctrl_ui_info.ctrl_name, &mc_ret_data[2],MAX_CTRL_UI_STRING_LEN);

	cam_ui_info->ctrl_ui_data.ctrl_ui_info.ctrl_ui_type = mc_ret_data[34];
	cam_ui_info->ctrl_ui_data.ctrl_ui_info.ctrl_ui_flags = mc_ret_data[35] << 8 |
	    mc_ret_data[36];

	if (cam_ui_info->ctrl_ui_data.ctrl_ui_info.ctrl_ui_type == V4L2_CTRL_TYPE_MENU) {
		cam_ui_info->ctrl_ui_data.ctrl_menu_info.num_menu_elem = mc_ret_data[37];

		cam_ui_info->ctrl_ui_data.ctrl_menu_info.menu =
		    devm_kzalloc(&client->dev,((cam_ui_info->ctrl_ui_data.ctrl_menu_info.num_menu_elem +1) * sizeof(char *)), GFP_KERNEL);
		for (i = 0; i < cam_ui_info->ctrl_ui_data.ctrl_menu_info.num_menu_elem; i++) {
			cam_ui_info->ctrl_ui_data.ctrl_menu_info.menu[i] =
			    devm_kzalloc(&client->dev,MAX_CTRL_UI_STRING_LEN, GFP_KERNEL);
			strncpy((char *)cam_ui_info->ctrl_ui_data.ctrl_menu_info.menu[i],
				&mc_ret_data[38 +(i *MAX_CTRL_UI_STRING_LEN)], MAX_CTRL_UI_STRING_LEN);

			debug_printk(" Menu Element %d : %s \n",
				     i, cam_ui_info->ctrl_ui_data.ctrl_menu_info.menu[i]);
		}

		cam_ui_info->ctrl_ui_data.ctrl_menu_info.menu[i] = NULL;
	}

 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;

}

static int cam_lane_configuration(struct i2c_client *client, struct ar0521 *priv)
{
	int ret = 0, err;
	uint16_t payload_data;
	unsigned char mc_data[10];
	uint32_t payload_len = 0;
	uint16_t cmd_status = 0;
	uint8_t retcode = 0, cmd_id = 0;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	payload_len = 2;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_LANE_CONFIG;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	/* Second Txn */
	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_LANE_CONFIG;

	/* Lane Configuration */
	payload_data = priv->mipi_lane_config == 4 ? NUM_LANES_4 : NUM_LANES_2;
	mc_data[2] = payload_data >> 8;
	mc_data[3] = payload_data & 0xff;

	/* CRC */
	mc_data[4] = errorcheck(&mc_data[2], payload_len);
	err = ar0521_write(client, mc_data, payload_len+3);

	if (err != 0) {
		dev_err(&client->dev," %s(%d) CAM Set Ctrl Error - %d \n", __func__,
				__LINE__, err);
		ret = -1;
		goto exit;
	}

	while (1) {
		yield();
		cmd_id = CMD_ID_LANE_CONFIG;
		if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) <
		    0) {
			dev_err(&client->dev," %s(%d) CAM Get CMD Status Error \n", __func__,
					__LINE__);
			ret = -1;
			goto exit;
		}


		if ((cmd_status == CAM_CMD_STATUS_SUCCESS)) {
			ret = 0;
			goto exit;
		}

		if ((cmd_status == CAM_CMD_STATUS_ISP_UNINIT) &&
		    (retcode == ERRCODE_SUCCESS)) {
			ret = 0;
			goto exit;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_ISP_UNINIT))) {
			dev_err(&client->dev,
					"(%s) %d CAM Get CMD Error STATUS = 0x%04x RET = 0x%02x\n",
					__func__, __LINE__, cmd_status, retcode);
			ret = -1;
			goto exit;
		}
	}

exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);
	return ret;
}

static int cam_list_ctrls(struct i2c_client *client,
			  ISP_CTRL_INFO * cam_ctrl, struct ar0521 *priv)
{
	uint32_t payload_len = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0;
	uint16_t index = 0;
	int ret = 0, err = 0,retry=30;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* Array of Ctrl Info */
	while (retry-- > 0) {
		/* First Txn Payload length = 0 */
		payload_len = 2;

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_GET_CTRL_INFO;
		mc_data[2] = payload_len >> 8;
		mc_data[3] = payload_len & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);

		err = ar0521_write(client, mc_data, TX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
				__func__, __LINE__, err);
			continue;
		}
		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_GET_CTRL_INFO;
		mc_data[2] = index >> 8;
		mc_data[3] = index & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);
		err = ar0521_write(client, mc_data, 5);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			continue;
		}

		err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			continue;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[4];
		calc_crc = errorcheck(&mc_ret_data[2], 2);
		if (orig_crc != calc_crc) {
			dev_err(&client->dev,
			    " %s(%d) CRC 0x%02x != 0x%02x \n",
			     __func__, __LINE__, orig_crc, calc_crc);
			continue;
		}

		if (((mc_ret_data[2] << 8) | mc_ret_data[3]) == 0) {
			priv->num_ctrls = index;
			break;
		}

		payload_len =
		    ((mc_ret_data[2] << 8) | mc_ret_data[3]) +
		    HEADER_FOOTER_SIZE;
		errcode = mc_ret_data[5];
		if (errcode != ERRCODE_SUCCESS) {
			dev_err(&client->dev,
			    " %s(%d) Errcode - 0x%02x \n",
			     __func__, __LINE__, errcode);
			continue;
		}

		memset(mc_ret_data, 0x00, payload_len);
		err = ar0521_read(client, mc_ret_data, payload_len);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) Error - %d \n",
			       __func__, __LINE__, err);
			continue;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[payload_len - 2];
		calc_crc =
		    errorcheck(&mc_ret_data[2],
				 payload_len - HEADER_FOOTER_SIZE);
		if (orig_crc != calc_crc) {
			dev_err(&client->dev,
			    " %s(%d) CRC 0x%02x != 0x%02x \n",
			     __func__, __LINE__, orig_crc, calc_crc);
			continue;
		}

		/* Verify Errcode */
		errcode = mc_ret_data[payload_len - 1];
		if (errcode != ERRCODE_SUCCESS) {
			dev_err(&client->dev,
			    " %s(%d) Errcode - 0x%02x \n",
			     __func__, __LINE__, errcode);
			continue;
		}

		if(cam_ctrl != NULL) {

			/* append ctrl info in array */
			cam_ctrl[index].ctrl_id =
				mc_ret_data[2] << 24 | mc_ret_data[3] << 16 | mc_ret_data[4]
				<< 8 | mc_ret_data[5];
			cam_ctrl[index].ctrl_type = mc_ret_data[6];

			switch (cam_ctrl[index].ctrl_type) {
				case CTRL_STANDARD:
					cam_ctrl[index].ctrl_data.std.ctrl_min =
						mc_ret_data[7] << 24 | mc_ret_data[8] << 16
						| mc_ret_data[9] << 8 | mc_ret_data[10];

					cam_ctrl[index].ctrl_data.std.ctrl_max =
						mc_ret_data[11] << 24 | mc_ret_data[12] <<
						16 | mc_ret_data[13]
						<< 8 | mc_ret_data[14];

					cam_ctrl[index].ctrl_data.std.ctrl_def =
						mc_ret_data[15] << 24 | mc_ret_data[16] <<
						16 | mc_ret_data[17]
						<< 8 | mc_ret_data[18];

					cam_ctrl[index].ctrl_data.std.ctrl_step =
						mc_ret_data[19] << 24 | mc_ret_data[20] <<
						16 | mc_ret_data[21]
						<< 8 | mc_ret_data[22];
					break;

				case CTRL_EXTENDED:
					/* Not Implemented */
					break;
			}

			priv->ctrldb[index] = cam_ctrl[index].ctrl_id;
		}
		index++;
	}

	if(retry == 0) {
		ret = -EIO;
		goto exit;
	}
 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;

}

static int cam_get_fw_version(struct i2c_client *client, unsigned char *fw_version, unsigned char *bin_fw_version)
{
	uint32_t payload_len = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0;
	int ret = 0, err = 0, loop, i=0, retry = 5;
	unsigned long bin_fw_pos = ARRAY_SIZE(g_cam_fw_buf)-VERSION_FILE_OFFSET;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* Get Bin Firmware version*/
	for(loop = bin_fw_pos; loop < (bin_fw_pos+64); loop=loop+2) {
		*(bin_fw_version+i) = (cam_bload_ascii2hex(g_cam_fw_buf[loop]) << 4 |
				cam_bload_ascii2hex(g_cam_fw_buf[loop+1]));
		i++;
	}

	while (retry-- > 0) {
		/* Query firmware version from CAM */
		payload_len = 0;

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_VERSION;
		mc_data[2] = payload_len >> 8;
		mc_data[3] = payload_len & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);

		err = ar0521_write(client, mc_data, TX_LEN_PKT);

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_VERSION;
		err = ar0521_write(client, mc_data, 2);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) CAM CMD ID Write PKT fw Version Error - %d \n", __func__,
					__LINE__, ret);
			ret = -EIO;
			continue;
		}

		err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) CAM CMD ID Read PKT fw Version Error - %d \n", __func__,
					__LINE__, ret);
			ret = -EIO;
			continue;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[4];
		calc_crc = errorcheck(&mc_ret_data[2], 2);
		if (orig_crc != calc_crc) {
			dev_err(&client->dev," %s(%d) CAM CMD ID fw Version Error CRC 0x%02x != 0x%02x \n",
					__func__, __LINE__, orig_crc, calc_crc);
			ret = -EINVAL;
			continue;
		}

		errcode = mc_ret_data[5];
		if (errcode != ERRCODE_SUCCESS) {
			dev_err(&client->dev," %s(%d) CAM CMD ID fw Errcode - 0x%02x \n", __func__,
					__LINE__, errcode);
			ret = -EIO;
			continue;
		}

		/* Read the actual version from CAM*/
		payload_len =
			((mc_ret_data[2] << 8) | mc_ret_data[3]) + HEADER_FOOTER_SIZE;
		memset(mc_ret_data, 0x00, payload_len);
		err = ar0521_read(client, mc_ret_data, payload_len);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) CAM fw CMD ID Read Version Error - %d \n", __func__,
					__LINE__, ret);
			ret = -EIO;
			continue;
		}

		/* Verify CRC */
		orig_crc = mc_ret_data[payload_len - 2];
		calc_crc = errorcheck(&mc_ret_data[2], 32);
		if (orig_crc != calc_crc) {
			dev_err(&client->dev," %s(%d) CAM fw  CMD ID Version CRC ERROR 0x%02x != 0x%02x \n",
					__func__, __LINE__, orig_crc, calc_crc);
			ret = -EINVAL;
			continue;
		}

		/* Verify Errcode */
		errcode = mc_ret_data[payload_len - 1];
		if (errcode != ERRCODE_SUCCESS) {
			dev_err(&client->dev," %s(%d) CAM fw CMD ID Read Payload Error - 0x%02x \n", __func__,
					__LINE__, errcode);
			ret = -EIO;
			continue;
		}
		if(ret == ERRCODE_SUCCESS)
			break;
	}
	if (retry == 0 && ret != ERRCODE_SUCCESS) {
		goto exit;
	}

	for (loop = 0 ; loop < VERSION_SIZE ; loop++ )
		*(fw_version+loop) = mc_ret_data[2+loop];

	/* Check for forced/always update field in the bin firmware version*/
	if(bin_fw_version[17] == '1') {
		dev_err(&client->dev, "Forced Update Enabled - Firmware Version - (%.8s - g%.7s) \n",
				&fw_version[2], &fw_version[18]);
		ret = 2;
		goto exit;
	}

	for(i = 0; i < VERSION_SIZE; i++) {
		if(bin_fw_version[i] != fw_version[i]) {
			debug_printk("Previous Firmware Version - (%.8s-g%.7s)\n",
					&fw_version[2], &fw_version[18]);
			debug_printk("Current Firmware Version - (%.8s-g%.7s)\n",
					&bin_fw_version[2], &bin_fw_version[18]);
			ret = 1;
			goto exit;
		}
	}
exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

static int cam_get_sensor_id(struct i2c_client *client, uint16_t * sensor_id)
{
	uint32_t payload_len = 0;
	uint8_t errcode = ERRCODE_SUCCESS, orig_crc = 0, calc_crc = 0;

	int ret = 0, err = 0;

	/* lock semaphore */
	mutex_lock(&cam_i2c_mutex);

	/* Read the version info. from Micro controller */

	/* First Txn Payload length = 0 */
	payload_len = 0;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_SENSOR_ID;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_SENSOR_ID;
	err = ar0521_write(client, mc_data, 2);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	err = ar0521_read(client, mc_ret_data, RX_LEN_PKT);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[4];
	calc_crc = errorcheck(&mc_ret_data[2], 2);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -EINVAL;
		goto exit;
	}

	errcode = mc_ret_data[5];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EIO;
		goto exit;
	}

	payload_len =
	    ((mc_ret_data[2] << 8) | mc_ret_data[3]) + HEADER_FOOTER_SIZE;

	memset(mc_ret_data, 0x00, payload_len);
	err = ar0521_read(client, mc_ret_data, payload_len);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		ret = -EIO;
		goto exit;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[payload_len - 2];
	calc_crc = errorcheck(&mc_ret_data[2], 2);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		ret = -EINVAL;
		goto exit;
	}

	/* Verify Errcode */
	errcode = mc_ret_data[payload_len - 1];
	if (errcode != ERRCODE_SUCCESS) {
		dev_err(&client->dev," %s(%d) Errcode - 0x%02x \n",
		       __func__, __LINE__, errcode);
		ret = -EIO;
		goto exit;
	}

	*sensor_id = mc_ret_data[2] << 8 | mc_ret_data[3];

 exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);

	return ret;
}

static int cam_get_cmd_status(struct i2c_client *client,
			      uint8_t * cmd_id, uint16_t * cmd_status,
			      uint8_t * ret_code)
{
	uint32_t payload_len = 0;
	uint8_t orig_crc = 0, calc_crc = 0;
	int err = 0;

	/* No Semaphore in Get command Status */

	/* First Txn Payload length = 0 */
	payload_len = 1;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_STATUS;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_GET_STATUS;
	mc_data[2] = *cmd_id;
	err = ar0521_write(client, mc_data, 3);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		return -EIO;
	}

	payload_len = CMD_STATUS_MSG_LEN;
	memset(mc_ret_data, 0x00, payload_len);
	err = ar0521_read(client, mc_ret_data, payload_len);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		return -EIO;
	}

	/* Verify CRC */
	orig_crc = mc_ret_data[payload_len - 2];
	calc_crc = errorcheck(&mc_ret_data[2], 3);
	if (orig_crc != calc_crc) {
		dev_err(&client->dev," %s(%d) CRC 0x%02x != 0x%02x \n",
		       __func__, __LINE__, orig_crc, calc_crc);
		return -EINVAL;
	}

	*cmd_id = mc_ret_data[2];
	*cmd_status = mc_ret_data[3] << 8 | mc_ret_data[4];
	*ret_code = mc_ret_data[payload_len - 1];

	return 0;
}

#if 0
static int cam_stream_on(struct i2c_client *client)
{
        unsigned char mc_data[100];
        uint32_t payload_len = 0;

        uint16_t cmd_status = 0;
        uint8_t retcode = 0, cmd_id = 0;
	int retry = 5,status_retry=1000, err = 0;

	/*lock semaphore*/
        mutex_lock(&cam_i2c_mutex);

	while(retry-- < 0) {
		/* First Txn Payload length = 0 */
		payload_len = 0;

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_STREAM_ON;
		mc_data[2] = payload_len >> 8;
		mc_data[3] = payload_len & 0xFF;
		mc_data[4] = errorcheck(&mc_data[2], 2);

		err= ar0521_write(client, mc_data, TX_LEN_PKT);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) CAM Stream On Write Error - %d \n", __func__,
					__LINE__, err);
			continue;
		}

		mc_data[0] = CMD_SIGNATURE;
		mc_data[1] = CMD_ID_STREAM_ON;
		err = ar0521_write(client, mc_data, 2);
		if (err != 0) {
			dev_err(&client->dev," %s(%d) CAM Stream On Write Error - %d \n", __func__,
					__LINE__, err);
			continue;
		}

		while (status_retry-- > 0) {
			/* Some Sleep for init to process */
			yield();

			cmd_id = CMD_ID_STREAM_ON;
			if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) <
					0) {
				dev_err(&client->dev," %s(%d) CAM Get CMD Stream On Error \n", __func__,
						__LINE__);
				err = -1;
				goto exit;
			}

			if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
					(retcode == ERRCODE_SUCCESS)) {
				debug_printk(" %s %d CAM Stream On Success !! \n", __func__, __LINE__);
				printk("Func %s retry %d\n",__func__, retry);
				err = 0;
				goto exit;
			}

			if ((retcode != ERRCODE_BUSY) &&
					((cmd_status != CAM_CMD_STATUS_PENDING))) {
				dev_err(&client->dev,
						"(%s) %d CAM Get CMD Stream On Error STATUS = 0x%04x RET = 0x%02x\n",
						__func__, __LINE__, cmd_status, retcode);
				err = -1;
				goto exit;
			}
			mdelay(1);
		}
		if(retry == 0)
			err = -1;
		break;
	}
 exit:
	/* unlock semaphore */
	        mutex_unlock(&cam_i2c_mutex);
		return err;

}
#endif

static int cam_stream_off(struct i2c_client *client)
{
        unsigned char mc_data[100];
        uint32_t payload_len = 0;

        uint16_t cmd_status = 0;
        uint8_t retcode = 0, cmd_id = 0;
	int retry = 1000, err = 0;
        /* call ISP init command */

	/*lock semaphore*/
        mutex_lock(&cam_i2c_mutex);

        /* First Txn Payload length = 0 */
        payload_len = 0;

        mc_data[0] = CMD_SIGNATURE;
        mc_data[1] = CMD_ID_STREAM_OFF;
        mc_data[2] = payload_len >> 8;
        mc_data[3] = payload_len & 0xFF;
        mc_data[4] = errorcheck(&mc_data[2], 2);

        ar0521_write(client, mc_data, TX_LEN_PKT);

        mc_data[0] = CMD_SIGNATURE;
        mc_data[1] = CMD_ID_STREAM_OFF;
        err = ar0521_write(client, mc_data, 2);
        if (err != 0) {
                dev_err(&client->dev," %s(%d) CAM Stream OFF Write Error - %d \n", __func__,
                       __LINE__, err);
                goto exit;
        }

        while (--retry > 0) {
                /* Some Sleep for init to process */
                yield();

                cmd_id = CMD_ID_STREAM_OFF;
                if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) <
                    0) {
                       dev_err(&client->dev," %s(%d) CAM Get CMD Stream Off Error \n", __func__,
                               __LINE__);
		       err = -1;
                        goto exit;
                }

                if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
                    (retcode == ERRCODE_SUCCESS)) {
                        debug_printk(" %s %d CAM Get CMD Stream off Success !! \n", __func__, __LINE__ );
			err = 0;
                        goto exit;
                }

                if ((retcode != ERRCODE_BUSY) &&
                    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
                       dev_err(&client->dev,
                            "(%s) %d CAM Get CMD Stream off Error STATUS = 0x%04x RET = 0x%02x\n",
                             __func__, __LINE__, cmd_status, retcode);
		       err = -1;
                        goto exit;
                }
		mdelay(1);
        }
exit:
	/* unlock semaphore */
	mutex_unlock(&cam_i2c_mutex);
	return err;
}

static int cam_isp_deinit(struct i2c_client *client)
{
	uint32_t payload_len = 0;

	uint16_t cmd_status = 0;
	uint8_t retcode = 0, cmd_id = 0;
	int retry = 1000, err = 0;

	pr_info("cam_isp_de-init\n");

	/* call ISP De-init command */

	/* First Txn Payload length = 0 */
	payload_len = 0;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_DE_INIT_CAM;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_DE_INIT_CAM;
	err = ar0521_write(client, mc_data, 2);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		return -EIO;
	}

	while (--retry > 0) {
		/* Some Sleep for init to process */
		mdelay(500);

		cmd_id = CMD_ID_DE_INIT_CAM;
		if (cam_get_cmd_status
		    (client, &cmd_id, &cmd_status, &retcode) < 0) {
			dev_err(&client->dev," %s(%d) Error \n",
			       __func__, __LINE__);
			return -EIO;
		}

		if ((cmd_status == CAM_CMD_STATUS_ISP_UNINIT) &&
		    ((retcode == ERRCODE_SUCCESS) || (retcode == ERRCODE_ALREADY))) {
			dev_err(&client->dev,"ISP De-initialized !! \n");
			//dev_err(" ISP Already Initialized !! \n");
			return 0;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			dev_err(&client->dev,
			    "(%s) %d De-Init Error STATUS = 0x%04x RET = 0x%02x\n",
			     __func__, __LINE__, cmd_status, retcode);
			return -EIO;
		}
	}
	dev_err(&client->dev,"ETIMEDOUT Error\n");
	return -ETIMEDOUT;
}


static int cam_isp_init(struct i2c_client *client)
{
	uint32_t payload_len = 0;

	uint16_t cmd_status = 0;
	uint8_t retcode = 0, cmd_id = 0;
	int retry = 1000, err = 0;

	pr_info("cam_isp_init\n");
	/* check current status - if initialized, no need for Init */
	cmd_id = CMD_ID_INIT_CAM;
	if (cam_get_cmd_status(client, &cmd_id, &cmd_status, &retcode) < 0) {
		dev_err(&client->dev," %s(%d) Error \n", __func__, __LINE__);
		return -EIO;
	}

	if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
	    (retcode == ERRCODE_SUCCESS)) {
		dev_err(&client->dev," Already Initialized !! \n");
		return 0;
	}

	/* call ISP init command */

	/* First Txn Payload length = 0 */
	payload_len = 0;

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_INIT_CAM;
	mc_data[2] = payload_len >> 8;
	mc_data[3] = payload_len & 0xFF;
	mc_data[4] = errorcheck(&mc_data[2], 2);

	ar0521_write(client, mc_data, TX_LEN_PKT);

	mc_data[0] = CMD_SIGNATURE;
	mc_data[1] = CMD_ID_INIT_CAM;
	err = ar0521_write(client, mc_data, 2);
	if (err != 0) {
		dev_err(&client->dev," %s(%d) Error - %d \n", __func__,
		       __LINE__, err);
		return -EIO;
	}

	while (--retry > 0) {
		/* Some Sleep for init to process */
		mdelay(500);

		cmd_id = CMD_ID_INIT_CAM;
		if (cam_get_cmd_status
		    (client, &cmd_id, &cmd_status, &retcode) < 0) {
			dev_err(&client->dev," %s(%d) Error \n",
			       __func__, __LINE__);
			return -EIO;
		}

		if ((cmd_status == CAM_CMD_STATUS_SUCCESS) &&
		    ((retcode == ERRCODE_SUCCESS) || (retcode == ERRCODE_ALREADY))) {
			dev_err(&client->dev,"ISP Already Initialized !! \n");
			//dev_err(" ISP Already Initialized !! \n");
			return 0;
		}

		if ((retcode != ERRCODE_BUSY) &&
		    ((cmd_status != CAM_CMD_STATUS_PENDING))) {
			dev_err(&client->dev,
			    "(%s) %d Init Error STATUS = 0x%04x RET = 0x%02x\n",
			     __func__, __LINE__, cmd_status, retcode);
			return -EIO;
		}
	}
	dev_err(&client->dev,"ETIMEDOUT Error\n");
	return -ETIMEDOUT;
}

unsigned short int cam_bload_calc_crc16(unsigned char *buf, int len)
{
	unsigned short int crc = 0;
	int i = 0;

	if (!buf || !(buf + len))
		return 0;

	for (i = 0; i < len; i++) {
		crc ^= buf[i];
	}

	return crc;
}

unsigned char cam_bload_inv_checksum(unsigned char *buf, int len)
{
	unsigned int checksum = 0x00;
	int i = 0;

	if (!buf || !(buf + len))
		return 0;

	for (i = 0; i < len; i++) {
		checksum = (checksum + buf[i]);
	}

	checksum &= (0xFF);
	return (~(checksum) + 1);
}

int cam_bload_get_version(struct i2c_client *client)
{
	int ret = 0;

	/*----------------------------- GET VERSION -------------------- */

	/*   Write Get Version CMD */
	g_bload_buf[0] = BL_GET_VERSION;
	g_bload_buf[1] = ~(BL_GET_VERSION);

	ret = ar0521_write(client, g_bload_buf, 2);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	/*   Wait for ACK or NACK */
	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	if (g_bload_buf[0] != 'y') {
		/*   NACK Received */
		dev_err(&client->dev," NACK Received... exiting.. \n");
		return -1;
	}

	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed\n");
		return -1;
	}

	/* ---------------- GET VERSION END ------------------- */

	return 0;
}

int cam_bload_parse_send_cmd(struct i2c_client *client,
			     unsigned char *bytearray, int rec_len)
{
	IHEX_RECORD *ihex_rec = NULL;
	unsigned char checksum = 0, calc_checksum = 0;
	int i = 0, ret = 0;

	if (!bytearray)
		return -1;

	ihex_rec = (IHEX_RECORD *) bytearray;
	ihex_rec->addr = htons(ihex_rec->addr);

	checksum = bytearray[rec_len - 1];

	calc_checksum = cam_bload_inv_checksum(bytearray, rec_len - 1);
	if (checksum != calc_checksum) {
		dev_err(&client->dev," Invalid Checksum 0x%02x != 0x%02x !! \n",
		       checksum, calc_checksum);
		return -1;
	}

	if ((ihex_rec->rectype == REC_TYPE_ELA)
	    && (ihex_rec->addr == 0x0000)
	    && (ihex_rec->datasize = 0x02)) {
		/*   Upper 32-bit configuration */
		g_bload_flashaddr = (ihex_rec->recdata[0] <<
				     24) | (ihex_rec->recdata[1]
					    << 16);

		debug_printk("Updated Flash Addr = 0x%08x \n",
			     g_bload_flashaddr);

	} else if (ihex_rec->rectype == REC_TYPE_DATA) {
		/*   Flash Data into Flashaddr */

		g_bload_flashaddr =
		    (g_bload_flashaddr & 0xFFFF0000) | (ihex_rec->addr);
		g_bload_crc16 ^=
		    cam_bload_calc_crc16(ihex_rec->recdata, ihex_rec->datasize);

		/*   Write Erase Pages CMD */
		g_bload_buf[0] = BL_WRITE_MEM_NS;
		g_bload_buf[1] = ~(BL_WRITE_MEM_NS);

		ret = ar0521_write(client, g_bload_buf, 2);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

		g_bload_buf[0] = (g_bload_flashaddr & 0xFF000000) >> 24;
		g_bload_buf[1] = (g_bload_flashaddr & 0x00FF0000) >> 16;
		g_bload_buf[2] = (g_bload_flashaddr & 0x0000FF00) >> 8;
		g_bload_buf[3] = (g_bload_flashaddr & 0x000000FF);
		g_bload_buf[4] =
		    g_bload_buf[0] ^ g_bload_buf[1] ^ g_bload_buf[2] ^
		    g_bload_buf[3];

		ret = ar0521_write(client, g_bload_buf, 5);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

		g_bload_buf[0] = ihex_rec->datasize - 1;
		checksum = g_bload_buf[0];
		for (i = 0; i < ihex_rec->datasize; i++) {
			g_bload_buf[i + 1] = ihex_rec->recdata[i];
			checksum ^= g_bload_buf[i + 1];
		}

		g_bload_buf[i + 1] = checksum;

		ret = ar0521_write(client, g_bload_buf, i + 2);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

 poll_busy:
		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] == RESP_BUSY)
			goto poll_busy;

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

	} else if (ihex_rec->rectype == REC_TYPE_SLA) {
		/*   Update Instruction pointer to this address */

	} else if (ihex_rec->rectype == REC_TYPE_EOF) {
		/*   End of File - Issue I2C Go Command */
		return 0;
	} else {

		/*   Unhandled Type */
		dev_err(&client->dev,"Unhandled Command Type \n");
		return -1;
	}

	return 0;
}

int cam_bload_go(struct i2c_client *client)
{
	int ret = 0;

	g_bload_buf[0] = BL_GO;
	g_bload_buf[1] = ~(BL_GO);

	ret = ar0521_write(client, g_bload_buf, 2);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Failed Read 1 \n");
		return -1;
	}

	/*   Start Address */
	g_bload_buf[0] = (FLASH_START_ADDRESS & 0xFF000000) >> 24;
	g_bload_buf[1] = (FLASH_START_ADDRESS & 0x00FF0000) >> 16;
	g_bload_buf[2] = (FLASH_START_ADDRESS & 0x0000FF00) >> 8;
	g_bload_buf[3] = (FLASH_START_ADDRESS & 0x000000FF);
	g_bload_buf[4] =
	    g_bload_buf[0] ^ g_bload_buf[1] ^ g_bload_buf[2] ^ g_bload_buf[3];

	ret = ar0521_write(client, g_bload_buf, 5);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Failed Read 1 \n");
		return -1;
	}

	if (g_bload_buf[0] != RESP_ACK) {
		/*   NACK Received */
		dev_err(&client->dev," NACK Received... exiting.. \n");
		return -1;
	}

	return 0;
}

int cam_bload_update_fw(struct i2c_client *client)
{
	/* exclude NULL character at end of string */
	unsigned long hex_file_size = ARRAY_SIZE(g_cam_fw_buf) - 1;
	unsigned char wbuf[MAX_BUF_LEN];
	int i = 0, recindex = 0, ret = 0;

	for (i = 0; i < hex_file_size; i++) {
		if ((recindex == 0) && (g_cam_fw_buf[i] == ':')) {
			/*  debug_printk("Start of a Record \n"); */
		} else if (g_cam_fw_buf[i] == CR) {
			/*   No Implementation */
		} else if (g_cam_fw_buf[i] == LF) {
			if (recindex == 0) {
				/*   Parsing Complete */
				break;
			}

			/*   Analyze Packet and Send Commands */
			ret = cam_bload_parse_send_cmd(client, wbuf, recindex);
			if (ret < 0) {
				dev_err(&client->dev,"Error in Processing Commands \n");
				break;
			}

			recindex = 0;

		} else {
			/*   Parse Rec Data */
			if ((ret = cam_bload_ascii2hex(g_cam_fw_buf[i])) < 0) {
				dev_err(&client->dev,
					"Invalid Character - 0x%02x !! \n",
				     g_cam_fw_buf[i]);
				break;
			}

			wbuf[recindex] = (0xF0 & (ret << 4));
			i++;

			if ((ret = cam_bload_ascii2hex(g_cam_fw_buf[i])) < 0) {
				dev_err(&client->dev,
				    "Invalid Character - 0x%02x !!!! \n",
				     g_cam_fw_buf[i]);
				break;
			}

			wbuf[recindex] |= (0x0F & ret);
			recindex++;
		}
	}

	debug_printk("Program FLASH Success !! - CRC = 0x%04x \n",
		     g_bload_crc16);

	/* ------------ PROGRAM FLASH END ----------------------- */

	return ret;
}

int cam_bload_erase_flash(struct i2c_client *client)
{
	unsigned short int pagenum = 0x0000;
	int ret = 0, i = 0, checksum = 0;

	/* --------------- ERASE FLASH --------------------- */

	for (i = 0; i < NUM_ERASE_CYCLES; i++) {

		checksum = 0x00;
		/*   Write Erase Pages CMD */
		g_bload_buf[0] = BL_ERASE_MEM_NS;
		g_bload_buf[1] = ~(BL_ERASE_MEM_NS);

		ret = ar0521_write(client, g_bload_buf, 2);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

		g_bload_buf[0] = (MAX_PAGES - 1) >> 8;
		g_bload_buf[1] = (MAX_PAGES - 1) & 0xFF;
		g_bload_buf[2] = g_bload_buf[0] ^ g_bload_buf[1];

		ret = ar0521_write(client, g_bload_buf, 3);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

		for (pagenum = 0; pagenum < MAX_PAGES; pagenum++) {
			g_bload_buf[(2 * pagenum)] =
			    (pagenum + (i * MAX_PAGES)) >> 8;
			g_bload_buf[(2 * pagenum) + 1] =
			    (pagenum + (i * MAX_PAGES)) & 0xFF;
			checksum =
			    checksum ^ g_bload_buf[(2 * pagenum)] ^
			    g_bload_buf[(2 * pagenum) + 1];
		}
		g_bload_buf[2 * MAX_PAGES] = checksum;

		ret = ar0521_write(client, g_bload_buf, (2 * MAX_PAGES) + 1);
		if (ret < 0) {
			dev_err(&client->dev,"Write Failed \n");
			return -1;
		}

 poll_busy:
		/*   Wait for ACK or NACK */
		ret = ar0521_read(client, g_bload_buf, 1);
		if (ret < 0) {
			dev_err(&client->dev,"Read Failed \n");
			return -1;
		}

		if (g_bload_buf[0] == RESP_BUSY)
			goto poll_busy;

		if (g_bload_buf[0] != RESP_ACK) {
			/*   NACK Received */
			dev_err(&client->dev," NACK Received... exiting.. \n");
			return -1;
		}

		debug_printk(" ERASE Sector %d success !! \n", i + 1);
	}

	/* ------------ ERASE FLASH END ----------------------- */

	return 0;
}

int cam_bload_read(struct i2c_client *client,
		   unsigned int g_bload_flashaddr, char *bytearray,
		   unsigned int len)
{
	int ret = 0;

	g_bload_buf[0] = BL_READ_MEM;
	g_bload_buf[1] = ~(BL_READ_MEM);

	ret = ar0521_write(client, g_bload_buf, 2);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	/*   Wait for ACK or NACK */
	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	if (g_bload_buf[0] != RESP_ACK) {
		/*   NACK Received */
		dev_err(&client->dev," NACK Received... exiting.. \n");
		return -1;
	}

	g_bload_buf[0] = (g_bload_flashaddr & 0xFF000000) >> 24;
	g_bload_buf[1] = (g_bload_flashaddr & 0x00FF0000) >> 16;
	g_bload_buf[2] = (g_bload_flashaddr & 0x0000FF00) >> 8;
	g_bload_buf[3] = (g_bload_flashaddr & 0x000000FF);
	g_bload_buf[4] =
	    g_bload_buf[0] ^ g_bload_buf[1] ^ g_bload_buf[2] ^ g_bload_buf[3];

	ret = ar0521_write(client, g_bload_buf, 5);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	/*   Wait for ACK or NACK */
	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	if (g_bload_buf[0] != RESP_ACK) {
		/*   NACK Received */
		dev_err(&client->dev," NACK Received... exiting.. \n");
		return -1;
	}

	g_bload_buf[0] = len - 1;
	g_bload_buf[1] = ~(len - 1);

	ret = ar0521_write(client, g_bload_buf, 2);
	if (ret < 0) {
		dev_err(&client->dev,"Write Failed \n");
		return -1;
	}

	/*   Wait for ACK or NACK */
	ret = ar0521_read(client, g_bload_buf, 1);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	if (g_bload_buf[0] != RESP_ACK) {
		/*   NACK Received */
		dev_err(&client->dev," NACK Received... exiting.. \n");
		return -1;
	}

	ret = ar0521_read(client, bytearray, len);
	if (ret < 0) {
		dev_err(&client->dev,"Read Failed \n");
		return -1;
	}

	return 0;
}

int cam_bload_verify_flash(struct i2c_client *client,
			   unsigned short int orig_crc)
{
	char bytearray[FLASH_READ_LEN];
	unsigned short int calc_crc = 0;
	unsigned int flash_addr = FLASH_START_ADDRESS, i = 0;

	while ((i + FLASH_READ_LEN) <= FLASH_SIZE) {
		memset(bytearray, 0x0, FLASH_READ_LEN);

		if (cam_bload_read
		    (client, flash_addr + i, bytearray, FLASH_READ_LEN) < 0) {
			dev_err(&client->dev," i2c_bload_read FAIL !! \n");
			return -1;
		}

		calc_crc ^= cam_bload_calc_crc16(bytearray, FLASH_READ_LEN);
		i += FLASH_READ_LEN;
	}

	if ((FLASH_SIZE - i) > 0) {
		memset(bytearray, 0x0, FLASH_READ_LEN);

		if (cam_bload_read
		    (client, flash_addr + i, bytearray, (FLASH_SIZE - i))
		    < 0) {
			dev_err(&client->dev," i2c_bload_read FAIL !! \n");
			return -1;
		}

		calc_crc ^= cam_bload_calc_crc16(bytearray, FLASH_READ_LEN);
	}

	if (orig_crc != calc_crc) {
		dev_err(&client->dev," CRC verification fail !! 0x%04x != 0x%04x \n",
		       orig_crc, calc_crc);
//		return -1;
	}

	debug_printk(" CRC Verification Success 0x%04x == 0x%04x \n",
		     orig_crc, calc_crc);

	return 0;
}

static int cam_fw_update(struct i2c_client *client, unsigned char *cam_fw_version)
{
	int ret = 0;
	g_bload_crc16 = 0;

	/* Read Firmware version from bootloader CAM */
	ret = cam_bload_get_version(client);
	if (ret < 0) {
		dev_err(&client->dev," Error in Get Version \n");
		goto exit;
	}

	debug_printk(" Get Version SUCCESS !! \n");

	/* Erase firmware present in the CAM and flash new firmware*/
	ret = cam_bload_erase_flash(client);
	if (ret < 0) {
		dev_err(&client->dev," Error in Erase Flash \n");
		goto exit;
	}

	debug_printk("Erase Flash Success !! \n");

	/* Read the firmware present in the bin file */
	if ((ret = cam_bload_update_fw(client)) < 0) {
		dev_err(&client->dev," Write Flash FAIL !! \n");
		goto exit;
	}

	/* Verify the checksum for the update firmware */
	if ((ret = cam_bload_verify_flash(client, g_bload_crc16)) < 0) {
		dev_err(&client->dev," verify_flash FAIL !! \n");
		goto exit;
	}

	/* Reverting from bootloader mode */
	/* I2C GO Command */
	if ((ret = cam_bload_go(client)) < 0) {
		dev_err(&client->dev," i2c_bload_go FAIL !! \n");
		goto exit;
	}

	if(cam_fw_version) {
		debug_printk("(%s) - Firmware Updated - (%.8s - g%.7s)\n",
				__func__, &cam_fw_version[2], &cam_fw_version[18]);
	}
 exit:
	return ret;
}

static int ar0521_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{
	struct camera_common_data *common_data;
	struct device_node *node = client->dev.of_node;
	struct ar0521 *priv;
	int gpio;

	unsigned char fw_version[32] = {0}, bin_fw_version[32] = {0};
	int ret, frm_fmt_size = 0, loop;
	uint16_t sensor_id = 0;
	int16_t reset_gpio = 0, pwdn_gpio = 0;
	uint32_t mipi_lane = 0;
	bool framesync_disabled;

	int err = 0, pwdn_gpio_toggle = 0;
	if (!IS_ENABLED(CONFIG_OF) || !node) {
		return -EINVAL;
	}


	reset_gpio = of_get_named_gpio(node, "reset-gpios", 0);
	if(reset_gpio < 0) {
		dev_err(&client->dev, "Unable to toggle GPIO\n");
		return -EINVAL;
	}

	pwdn_gpio = of_get_named_gpio(node, "pwdn-gpios", 0);
	if(pwdn_gpio < 0) {
		dev_err(&client->dev, "Unable to toggle GPIO\n");
		return -EINVAL;
	}

	err = gpio_request(reset_gpio,"cam-reset");
	if (err < 0) {
		dev_err(&client->dev,"%s[%d]:GPIO reset Fail, err:%d",__func__,__LINE__, err);
		return -EINVAL;
	}

	err = gpio_request(pwdn_gpio,"cam-boot");
	if (err < 0) {
		dev_err(&client->dev,"%s[%d]:%dGPIO boot Fail\n",__func__,__LINE__,err);
		return -EINVAL;
	}
	err = of_property_read_u32(node, "camera_mipi_lanes", &mipi_lane);
        if (err < 0) {
                dev_err(&client->dev, "Can not get Camera MIPI Lanes\n");
                return -EINVAL;
        }

	framesync_disabled = of_property_read_bool(node, "disable-framesync");

	//err = gpio_direction_output(pwdn_gpio, 0);
	common_data =
	    devm_kzalloc(&client->dev,
			 sizeof(struct camera_common_data), GFP_KERNEL);
	if (!common_data)
		return -ENOMEM;

	priv =
	    devm_kzalloc(&client->dev,
			 sizeof(struct ar0521) +
			 sizeof(struct v4l2_ctrl *) * AR0521_NUM_CONTROLS /**num_ctrls*/,
			 GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	/* MIPI SELECT GPIO for MIPI Lane select
         * For 2 Lane config:- Mipi select gpio LOW state
         *      gpio_set_value(gpio, 0);
         * For 4 Lane config:- Mipi select gpio HIGH state
         *      gpio_set_value(gpio, 1);
         */
        gpio = of_get_named_gpio(node, "mipi-select-gpios", 0);
	if (gpio < 0) {
		/* reset-gpio is not absoluctly needed */
		dev_dbg(&client->dev, "mipi select gpios not in DT\n");
		gpio = 0;
	} else {
		gpio_request(gpio, "mipi_select");
		gpio_direction_output(gpio, 1);
		if(mipi_lane == NUM_LANES_2)
			gpio_set_value(gpio, 0);
		else if(mipi_lane == NUM_LANES_4)
			gpio_set_value(gpio, 1);
	}

	priv->pdata = ar0521_parse_dt(client);
	if (!priv->pdata) {
		dev_err(&client->dev, "unable to get platform data\n");
		return -EFAULT;
	}
	priv->i2c_client = client;
	priv->s_data = common_data;
	priv->subdev = &common_data->subdev;
	priv->subdev->dev = &client->dev;
	priv->s_data->dev = &client->dev;
	common_data->priv = (void *)priv;
	priv->mipi_lane_config = mipi_lane;
	priv->framesync_enabled = !framesync_disabled;
	priv->force_config = false;
#if ISP_PWDN_WKUP
	priv->power_on = 0;
#endif
#ifdef FRAMESYNC_ENABLE
	priv->last_sync_mode = 1;
#endif

	err = ar0521_power_get(priv);
	if (err)
		return err;


	err = ar0521_power_on(common_data);
	if (err)
		return err;

	/* Reset Release for CAM */
	toggle_gpio(pwdn_gpio, 0);
	msleep(1);
	toggle_gpio(reset_gpio, 0);
	msleep(1);
	toggle_gpio(reset_gpio, 1);
	msleep(10);

	ret = cam_get_fw_version(client, fw_version, bin_fw_version);
	if (ret != 0) {

		dev_err(&client->dev," Trying to Detect Bootloader mode\n");
		toggle_gpio(reset_gpio, 0);
		msleep(1);
		toggle_gpio(pwdn_gpio, 1);
		msleep(10);
		toggle_gpio(reset_gpio, 1);
		msleep(10);
		for(loop = 0;loop < 10; loop++) {
			err = cam_bload_get_version(client);
			if (err < 0) {
				/* Trial and Error for 10 ms (1ms * 10) */
				msleep(1);
				continue;
			} else {
				dev_err(&client->dev," Get Bload Version Success\n");
				pwdn_gpio_toggle = 1;
				break;
			}
		}

		if(loop == 10) {
			dev_err(&client->dev, "Error updating firmware \n");
			return -EINVAL;
		}

		if (cam_fw_update(client, NULL) < 0)
			return -EFAULT;

		if( pwdn_gpio_toggle == 1)
			toggle_gpio(pwdn_gpio, 0);

		/* Allow FW Updated Driver to reboot */
		msleep(10);

		for(loop = 0;loop < 100; loop++) {
			err = cam_get_fw_version(client, fw_version, bin_fw_version);
			if (err < 0) {
				/* Trial and Error for 100 ms (1ms * 100) */
				msleep(1);
				continue;
			} else {
				dev_err(&client->dev," Get FW Version Success\n");
				break;
			}
		}
		if(loop == 100) {
			dev_err(&client->dev, "Error updating firmware \n");
			return -EINVAL;
		}
		debug_printk("Current Firmware Version - (%.8s-g%.7s).",
				&fw_version[2],&fw_version[18]);
	} else {
		/* Same firmware version in CAM and Bin File */
		debug_printk("Current Firmware Version - (%.8s-g%.7s)",
				&fw_version[2],&fw_version[18]);
	}
	/* Configure MIPI Lanes of the Sensor */
	if (cam_lane_configuration(client, priv) < 0) {
		dev_err(&client->dev, "%s, Failed to send Calibration Data\n",__func__);
		return -EFAULT;
	}


	/* Query the number of controls from CAM*/
	if(cam_list_ctrls(client, NULL, priv) < 0) {
		dev_err(&client->dev, "%s, Failed to init controls \n", __func__);
		return -EFAULT;
	}

	/*Query the number for Formats available from CAM */
	if(cam_list_fmts(client, NULL, &frm_fmt_size, priv) < 0) {
		dev_err(&client->dev, "%s, Failed to init formats \n", __func__);
		return -EFAULT;
	}

	priv->cam_ctrl_info = devm_kzalloc(&client->dev, sizeof(ISP_CTRL_INFO) * priv->num_ctrls, GFP_KERNEL);
	if(!priv->cam_ctrl_info) {
		dev_err(&client->dev, "Unable to allocate memory \n");
		return -ENOMEM;
	}

	priv->ctrldb = devm_kzalloc(&client->dev, sizeof(uint32_t) * priv->num_ctrls, GFP_KERNEL);
	if(!priv->ctrldb) {
		dev_err(&client->dev, "Unable to allocate memory \n");
		return -ENOMEM;
	}

	priv->stream_info = devm_kzalloc(&client->dev, sizeof(ISP_STREAM_INFO) * (frm_fmt_size + 1), GFP_KERNEL);

	priv->streamdb = devm_kzalloc(&client->dev, sizeof(int) * (frm_fmt_size + 1), GFP_KERNEL);
	if(!priv->streamdb) {
		dev_err(&client->dev,"Unable to allocate memory \n");
		return -ENOMEM;
	}

	priv->cam_frmfmt = devm_kzalloc(&client->dev, sizeof(struct camera_common_frmfmt) * (frm_fmt_size), GFP_KERNEL);
	if(!priv->cam_frmfmt) {
		dev_err(&client->dev, "Unable to allocate memory \n");
		return -ENOMEM;
	}

	if (cam_isp_init(client) < 0) {
		dev_err(&client->dev, "Unable to INIT ISP \n");
		return -EFAULT;
	}

	if (cam_get_sensor_id(client, &sensor_id) < 0) {
		dev_err(&client->dev, "Unable to get CAM Sensor ID \n");
		return -EFAULT;
	}

	printk("SENSOR ID=0x%04x\n",sensor_id);

	err = cam_stream_off(client);
	if(err!= 0){
		dev_err(&client->dev,"%s (%d) Stream_Off \n", __func__, __LINE__);
		return err;
	}

	for(loop = 0; loop < frm_fmt_size; loop++) {
		priv->cam_frmfmt[loop].framerates = devm_kzalloc(&client->dev, sizeof(int) * MAX_NUM_FRATES, GFP_KERNEL);
		if(!priv->cam_frmfmt[loop].framerates) {
			dev_err(&client->dev, "Unable to allocate memory \n");
			return -ENOMEM;
		}
	}

	/* Enumerate Formats */
	if (cam_list_fmts(client, priv->stream_info, &frm_fmt_size, priv) < 0) {
		dev_err(&client->dev, "Unable to List Fmts \n");
		return -EFAULT;
	}

	common_data->ops = NULL;
	common_data->ctrl_handler = &priv->ctrl_handler;
	common_data->frmfmt = priv->cam_frmfmt;
	common_data->colorfmt =
	    camera_common_find_datafmt(AR0521_DEFAULT_DATAFMT);
	common_data->power = &priv->power;
	common_data->ctrls = priv->ctrls;
	common_data->priv = (void *)priv;
	common_data->numctrls = priv->num_ctrls;
	common_data->numfmts = frm_fmt_size;
	common_data->def_mode = AR0521_DEFAULT_MODE;
	common_data->def_width = AR0521_DEFAULT_WIDTH;
	common_data->def_height = AR0521_DEFAULT_HEIGHT;
	common_data->fmt_width = common_data->def_width;
	common_data->fmt_height = common_data->def_height;
	common_data->def_clk_freq = 24000000;

	priv->i2c_client = client;
	priv->s_data = common_data;
	priv->subdev = &common_data->subdev;
	priv->subdev->dev = &client->dev;
	priv->s_data->dev = &client->dev;
	priv->prev_index = 0xFFFE;

	err = camera_common_initialize(common_data, "ar0521");
	if (err) {
		dev_err(&client->dev, "Failed to initialize ar0521.\n");
		return err;
	}

	/* Get CAM FW version to find the availabity of MC chip */

	v4l2_i2c_subdev_init(priv->subdev, client, &ar0521_subdev_ops);

	/* Enumerate Ctrls */
	err = ar0521_ctrls_init(priv, priv->cam_ctrl_info);
	if (err)
		return err;

	priv->subdev->internal_ops = &ar0521_subdev_internal_ops;
	priv->subdev->flags |=
	    V4L2_SUBDEV_FL_HAS_DEVNODE | V4L2_SUBDEV_FL_HAS_EVENTS;

	/*
	  To unload the module driver module, 
	  Set (struct v4l2_subdev *)priv->subdev->sd to NULL.
	  Refer tegracam_v4l2subdev_register() in tegracam_v4l2.c
	 */
	
	if (priv->subdev->owner == THIS_MODULE) {
		common_data->owner = priv->subdev->owner;
		priv->subdev->owner = NULL;
	} else {
		// It shouldn't come here in probe();
		;
	}

#if defined(CONFIG_MEDIA_CONTROLLER)
	priv->pad.flags = MEDIA_PAD_FL_SOURCE;
	priv->subdev->entity.ops = &ar0521_media_ops;
	err = tegra_media_entity_init(&priv->subdev->entity, 1, &priv->pad, true, true);
	if (err < 0) {
		dev_err(&client->dev, "unable to init media entity\n");
		return err;
	}
#endif

	err = v4l2_async_register_subdev(priv->subdev);
	if (err)
		return err;

	if(!init_err_hand_q) {
		pr_info("Initializing err_hand_q\n");
		/* Initialize the WAIT QUEUE head */
		init_waitqueue_head(&econ_err_hand_q);
		init_err_hand_q = 1;
	}

	/*Start Error Handling Thread*/
	strm_mon[num_cam].strm_client_mon = client;
	strm_mon[num_cam].strm_priv_mon = priv;
	if (strcmp(client->dev.of_node->name, "ar0521_a") == 0) {
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[0], CAM_NAME_SIZE);
	}
	else if (strcmp(client->dev.of_node->name, "ar0521_b") == 0) {
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[1], CAM_NAME_SIZE);
	}
	else if(strcmp(client->dev.of_node->name, "ar0521_c") == 0) {
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[2], CAM_NAME_SIZE);
	}
	else if(strcmp(client->dev.of_node->name, "ar0521_d") == 0) {
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[3], CAM_NAME_SIZE);
	}
	else if (strcmp(client->dev.of_node->name, "ar0521_e") == 0)
	{
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[4], CAM_NAME_SIZE);
	}
	else if (strcmp(client->dev.of_node->name, "ar0521_f") == 0)
	{
		memset(strm_mon[num_cam].camera_name, 0, CAM_NAME_SIZE);
		memcpy(strm_mon[num_cam].camera_name, &dev_name_compare[5], CAM_NAME_SIZE);
	}

	if( is_stream_monitor_thrd == 0){
		strm_mon_thrd = kthread_create(stream_monitor_thread, &strm_mon[num_cam], "strm_mon_thrd");
		if(strm_mon_thrd != NULL)
		{
			wake_up_process(strm_mon_thrd);
			printk("Stream Monitor Thread is created\n");
		}
		else
		{
			dev_err(&client->dev, "Could not create the stream monitor thread so stopping\n");
			kthread_stop(strm_mon_thrd);
		}
		is_stream_monitor_thrd = 1;
	}

	num_cam++;

	return 0;
}

#define FREE_SAFE(dev, ptr) \
	if(ptr) { \
		devm_kfree(dev, ptr); \
	}

static int ar0521_remove(struct i2c_client *client)
{
	struct camera_common_data *s_data = to_camera_common_data(&client->dev);
	struct ar0521 *priv = (struct ar0521 *)s_data->priv;
	struct device_node *node = client->dev.of_node;
	int loop = 0;
	uint16_t reset_gpio = 0, pwdn_gpio = 0;
		
	if (!priv || !priv->pdata)
		return -1;

	// Release the Gpios
	reset_gpio = of_get_named_gpio(node, "reset-gpios", 0);
	if(reset_gpio < 0) {
		dev_err(&client->dev, "Unable to get reset GPIO\n");
		return -EINVAL;
	}

	pwdn_gpio = of_get_named_gpio(node, "pwdn-gpios", 0);
	if(pwdn_gpio < 0) {
		dev_err(&client->dev, "Unable to get power GPIO\n");
		return -EINVAL;
	}
	gpio_free(reset_gpio);
	gpio_free(pwdn_gpio);
	
	v4l2_async_unregister_subdev(priv->subdev);
#if defined(CONFIG_MEDIA_CONTROLLER)
	media_entity_cleanup(&priv->subdev->entity);
#endif

	v4l2_ctrl_handler_free(&priv->ctrl_handler);
	//ar0521_power_put(priv);
	camera_common_remove_debugfs(s_data);
 
	if(is_stream_monitor_thrd == 1) {
		/* For Err Handle Thread*/
		stop_thread = 1;
		econ_frame_err_track = 1;
		wake_up_interruptible(&econ_err_hand_q);
		printk("Stopping the Error Handling Thread\n");
		kthread_stop(strm_mon_thrd);
		is_stream_monitor_thrd = 0;
	}

	/* Free up memory */
	for(loop = 0; loop < priv->cam_ctrl_info->ctrl_ui_data.ctrl_menu_info.num_menu_elem
			; loop++) {
		FREE_SAFE(&client->dev, priv->cam_ctrl_info->ctrl_ui_data.ctrl_menu_info.menu[loop]);
	}

	FREE_SAFE(&client->dev, priv->cam_ctrl_info->ctrl_ui_data.ctrl_menu_info.menu);

	FREE_SAFE(&client->dev, priv->cam_ctrl_info);

	for(loop = 0; loop < s_data->numfmts; loop++ ) {
		FREE_SAFE(&client->dev, (void *)priv->cam_frmfmt[loop].framerates);
	}

	FREE_SAFE(&client->dev, priv->cam_frmfmt);

	FREE_SAFE(&client->dev, priv->ctrldb);
	FREE_SAFE(&client->dev, priv->streamdb);

	FREE_SAFE(&client->dev, priv->stream_info);
	FREE_SAFE(&client->dev, fw_version);
	FREE_SAFE(&client->dev, priv->pdata);
	FREE_SAFE(&client->dev, priv->s_data);
	FREE_SAFE(&client->dev, priv);
	return 0;
}

static const struct i2c_device_id ar0521_id[] = {
	{"ar0521", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, ar0521_id);

static struct i2c_driver ar0521_i2c_driver = {
	.driver = {
		   .name = "ar0521",
		   .owner = THIS_MODULE,
		   .of_match_table = of_match_ptr(ar0521_of_match),
		   },
	.probe = ar0521_probe,
	.remove = ar0521_remove,
	.id_table = ar0521_id,
};

module_i2c_driver(ar0521_i2c_driver);

MODULE_DESCRIPTION("V4L2 driver for e-CAM50_CUNANO");
MODULE_AUTHOR("E-Con Systems");
MODULE_LICENSE("GPL v2");
