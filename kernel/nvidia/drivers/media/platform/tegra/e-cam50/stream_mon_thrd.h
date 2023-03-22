#define CAM_NAME_SIZE 26

struct stream_monitor
{
	struct i2c_client *strm_client_mon;
	struct ar0521 *strm_priv_mon;
	char camera_name[26];
};

enum find_cam{
	CAM30_42 = 0,
	CAM31_42 = 1,
	CAM32_42 = 2,
	CAM33_42 = 3,
	CAM34_42 = 4,
	CAM35_42 = 5,
};

char dev_name_compare[6][26] = {
	"vi-output, ar0521 30-0042",
	"vi-output, ar0521 31-0042",
	"vi-output, ar0521 32-0042",
	"vi-output, ar0521 33-0042",
	"vi-output, ar0521 34-0042",
	"vi-output, ar0521 35-0042"
};

int mcu_err_handle(struct i2c_client *err_client, struct ar0521 *err_priv);
int find_err_cam(char *err_cam_name);

