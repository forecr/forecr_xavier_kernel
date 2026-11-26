/*
 *
 *  Realtek Bluetooth USB download firmware driver
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/errno.h>
#include <linux/skbuff.h>
#include <linux/usb.h>
#include <linux/dcache.h>
#include <linux/in.h>
#include <net/sock.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/skbuff.h>
#include <linux/errno.h>
#include <linux/usb.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/vmalloc.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
#include <asm/unaligned.h>
#else
#include <linux/unaligned.h>
#endif


/* #if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
 * #include <linux/pm_runtime.h>
 * #endif
 */

#include <linux/rcupdate.h>

#include <linux/firmware.h>
#include <linux/suspend.h>
#include <net/bluetooth/bluetooth.h>
#include <net/bluetooth/hci_core.h>
#include <net/bluetooth/hci.h>

#include "btrtl_misc.h"

#include <linux/file.h>
#include <linux/ctype.h>
#define BDADDR_STRING_LEN	17
#define BDADDR_FILE		"/opt/bdaddr"

struct hci_dev __rcu *controller = NULL;

struct cfg_list_item {
	struct list_head list;
	u16 offset;
	u8 len;
	u8 data[];
};

static struct list_head list_configs;

#define EXTRA_CONFIG_FILE	"/opt/rtk_btconfig.txt"
static struct list_head list_extracfgs;

static LIST_HEAD(apcf_filters);
static LIST_HEAD(apcf_wakeups);
static DEFINE_MUTEX(cfg_lock);

#define CMD_CMP_EVT		    0x0e
#define PKT_LEN			    300
#define MSG_TO			    1000	//us
#define PATCH_SEG_MAX	    252
#define DATA_END		    0x80
#define DOWNLOAD_OPCODE	    0xfc20
/* This command is used only for TV patch
 * if host is going to suspend state, it should send this command to
 * Controller. Controller will scan the special advertising packet
 * which indicates Controller to wake up host */
#define STARTSCAN_OPCODE	    0xfc28
#define TRUE			    1
#define FALSE			    0
#define CMD_HDR_LEN		    sizeof(struct hci_command_hdr)
#define EVT_HDR_LEN		    sizeof(struct hci_event_hdr)
#define CMD_CMP_LEN		    sizeof(struct hci_ev_cmd_complete)

#define HCI_CMD_READ_BD_ADDR                0x1009
#define HCI_VENDOR_CHANGE_BDRATE            0xfc17
#define HCI_VENDOR_READ_RTK_ROM_VERISION    0xfc6d
#ifndef HCI_OP_READ_LOCAL_VERSION
#define HCI_OP_READ_LOCAL_VERSION           0x1001
#endif
#define HCI_VENDOR_READ_CMD                 0xfc61
#define HCI_VENDOR_WRITE_CMD                0xfc62
#define HCI_VENDOR_READ_PATCH_VERSION       0xfdce
#define HCI_VENDOR_READ_CHIP_ID             0xfc6f
#define HCI_VENDOR_WDG_RESET                0xfc8e
#define HCI_VENDOR_WRITE_TB                 0xfdcf
#define HCI_VENDOR_ENHANCED_DOWNLOAD_MODE   0xfc1f

#define ROM_LMP_NONE                0x0000
#define ROM_LMP_8723a               0x1200
#define ROM_LMP_8723b               0x8723
#define ROM_LMP_8821a               0X8821
#define ROM_LMP_8761a               0X8761
#define ROM_LMP_8822b               0X8822
#define ROM_LMP_8852a               0x8852
#define ROM_LMP_8851b               0x8851
#define ROM_LMP_8922a               0x8922
#define ROM_LMP_8723c               0x8703

#define FW_TYPE_V0		0
#define FW_TYPE_V1		1
#define FW_TYPE_V2		2
#define FW_TYPE_V3_1		3
#define FW_TYPE_V3_2		4
#define is_v3_fw(type)	(type == FW_TYPE_V3_1 || type == FW_TYPE_V3_2)

#define PATCH_SNIPPETS		0x01
#define PATCH_DUMMY_HEADER	0x02
#define PATCH_SECURITY_HEADER	0x03
#define PATCH_OTA_FLAG		0x04
#define SECTION_HEADER_SIZE	8

struct hci_rp_read_eco {
	u8 status;
	u8 eco;
} __attribute__((packed));

struct hci_rp_read_sec {
	u8 status;
	u8 key_id;
} __attribute__((packed));

struct hci_rp_read_chip_id {
	u8 status;
	u8 chip_id;
} __attribute__((packed));

struct hci_rp_enhanced_download_mode {
	u8 status;
	u8 rom_patched;
	__le16 handle;
	__le16 acldata_pkt_len;
	u8 acl_cnt;
} __attribute__((packed));

struct hci_ev_patch_status_evt {
	u8 evt;
	u8 len;
	u8 reserved;
	u8 index;
	__le16 handle;
	__le32 loaded_len;
} __attribute__((packed));

struct image_match_info {
	u16 chip_id;
	u8  ic_cut;
	u8  key_id;
	u8  ota_enable;
};

struct rtk_eversion_evt {
	uint8_t status;
	uint8_t version;
} __attribute__ ((packed));

struct rtk_security_proj_evt {
	uint8_t status;
	uint8_t key_id;
} __attribute__ ((packed));

struct rtk_chip_type_evt {
	uint8_t status;
	uint16_t chip;
} __attribute__ ((packed));

enum rtk_read_class {
	READ_NONE = 0,
	READ_CHIP_TYPE = 1,
	READ_CHIP_VER = 2,
	READ_SEC_PROJ = 3
};

struct rtk_epatch_entry {
	uint16_t chipID;
	uint16_t patch_length;
	uint32_t start_offset;
} __attribute__ ((packed));

struct rtk_epatch {
	uint8_t signature[8];
	__le32 fw_version;
	__le16 number_of_total_patch;
	struct rtk_epatch_entry entry[0];
} __attribute__ ((packed));

struct rtk_extension_entry {
	uint8_t opcode;
	uint8_t length;
	uint8_t *data;
} __attribute__ ((packed));

struct rtb_section_hdr {
	uint32_t opcode;
	uint32_t section_len;
	uint32_t soffset;
} __attribute__ ((packed));

struct rtb_new_patch_hdr {
	uint8_t signature[8];
	uint8_t fw_version[8];
	__le32 number_of_section;
} __attribute__ ((packed));

//signature: Realtech
static const uint8_t RTK_EPATCH_SIGNATURE[8] =
    { 0x52, 0x65, 0x61, 0x6C, 0x74, 0x65, 0x63, 0x68 };

//signature: RTBTCore
static const uint8_t RTK_EPATCH_SIGNATURE_NEW[8] =
    { 0x52, 0x54, 0x42, 0x54, 0x43, 0x6F, 0x72, 0x65 };

#define RTK_EPATCH_SIGNATURE_V3 "BTNIC003"
#define RTL_PATCH_V3_1			0x01
#define RTL_PATCH_V3_PATCH_IMAGE	0x02

#define DL_FIX_CI_ID		0
#define DL_FIX_CI_ADDR		1
#define DL_FIX_PATCH_ADDR	2
#define DL_FIX_SEC_HDR_ADDR	3
#define DL_FIX_ADDR_MAX		4

#define IMAGE_ID_F000		0xf000
#define IMAGE_ID_F001		0xf001
#define IMAGE_ID_F002		0xf002

struct rtl_epatch_header_v3 {
	__u8 signature[8];
	__u8 timestamp[8];
	__le32 ver_rsvd;
	__le32 num_sections;
} __packed;

struct rtl_section_v3 {
	__le32 opcode;
	__le64 len;
	u8 data[];
} __packed;

struct rtl_addr_fix {
	u32 addr;
	u32 value;
};

struct rtl_subsection_v3 {
	u16 image_id;
	u8 index;
	u8 config_rule;
	u8 need_config;

	struct rtl_addr_fix fix[DL_FIX_ADDR_MAX];

	u32 patch_image_len;
	u8 *patch_image_buf;
	u32 image_ver;

	u8  *cfg_buf;
	u16 cfg_len;

	struct list_head list;
};

struct rtl_sec_hdr_v3 {
	__le16 chip_id;
	u8 ic_cut;
	u8 key_id;
	u8 enable_ota;
	__le16 image_id;
	u8 config_rule;
	u8 need_config;
	u8 rsv[950];

	__le64 addr_fix[DL_FIX_ADDR_MAX * 2];
	u8 index;

	__le64 patch_image_len;
	__u8 data[];
} __packed;

struct rtl_iovec {
	u8 *data;
	u32 len;
};

struct rtl_vendor_write_cmd {
	u8 type;
	__le32 addr;
	__le32 val;
} __packed;

//Extension Section IGNATURE:0x77FD0451
static const uint8_t Extension_Section_SIGNATURE[4] = { 0x51, 0x04, 0xFD, 0x77 };

static LIST_HEAD(list_secs);

static const struct {
	__u16 lmp_subver;
	__u8 id;
} project_id_to_lmp_subver[] = {
	{ ROM_LMP_8723a, 0 },
	{ ROM_LMP_8723b, 1 },
	{ ROM_LMP_8821a, 2 },
	{ ROM_LMP_8761a, 3 },
	{ ROM_LMP_8723c, 7 },
	{ ROM_LMP_8822b, 8 },   /* 8822B */
	{ ROM_LMP_8723b, 9 },	/* 8723D */
	{ ROM_LMP_8821a, 10 },	/* 8821C */
	{ ROM_LMP_8822b, 13 },	/* 8822C */
	{ ROM_LMP_8761a, 14 },	/* 8761B */
	{ ROM_LMP_8852a, 18 },	/* 8852A */
	{ ROM_LMP_8723b, 19 },  /* 8733B */
	{ ROM_LMP_8852a, 20 },	/* 8852B */
	{ ROM_LMP_8852a, 25 },	/* 8852C */
	{ ROM_LMP_8822b, 33 },  /* 8822E */
	{ ROM_LMP_8851b, 36 },  /* 8851B */
	{ ROM_LMP_8852a, 42 },  /* 8852D */
	{ ROM_LMP_8922a, 44 },  /* 8922A */
	{ ROM_LMP_8852a, 47 },  /* 8852BT */
	{ ROM_LMP_8761a, 51 },  /* 8761C */
};

enum rtk_endpoit {
	CTRL_EP = 0,
	INTR_EP = 1,
	BULK_EP = 2,
	ISOC_EP = 3
};

/* software id */
#define RTLPREVIOUS	0x00
#define RTL8822BU	0x70
#define RTL8723DU	0x71
#define RTL8821CU	0x72
#define RTL8822CU	0x73
#define RTL8761BU	0x74
#define RTL8852AU	0x75
#define RTL8733BU	0x76
#define RTL8852BU	0x77
#define RTL8852CU	0x78
#define RTL8822EU	0x79
#define RTL8851BU	0x7A
#define RTL8852DU	0x7B
#define RTL8922AU	0x7C
#define RTL8852BTU	0x7D
#define RTL8761CU	0x80
#define RTL8723CU	0x81
#define RTL8922DU	0x82

typedef struct {
	dev_data *dev_entry;
	int pipe_in, pipe_out;
	uint8_t *send_pkt;
	uint8_t *rcv_pkt;
	struct hci_command_hdr *cmd_hdr;
	struct hci_event_hdr *evt_hdr;
	struct hci_ev_cmd_complete *cmd_cmp;
	uint8_t *req_para, *rsp_para;
	uint8_t *fw_data;
	int pkt_len, fw_len;
	u8 enhanced_download;
	u8 acl_cnt;
	u16 handle;
	u16 acldata_pkt_len;
	bool bulk_mode;
} xchange_data;

typedef struct {
	uint8_t index;
	uint8_t data[PATCH_SEG_MAX];
} __attribute__ ((packed)) download_cp;

typedef struct {
	uint8_t status;
	uint8_t index;
} __attribute__ ((packed)) download_rp;

#define RTK_VENDOR_CONFIG_MAGIC 0x8723ab55
static const u8 cfg_magic[4] = { 0x55, 0xab, 0x23, 0x87 };
struct rtk_bt_vendor_config_entry {
	__le16 offset;
	uint8_t entry_len;
	uint8_t entry_data[0];
} __attribute__ ((packed));

struct rtk_bt_vendor_config {
	__le32 signature;
	__le16 data_len;
	struct rtk_bt_vendor_config_entry entry[0];
} __attribute__ ((packed));
#define BT_CONFIG_HDRLEN		sizeof(struct rtk_bt_vendor_config)

static uint8_t gEVersion = 0xFF;
static uint8_t g_key_id = 0;

static dev_data *dev_data_find(struct usb_interface *intf);
static patch_info *get_patch_entry(struct usb_device *udev);
static int load_firmware(dev_data *dev_entry, xchange_data *xdata);
static int init_xdata(xchange_data * xdata, dev_data * dev_entry);
static int check_fw_version(xchange_data *xdata, dev_data *dev_entry);
static int download_data(xchange_data * xdata);
static int send_hci_cmd(xchange_data * xdata);
static int rcv_hci_evt(xchange_data * xdata);
static uint8_t rtk_get_eversion(dev_data * dev_entry);
static int rtk_vendor_read(dev_data * dev_entry, uint8_t class);
static u8 *load_config(dev_data *dev_entry, int *length);
static int check_fw_version_v3(dev_data *dev_entry, xchange_data *xdata);

static patch_info fw_patch_table[] = {
/* { pid, lmp_sub, mp_fw_name, fw_name, config_name, chip_type } */
	{0x1724, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* RTL8723A */
	{0x8723, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AE */
	{0xA723, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AE for LI */
	{0x0723, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AE */
	{0x3394, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AE for Azurewave */

	{0x0724, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AU */
	{0x8725, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AU */
	{0x872A, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AU */
	{0x872B, 0x1200, "mp_rtl8723a_fw", "rtl8723a_fw", "rtl8723a_config", RTLPREVIOUS},	/* 8723AU */

	{0xb720, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BU */
	{0xb72A, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BU */
	{0xb728, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for LC */
	{0xb723, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */
	{0xb72B, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */
	{0xb001, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for HP */
	{0xb002, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */
	{0xb003, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */
	{0xb004, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */
	{0xb005, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE */

	{0x3410, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Azurewave */
	{0x3416, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Azurewave */
	{0x3459, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Azurewave */
	{0xE085, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Foxconn */
	{0xE08B, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Foxconn */
	{0xE09E, 0x8723, "mp_rtl8723b_fw", "rtl8723b_fw", "rtl8723b_config", RTLPREVIOUS},	/* RTL8723BE for Foxconn */

	{0xA761, 0x8761, "mp_rtl8761a_fw", "rtl8761au_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AU only */
	{0x818B, 0x8761, "mp_rtl8761a_fw", "rtl8761aw_fw", "rtl8761aw_config", RTLPREVIOUS},	/* RTL8761AW + 8192EU */
	{0x818C, 0x8761, "mp_rtl8761a_fw", "rtl8761aw_fw", "rtl8761aw_config", RTLPREVIOUS},	/* RTL8761AW + 8192EU */
	{0x8760, 0x8761, "mp_rtl8761a_fw", "rtl8761au8192ee_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AU + 8192EE */
	{0xB761, 0x8761, "mp_rtl8761a_fw", "rtl8761au_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AUV only */
	{0x8761, 0x8761, "mp_rtl8761a_fw", "rtl8761au8192ee_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AU + 8192EE for LI */
	{0x8A60, 0x8761, "mp_rtl8761a_fw", "rtl8761au8812ae_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AU + 8812AE */
	{0x3527, 0x8761, "mp_rtl8761a_fw", "rtl8761au8192ee_fw", "rtl8761a_config", RTLPREVIOUS},	/* RTL8761AU + 8814AE */

	{0x8821, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */
	{0x0821, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */
	{0x0823, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AU */
	{0x3414, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */
	{0x3458, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */
	{0x3461, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */
	{0x3462, 0x8821, "mp_rtl8821a_fw", "rtl8821a_fw", "rtl8821a_config", RTLPREVIOUS},	/* RTL8821AE */

	{0xb82c, 0x8822, "mp_rtl8822bu_fw", "rtl8822bu_fw", "rtl8822bu_config", RTL8822BU}, /* RTL8822BU */

	{0xd720, 0x8723, "mp_rtl8723du_fw", "rtl8723du_fw", "rtl8723du_config", RTL8723DU}, /* RTL8723DU */
	{0xd723, 0x8723, "mp_rtl8723du_fw", "rtl8723du_fw", "rtl8723du_config", RTL8723DU}, /* RTL8723DU */
	{0xd739, 0x8723, "mp_rtl8723du_fw", "rtl8723du_fw", "rtl8723du_config", RTL8723DU}, /* RTL8723DU */
	{0xb009, 0x8723, "mp_rtl8723du_fw", "rtl8723du_fw", "rtl8723du_config", RTL8723DU}, /* RTL8723DU */
	{0x0231, 0x8723, "mp_rtl8723du_fw", "rtl8723du_fw", "rtl8723du_config", RTL8723DU}, /* RTL8723DU for LiteOn */

	{0xb703, 0x8703, "mp_rtl8723cu_fw", "rtl8723cu_fw", "rtl8723cu_config", RTL8723CU}, /* RTL8723CU */

	{0xb820, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CU */
	{0xc820, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CU */
	{0xc821, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc823, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc824, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc825, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc827, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc025, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc024, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc030, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xb00a, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xb00e, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0xc032, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0x4000, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for LiteOn */
	{0x4001, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for LiteOn */
	{0x3529, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3530, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3532, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3533, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3538, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3539, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3558, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3559, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3581, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for Azurewave */
	{0x3540, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE */
	{0x3541, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for GSD */
	{0x3543, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CE for GSD */
	{0xc80c, 0x8821, "mp_rtl8821cu_fw", "rtl8821cu_fw", "rtl8821cu_config", RTL8821CU}, /* RTL8821CUH */

	{0xc82c, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CU */
	{0xc82e, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CU */
	{0xc81d, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CU */
	{0xd820, 0x8822, "mp_rtl8821du_fw", "rtl8821du_fw", "rtl8821du_config", RTL8822CU}, /* RTL8821DU */
	{0x053b, 0x8822, "mp_rtl8821du_fw", "rtl8821du_fw", "rtl8821du_config", RTL8822CU}, /* RTL8821DU for Epson*/

	{0xc822, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc82b, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xb00c, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xb00d, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc123, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc126, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc127, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc128, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc129, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc131, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc136, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x3549, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE for Azurewave */
	{0x3548, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE for Azurewave */
	{0xc125, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x4005, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE for LiteOn */
	{0x3051, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE for LiteOn */
	{0x18ef, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x161f, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x3053, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc547, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x3553, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0x3555, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE */
	{0xc82f, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE-VS */
	{0xc02f, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE-VS */
	{0xc03f, 0x8822, "mp_rtl8822cu_fw", "rtl8822cu_fw", "rtl8822cu_config", RTL8822CU}, /* RTL8822CE-VS */

	{0x8771, 0x8761, "mp_rtl8761b_fw", "rtl8761bu_fw", "rtl8761bu_config", RTL8761BU}, /* RTL8761BU only */
	{0x876e, 0x8761, "mp_rtl8761b_fw", "rtl8761bu_fw", "rtl8761bu_config", RTL8761BU}, /* RTL8761BUE */
	{0xa725, 0x8761, "mp_rtl8761b_fw", "rtl8725au_fw", "rtl8725au_config", RTL8761BU}, /* RTL8725AU */
	{0xa72A, 0x8761, "mp_rtl8761b_fw", "rtl8725au_fw", "rtl8725au_config", RTL8761BU}, /* RTL8725AU BT only */

	{0x885a, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AU */
	{0x8852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xa852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x2852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x385a, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x3852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x1852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x4852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x4006, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x3561, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x3562, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x588a, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x589a, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x590a, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xc125, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xe852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xb852, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xc549, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0xc127, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */
	{0x3565, 0x8852, "mp_rtl8852au_fw", "rtl8852au_fw", "rtl8852au_config", RTL8852AU}, /* RTL8852AE */

	{0xb733, 0x8723, "mp_rtl8733bu_fw", "rtl8733bu_fw", "rtl8733bu_config", RTL8733BU}, /* RTL8733BU */
	{0xb73a, 0x8723, "mp_rtl8733bu_fw", "rtl8733bu_fw", "rtl8733bu_config", RTL8733BU}, /* RTL8733BU */
	{0xf72b, 0x8723, "mp_rtl8733bu_fw", "rtl8733bu_fw", "rtl8733bu_config", RTL8733BU}, /* RTL8733BU */

	{0x8851, 0x8852, "mp_rtl8851au_fw", "rtl8851au_fw", "rtl8851au_config", RTL8852BU}, /* RTL8851AU */
	{0xa85b, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BU */
	{0xb85b, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0xb85c, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x3571, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x3570, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x3572, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x4b06, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x885b, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x886b, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x887b, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0xc559, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0xb052, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0xb152, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0xb252, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x4853, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */
	{0x1670, 0x8852, "mp_rtl8852bu_fw", "rtl8852bu_fw", "rtl8852bu_config", RTL8852BU}, /* RTL8852BE */

	{0xc85a, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CU */
	{0xc85d, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CU */
	{0x0852, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x5852, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0xc85c, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x885c, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x886c, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x887c, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x4007, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x1675, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */
	{0x3586, 0x8852, "mp_rtl8852cu_fw", "rtl8852cu_fw", "rtl8852cu_config", RTL8852CU}, /* RTL8852CE */

	{0xe822, 0x8822, "mp_rtl8822eu_fw", "rtl8822eu_fw", "rtl8822eu_config", RTL8822EU}, /* RTL8822EU */
	{0xa82a, 0x8822, "mp_rtl8822eu_fw", "rtl8822eu_fw", "rtl8822eu_config", RTL8822EU}, /* RTL8822EU */
	{0x20f5, 0x8822, "mp_rtl8822eu_fw", "rtl8822eu_fw", "rtl8822eu_config", RTL8822EU}, /* RTL8822EU */

	{0xb850, 0x8851, "mp_rtl8851bu_fw", "rtl8851bu_fw", "rtl8851bu_config", RTL8851BU}, /* RTL8851BE */
	{0xb851, 0x8851, "mp_rtl8851bu_fw", "rtl8851bu_fw", "rtl8851bu_config", RTL8851BU}, /* RTL8851BU */

	{0xd85a, 0x8852, "mp_rtl8852du_fw", "rtl8852du_fw", "rtl8852du_config", RTL8852DU}, /* RTL8852DU */
	{0xd85b, 0x8852, "mp_rtl8852du_fw", "rtl8852du_fw", "rtl8852du_config", RTL8852DU}, /* RTL8852DU */

	{0x892a, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AU */
	{0x8922, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */
	{0xa890, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */
	{0xa891, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */
	{0xa892, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */
	{0xd922, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */
	{0xb85f, 0x8922, "mp_rtl8922au_fw", "rtl8922au_fw", "rtl8922au_config", RTL8922AU}, /* RTL8922AE */

	{0xc852, 0x8852, "mp_rtl8852btu_fw", "rtl8852btu_fw", "rtl8852btu_config", RTL8852BTU}, /* RTL8852BTU */
	{0xc853, 0x8852, "mp_rtl8852btu_fw", "rtl8852btu_fw", "rtl8852btu_config", RTL8852BTU}, /* RTL8852BTU */
	{0x8520, 0x8852, "mp_rtl8852btu_fw", "rtl8852btu_fw", "rtl8852btu_config", RTL8852BTU}, /* RTL8852BTE */

	{0xc761, 0x8761, "mp_rtl8761cu_fw", "rtl8761cu_mx_fw", "rtl8761cu_mx_config", RTL8761CU}, /* RTL8761CU */
	{0x890d, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	{0x891d, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	{0x892d, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	{0x2850, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	{0xa82d, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	{0x882d, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */
	//{0x20f5, 0x8922, "mp_rtl8922du_fw", "rtl8922du_fw", "rtl8922du_config", RTL8922DU}, /* RTL8922DU */

/* NOTE: must append patch entries above the null entry */
	{0, 0, NULL, NULL, NULL, 0}
};

static LIST_HEAD(dev_data_list);

static void util_hexdump(const u8 *buf, size_t len)
{
	static const char hexdigits[] = "0123456789abcdef";
	char str[16 * 3];
	size_t i;

	if (!buf || !len)
		return;

	for (i = 0; i < len; i++) {
		str[((i % 16) * 3)] = hexdigits[buf[i] >> 4];
		str[((i % 16) * 3) + 1] = hexdigits[buf[i] & 0xf];
		str[((i % 16) * 3) + 2] = ' ';
		if ((i + 1) % 16 == 0) {
			str[16 * 3 - 1] = '\0';
			RTKBT_DBG("%s", str);
		}
	}

	if (i % 16 > 0) {
		str[(i % 16) * 3 - 1] = '\0';
		RTKBT_DBG("%s", str);
	}
}

static int __send_hci_cmd_bulk_mode(struct usb_device *udev, u8 *data, int len)
{
	unsigned int pipe;
	int err = -EINVAL;
	u8 *mem;
	int length;
	u16 opcode;

	length = len + 1;
	mem = kzalloc(length, GFP_ATOMIC);
	if (!mem)
		return -ENOMEM;
	opcode = get_unaligned_le16(data);
	mem[0] = 0x01; /* HCI_COMMAND_PKT (0x01) */
	memcpy(mem + 1, data, len);
	pipe = usb_sndbulkpipe(udev, BULK_EP);
	err = usb_bulk_msg(udev, pipe, mem, length, NULL, 1000);
	kfree(mem);

	if (err < 0)
		RTKBT_ERR("%s: send cmd (%04x) err (%d)", __func__, opcode, err);
	else
		RTKBT_INFO("%s: succeed to send cmd (%04x)", __func__, opcode);

	return err;
}

int __rtk_send_hci_cmd(struct usb_device *udev, u8 *buf, u16 size)
{
	int result;
	unsigned int pipe = usb_sndctrlpipe(udev, 0);
	struct usb_interface *intf;

	util_hexdump(buf, size);

	if (udev->actconfig) {
		intf = udev->actconfig->interface[0];
		if (intf->cur_altsetting->desc.bAlternateSetting != 0)
			return __send_hci_cmd_bulk_mode(udev, buf, size);
	}

	result = usb_control_msg(udev, pipe, 0, USB_TYPE_CLASS, 0, 0,
				 buf, size, 1000); /* 1000 msecs */

	if (result < 0)
		RTKBT_ERR("%s: Couldn't send hci cmd, err %d",
			  __func__, result);

	return result;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
static inline struct inode *file_inode(const struct file *f)
{
	return f->f_path.dentry->d_inode;
}
#endif

static int config_lists_init(void)
{
	INIT_LIST_HEAD(&list_configs);
	INIT_LIST_HEAD(&list_extracfgs);

	return 0;
}

static void config_lists_free(void)
{
	struct list_head *iter;
	struct list_head *tmp;
	struct list_head *head;
	struct cfg_list_item *n;

	if (!list_empty(&list_extracfgs))
		list_splice_tail(&list_extracfgs, &list_configs);
	head = &list_configs;
	list_for_each_safe(iter, tmp, head) {
		n = list_entry(iter, struct cfg_list_item, list);
		if (n) {
			list_del(&n->list);
			kfree(n);
		}
	}

	INIT_LIST_HEAD(&list_configs);
	INIT_LIST_HEAD(&list_extracfgs);
}

static void list_secs_free(xchange_data *xdata)
{
	//struct rtl_subsection_v3 *entry, *tmp;
	struct list_head *pos;
	struct list_head *next;
	struct rtl_subsection_v3 *entry;

	if (xdata && is_v3_fw(xdata->dev_entry->fw_type)) {
		//list_for_each_entry_safe(entry, tmp, &list_secs, list) {
		list_for_each_safe(pos, next, &list_secs) {
			entry = list_entry(pos, struct rtl_subsection_v3, list);
			list_del(&entry->list);
			if (entry->patch_image_buf)
				vfree(entry->patch_image_buf);
			if (entry->cfg_buf)
				vfree(entry->cfg_buf);
			kfree(entry);
		}
	}
}

static void line_process(char *buf, int len)
{
	char *argv[32];
	int argc = 0;
	unsigned long offset;
	u8 l;
	u8 i = 0;
	char *ptr = buf;
	char *head = buf;
	struct cfg_list_item *item;

	while ((ptr = strsep(&head, ", \t")) != NULL) {
		if (!ptr[0])
			continue;
		argv[argc++] = ptr;
		if (argc >= 32) {
			RTKBT_WARN("%s: Config item is too long", __func__);
			break;
		}
	}

	if (argc < 4) {
		RTKBT_WARN("%s: Invalid Config item, ignore", __func__);
		return;
	}

	offset = simple_strtoul(argv[0], NULL, 16);
	offset = offset | (simple_strtoul(argv[1], NULL, 16) << 8);
	l = (u8)simple_strtoul(argv[2], NULL, 16);
	if (l != (u8)(argc - 3)) {
		RTKBT_ERR("invalid len %u", l);
		return;
	}

	item = kzalloc(sizeof(*item) + l, GFP_KERNEL);
	if (!item) {
		RTKBT_WARN("%s: Cannot alloc mem for item, %04lx, %u", __func__,
			   offset, l);
		return;
	}

	item->offset = (u16)offset;
	item->len = l;
	for (i = 0; i < l; i++)
		item->data[i] = (u8)simple_strtoul(argv[3 + i], NULL, 16);
	list_add_tail(&item->list, &list_extracfgs);
}

static void store_apcf_cfg(u8 *buf, int len, struct list_head *head)
{
	u8 *p = NULL;
	struct cfg_apcf_item *n = NULL;

	if (!buf || !len || !head)
		return;

	p = vmalloc(sizeof(*n) + len);
	if (!p)
		return;
	n = (struct cfg_apcf_item *)p;
	INIT_LIST_HEAD(&n->list);
	memcpy(n->data, buf, len);
	n->len = len;

	mutex_lock(&cfg_lock);
	list_add_tail(&n->list, head);
	mutex_unlock(&cfg_lock);
}

void drain_apcf_cfg(struct list_head *head, int type)
{
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct list_head *hl = NULL;
	struct cfg_apcf_item *c;

	switch (type) {
	case CFG_TYPE_APCF_FILTER:
		hl = &apcf_filters;
		break;
	case CFG_TYPE_APCF_WAKEUP:
		hl = &apcf_wakeups;
		break;
	default:
		mutex_lock(&cfg_lock);
		list_for_each_safe(pos, next, &apcf_filters) {
			c = list_entry(pos, struct cfg_apcf_item, list);
			list_del(&c->list);
			vfree(c);
		}
		list_for_each_safe(pos, next, &apcf_wakeups) {
			c = list_entry(pos, struct cfg_apcf_item, list);
			list_del(&c->list);
			vfree(c);
		}
		mutex_unlock(&cfg_lock);
		return;
	}

	mutex_lock(&cfg_lock);
	list_for_each_safe(pos, next, hl) {
		c = list_entry(pos, struct cfg_apcf_item, list);
		list_del(&c->list);
		if (head)
			list_add_tail(&c->list, head);
		else
			vfree(c);
	}
	mutex_unlock(&cfg_lock);
}

static void line_process_apcf_filter(u8 *buf, int len)
{
	store_apcf_cfg(buf, len, &apcf_filters);
}

static void line_process_apcf_wakeup(u8 *buf, int len)
{
	store_apcf_cfg(buf, len, &apcf_wakeups);
}

static void config_process(u8 *buff, int len, int type)
{
	char *head = (void *)buff;
	char *ptr = (void *)buff;

	while ((ptr = strsep(&head, "\n\r")) != NULL) {
		if (!ptr[0])
			continue;
		switch (type) {
		case CFG_TYPE_BT:
			line_process(ptr, strlen(ptr) + 1);
			break;
		case CFG_TYPE_APCF_FILTER:
			line_process_apcf_filter(ptr, strlen(ptr) + 1);
			break;
		case CFG_TYPE_APCF_WAKEUP:
			line_process_apcf_wakeup(ptr, strlen(ptr) + 1);
			break;
		}
	}
}

void config_file_proc(const char *path, int type)
{
	int size;
	int rc;
	struct file *file;
	u8 tbuf[256];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	loff_t pos = 0;
#endif

	file = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(file))
		return;

	if (!S_ISREG(file_inode(file)->i_mode))
		return;
	size = i_size_read(file_inode(file));
	if (size <= 0)
		return;

	memset(tbuf, 0, sizeof(tbuf));
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	rc = kernel_read(file, tbuf, size, &pos);
#else
	rc = kernel_read(file, 0, tbuf, size);
#endif
	fput(file);
	if (rc != size) {
		if (rc >= 0)
			rc = -EIO;
		return;
	}

	tbuf[rc++] = '\n';
	tbuf[rc++] = '\0';
	config_process(tbuf, rc, type);
}

int patch_add(struct usb_interface *intf)
{
	dev_data *dev_entry;
	struct usb_device *udev;

	RTKBT_DBG("patch_add");
	dev_entry = dev_data_find(intf);
	if (NULL != dev_entry) {
		return -1;
	}

	udev = interface_to_usbdev(intf);
#ifdef BTUSB_RPM
	RTKBT_DBG("auto suspend is enabled");
	usb_enable_autosuspend(udev);
	pm_runtime_set_autosuspend_delay(&(udev->dev), 2000);
#else
	RTKBT_DBG("auto suspend is disabled");
	usb_disable_autosuspend(udev);
#endif

	dev_entry = kzalloc(sizeof(dev_data), GFP_KERNEL);
	if (!dev_entry)
		return -1;
	dev_entry->intf = intf;
	dev_entry->udev = udev;
	dev_entry->patch_entry = get_patch_entry(udev);
	if (NULL == dev_entry->patch_entry) {
		kfree(dev_entry);
		return -1;
	}
	list_add(&dev_entry->list_node, &dev_data_list);

	/* Should reset the gEVersion to 0xff, otherwise the stored gEVersion
	 * would cause rtk_get_eversion() returning previous gEVersion if
	 * change to different ECO chip.
	 * This would cause downloading wrong patch, and the controller can't
	 * work. */
	RTKBT_DBG("%s: Reset gEVersion to 0xff", __func__);
	gEVersion = 0xff;

	return 0;
}

void patch_remove(struct usb_interface *intf)
{
	dev_data *dev_entry;
	struct usb_device *udev;

	udev = interface_to_usbdev(intf);
#ifdef BTUSB_RPM
	usb_disable_autosuspend(udev);
#endif

	dev_entry = dev_data_find(intf);
	if (NULL == dev_entry) {
		return;
	}

	RTKBT_DBG("patch_remove");
	list_del(&dev_entry->list_node);
	kfree(dev_entry);
}

static int send_reset_command(xchange_data *xdata)
{
	int ret_val;

	RTKBT_DBG("HCI reset.");

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_OP_RESET);
	xdata->cmd_hdr->plen = 0;
	xdata->pkt_len = CMD_HDR_LEN;

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("failed to send hci cmd.");
		return ret_val;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("failed to recv hci event.");
		return ret_val;
	}

	return 0;
}

static inline int get_max_patch_size(u8 chip_type)
{
	int max_patch_size = 0;

	switch (chip_type) {
	case RTLPREVIOUS:
		max_patch_size = 24 * 1024;
		break;
	case RTL8822BU:
		max_patch_size = 25 * 1024;
		break;
	case RTL8723DU:
	case RTL8723CU:
	case RTL8822CU:
	case RTL8761BU:
	case RTL8821CU:
		max_patch_size = 40 * 1024;
		break;
	case RTL8852AU:
		max_patch_size = 0x114D0 + 529; /* 69.2KB */
		break;
	case RTL8733BU:
		max_patch_size = 0xC4Cf + 529; /* 49.2KB */
		break;
	case RTL8852BU:
	case RTL8851BU:
		max_patch_size = 0x104D0 + 529;  /* 65KB */
		break;
	case RTL8852CU:
		max_patch_size = 0x130D0 + 529; /* 76.2KB */
		break;
	case RTL8822EU:
		max_patch_size = 0x24620 + 529;    /* 145KB */
		break;
	case RTL8852DU:
		max_patch_size = 0x20D90 + 529;  /* 131KB */
		break;
	case RTL8922AU:
		max_patch_size = 0x23810 + 529;  /* 142KB */
		break;
	case RTL8852BTU:
		max_patch_size = 0x27E00 + 529; /* 159.5KB */
		break;
	case RTL8761CU:
		max_patch_size = 1024 * 1024; /* 1MB */
		break;
	default:
		max_patch_size = 40 * 1024;
		break;
	}

	return max_patch_size;
}

static int rtk_vendor_write(dev_data * dev_entry)
{
	int ret_val;

	xchange_data *xdata = NULL;
	unsigned char cmd_buf[] = {0x31, 0x90, 0xd0, 0x29, 0x80, 0x00, 0x00,
	0x00, 0x00};

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (NULL == xdata) {
		RTKBT_DBG("NULL == xdata");
		return -1;
	}

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val) {
		RTKBT_ERR("failed to init xdata");
		goto end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_VENDOR_WRITE_CMD);
	xdata->cmd_hdr->plen = 9;
	memcpy(xdata->send_pkt, &(xdata->cmd_hdr->opcode), 2);
	memcpy(xdata->send_pkt+2, &(xdata->cmd_hdr->plen), 1);

	memcpy(xdata->send_pkt+3, cmd_buf, sizeof(cmd_buf));

	xdata->pkt_len = CMD_HDR_LEN + 9;

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to send HCI command.", __func__);
		goto end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to receive HCI event.", __func__);
		goto end;
	}

	ret_val = 0;
end:
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	return ret_val;
}


static int check_fw_chip_ver(dev_data * dev_entry, xchange_data * xdata)
{
	int ret_val;
	uint16_t chip = 0;
	uint16_t chip_ver = 0;
	uint16_t lmp_subver, hci_rev;
	patch_info *patch_entry;
	struct hci_rp_read_local_version *read_ver_rsp;
	struct sk_buff *skb = NULL;
	u16 opcode;

	if (!dev_entry || !xdata)
		return -EINVAL;

	patch_entry = dev_entry->patch_entry;
	if (!patch_entry)
		return -ENODEV;

	opcode = HCI_VENDOR_READ_CHIP_ID;
	skb = rtk_hci_cmd_sync(xdata->dev_entry, opcode, 0, NULL, 1000);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("send %04x error (%lu)", opcode, PTR_ERR(skb));
		return -EIO;
	}

	/* The ID for 8922D is 0x37 */
	if (!skb->data[0] && skb->data[1] >= 0x37)
		goto check_ver;

	chip = rtk_vendor_read(dev_entry, READ_CHIP_TYPE);
	if(chip == 0x8822) {
		chip_ver = rtk_vendor_read(dev_entry, READ_CHIP_VER);
		if(chip_ver == 0x000e) {
			return 0;
		}
	}

check_ver:
	ret_val = check_fw_version(xdata, dev_entry);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to get Local Version Information");
		return ret_val;

	} else if (ret_val > 0) {
		RTKBT_DBG("Firmware already exists");
		/* Patch alread exists, just return */
		if (gEVersion == 0xff) {
			RTKBT_DBG("global_version is not set, get it!");
			gEVersion = rtk_get_eversion(dev_entry);
		}
		return ret_val;
	} else {
		read_ver_rsp = (struct hci_rp_read_local_version *)(xdata->rsp_para);
		lmp_subver = le16_to_cpu(read_ver_rsp->lmp_subver);
		hci_rev = le16_to_cpu(read_ver_rsp->hci_rev);
		if (lmp_subver == 0x8852 && hci_rev == 0x000d)
			ret_val = rtk_vendor_write(dev_entry);
	}

	return ret_val;
}

static void *rtl_iov_pull_data(struct rtl_iovec *iov, u32 len)
{
	void *data = iov->data;

	if (iov->len < len)
		return NULL;

	iov->data += len;
	iov->len  -= len;

	return data;
}

static int btrtl_vendor_write_mem(dev_data * dev_entry, u32 addr, u32 val)
{
	int ret_val;
	xchange_data *xdata = NULL;
	struct rtl_vendor_write_cmd cp;

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (!xdata) {
		RTKBT_ERR("failed to init xdata");
		return -ENOMEM;
	}

	cp.type = 0x21;
	cp.addr = cpu_to_le32(addr);
	cp.val = cpu_to_le32(val);

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val) {
		RTKBT_ERR("%s: failed to init xdata", __func__);
		goto end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_VENDOR_WRITE_CMD);
	xdata->cmd_hdr->plen = sizeof(cp);
	memcpy(xdata->req_para, &cp, sizeof(cp));
	xdata->pkt_len = CMD_HDR_LEN + sizeof(cp);

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to send HCI command.", __func__);
		goto end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to receive HCI event.", __func__);
		goto end;
	}

	ret_val = 0;
end:
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	return ret_val;
}

static void btrtl_insert_ordered_subsec_v3(struct rtl_subsection_v3 *image)
{
	struct list_head *pos;
	struct list_head *next;
	struct rtl_subsection_v3 *node;

	list_for_each_safe(pos, next, &list_secs) {
		node = list_entry(pos, struct rtl_subsection_v3, list);

		if (node->image_id > image->image_id) {
			__list_add(&image->list, pos->prev, pos);
			return;
		}

		if (node->image_id == image->image_id &&
		    node->index > image->index) {
			__list_add(&image->list, pos->prev, pos);
			return;
		}
	}
	__list_add(&image->list, pos->prev, pos);
}

static struct rtl_subsection_v3 *btrtl_reverse_search_subsec(void)
{
	struct rtl_subsection_v3 *sec = NULL;

	list_for_each_entry_reverse(sec, &list_secs, list) {
		RTKBT_DBG("addr: 0x%08x, value: 0x%08x",
			  sec->fix[DL_FIX_SEC_HDR_ADDR].addr,
			  sec->fix[DL_FIX_SEC_HDR_ADDR].value);
		/* Check sec hdr in section */
		if (!sec->fix[DL_FIX_SEC_HDR_ADDR].value ||
		    !sec->fix[DL_FIX_SEC_HDR_ADDR].addr ||
		    sec->fix[DL_FIX_SEC_HDR_ADDR].addr == 0xffffffff)
			continue;
		return sec;
	}

	return NULL;
}

static int rtk_enhanced_download_mode_enable(xchange_data *xdata)
{
	u16 opcode = HCI_VENDOR_ENHANCED_DOWNLOAD_MODE;
	struct hci_rp_enhanced_download_mode *ev;
	struct sk_buff *skb;
	u8 val = 1;
	int ret = -EINVAL;

	skb = rtk_hci_cmd_sync(xdata->dev_entry, opcode, 1, &val, 1000);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("send %04x error (%lu)", opcode, PTR_ERR(skb));
		return -EIO;
	}
	if (skb->len != sizeof(*ev)) {
		RTKBT_ERR("got invalid cmd complete, %u %lu", skb->len,
				sizeof(*ev));
		goto err;
	}
	ev = (struct hci_rp_enhanced_download_mode *)skb->data;
	if (ev->status) {
		RTKBT_ERR("%s: status 0x%02x", __func__, ev->status);
		goto err;
	}
	if (ev->rom_patched) {
		RTKBT_INFO("%s: rom has been patched", __func__);
		/* FIXME: On FPGA, the rom_patched is true, but we
		 * continue the acl download
		 */
	}
	xdata->handle = le16_to_cpu(ev->handle);
	xdata->acl_cnt = ev->acl_cnt;
	xdata->acldata_pkt_len = le16_to_cpu(ev->acldata_pkt_len);
	kfree_skb(skb);

	xdata->dev_entry->enh_dl_enabled = 1;

	RTKBT_INFO("Enhanced download mode enabled, handle %04x, acl %u*%u",
		   xdata->handle, xdata->acl_cnt, xdata->acldata_pkt_len);

	return 0;
err:
	kfree_skb(skb);
	return ret;
}

static int download_patch_sections(dev_data *dev_entry, xchange_data *xdata)
{
	struct rtl_subsection_v3 *sec, *tmp;
	u8 *buf = NULL;
	u8 *fw_data;
	u8 i;
	int patch_size = 0;
	int ret_val = -1;
	struct sk_buff *skb = NULL;
	struct hci_dl_rsp {
		u8 status;
		u8 index;
		u8 error;
	} __packed *rp;

	switch (dev_entry->patch_entry->chip_type) {
	case RTL8922DU:
	case RTL8852CU:
		rtk_enhanced_download_mode_enable(xdata);
		break;
	default:
		break;
	}

	list_for_each_entry_safe(sec, tmp, &list_secs, list) {
		RTKBT_DBG("RTL: image (%04x:%02x)", sec->image_id, sec->index);

		for (i = DL_FIX_CI_ID; i < DL_FIX_ADDR_MAX; i++) {
			if (!sec->fix[i].addr ||
			    sec->fix[i].addr == 0xffffffff) {
				RTKBT_WARN("%s: no need to write addr 0x%08x",
					   __func__, sec->fix[i].addr);
				continue;
			}
			RTKBT_DBG("%s: write addr and val, 0x%08x, 0x%08x",
				  __func__, sec->fix[i].addr,
				  sec->fix[i].value);
			if (btrtl_vendor_write_mem(dev_entry, sec->fix[i].addr,
						   sec->fix[i].value)) {
				RTKBT_ERR("RTL: write reg failed");
				goto exit_fail;
			}
			continue;
		}

		xdata->fw_len = sec->patch_image_len + sec->cfg_len;
		/* if (&tmp->list == &list_secs) {
		 * 	if (config_len)
		 * 		xdata->fw_len = sec->patch_image_len + config_len;
		 * }
		 */

		buf = vzalloc(xdata->fw_len);
		if (!buf) {
			RTKBT_ERR("Couldn't alloc buf for image data");
			goto exit_fail;
		}
		memcpy(buf, sec->patch_image_buf, sec->patch_image_len);
		if (sec->cfg_len > 0)
			memcpy(buf + sec->patch_image_len, sec->cfg_buf,
			       sec->cfg_len);

		fw_data = xdata->fw_data;
		xdata->fw_data = buf;

		RTKBT_DBG("patch image (%04x:%02x). len: %d",
			  sec->image_id, sec->index, xdata->fw_len);
		RTKBT_DBG("buf %p, image buf %p, len %u", buf,
			  sec->patch_image_buf, sec->patch_image_len);

		/* FIXME: What if length is uncertain ? */
		patch_size += xdata->fw_len;

		ret_val = download_data(xdata);

		vfree(buf);
		xdata->fw_data = fw_data;
		if (ret_val < 0) {
			RTKBT_ERR("download_data failed, err %d", ret_val);
			goto exit_fail;
		}

		if (sec->list.next != &list_secs &&
		    sec->image_id == tmp->image_id)
			continue;

		if (dev_entry->fw_type == FW_TYPE_V3_1)
			continue;

		i = 0x80;
		skb = rtk_hci_cmd_sync(xdata->dev_entry, DOWNLOAD_OPCODE,
				       1, &i, 200);
		if (IS_ERR(skb) || !skb) {
			ret_val = -EIO;
			RTKBT_ERR("%s: failed to issue last %04x, %ld",
				  __func__, DOWNLOAD_OPCODE, PTR_ERR(skb));
			skb = NULL;
			goto exit_fail;
		}
		rp = (void *)skb->data;
		if (rp->error == 0x02) {
			/* verification failure */
			ret_val = -EFAULT;
			goto exit_fail;
		}

		kfree_skb(skb);
		skb = NULL;
	}
	ret_val = 0;

 exit_fail:
	if (skb)
		kfree_skb(skb);
	return ret_val;
}

static int btrtl_finalize_download(dev_data *dev_entry)
{
	u8 params[2] = {
		0x03, 0xb2
	};
	struct sk_buff *skb;
	u16 opcode;
	u32 len;
	const struct firmware *fw;
	int ret;
	char *str;
	char *p;
	char tmp_name[32];
	char filename[64];
	u8 *buf;

	opcode = HCI_VENDOR_WDG_RESET;
	len = 2;
	if (dev_entry->opcode == RTL_PATCH_V3_1) {
		opcode = 0xfc20;
		params[0] = 0x80;
		len = 1;
	}
	skb = rtk_hci_cmd_sync(dev_entry, opcode, len,
			       params, 1000);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("%s: watchdog reset error %lu", __func__,
			  PTR_ERR(skb));
		return -EIO;
	}
	RTKBT_INFO("%s: status %02x", __func__, skb->data[0]);
	kfree_skb(skb);

	if (dev_entry->opcode == RTL_PATCH_V3_1)
		goto done;

	str = dev_entry->patch_entry->config_name;
	len = strlen(str);
	if (len > sizeof(tmp_name) - 1)
		len = sizeof(tmp_name) - 1;
	memcpy(tmp_name, str, len);
	tmp_name[len] = '\0';

	str = tmp_name;
	p = strsep(&str, ".");
	ret = snprintf(filename, sizeof(filename), "%s_pta", p);
	if (str)
		snprintf(filename + ret, sizeof(filename) - ret, ".%s", str);

	ret = request_firmware(&fw, filename, &dev_entry->udev->dev);
	if (ret < 0) {
		RTKBT_WARN("%s: no PTA file", filename);
		goto done;
	}

	buf = kzalloc(fw->size + 1, GFP_KERNEL);
	if (!buf) {
		RTKBT_ERR("%s: failed to alloc buf for PTA file", __func__);
		release_firmware(fw);
		goto done;
	}
	buf[0] = 0x07;
	memcpy(buf + 1, fw->data, fw->size);
	skb = rtk_hci_cmd_sync(dev_entry, 0xfd82, fw->size + 1, buf, 1000);
	kfree(buf);
	release_firmware(fw);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("%s: failed to load PTA", __func__);
		goto done;
	}
	RTKBT_INFO("%s: succeed to load PTA, (%u)", __func__, skb->data[0]);
	kfree_skb(skb);

done:
	return 0;
}

static int rtl_wait_fw_ready_evt(xchange_data *xdata)
{
	int ret;
	int rcv_len;
	int count = 5;
	u8 *p;
	struct usb_device *udev = xdata->dev_entry->udev;

	if (!udev)
		return -EINVAL;

	while (count-- > 0) {
		if (xdata->bulk_mode)
			ret = usb_bulk_msg(udev, usb_rcvbulkpipe(udev, 0x02),
					   (void *)(xdata->rcv_pkt), PKT_LEN,
					   &rcv_len, 8000);
		else
			ret = usb_interrupt_msg(udev, xdata->pipe_in,
						(void *)(xdata->rcv_pkt), PKT_LEN,
						&rcv_len, 8000);
		if (ret < 0) {
			RTKBT_ERR("%s; no usb intr msg for hci event, err %d",
				  __func__, ret);
			return ret;
		}
		if (xdata->bulk_mode && rcv_len > 1)
			/* Remove the first byte (pkt type) */
			memmove(xdata->rcv_pkt, xdata->rcv_pkt + 1,
				rcv_len - 1);
		p = xdata->rcv_pkt + EVT_HDR_LEN;
		if (!ret && xdata->evt_hdr->evt == 0xff && *p == 0x77)
			return 0;
	}

	return -ETIMEDOUT;
}

static int rtl_download_firmware_v3(dev_data * dev_entry, xchange_data *xdata)
{
	struct rtl_subsection_v3 *subsec;
	int ret = -1;
	u32 fix_value;
	struct sk_buff *skb;

	if (dev_entry->fw_type == FW_TYPE_V3_2) {
		skb = rtk_hci_cmd_sync(dev_entry, HCI_VENDOR_WRITE_TB, 0,
				       NULL, 1000);
		if (IS_ERR(skb) || !skb) {
			RTKBT_ERR("%s: write tb error %lu", __func__,
				  PTR_ERR(skb));
			return -EIO;
		}
		RTKBT_INFO("%s: status %02x", __func__, skb->data[0]);
		if (skb->data[0] == 0x03) {
			kfree_skb(skb);
			ret = rtl_wait_fw_ready_evt(xdata);
			goto done;
		}
		kfree_skb(skb);
	}

	/* Download each patch section */
	ret = download_patch_sections(dev_entry, xdata);
	if(ret < 0)
		goto exit_fail;

	/* Security check, there is no need for new v3 fw */
	if (dev_entry->fw_type == FW_TYPE_V3_1) {
		subsec = btrtl_reverse_search_subsec();
		if (subsec) {
			fix_value = subsec->fix[DL_FIX_PATCH_ADDR].value +
					subsec->patch_image_len -
					subsec->fix[DL_FIX_SEC_HDR_ADDR].value;
			RTKBT_DBG("image_id %04x, index %02x, fix value %08x",
				  subsec->image_id, subsec->index, fix_value);
			ret = btrtl_vendor_write_mem(dev_entry,
					subsec->fix[DL_FIX_PATCH_ADDR].addr,
					fix_value);
			if (ret) {
				RTKBT_ERR("security check failed");
				goto exit_fail;

			}
		}
	}

	ret = btrtl_finalize_download(dev_entry);
	if(ret < 0)
		goto exit_fail;

done:
	RTKBT_DBG("firmware download success");
	return ret;

exit_fail:
	RTKBT_DBG("firmware download failure");
	return ret;
}

int download_patch(struct usb_interface *intf)
{
	dev_data *dev_entry;
	patch_info *pinfo;
	xchange_data *xdata = NULL;
	uint8_t *fw_buf;
	int ret_val = 0;
	int max_patch_size = 0;

	RTKBT_DBG("download_patch start");
	dev_entry = dev_data_find(intf);
	if (NULL == dev_entry) {
		ret_val = -1;
		RTKBT_ERR("NULL == dev_entry");
		goto patch_end;
	}

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (NULL == xdata) {
		ret_val = -1;
		RTKBT_DBG("NULL == xdata");
		goto patch_end;
	}

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val) {
		RTKBT_ERR("failed to init xdata");
		goto patch_end;
	}

	ret_val = check_fw_chip_ver(dev_entry, xdata);
	if (ret_val != 0 )
		goto patch_end;

	xdata->fw_len = load_firmware(dev_entry, xdata);
	if (xdata->fw_len <= 0) {
		RTKBT_ERR("load firmware failed!");
		ret_val = -1;
		goto patch_end;
	}

	if (is_v3_fw(dev_entry->fw_type)) {
		/* FIXME: What is the maximum patch size */
		if (xdata->fw_len <= 1000000) {
			ret_val = rtl_download_firmware_v3(dev_entry, xdata);
			if (ret_val < 0) {
				RTKBT_ERR("download firmware v3 err %d",
					  ret_val);
				goto patch_end;
			}
			if (dev_entry->fw_type == FW_TYPE_V3_2)
				ret_val = check_fw_version_v3(dev_entry, xdata);
			goto patch_end;
		} else
			goto patch_end;
	}

	fw_buf = xdata->fw_data;

	pinfo = dev_entry->patch_entry;
	if (!pinfo) {
		RTKBT_ERR("%s: No patch entry", __func__);
		ret_val = -1;
		goto patch_fail;
	}
	max_patch_size = get_max_patch_size(pinfo->chip_type);
	if (xdata->fw_len > max_patch_size) {
		RTKBT_ERR("FW/CONFIG total length larger than allowed %d",
			  max_patch_size);
		ret_val = -1;
		goto patch_fail;
	}

	ret_val = download_data(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("download_data failed, err %d", ret_val);
		goto patch_fail;
	}

	ret_val = check_fw_version(xdata, dev_entry);
	if (ret_val <= 0) {
		RTKBT_ERR("%s: Read Local Version Info failure after download",
			  __func__);
		ret_val = -1;
		goto patch_fail;
	}

	ret_val = 0;
patch_fail:
	vfree(fw_buf);
patch_end:
	/* Make sure all the section nodes freed */
	list_secs_free(xdata);
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	RTKBT_DBG("Rtk patch end %d", ret_val);
	return ret_val;
}

#ifdef RTKBT_SWITCH_PATCH
/* @return:
 * -1: error
 * 0: download patch successfully
 * >0: patch already exists  */
int download_special_patch(struct usb_interface *intf, const char *special_name)
{
	dev_data *dev_entry;
	patch_info *pinfo;
	xchange_data *xdata = NULL;
	uint8_t *fw_buf;
	int result;
	char name1[64];
	char *origin_name1;
	char name2[64];
	char *origin_name2;
	int max_patch_size = 0;

	RTKBT_DBG("Download LPS Patch start");
	dev_entry = dev_data_find(intf);
	if (!dev_entry) {
		RTKBT_ERR("No Patch found");
		return -1;
	}

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (!xdata) {
		RTKBT_ERR("Couldn't alloc xdata");
		return -1;
	}

	result = init_xdata(xdata, dev_entry);
	if (result) {
		RTKBT_ERR("failed to init xdata");
		goto patch_end;
	}

	result = check_fw_version(xdata, dev_entry);
	if (result < 0) {
		RTKBT_ERR("Failed to get Local Version Information");
		goto patch_end;

	} else if (result > 0) {
		RTKBT_DBG("Firmware already exists");
		/* Patch alread exists, just return */
		if (gEVersion == 0xff) {
			RTKBT_DBG("global_version is not set, get it!");
			gEVersion = rtk_get_eversion(dev_entry);
		}
		goto patch_end;
	}
	memset(name1, 0, sizeof(name1));
	memset(name2, 0, sizeof(name2));
	origin_name1 = dev_entry->patch_entry->patch_name;
	origin_name2 = dev_entry->patch_entry->config_name;
	memcpy(name1, special_name, strlen(special_name));
	strncat(name1, origin_name1, sizeof(name1) - 1 - strlen(special_name));
	memcpy(name2, special_name, strlen(special_name));
	strncat(name2, origin_name2, sizeof(name2) - 1 - strlen(special_name));
	dev_entry->patch_entry->patch_name = name1;
	dev_entry->patch_entry->config_name = name2;
	RTKBT_INFO("Loading %s and %s", name1, name2);
	xdata->fw_len = load_firmware(dev_entry, xdata);
	dev_entry->patch_entry->patch_name = origin_name1;
	dev_entry->patch_entry->config_name = origin_name2;
	if (xdata->fw_len <= 0) {
		result = -1;
		RTKBT_ERR("load firmware failed!");
		goto patch_end;
	}

	fw_buf = xdata->fw_data;

	pinfo = dev_entry->patch_entry;
	/*
	if (!pinfo) {
		RTKBT_ERR("%s: No patch entry", __func__);
		result = -1;
		goto patch_fail;
	}
	*/
	max_patch_size = get_max_patch_size(pinfo->chip_type);
	if (xdata->fw_len > max_patch_size) {
		result = -1;
		RTKBT_ERR("FW/CONFIG total length larger than allowed %d",
			  max_patch_size);
		goto patch_fail;
	}

	result = download_data(xdata);
	if (result < 0) {
		RTKBT_ERR("download_data failed, err %d", result);
		goto patch_fail;
	}

	result = check_fw_version(xdata, dev_entry);
	if (result <= 0) {
		RTKBT_ERR("%s: Read Local Version Info failure after download",
			  __func__);
		result = -1;
		goto patch_fail;
	}

	result = 0;

patch_fail:
	kfree(fw_buf);
patch_end:
	if (xdata->send_pkt)
		kfree(xdata->send_pkt);
	if (xdata->rcv_pkt)
		kfree(xdata->rcv_pkt);
	kfree(xdata);
	RTKBT_DBG("Download LPS Patch end %d", result);

	return result;
}
#endif

int setup_btrealtek_flag(struct usb_interface *intf, struct hci_dev *hdev)
{
	dev_data *dev_entry;
	patch_info *pinfo;
	int ret_val = 0;

	dev_entry = dev_data_find(intf);
	if (NULL == dev_entry) {
		ret_val = -1;
		RTKBT_ERR("%s: NULL == dev_entry", __func__);
		return ret_val;
	}

	pinfo = dev_entry->patch_entry;
	if (!pinfo) {
		RTKBT_ERR("%s: No patch entry", __func__);
		ret_val = -1;
		return ret_val;
	}

	switch (pinfo->chip_type){
	case RTL8852CU:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		btrealtek_set_flag(hdev, REALTEK_ALT6_CONTINUOUS_TX_CHIP);
#endif
		break;
	default:
		break;
	}

	return ret_val;
}

#if defined RTKBT_SUSPEND_WAKEUP || defined RTKBT_SHUTDOWN_WAKEUP || defined RTKBT_SWITCH_PATCH
int set_scan(struct usb_interface *intf)
{
	dev_data *dev_entry;
	xchange_data *xdata = NULL;
	int result;

	RTKBT_DBG("%s", __func__);
	dev_entry = dev_data_find(intf);
	if (!dev_entry)
		return -1;

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (!xdata) {
		RTKBT_ERR("Could not alloc xdata");
		return -1;
	}

	result = init_xdata(xdata, dev_entry);
	if (result) {
		RTKBT_ERR("failed to init xdata");
		goto end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(STARTSCAN_OPCODE);
	xdata->cmd_hdr->plen = 1;
	xdata->pkt_len = CMD_HDR_LEN + 1;
	xdata->send_pkt[CMD_HDR_LEN] = 1;

	result = send_hci_cmd(xdata);
	if (result < 0)
		goto end;

end:
	kfree(xdata->send_pkt);
	kfree(xdata->rcv_pkt);
	kfree(xdata);

	RTKBT_DBG("%s done", __func__);

	return result;
}
#endif

dev_data *dev_data_find(struct usb_interface * intf)
{
	dev_data *dev_entry = NULL;

	list_for_each_entry(dev_entry, &dev_data_list, list_node) {
		if (dev_entry->intf == intf) {
			patch_info *patch = dev_entry->patch_entry;
			if (!patch)
				return NULL;

			RTKBT_INFO("chip type value: 0x%02x", patch->chip_type);
			return dev_entry;
		}
	}

	return NULL;
}

patch_info *get_patch_entry(struct usb_device * udev)
{
	patch_info *patch_entry;
	uint16_t pid;

	patch_entry = fw_patch_table;
	pid = le16_to_cpu(udev->descriptor.idProduct);
	RTKBT_DBG("pid = 0x%x", pid);
	while (pid != patch_entry->prod_id) {
		if (0 == patch_entry->prod_id) {
			RTKBT_DBG
			    ("get_patch_entry =NULL, can not find device pid in patch_table");
			return NULL;	//break;
		}
		patch_entry++;
	}

	return patch_entry;
}

static int is_mac(u8 chip_type, u16 offset)
{
	int result = 0;

	switch (chip_type) {
	case RTL8822BU:
	case RTL8723DU:
	case RTL8723CU:
	case RTL8821CU:
		if (offset == 0x0044)
			return 1;
		break;
	case RTL8822CU:
	case RTL8761BU:
	case RTL8852AU:
	case RTL8733BU:
	case RTL8852BU:
	case RTL8852CU:
	case RTL8822EU:
	case RTL8851BU:
	case RTL8852DU:
	case RTL8922AU:
	case RTL8852BTU:
	case RTL8761CU:
		if (offset == 0x0030)
			return 1;
		break;
	case RTLPREVIOUS:
		if (offset == 0x003c)
			return 1;
		break;
	}

	return result;
}

static uint16_t get_mac_offset(u8 chip_type)
{
	switch (chip_type) {
	case RTL8822BU:
	case RTL8723DU:
	case RTL8723CU:
	case RTL8821CU:
		return 0x0044;
	case RTL8822CU:
	case RTL8761BU:
	case RTL8852AU:
	case RTL8733BU:
	case RTL8852BU:
	case RTL8852CU:
	case RTL8822EU:
	case RTL8851BU:
	case RTL8852DU:
	case RTL8922AU:
	case RTL8852BTU:
	case RTL8761CU:
		return 0x0030;
	case RTLPREVIOUS:
		return 0x003c;
	default:
		return 0x003c;
	}
}

static void merge_configs(struct list_head *head, struct list_head *head2)
{
	struct list_head *epos, *enext;
	struct list_head *pos, *next;
	struct cfg_list_item *n;
	struct cfg_list_item *extra;

	if (!head || !head2)
		return;

	if (list_empty(head2))
		return;

	if (list_empty(head)) {
		list_splice_tail(head2, head);
		INIT_LIST_HEAD(head2);
		return;
	}

	/* Add or update & replace */
	list_for_each_safe(epos, enext, head2) {
		extra = list_entry(epos, struct cfg_list_item, list);

		list_for_each_safe(pos, next, head) {
			n = list_entry(pos, struct cfg_list_item, list);
			if (extra->offset == n->offset) {
				if (extra->len < n->len) {
					/* Update the cfg data */
					RTKBT_INFO("Update cfg: ofs %04x len %u",
						   n->offset, n->len);
					memcpy(n->data, extra->data,
					       extra->len);
					list_del(epos);
					kfree(extra);
					break;
				} else {
					/* Replace the item */
					list_del(epos);
					list_replace_init(pos, epos);
					/* free the old item */
					kfree(n);
				}
			}

		}

	}

	if (list_empty(head2))
		return;
	list_for_each_safe(epos, enext, head2) {
		extra = list_entry(epos, struct cfg_list_item, list);
		RTKBT_INFO("Add new cfg: ofs %04x, len %u", extra->offset,
			   extra->len);
		/* Add the item to list */
		list_del(epos);
		list_add_tail(epos, head);
	}
}

static int rtk_parse_config_file(u8 *config_buf, int filelen)
{
	struct rtk_bt_vendor_config *config = (void *)config_buf;
	u16 config_len = 0, temp = 0;
	struct rtk_bt_vendor_config_entry *entry = NULL;
	u32 i = 0;
	struct cfg_list_item *item;

	if (!config_buf)
		return -EINVAL;

	config_len = le16_to_cpu(config->data_len);
	entry = config->entry;

	if (le32_to_cpu(config->signature) != RTK_VENDOR_CONFIG_MAGIC) {
		RTKBT_ERR("sig magic num %08x,  not rtk vendor magic %08x",
			  config->signature, RTK_VENDOR_CONFIG_MAGIC);
		return -1;
	}

	if (config_len != filelen - BT_CONFIG_HDRLEN) {
		RTKBT_ERR("config length %u is not right %u", config_len,
			  (u16)(filelen - BT_CONFIG_HDRLEN));
		return -1;
	}

	for (i = 0; i < config_len;) {
		/* Add config item to list */
		item = kzalloc(sizeof(*item) + entry->entry_len, GFP_KERNEL);
		if (item) {
			item->offset = le16_to_cpu(entry->offset);
			item->len = entry->entry_len;
			memcpy(item->data, entry->entry_data, item->len);
			list_add_tail(&item->list, &list_configs);
		} else {
			RTKBT_ERR("Cannot alloc mem for entry %04x, %u",
				  entry->offset, entry->entry_len);
			break;
		}

		temp = entry->entry_len +
			sizeof(struct rtk_bt_vendor_config_entry);
		i += temp;
		entry =
		    (struct rtk_bt_vendor_config_entry *)((uint8_t *) entry +
							  temp);
	}

	return 0;
}

static uint8_t rtk_get_fw_project_id(uint8_t * p_buf)
{
	uint8_t opcode;
	uint8_t len;
	uint8_t data = 0;

	do {
		opcode = *p_buf;
		len = *(p_buf - 1);
		if (opcode == 0x00) {
			if (len == 1) {
				data = *(p_buf - 2);
				RTKBT_DBG
				    ("rtk_get_fw_project_id: opcode %d, len %d, data %d",
				     opcode, len, data);
				break;
			} else {
				RTKBT_ERR
				    ("rtk_get_fw_project_id: invalid len %d",
				     len);
			}
		}
		p_buf -= len + 2;
	} while (*p_buf != 0xFF);

	return data;
}

struct rtb_ota_flag {
	uint8_t eco;
	uint8_t enable;
	uint16_t reserve;
} __attribute__ ((packed));

struct rtb_security_hdr {
	uint8_t eco;
	uint8_t pri;
	uint8_t key_id;
	uint8_t reserve;
	uint32_t security_len;
	uint8_t *payload;
} __attribute__ ((packed));

struct rtb_dummy_hdr {
	uint8_t eco;
	uint8_t pri;
	uint8_t reserve;
	uint32_t dummy_len;
	uint8_t *payload;
} __attribute__ ((packed));

struct rtb_snippet_hdr {
	uint8_t eco;
	uint8_t pri;
	uint16_t reserve;
	uint32_t snippet_len;
	uint8_t *payload;
} __attribute__ ((packed));

struct patch_node {
	uint8_t eco;
	uint8_t pri;
	uint8_t key_id;
	uint8_t reserve;
	uint32_t len;
	uint8_t *payload;
	struct list_head list;
} __attribute__ ((packed));

/* Add a node to alist that is in ascending order. */
static void insert_queue_sort(struct list_head *head, struct patch_node *node)
{
	struct list_head *pos;
	struct list_head *next;
	struct patch_node *tmp;

	if(!head || !node) {
		return;
	}
	list_for_each_safe(pos, next, head) {
		tmp = list_entry(pos, struct patch_node, list);
		if(tmp->pri >= node->pri)
			break;
	}
	__list_add(&node->list, pos->prev, pos);
}

static int insert_patch(struct patch_node *patch_node_hdr, uint8_t *section_pos,
		uint32_t opcode, uint32_t *patch_len, uint8_t *sec_flag)
{
	struct patch_node *tmp;
	int i;
	uint32_t numbers;
	uint32_t section_len = 0;
	uint8_t eco = 0;
	uint8_t *pos = section_pos + 8;

	numbers = get_unaligned_le16(pos);
	RTKBT_DBG("number 0x%04x", numbers);

	pos += 4;
	for (i = 0; i < numbers; i++) {
		eco = (uint8_t)*(pos);
		RTKBT_DBG("eco 0x%02x, Eversion:%02x", eco, gEVersion);
		if (eco == gEVersion + 1) {
			tmp = (struct patch_node*)kzalloc(sizeof(struct patch_node), GFP_KERNEL);
			if (!tmp) {
				RTKBT_ERR("Cannot alloc mem for patch node");
				continue;
			}
			tmp->pri = (uint8_t)*(pos + 1);
			if(opcode == PATCH_SECURITY_HEADER)
				tmp->key_id = (uint8_t)*(pos + 1);

			section_len = get_unaligned_le32(pos + 4);
			tmp->len =  section_len;
			*patch_len += section_len;
			RTKBT_DBG("Pri:%d, Patch length 0x%04x", tmp->pri, tmp->len);
			tmp->payload = pos + 8;
			if(opcode != PATCH_SECURITY_HEADER) {
				insert_queue_sort(&(patch_node_hdr->list), tmp);
			} else {
				if((g_key_id == tmp->key_id) && (g_key_id > 0)) {
					insert_queue_sort(&(patch_node_hdr->list), tmp);
					*sec_flag = 1;
				} else {
					pos += (8 + section_len);
					kfree(tmp);
					continue;
				}
			}
		} else {
			section_len =  get_unaligned_le32(pos + 4);
			RTKBT_DBG("Patch length 0x%04x", section_len);
		}
		pos += (8 + section_len);
	}
	return 0;
}

static uint8_t *rtb_get_patch_header(int *len,
		struct patch_node *patch_node_hdr, uint8_t * epatch_buf,
		uint8_t key_id)
{
	uint16_t i, j;
	struct rtb_new_patch_hdr *new_patch;
	uint8_t sec_flag = 0;
	uint32_t number_of_ota_flag;
	uint32_t patch_len = 0;
	uint8_t *section_pos;
	uint8_t *ota_flag_pos;
	uint32_t number_of_section;

	struct rtb_section_hdr section_hdr;
	struct rtb_ota_flag ota_flag;

	new_patch = (struct rtb_new_patch_hdr *)epatch_buf;
	number_of_section = le32_to_cpu(new_patch->number_of_section);

	RTKBT_DBG("FW version 0x%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x",
				*(epatch_buf + 8), *(epatch_buf + 9), *(epatch_buf + 10),
				*(epatch_buf + 11),*(epatch_buf + 12), *(epatch_buf + 13),
				*(epatch_buf + 14), *(epatch_buf + 15));

	section_pos = epatch_buf + 20;

	for (i = 0; i < number_of_section; i++) {
		section_hdr.opcode = get_unaligned_le32(section_pos);
		section_hdr.section_len = get_unaligned_le32(section_pos + 4);
		RTKBT_DBG("opcode 0x%04x", section_hdr.opcode);

		switch (section_hdr.opcode) {
		case PATCH_SNIPPETS:
			insert_patch(patch_node_hdr, section_pos, PATCH_SNIPPETS, &patch_len, NULL);
			break;
		case PATCH_SECURITY_HEADER:
			if(!g_key_id)
				break;

			sec_flag = 0;
			insert_patch(patch_node_hdr, section_pos, PATCH_SECURITY_HEADER, &patch_len, &sec_flag);
			if(sec_flag)
				break;

			for (i = 0; i < number_of_section; i++) {
				section_hdr.opcode = get_unaligned_le32(section_pos);
				section_hdr.section_len = get_unaligned_le32(section_pos + 4);
				if(section_hdr.opcode == PATCH_DUMMY_HEADER) {
					insert_patch(patch_node_hdr, section_pos, PATCH_DUMMY_HEADER, &patch_len, NULL);
				}
				section_pos += (SECTION_HEADER_SIZE + section_hdr.section_len);
			}
			break;
		case PATCH_DUMMY_HEADER:
			if(g_key_id) {
				break;
			}
			insert_patch(patch_node_hdr, section_pos, PATCH_DUMMY_HEADER, &patch_len, NULL);
			break;
		case PATCH_OTA_FLAG:
			ota_flag_pos = section_pos + 4;
			number_of_ota_flag = get_unaligned_le32(ota_flag_pos);
			ota_flag.eco = (uint8_t)*(ota_flag_pos + 1);
			if (ota_flag.eco == gEVersion + 1) {
				for (j = 0; j < number_of_ota_flag; j++) {
					if (ota_flag.eco == gEVersion + 1) {
						ota_flag.enable = get_unaligned_le32(ota_flag_pos + 4);
					}
				}
			}
			break;
		default:
			RTKBT_INFO("Unknown Opcode. Ignore");
		}
		section_pos += (SECTION_HEADER_SIZE + section_hdr.section_len);
	}
	*len = patch_len;

	return NULL;
}

static int rtk_get_patch_entry(uint8_t * epatch_buf,
				struct rtk_epatch_entry *entry)
{
	uint32_t svn_ver;
	uint32_t coex_ver;
	uint32_t tmp;
	uint16_t i;
	uint16_t number_of_total_patch;
	struct rtk_epatch *epatch_info = (struct rtk_epatch *)epatch_buf;

	number_of_total_patch =
	    le16_to_cpu(epatch_info->number_of_total_patch);
	RTKBT_DBG("fw_version = 0x%x", le32_to_cpu(epatch_info->fw_version));
	RTKBT_DBG("number_of_total_patch = %d", number_of_total_patch);

	/* get right epatch entry */
	for (i = 0; i < number_of_total_patch; i++) {
		if (get_unaligned_le16(epatch_buf + 14 + 2 * i) ==
		    gEVersion + 1) {
			entry->chipID = gEVersion + 1;
			entry->patch_length = get_unaligned_le16(epatch_buf +
					14 +
					2 * number_of_total_patch +
					2 * i);
			entry->start_offset = get_unaligned_le32(epatch_buf +
					14 +
					4 * number_of_total_patch +
					4 * i);
			break;
		}
	}

	if (i >= number_of_total_patch) {
		entry->patch_length = 0;
		entry->start_offset = 0;
		RTKBT_ERR("No corresponding patch found\n");
		return 0;
	}

	svn_ver = get_unaligned_le32(epatch_buf +
				entry->start_offset +
				entry->patch_length - 8);
	coex_ver = get_unaligned_le32(epatch_buf +
				entry->start_offset +
				entry->patch_length - 12);

	RTKBT_DBG("chipID %d", entry->chipID);
	RTKBT_DBG("patch_length 0x%04x", entry->patch_length);
	RTKBT_DBG("start_offset 0x%08x", entry->start_offset);

	RTKBT_DBG("Svn version: %8d", svn_ver);
	tmp = ((coex_ver >> 16) & 0x7ff) + (coex_ver >> 27) * 10000;
	RTKBT_DBG("Coexistence: BTCOEX_20%06d-%04x",
		  tmp, (coex_ver & 0xffff));

	return 0;
}

static int bachk(const char *str)
{
	if (!str)
		return -1;

	if (strlen(str) != 17)
		return -1;

	while (*str) {
		if (!isxdigit(*str++))
			return -1;

		if (!isxdigit(*str++))
			return -1;

		if (*str == 0)
			break;

		if (*str++ != ':')
			return -1;
	}

	return 0;
}

static int request_bdaddr(u8 *buf)
{
	int size;
	int rc;
	struct file *file;
	u8 tbuf[BDADDR_STRING_LEN + 1];
	char *str;
	int i;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	loff_t pos = 0;
#endif

	if (!buf)
		return -EINVAL;

	file = filp_open(BDADDR_FILE, O_RDONLY, 0);
	if (IS_ERR(file))
		return -ENOENT;

	if (!S_ISREG(file_inode(file)->i_mode))
		return -EINVAL;
	size = i_size_read(file_inode(file));
	if (size <= 0)
		return -EINVAL;

	if (size > BDADDR_STRING_LEN)
		size = BDADDR_STRING_LEN;

	memset(tbuf, 0, sizeof(tbuf));
	RTKBT_INFO("size = %d", size);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	rc = kernel_read(file, tbuf, size, &pos);
#else
	rc = kernel_read(file, 0, tbuf, size);
#endif
	fput(file);
	if (rc != size) {
		if (rc >= 0)
			rc = -EIO;
		goto fail;
	}

	if (bachk(tbuf) < 0) {
		rc = -EINVAL;
		goto fail;
	}

	str = tbuf;
	for (i = 5; i >= 0; i--) {
		buf[i] = simple_strtol(str, NULL, 16);
		str += 3;
	}

	return size;
fail:
	return rc;
}

static u8 *load_config(dev_data *dev_entry, int *length)
{
	patch_info *patch_entry;
	const char *config_name;
	const struct firmware *fw;
	struct usb_device *udev;
	int result;
	u8 *buf;
	u8 *p;
	u16 config_len;
	u16 dlen;
	u8 tmp_buf[32];
	int file_sz;
	struct cfg_list_item *n;
	struct list_head *pos, *next;
	u8 chip_type;

	config_lists_init();
	patch_entry = dev_entry->patch_entry;
	config_name = patch_entry->config_name;
	udev = dev_entry->udev;
	chip_type = patch_entry->chip_type;

	RTKBT_INFO("config filename %s", config_name);
	result = request_firmware(&fw, config_name, &udev->dev);
	if (result < 0)
		return NULL;

	file_sz = fw->size;
	buf = (u8 *)fw->data;

	/* Load extra configs */
	config_file_proc(EXTRA_CONFIG_FILE, CFG_TYPE_BT);
	list_for_each_safe(pos, next, &list_extracfgs) {
		n = list_entry(pos, struct cfg_list_item, list);
		RTKBT_INFO("extra cfg: ofs %04x, len %u", n->offset, n->len);
	}

	/* Load extra bdaddr config */
	memset(tmp_buf, 0, sizeof(tmp_buf));
	result = request_bdaddr(tmp_buf);
	if (result > 0) {
		n = kzalloc(sizeof(*n) + 6, GFP_KERNEL);
		if (n) {
			n->offset = get_mac_offset(patch_entry->chip_type);
			n->len = 6;
			memcpy(n->data, tmp_buf, 6);
			list_add_tail(&n->list, &list_extracfgs);
		} else {
			RTKBT_WARN("Couldn't alloc mem for bdaddr");
		}
	} else {
		if (result == -ENOENT)
			RTKBT_DBG("no bdaddr file %s", BDADDR_FILE);
		else
			RTKBT_WARN("invalid customer bdaddr %d", result);
	}

	RTKBT_INFO("Origin cfg len %u", (u16)file_sz);
	util_hexdump((const u8 *)buf, file_sz);

	result = rtk_parse_config_file(buf, file_sz);
	if (result < 0) {
		RTKBT_ERR("Parse config file error");
		buf = NULL;
		goto done;
	}

	merge_configs(&list_configs, &list_extracfgs);

	/* Calculate the config_len */
	config_len = 4; /* magic word length */
	config_len += 2; /* data length field */
	dlen = 0;
	list_for_each_safe(pos, next, &list_configs) {
		n = list_entry(pos, struct cfg_list_item, list);
		switch (n->offset) {
		case 0x003c:
		case 0x0030:
		case 0x0044:
			if (is_mac(chip_type, n->offset) && n->len == 6) {
				char s[18];
				sprintf(s, "%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
					n->data[5], n->data[4],
					n->data[3], n->data[2],
					n->data[1], n->data[0]);
				RTKBT_INFO("bdaddr ofs %04x, %s", n->offset, s);
			}
			break;
		default:
			break;
		}

		config_len += (3 + n->len);
		dlen += (3 + n->len);
	}


	buf = kzalloc(config_len, GFP_KERNEL);
	if (!buf) {
		RTKBT_ERR("Couldn't alloc buf for configs");
		goto done;
	}

	/* Save configs to a buffer */
	memcpy(buf, cfg_magic, 4);
	buf[4] = dlen & 0xff;
	buf[5] = (dlen >> 8) & 0xff;
	p = buf + 6;
	list_for_each_safe(pos, next, &list_configs) {
		n = list_entry(pos, struct cfg_list_item, list);
		p[0] = n->offset & 0xff;
		p[1] = (n->offset >> 8) & 0xff;
		p[2] = n->len;
		memcpy(p + 3, n->data, n->len);
		p += (3 + n->len);
	}

	RTKBT_INFO("New cfg len %u", config_len);
	util_hexdump((const u8 *)buf, config_len);

	*length = config_len;

done:
	config_lists_free();
	release_firmware(fw);

	return buf;
}

static int rtl_vendor_read(dev_data *dev_entry, u8 type, u32 addr, u8 *rp)
{
	int ret_val = 0;
	xchange_data *xdata = NULL;
	u8 param[5];
	u8 rp_len = 0;

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (!xdata) {
		RTKBT_ERR("failed to alloc xdata");
		return -ENOMEM;
	}

	switch (type) {
	case 0x21:
		rp_len = 4;
		break;
	case 0x10:
		rp_len = 2;
		break;
	default:
		RTKBT_ERR("Unknown read type 0x%02x", type);
		goto read_end;
	}
	param[0] = type;

	put_unaligned_le32(addr, &param[1]);

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to init xdata");
		goto read_end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_VENDOR_READ_CMD);
	xdata->cmd_hdr->plen = sizeof(param);
	memcpy(xdata->req_para, param, sizeof(param));
	xdata->pkt_len = CMD_HDR_LEN + sizeof(param);

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to send read RTK chip_type cmd.");
		goto read_end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to receive HCI event for chip type.");
		goto read_end;
	}

	if (rp_len)
		memcpy(rp, xdata->rsp_para + 1, rp_len);

read_end:
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	return ret_val;
}

static int rtk_vendor_read(dev_data * dev_entry, uint8_t class)
{
	struct rtk_chip_type_evt *chip_type;
	struct rtk_security_proj_evt *sec_proj;
	patch_info *patch_entry;
	int ret_val = 0;
	xchange_data *xdata = NULL;
	unsigned char cmd_ct_buf[] = {0x10, 0x38, 0x04, 0x28, 0x80};
	unsigned char cmd_cv_buf[] =  {0x10, 0x3A, 0x04, 0x28, 0x80};
	unsigned char cmd_sec_buf[] = {0x10, 0xA4, 0xAD, 0x00, 0xb0};

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (NULL == xdata) {
		ret_val = 0xFE;
		RTKBT_DBG("NULL == xdata");
		return ret_val;
	}

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val) {
		ret_val = 0xFE;
		RTKBT_ERR("failed to init xdata");
		goto read_end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_VENDOR_READ_CMD);
	xdata->cmd_hdr->plen = 5;
	memcpy(xdata->send_pkt, &(xdata->cmd_hdr->opcode), 2);
	memcpy(xdata->send_pkt+2, &(xdata->cmd_hdr->plen), 1);

	switch (class) {
	case READ_CHIP_TYPE:
		memcpy(xdata->send_pkt+3, cmd_ct_buf, sizeof(cmd_ct_buf));
		break;
	case READ_CHIP_VER:
		memcpy(xdata->send_pkt+3, cmd_cv_buf, sizeof(cmd_cv_buf));
		break;
	case READ_SEC_PROJ:
		memcpy(xdata->send_pkt+3, cmd_sec_buf, sizeof(cmd_sec_buf));
		break;
	default:
		break;
	}

	xdata->pkt_len = CMD_HDR_LEN + 5;

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to send read RTK chip_type cmd.");
		ret_val = 0xFE;
		goto read_end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to receive HCI event for chip type.");
		ret_val = 0xFE;
		goto read_end;
	}

	patch_entry = xdata->dev_entry->patch_entry;
	if(class == READ_SEC_PROJ){
		sec_proj = (struct rtk_security_proj_evt *)(xdata->rsp_para);
		RTKBT_DBG("sec_proj->status = 0x%x, sec_proj->key_id = 0x%x",
		  sec_proj->status, sec_proj->key_id);
		if (sec_proj->status) {
			ret_val = 0;
		} else {
			ret_val = sec_proj->key_id;
			g_key_id = sec_proj->key_id;
		}
	} else {
		chip_type = (struct rtk_chip_type_evt *)(xdata->rsp_para);
		RTKBT_DBG("chip_type->status = 0x%x, chip_type->chip = 0x%x",
			  chip_type->status, chip_type->chip);
		if (chip_type->status) {
			ret_val = 0;
		} else {
			ret_val = chip_type->chip;
		}
	}

read_end:
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	return ret_val;
}

static int rtk_vendor_read_reg32(dev_data *dev_entry, u32 addr, u8 *rp)
{
	return rtl_vendor_read(dev_entry, 0x21, addr, rp);
}

static int rtk_vendor_read_reg16(dev_data *dev_entry, u32 addr, u8 *rp)
{
	return rtl_vendor_read(dev_entry, 0x10, addr, rp);
}

static int needs_hci_upgrade(xchange_data *xdata, u8 *buf, u32 buf_len)
{
	struct {
		u8 status;
		u8 subopcode;
		u8 ota;
	} __attribute__((packed)) *evt_params;
#define UPG_DL_BLOCK_SIZE   128
#define UPG_SUBCMD_CODE     0x01
	u8 len = UPG_DL_BLOCK_SIZE;
	u8 *cmd_params;
	int ret;

	cmd_params = xdata->req_para;
	evt_params = (void *)xdata->rsp_para;
	xdata->cmd_hdr->opcode = cpu_to_le16(0xfdbb);
	if (buf_len < len)
		len = buf_len;
	xdata->cmd_hdr->plen = 1 + len;
	xdata->pkt_len = sizeof(*xdata->cmd_hdr) + xdata->cmd_hdr->plen;
	*cmd_params++ = UPG_SUBCMD_CODE;
	memcpy(cmd_params, buf, len);

	ret = send_hci_cmd(xdata);
	if (ret < 0)
		return ret;

	ret = rcv_hci_evt(xdata);
	if (ret < 0)
		return ret;

	if (evt_params->status) {
		RTKBT_ERR("needs_hci_upgrade: status %02x", evt_params->status);
		return -1;
	}

	if (evt_params->subopcode != UPG_SUBCMD_CODE) {
		RTKBT_ERR("needs_hci_upgrade: return subopcode %02x",
			  evt_params->subopcode);
		return -2;
	}

	RTKBT_INFO("needs_hci_upgrade: state %02x", evt_params->ota);

	return evt_params->ota;
}

static int btrtl_parse_config(struct rtl_subsection_v3 *subsec,
			      dev_data *dev_entry)
{
	char tmp_name[32];
	char filename[64];
	int len;
	char *str;
	char *p;
	int ret = 0;
	u8 *cfg_buf = NULL;
	const struct firmware *fw;

	if (!subsec || !dev_entry || !dev_entry->patch_entry ||
	    !dev_entry->udev)
		return -EINVAL;

	str = dev_entry->patch_entry->config_name;

	if (dev_entry->fw_type == FW_TYPE_V3_1) {
		if (!subsec->image_id && !subsec->index) {
			if (!dev_entry->cfg_len)
				goto done;
			cfg_buf = vmalloc(dev_entry->cfg_len);
			if (!cfg_buf)
				return -ENOMEM;
			memcpy(cfg_buf, dev_entry->cfg_buf, dev_entry->cfg_len);

			subsec->cfg_buf = cfg_buf;
			subsec->cfg_len = dev_entry->cfg_len;

			dev_entry->cfg_len = 0;
			/* This buffer will be released at the end of the
			 * load_firmware()
			 */
			dev_entry->cfg_buf = NULL;
		}
		goto done;
	}

	len = strlen(str);
	if (len > sizeof(tmp_name) - 1)
		len = sizeof(tmp_name) - 1;
	memcpy(tmp_name, str, len);
	tmp_name[len] = '\0';

	str = tmp_name;
	p = strsep(&str, ".");

	ret = snprintf(filename, sizeof(filename), "%s", p);
	if (subsec->config_rule && subsec->need_config) {
		switch (subsec->image_id) {
		case IMAGE_ID_F000:
		case IMAGE_ID_F001:
		case IMAGE_ID_F002:
			ret += snprintf(filename + ret, sizeof(filename) - ret,
					"_%04x", subsec->image_id);
			break;
		default:
			goto done;
		}
	} else {
		goto done;
	}

	if (str)
		snprintf(filename + ret, sizeof(filename) - ret, ".%s", str);

	RTKBT_INFO("config file: %s", filename);
	ret = request_firmware(&fw, filename, &dev_entry->udev->dev);
	if (ret < 0) {
		RTKBT_ERR("request_firmware [%s] error", filename);
		len = 4;
		cfg_buf = vmalloc(len);
		if (!cfg_buf) {
			RTKBT_ERR("alloc cfg buf error");
			return -ENOMEM;
		}
		memset(cfg_buf, 0xff, len);
		subsec->cfg_buf = cfg_buf;
		subsec->cfg_len = len;

		return 0;
	}
	cfg_buf = vmalloc(fw->size);
	if (!cfg_buf) {
		ret = -ENOMEM;
		goto err;
	}
	memcpy(cfg_buf, fw->data, fw->size);
	len = fw->size;
	release_firmware(fw);

	subsec->cfg_buf = cfg_buf;
	subsec->cfg_len = len;
done:
	return 0;
err:
	release_firmware(fw);
	return ret;
}

static int rtlbt_parse_section_v3(u32 opcode, u8 *data, u32 len,
				  dev_data *dev_entry,
				  struct image_match_info *info)
{
	struct rtl_subsection_v3 *subsec;
	struct rtl_sec_hdr_v3 *sec_hdr;
	u8 *ptr;
	u32 sec_len;
	int rc = 0;
	struct rtl_iovec iov = {
		.data = data,
		.len  = len,
	};
	u8 i;
	u16 chip_id;
	u16 image_id;

	if (!data || !len || !info)
		return -EINVAL;

	sec_hdr = rtl_iov_pull_data(&iov, sizeof(*sec_hdr));
	if (!sec_hdr) {
		RTKBT_ERR("get section header failed");
		return -1;
	}

	if (dev_entry->opcode && dev_entry->opcode != opcode) {
		RTKBT_ERR("invalid opcode 0x%02x", opcode);
		return -EINVAL;
	}

	if (!dev_entry->opcode) {
		dev_entry->opcode = opcode;
		switch (dev_entry->opcode) {
		case RTL_PATCH_V3_1:
			dev_entry->fw_type = FW_TYPE_V3_1;
			break;
		case RTL_PATCH_V3_PATCH_IMAGE:
			dev_entry->fw_type = FW_TYPE_V3_2;
			break;
		default:
			return -EINVAL;
		}
		RTKBT_INFO("fw_type %u", dev_entry->fw_type);
	}

	sec_len = le64_to_cpu(sec_hdr->patch_image_len);

	chip_id = le16_to_cpu(sec_hdr->chip_id);
	image_id = le16_to_cpu(sec_hdr->image_id);
	RTKBT_DBG("image (%04x:%02x), chip id %u, ic cut 0x%02x, len %08x",
		  image_id, sec_hdr->index, chip_id, sec_hdr->ic_cut, sec_len);

	if (info->key_id != sec_hdr->key_id) {
		RTKBT_ERR("invalid key_id (%u, %u)", sec_hdr->key_id,
			  info->key_id);
		return -EINVAL;
	}

	if (sec_hdr->ic_cut != info->ic_cut) {
		RTKBT_INFO("unused chip_id (%u, %u), ic_cut (%u, %u)",
			   chip_id, info->chip_id, sec_hdr->ic_cut,
			   info->ic_cut);
		return -EINVAL;
	}

	if (dev_entry->fw_type == FW_TYPE_V3_2 &&
	    (u8)chip_id != info->chip_id) {
		RTKBT_ERR("invalid chip_id (%u, %u)", chip_id, info->chip_id);
		return -EINVAL;
	}

	ptr = rtl_iov_pull_data(&iov, sec_len);
	if (!ptr)
		return -1;

	subsec = kzalloc(sizeof(*subsec), GFP_KERNEL);
	if (!subsec)
		return -ENOMEM;
	subsec->index = sec_hdr->index;
	subsec->image_id = image_id;
	subsec->config_rule = sec_hdr->config_rule;
	subsec->need_config = sec_hdr->need_config;

	for (i = 0; i < DL_FIX_ADDR_MAX; i++) {
		subsec->fix[i].addr =
			(u32)le64_to_cpu(sec_hdr->addr_fix[i * 2]);
		subsec->fix[i].value =
			(u32)le64_to_cpu(sec_hdr->addr_fix[i * 2 + 1]);
	}

	subsec->patch_image_len = (u32)le64_to_cpu(sec_hdr->patch_image_len);
	subsec->patch_image_buf = vmalloc(subsec->patch_image_len);
	if (!subsec->patch_image_buf) {
		rc = -ENOMEM;
		goto err;
	}
	memcpy(subsec->patch_image_buf, ptr, subsec->patch_image_len);
	subsec->image_ver =
		get_unaligned_le32(ptr + subsec->patch_image_len - 4);
	RTKBT_DBG("image version: %08x", subsec->image_ver);

	btrtl_parse_config(subsec, dev_entry);

	rc = subsec->patch_image_len;

	btrtl_insert_ordered_subsec_v3(subsec);

	return rc;
err:
	kfree(subsec);
	return rc;
}

static int rtlbt_parse_firmware_v3(dev_data *dev_entry, uint8_t *fw_data,
				   int fw_size)
{
	struct rtl_epatch_header_v3 *hdr;
	int rc;
	u32 num_sections;
	struct rtl_section_v3 *section;
	u32 section_len;
	u32 opcode;
	int len = 0;
	int i;
	u8 *ptr;
	struct rtl_iovec iov = {
		.data = fw_data,
		.len = fw_size,	/* Cut the tail */
	};
	struct image_match_info info;
	struct sk_buff *skb = NULL;
	struct hci_rp_read_eco *rp_eco;
	struct hci_rp_read_chip_id *rp_chip_id;
	u16 cmd_opc;
	u32 addr = 0xb000ada4;
	u8 reg_val[2];

	/* For old combochips running in ROM code, they return unknown hci cmd
	 * when received hci vendor cmd 0xfc6f.
	 * The comparison of the chip id between image section and the read chip
	 * id would be ignored.
	 */
	memset(&info, 0, sizeof(info));

	cmd_opc = HCI_VENDOR_READ_RTK_ROM_VERISION;
	skb = rtk_hci_cmd_sync(dev_entry, cmd_opc, 0, NULL, 1000);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("%s: read ECO version error %lu", __func__,
			  PTR_ERR(skb));
		return -EIO;
	}

	rp_eco = (void *)skb->data;
	if (rp_eco->status) {
		RTKBT_ERR("%s: status of cc (%04x) is %02x", __func__,
			  cmd_opc, rp_eco->status);
		kfree_skb(skb);
		return -EINVAL;
	}
	info.ic_cut = rp_eco->eco + 1;
	kfree_skb(skb);

	cmd_opc = HCI_VENDOR_READ_CHIP_ID;
	skb = rtk_hci_cmd_sync(dev_entry, cmd_opc, 0, NULL, 1000);
	if (IS_ERR(skb) || !skb) {
		RTKBT_ERR("%s: read chip id error %lu", __func__,
			  PTR_ERR(skb));
		return -EIO;
	}
	rp_chip_id = (void *)skb->data;
	if (!rp_chip_id->status)
		info.chip_id = rp_chip_id->chip_id;
	kfree_skb(skb);

	if (info.chip_id >= 55) {
		/* A0010DA4 */
		addr = 0xa0010da4;
	}

	rc = rtk_vendor_read_reg16(dev_entry, addr, reg_val);
	if (rc < 0)
		return -EIO;

	info.key_id = reg_val[0];
	RTKBT_DBG("%s: key id %u", __func__, info.key_id);

	hdr = rtl_iov_pull_data(&iov, sizeof(*hdr));
	if (!hdr)
		return -EINVAL;
	num_sections = le32_to_cpu(hdr->num_sections);

	RTKBT_DBG("Timestamp %08x-%08x, num %u",
		  get_unaligned_le32(hdr->timestamp),
		  get_unaligned_le32(hdr->timestamp + 4), num_sections);

	for (i = 0; i < num_sections; i++) {
		RTKBT_DBG("rtl section: %ld", sizeof(*section));
		section = rtl_iov_pull_data(&iov, sizeof(*section));
		if (!section)
			break;
		section_len = le64_to_cpu(section->len);
		opcode = le32_to_cpu(section->opcode);

		RTKBT_DBG("opcode 0x%04x, %08x", section->opcode, section_len);

		ptr = rtl_iov_pull_data(&iov, section_len);
		if (!ptr) {
			RTKBT_DBG("get payload data failed");
			break;
		}

		switch (opcode) {
		case RTL_PATCH_V3_1:
		case RTL_PATCH_V3_PATCH_IMAGE:
			rc = rtlbt_parse_section_v3(opcode, ptr, section_len,
						    dev_entry, &info);
			break;
		default:
			RTKBT_INFO("Unknown Opcode. Ignore");
			rc = 0;
			break;
		}
		if (rc < 0) {
			RTKBT_ERR("RTL: Parse section (%u) err %d", opcode, rc);
			continue;
		}
		len += rc;
	}

	RTKBT_DBG("image payload total len: 0x%08x", len);
	if (!len)
		return -ENODATA;

	return len;
}

/* buff: points to the allocated buffer that stores extracted fw and config
 * This function returns the total length of extracted fw and config
 */
int load_firmware(dev_data *dev_entry, xchange_data *xdata)
{
	const struct firmware *fw;
	struct usb_device *udev;
	patch_info *patch_entry;
	char *fw_name;
	int fw_len = 0, ret_val = 0, config_len = 0, buf_len = -1;
	uint8_t *buf = NULL, *config_file_buf = NULL, *epatch_buf = NULL;
	uint8_t proj_id = 0;
	uint8_t need_download_fw = 1;
	uint16_t lmp_version;
	struct rtk_epatch_entry current_entry = { 0 };
	struct list_head *pos, *next;
	struct patch_node *tmp;
	struct patch_node patch_node_hdr;
	int i;

	RTKBT_DBG("load_firmware start");

	udev = dev_entry->udev;
	patch_entry = dev_entry->patch_entry;
	lmp_version = patch_entry->lmp_sub;
	RTKBT_DBG("lmp_version = 0x%04x", lmp_version);

	config_file_buf = load_config(dev_entry, &config_len);

	fw_name = patch_entry->patch_name;
	RTKBT_DBG("fw name is  %s", fw_name);
	ret_val = request_firmware(&fw, fw_name, &udev->dev);
	if (ret_val < 0) {
		RTKBT_ERR("request_firmware error");
		goto fw_fail;
	}

	INIT_LIST_HEAD(&patch_node_hdr.list);

	epatch_buf = vzalloc(fw->size);
	if (!epatch_buf)
		goto alloc_fail;

	memcpy(epatch_buf, fw->data, fw->size);
	buf_len = fw->size + config_len;

	if (lmp_version == ROM_LMP_8723a) {
		RTKBT_DBG("This is 8723a, use old patch style!");

		if (memcmp(epatch_buf, RTK_EPATCH_SIGNATURE, 8) == 0) {
			RTKBT_ERR("8723a Check signature error!");
			need_download_fw = 0;
			goto sign_err;
		}
		buf = vzalloc(buf_len);
		if (!buf) {
			RTKBT_ERR("Can't alloc memory for fw&config");
			buf_len = -1;
			goto alloc_buf_err;
		}

		RTKBT_DBG("8723a, fw copy direct");
		memcpy(buf, epatch_buf, fw->size);
		if (config_len)
			memcpy(&buf[buf_len - config_len], config_file_buf,
			       config_len);

		goto done;
	}

	RTKBT_DBG("This is not 8723a, use new patch style!");
	dev_entry->fw_type = FW_TYPE_V1;

	/* Get version from ROM */
	gEVersion = rtk_get_eversion(dev_entry);
	RTKBT_DBG("%s: New gEVersion %d", __func__, gEVersion);
	if (gEVersion == 0xFE) {
		RTKBT_ERR("%s: Read ROM version failure", __func__);
		need_download_fw = 0;
		goto alloc_fail;
	}

	if (!memcmp(epatch_buf, RTK_EPATCH_SIGNATURE_V3, 8)) {
		RTKBT_DBG("parse format patch v3");
		dev_entry->cfg_buf = config_file_buf;
		dev_entry->cfg_len = config_len;
		buf_len = rtlbt_parse_firmware_v3(dev_entry, epatch_buf,
						  fw->size);
		goto done;
	}

	/* check Signature and Extension Section Field */
	if ((memcmp(epatch_buf, RTK_EPATCH_SIGNATURE, 8) &&
	     memcmp(epatch_buf, RTK_EPATCH_SIGNATURE_NEW, 8)) ||
	    memcmp(epatch_buf + buf_len - config_len - 4,
		   Extension_Section_SIGNATURE, 4) != 0) {
		RTKBT_ERR("Check SIGNATURE error! do not download fw");
		need_download_fw = 0;
		goto sign_err;
	}

	proj_id = rtk_get_fw_project_id(epatch_buf + buf_len - config_len - 5);

	for (i = 0; i < ARRAY_SIZE(project_id_to_lmp_subver); i++) {
		if (proj_id == project_id_to_lmp_subver[i].id &&
		    lmp_version == project_id_to_lmp_subver[i].lmp_subver) {
			break;
		}
	}

	if (i >= ARRAY_SIZE(project_id_to_lmp_subver)) {
		RTKBT_ERR("lmp_version %04x, project_id %u, does not match!!!",
			  lmp_version, proj_id);
		need_download_fw = 0;
		goto proj_id_err;
	}

	RTKBT_DBG("lmp_version is %04x, project_id is %u, match!",
		  lmp_version, proj_id);

	if (memcmp(epatch_buf, RTK_EPATCH_SIGNATURE_NEW, 8) == 0) {
		int key_id = rtk_vendor_read(dev_entry, READ_SEC_PROJ);
		int tmp_len = 0;

		dev_entry->fw_type = FW_TYPE_V2;

		RTKBT_DBG("%s: key id %d", __func__, key_id);
		if (key_id < 0) {
			RTKBT_ERR("%s: Read key id failure", __func__);
			need_download_fw = 0;
			fw_len = 0;
			goto extract_err;
		}
		rtb_get_patch_header(&buf_len, &patch_node_hdr, epatch_buf,
				     key_id);
		if (!buf_len)
			goto extract_err;
		RTKBT_DBG("buf_len = 0x%x", buf_len);
		buf_len += config_len;

		buf = vzalloc(buf_len);
		if (!buf) {
			RTKBT_ERR("Can't alloc memory for multi fw&config");
			buf_len = -1;
			goto alloc_buf_err;
		}

		list_for_each_safe(pos, next, &patch_node_hdr.list) {
			tmp = list_entry(pos, struct patch_node, list);
			RTKBT_DBG("len = 0x%x", tmp->len);
			memcpy(buf + tmp_len, tmp->payload, tmp->len);
			tmp_len += tmp->len;
			list_del_init(pos);
			kfree(tmp);
		}
		if (config_len)
			memcpy(&buf[buf_len - config_len], config_file_buf,
			       config_len);
	} else {
		rtk_get_patch_entry(epatch_buf, &current_entry);

		if (current_entry.patch_length == 0)
			goto extract_err;

		buf_len = current_entry.patch_length + config_len;
		RTKBT_DBG("buf_len = 0x%x", buf_len);

		buf = vzalloc(buf_len);
		if (!buf) {
			RTKBT_ERR("Can't alloc memory for multi fw&config");
			buf_len = -1;
			goto alloc_buf_err;
		}

		memcpy(buf, epatch_buf + current_entry.start_offset,
		       current_entry.patch_length);
		/* Copy fw version */
		memcpy(buf + current_entry.patch_length - 4, epatch_buf + 8, 4);
		if (config_len)
			memcpy(&buf[buf_len - config_len], config_file_buf,
			       config_len);
	}

	if (patch_entry->chip_type == RTL8761CU) {
		if (needs_hci_upgrade(xdata, buf, buf_len) <= 0) {
			if (config_len > 0) {
				memmove(buf, buf + buf_len - config_len,
					config_len);
				buf_len = config_len;
			} else {
#define FAKE_SEG_LEN	16
				if (buf_len > FAKE_SEG_LEN)
					buf_len = FAKE_SEG_LEN;
				memset(buf, 0, buf_len);
			}
		} else {
			/* It does not need to download config when upgrading */
			buf_len -= config_len;
		}
	}

done:
	RTKBT_DBG("fw:%s exists, config file:%s exists",
		  buf_len > 0 ? "" : "not", config_len > 0 ? "" : "not");
	if (buf && buf_len > 0 && need_download_fw) {
		fw_len = buf_len;
		xdata->fw_data = buf;
	} else if (is_v3_fw(dev_entry->fw_type))
		fw_len = buf_len;

	RTKBT_DBG("load_firmware done");
alloc_buf_err:
extract_err:
	/* Make sure all the patch nodes freed */
	list_for_each_safe(pos, next, &patch_node_hdr.list) {
		tmp = list_entry(pos, struct patch_node, list);
		list_del_init(pos);
		kfree(tmp);
	}
proj_id_err:
sign_err:
alloc_fail:
	release_firmware(fw);

	if (epatch_buf)
		vfree(epatch_buf);

fw_fail:
	if (config_file_buf)
		kfree(config_file_buf);

	if (fw_len == 0 && buf)
		vfree(buf);

	return fw_len;
}

static int init_xdata(xchange_data *xdata, dev_data *dev_entry)
{
	int ret = -1;
	struct usb_interface *intf;

	if (!xdata || !dev_entry)
		return -EINVAL;

	intf = dev_entry->intf;
	if (!intf)
		return -EINVAL;
	memset(xdata, 0, sizeof(xchange_data));
	if (intf->cur_altsetting->desc.bAlternateSetting != 0)
		xdata->bulk_mode = true;
	xdata->dev_entry = dev_entry;
	xdata->pipe_in = usb_rcvintpipe(dev_entry->udev, INTR_EP);
	xdata->pipe_out = usb_sndctrlpipe(dev_entry->udev, CTRL_EP);
	xdata->send_pkt = kzalloc(PKT_LEN, GFP_KERNEL);
	if (!xdata->send_pkt) {
		ret = -ENOMEM;
		goto err;
	}
	xdata->rcv_pkt = kzalloc(PKT_LEN, GFP_KERNEL);
	if (!xdata->rcv_pkt) {
		ret = -ENOMEM;
		goto err;
	}
	xdata->cmd_hdr = (struct hci_command_hdr *)(xdata->send_pkt);
	xdata->evt_hdr = (struct hci_event_hdr *)(xdata->rcv_pkt);
	xdata->cmd_cmp =
	    (struct hci_ev_cmd_complete *)(xdata->rcv_pkt + EVT_HDR_LEN);
	xdata->req_para = xdata->send_pkt + CMD_HDR_LEN;
	xdata->rsp_para = xdata->rcv_pkt + EVT_HDR_LEN + CMD_CMP_LEN;

	return 0;
err:
	if (xdata->send_pkt)
		kfree(xdata->send_pkt);
	if (xdata->rcv_pkt)
		kfree(xdata->rcv_pkt);
	memset(xdata, 0, sizeof(xchange_data));
	return ret;
}

int check_fw_version(xchange_data *xdata, dev_data *dev_entry)
{
	struct hci_rp_read_local_version *read_ver_rsp;
	patch_info *patch_entry;
	int ret_val;
	int retry = 0;
	uint16_t lmp_subver, hci_rev, manufacturer;

	/* Ensure that the first cmd is hci reset after system suspend
	 * or system reboot */
	if (!is_v3_fw(dev_entry->fw_type))
		send_reset_command(xdata);

get_ver:
	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_OP_READ_LOCAL_VERSION);
	xdata->cmd_hdr->plen = 0;
	xdata->pkt_len = CMD_HDR_LEN;

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to send HCI command.", __func__);
		goto version_end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("%s: Failed to receive HCI event.", __func__);
		goto version_end;
	}

	patch_entry = xdata->dev_entry->patch_entry;
	read_ver_rsp = (struct hci_rp_read_local_version *)(xdata->rsp_para);
	lmp_subver = le16_to_cpu(read_ver_rsp->lmp_subver);
	hci_rev = le16_to_cpu(read_ver_rsp->hci_rev);
	manufacturer = le16_to_cpu(read_ver_rsp->manufacturer);

	RTKBT_DBG("read_ver_rsp->lmp_subver = 0x%x", lmp_subver);
	RTKBT_DBG("read_ver_rsp->hci_rev = 0x%x", hci_rev);
	RTKBT_DBG("patch_entry->lmp_sub = 0x%x", patch_entry->lmp_sub);

	if (patch_entry->lmp_sub != lmp_subver)
		return 1;

	ret_val = 0;
version_end:
	if (ret_val) {
		send_reset_command(xdata);
		retry++;
		if (retry < 2)
			goto get_ver;
	}

	return ret_val;
}

int check_fw_version_v3(dev_data *dev_entry, xchange_data *xdata)
{
	struct rtl_subsection_v3 *sec;
	struct list_head *pos;
	struct list_head *next;
	//struct rtl_subsection_v3 *sec, *tmp;
	u32 sec_hdr_size_val, addr;
	u8 reg_val[4];
	int ret = -1;
	int retry = 0;
	struct sk_buff *skb;
	struct hci_vendor_read_patch_ver_rp {
		u8 status;
		u8 version[4];
	} __packed *rp;
	u32 ver = 0xffff;
	u16 opcode;

	/*
	 * Ensure that the first cmd is hci reset after system suspend
	 * or system reboot
	 */
	/* send_reset_command(xdata) */;

get_ver:
	//list_for_each_entry_safe(sec, tmp, &list_secs, list) {
	list_for_each_safe(pos, next, &list_secs) {
		sec = list_entry(pos, struct rtl_subsection_v3, list);
		RTKBT_DBG("%s: image (%04x:%02x)", __func__, sec->image_id,
			  sec->index);

		if (sec->image_id != IMAGE_ID_F000 &&
		    sec->image_id != IMAGE_ID_F001 &&
		    sec->image_id != IMAGE_ID_F002 &&
		    sec->image_id != 0x0000)
			continue;
		if (sec->index > 0) {
			sec_hdr_size_val = sec->fix[DL_FIX_SEC_HDR_ADDR].value;
			if (sec->fix[DL_FIX_SEC_HDR_ADDR].addr == 0 ||
			    sec->fix[DL_FIX_SEC_HDR_ADDR].addr == 0xFFFFFFFF)
				sec_hdr_size_val = 0;
			addr = sec->fix[DL_FIX_PATCH_ADDR].value +
					sec->patch_image_len -
					sec_hdr_size_val - 4;
			ret = rtk_vendor_read_reg32(dev_entry, addr, reg_val);
			if (ret <= 0) {
				RTKBT_ERR("read (%08x) failed, %d", addr, ret);
				ret = -EIO;
				goto version_end;
			}
			if (sec->image_ver != get_unaligned_le32(reg_val)) {
				RTKBT_DBG("version mismatch, index %02x, (%08x:%08x)",
					  sec->index, sec->image_ver,
					  get_unaligned_le32(reg_val));
				ret = -EINVAL;
				goto version_end;
			}
			continue;
		}

		if (sec->image_id == IMAGE_ID_F000 ||
		    sec->image_id == IMAGE_ID_F001) {
			u8 payload[4];

			ver = 0xffff;
			opcode = HCI_VENDOR_READ_PATCH_VERSION;
			put_unaligned_le32(sec->fix[DL_FIX_CI_ID].value, payload);
			skb = rtk_hci_cmd_sync(dev_entry, opcode, 4,
					       payload, 200);
			if (IS_ERR(skb) || !skb) {
				RTKBT_ERR("cmd (%04x) failed, %ld", opcode,
					  PTR_ERR(skb));
				ret = -EIO;
				goto version_end;
			}
			rp = NULL;
			if (skb->len == sizeof(*rp))
				rp = (void *)skb->data;
			if (rp && !rp->status)
				ver = get_unaligned_le32(rp->version);
			kfree_skb(skb);
			if (sec->image_ver != ver) {
				RTKBT_DBG("version mismatch, image (%04x:%02x),"
					  " (%08x:%08x)", sec->image_id,
					  sec->index, sec->image_ver, ver);
				ret = -EINVAL;
				goto version_end;
			}
		} else if (sec->image_id == IMAGE_ID_F002 ||
			   sec->image_id == 0) {
			struct hci_rp_read_local_version *rsp;
			u16 hci_rev;
			u16 lmp_subver;

			ver = 0xffff;
			opcode = HCI_OP_READ_LOCAL_VERSION;
			skb = rtk_hci_cmd_sync(dev_entry, opcode, 0, NULL,
					       200);
			if (IS_ERR(skb) || !skb) {
				RTKBT_ERR("cmd (%04x) failed, %ld", opcode,
					  PTR_ERR(skb));
				ret = -EIO;
				goto version_end;
			}
			rsp = NULL;
			if (skb->len == sizeof(*rsp))
				rsp = (void *)skb->data;
			if (rsp && !rsp->status) {
				hci_rev = le16_to_cpu(rsp->hci_rev);
				lmp_subver = le16_to_cpu(rsp->lmp_subver);
				ver = ((u32)hci_rev << 16) | lmp_subver;
			}
			kfree_skb(skb);
			if (sec->image_ver != ver) {
				RTKBT_DBG("version mismatch, image (%04x:%02x),"
					  " (%08x:%08x)", sec->image_id,
					  sec->index, sec->image_ver, ver);
				ret = -EINVAL;
				goto version_end;
			}
		}
	}
	ret = 0;
version_end:
	if (ret) {
		send_reset_command(xdata);
		retry++;
		if (retry < 2) {
			RTKBT_WARN("%s: check version again", __func__);
			goto get_ver;
		}
	}

	return ret;
}


uint8_t rtk_get_eversion(dev_data * dev_entry)
{
	struct rtk_eversion_evt *eversion;
	patch_info *patch_entry;
	int ret_val = 0;
	xchange_data *xdata = NULL;

	RTKBT_DBG("%s: gEVersion %d", __func__, gEVersion);
	if (gEVersion != 0xFF && gEVersion != 0xFE) {
		RTKBT_DBG("gEVersion != 0xFF, return it directly!");
		return gEVersion;
	}

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (NULL == xdata) {
		ret_val = 0xFE;
		RTKBT_DBG("NULL == xdata");
		return ret_val;
	}

	ret_val = init_xdata(xdata, dev_entry);
	if (ret_val) {
		ret_val = 0xFE;
		RTKBT_ERR("failed to init xdata");
		goto version_end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(HCI_VENDOR_READ_RTK_ROM_VERISION);
	xdata->cmd_hdr->plen = 0;
	xdata->pkt_len = CMD_HDR_LEN;

	ret_val = send_hci_cmd(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to send read RTK rom version cmd.");
		ret_val = 0xFE;
		goto version_end;
	}

	ret_val = rcv_hci_evt(xdata);
	if (ret_val < 0) {
		RTKBT_ERR("Failed to receive HCI event for rom version.");
		ret_val = 0xFE;
		goto version_end;
	}

	patch_entry = xdata->dev_entry->patch_entry;
	eversion = (struct rtk_eversion_evt *)(xdata->rsp_para);
	RTKBT_DBG("eversion->status = 0x%x, eversion->version = 0x%x",
		  eversion->status, eversion->version);
	if (eversion->status) {
		ret_val = 0;
		//global_eversion = 0;
	} else {
		ret_val = eversion->version;
		//global_eversion = eversion->version;
	}

version_end:
	if (xdata != NULL) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}
	return ret_val;
}

static int download_acldata(xchange_data *xdata)
{
	u8 *pcur;
	int pkt_len, frag_num, frag_len;
	int len = 0;
	int i, ret = -EINVAL;
	int j = 0;
	int loop = 1;
	u8 index;
	u16 max_payload_len = xdata->acldata_pkt_len;
	struct hci_acl_hdr *hdr;
	u8 *buf;
	u8 *evt;
	int max_evt_len = 1 + 1 + 255;
	int pipe_bulk_out;
	int pipe_intr_in = xdata->pipe_in;
	struct usb_device *udev = xdata->dev_entry->udev;
	struct hci_ev_patch_status_evt *ev;
	int loaded_len = 0;
	u16 handle = xdata->handle | 0x8000;
	u8 acl_cnt;
	u8 *mem = NULL;
	u8 *p;

	RTKBT_DBG("download_acldata start");

	pipe_bulk_out = usb_sndbulkpipe(udev, 0x02);
	len = sizeof(*hdr) + 1 + max_payload_len;
	if (xdata->bulk_mode) {
		len++;
		max_evt_len++;
		pipe_intr_in = usb_rcvbulkpipe(udev, 0x02);
	}
	mem = kmalloc(len, GFP_KERNEL);
	if (!mem)
		return -ENOMEM;
	buf = mem;
	if (xdata->bulk_mode) {
		*buf = 0x02;
		buf++;
	}
	evt = kmalloc(max_evt_len, GFP_KERNEL);
	if (!evt) {
		ret = -ENOMEM;
		goto err_nomem;
	}

	/* Caution: for fw of v3 format, the start index must be 1 */
	if (is_v3_fw(xdata->dev_entry->fw_type))
		j = 1;

	RTKBT_DBG("%s: fw_len %08x", __func__, xdata->fw_len);

	max_payload_len = (max_payload_len - 1) & ~0x3;
	pcur = xdata->fw_data;
	pkt_len = sizeof(*hdr) + 1 + max_payload_len;
	if (xdata->bulk_mode)
		pkt_len++;
	frag_num = xdata->fw_len / max_payload_len + 1;
	frag_len = max_payload_len;

	hdr = (struct hci_acl_hdr *)buf;
	hdr->handle = cpu_to_le16(handle);

	acl_cnt = 1;
	//acl_cnt = xdata->acl_cnt;
	for (i = 0; i < frag_num; i++) {
		index = j++;

		if (index == 0x7f)
			j = 1;

		if (i == (frag_num - 1)) {
			if (!is_v3_fw(xdata->dev_entry->fw_type))
				index |= DATA_END;
			frag_len = xdata->fw_len % max_payload_len;
			pkt_len -= (max_payload_len - frag_len);
		}
		hdr->dlen = cpu_to_le16(1 + frag_len);
		buf[sizeof(*hdr)] = index;
		memcpy(buf + sizeof(*hdr) + 1, pcur, frag_len);

		ret = usb_bulk_msg(udev, pipe_bulk_out, mem, pkt_len,
					&len, 3000);

		if (ret < 0 || (len != pkt_len)) {
			RTKBT_ERR("%s: send bulk msg error (%d), %d %d",
				  __func__, ret, len, pkt_len);
			goto err;
		}
		loaded_len += frag_len;

		acl_cnt--;
		if (acl_cnt > 0) {
			pcur += max_payload_len;
			continue;
		}

		loop = 3;
		while (--loop > 0) {
			if (xdata->bulk_mode)
				ret = usb_bulk_msg(udev, pipe_intr_in, evt,
						   max_evt_len, &len, 1000);
			else
				ret = usb_interrupt_msg(udev, pipe_intr_in, evt,
							max_evt_len, &len,
							1000);
			if (ret < 0) {
				RTKBT_ERR("%s: recv evt error (%d)",
					  __func__, ret);
				goto err;
			}
			p = evt;
			if (xdata->bulk_mode)
				p++;
			ev = (struct hci_ev_patch_status_evt *)p;
			if (ev->evt != 0xff)
				continue;
			if (len < sizeof(*ev))
				continue;
			/* if the acl_cnt is more than one, what does the
			 * ev->index mean ?
			 */
			/* if (index != ev->index) {
			 * 	ret = -EINVAL;
			 * 	RTKBT_ERR("%s: index mismatch (%u %u)",
			 * 		  __func__, index, ev->index);
			 * 	goto err;
			 * }
			 */
			if ((handle & 0xfff) != le16_to_cpu(ev->handle)) {
				ret = -EINVAL;
				RTKBT_ERR("%s: handle mismatch (%04x %04x)",
					  __func__, handle & 0xfff,
					  le16_to_cpu(ev->handle));
				goto err;
			}
			/* FIXME: Should we check the loaded_len ? */
			break;
		}
		if (loop <= 0) {
			RTKBT_ERR("%s: didn't receive patch status evt",
				  __func__);
			ret = -EIO;
			goto err;
		}

		acl_cnt++;
		pcur += max_payload_len;
	}

	kfree(mem);
	kfree(evt);
	RTKBT_DBG("download_acldata done");
	/* FIXME: How long should we wait ?
	 * By testing rtl8822cu, we get the following suitable delay time
	 */
	mdelay(5);
	return xdata->fw_len;
err:
	kfree(evt);
err_nomem:
	kfree(mem);
	return ret;
}

int download_data(xchange_data * xdata)
{
	download_cp *cmd_para;
	download_rp *evt_para;
	uint8_t *pcur;
	int pkt_len, frag_num, frag_len;
	int i, ret_val;
	int j = 0;

	if (xdata->dev_entry->enh_dl_enabled)
		return download_acldata(xdata);

	RTKBT_DBG("download_data start");

	/* Caution: for fw of v3 format, the start index must be 1 */
	if (is_v3_fw(xdata->dev_entry->fw_type))
		j = 1;

	cmd_para = (download_cp *) xdata->req_para;
	evt_para = (download_rp *) xdata->rsp_para;
	pcur = xdata->fw_data;
	pkt_len = CMD_HDR_LEN + sizeof(download_cp);
	frag_num = xdata->fw_len / PATCH_SEG_MAX + 1;
	frag_len = PATCH_SEG_MAX;

	for (i = 0; i < frag_num; i++) {
		cmd_para->index = j++;

		if(cmd_para->index == 0x7f)
			j = 1;

		if (i == (frag_num - 1)) {
			if (!is_v3_fw(xdata->dev_entry->fw_type))
				cmd_para->index |= DATA_END;
			frag_len = xdata->fw_len % PATCH_SEG_MAX;
			pkt_len -= (PATCH_SEG_MAX - frag_len);
		}
		xdata->cmd_hdr->opcode = cpu_to_le16(DOWNLOAD_OPCODE);
		xdata->cmd_hdr->plen = sizeof(uint8_t) + frag_len;
		xdata->pkt_len = pkt_len;
		memcpy(cmd_para->data, pcur, frag_len);

		ret_val = send_hci_cmd(xdata);
		if (ret_val < 0) {
			return ret_val;
		}

		ret_val = rcv_hci_evt(xdata);
		if (ret_val < 0) {
			return ret_val;
		}

		if (0 != evt_para->status) {
			return -1;
		}

		pcur += PATCH_SEG_MAX;
	}

	RTKBT_DBG("download_data done");
	return xdata->fw_len;
}

static int recv_hci_evt_bulk_mode(xchange_data *xdata)
{
	unsigned int pipe;
	int err = -EINVAL;
	struct usb_device *udev = xdata->dev_entry->udev;
	u8 *data;
	u8 *p;
	int len = PKT_LEN;
	int i;
	int actual_length;
	struct hci_event_hdr *hdr;
	struct hci_ev_cmd_complete *cc;
	u16 opcode;
	int count = 10;

	data = kzalloc(len, GFP_ATOMIC);
	if (!data)
		return -ENOMEM;
	pipe = usb_rcvbulkpipe(udev, BULK_EP);

	while (count-- > 0) {
		for (i = 0; i < 5; i++) {
			actual_length = 0;
			err = usb_bulk_msg(udev, pipe, data, len,
					   &actual_length, 1000);
			if (!err)
				break;
		}
		if (err < 0) {
			RTKBT_ERR("%s; rx timeout or err for hci ev, err (%d)",
				  __func__, err);
			goto done;
		}

		if (actual_length < 1 + sizeof(*hdr) + sizeof(*cc)) {
			RTKBT_WARN("%s: received pkt too short %d", __func__,
				   actual_length);
			continue;
		}
		if (data[0] != HCI_EVENT_PKT) {
			RTKBT_WARN("%s: The pkt is not a hci event", __func__);
			continue;
		}
		p = data + 1;
		hdr = (struct hci_event_hdr *)p;
		if (hdr->evt != CMD_CMP_EVT) {
			RTKBT_WARN("%s: The event 0x%02x is not expected",
				   __func__, hdr->evt);
			continue;
		}
		cc = (struct hci_ev_cmd_complete *)(p + sizeof(*hdr));
		opcode = get_unaligned_le16(&cc->opcode);
		if (opcode == le16_to_cpu(xdata->cmd_hdr->opcode)) {
			memcpy(xdata->rcv_pkt, p, 2 + hdr->plen);
			util_hexdump(xdata->rcv_pkt, 2 + hdr->plen);
			break;
		}
	}
	err = 0;
done:
	kfree(data);
	return err;
}

int send_hci_cmd(xchange_data * xdata)
{
	int ret_val;

	if (xdata->bulk_mode)
		return __send_hci_cmd_bulk_mode(xdata->dev_entry->udev,
						xdata->send_pkt,
						xdata->pkt_len);

	ret_val = usb_control_msg(xdata->dev_entry->udev, xdata->pipe_out,
				  0, USB_TYPE_CLASS, 0, 0,
				  (void *)(xdata->send_pkt),
				  xdata->pkt_len, MSG_TO);

	if (ret_val < 0)
		RTKBT_ERR("%s; failed to send ctl msg for hci cmd, err %d",
			  __func__, ret_val);

	return ret_val;
}

int rcv_hci_evt(xchange_data * xdata)
{
	int ret_len = 0, ret_val = 0;
	int i;			// Added by Realtek

	if (xdata->bulk_mode)
		return recv_hci_evt_bulk_mode(xdata);

	while (1) {
		// **************************** Modifed by Realtek (begin)
		for (i = 0; i < 5; i++)	// Try to send USB interrupt message 5 times.
		{
			ret_val =
			    usb_interrupt_msg(xdata->dev_entry->udev,
					      xdata->pipe_in,
					      (void *)(xdata->rcv_pkt), PKT_LEN,
					      &ret_len, MSG_TO);
			if (ret_val >= 0)
				break;
		}
		// **************************** Modifed by Realtek (end)

		if (ret_val < 0) {
			RTKBT_ERR("%s; no usb intr msg for hci event, err %d",
				  __func__, ret_val);
			return ret_val;
		}

		if (CMD_CMP_EVT == xdata->evt_hdr->evt) {
			if (xdata->cmd_hdr->opcode == xdata->cmd_cmp->opcode)
				return ret_len;
		}
	}
}

#ifndef BT_SKB_RESERVE
#define BT_SKB_RESERVE		8
#endif

struct sk_buff *rtk_hci_cmd_sync(dev_data *dev_entry, u16 opcode, u8 plen,
				 u8 *param, u32 timeout)
{
	xchange_data *xdata = NULL;
	int ret = -EIO;
	int rp_len = 0;
	struct sk_buff *skb = NULL;
	u8 *p;
	struct hci_event_hdr *hdr;

	xdata = kzalloc(sizeof(xchange_data), GFP_KERNEL);
	if (!xdata) {
		RTKBT_ERR("allocate xdata error");
		ret = -ENOMEM;
		return NULL;
	}

	ret = init_xdata(xdata, dev_entry);
	if (ret) {
		RTKBT_ERR("failed to init xdata");
		goto end;
	}

	xdata->cmd_hdr->opcode = cpu_to_le16(opcode);
	if (param && plen) {
		xdata->cmd_hdr->plen = plen;
		memcpy(xdata->req_para, param, plen);
		xdata->pkt_len = CMD_HDR_LEN + plen;
	} else {
		xdata->pkt_len = CMD_HDR_LEN;
	}

	ret = send_hci_cmd(xdata);
	if (ret < 0) {
		RTKBT_ERR("Failed to send HCI cmd %04x, err %d", opcode, ret);
		goto end;
	}

	/* FIXME: There might be some previous events held in controller.
	 * So we needs set the proper loop count to read all events include the
	 * recent cmd complete event for this hci command.
	 */
	ret = rcv_hci_evt(xdata);
	if (ret < 0) {
		RTKBT_ERR("Failed to receive cmd complete for %04x", opcode);
		goto end;
	}
	p = xdata->rcv_pkt;
	hdr = (void *)p;
	rp_len = sizeof(*hdr) + hdr->plen;

	if (rp_len > 0) {
		skb = alloc_skb(rp_len + BT_SKB_RESERVE, GFP_KERNEL);
		if (skb) {
			skb_reserve(skb, BT_SKB_RESERVE);
		} else {
			ret = -ENOMEM;
			goto end;
		}
		memcpy(skb_put(skb, rp_len), xdata->rcv_pkt, rp_len);
		util_hexdump(skb->data, skb->len);
		skb_pull(skb, EVT_HDR_LEN + CMD_CMP_LEN);
	}

end:
	if (xdata) {
		if (xdata->send_pkt)
			kfree(xdata->send_pkt);
		if (xdata->rcv_pkt)
			kfree(xdata->rcv_pkt);
		kfree(xdata);
	}

	return ret >= 0 ? skb : NULL;
}

void print_acl(struct sk_buff *skb, int dataOut)
{
#if PRINT_ACL_DATA
	uint wlength = skb->len;
	uint icount = 0;
	u16 *handle = (u16 *) (skb->data);
	u16 dataLen = *(handle + 1);
	u8 *acl_data = (u8 *) (skb->data);
//if (0==dataOut)
	printk("%d handle:%04x,len:%d,", dataOut, *handle, dataLen);
//else
//      printk("In handle:%04x,len:%d,",*handle,dataLen);
/*	for(icount=4;(icount<wlength)&&(icount<32);icount++)
		{
			printk("%02x ",*(acl_data+icount) );
		}
	printk("\n");
*/
#endif
}

void print_command(struct sk_buff *skb)
{
#if PRINT_CMD_EVENT
	uint wlength = skb->len;
	uint icount = 0;
	u16 *opcode = (u16 *) (skb->data);
	u8 *cmd_data = (u8 *) (skb->data);
	u8 paramLen = *(cmd_data + 2);

	switch (*opcode) {
	case HCI_OP_INQUIRY:
		printk("HCI_OP_INQUIRY");
		break;
	case HCI_OP_INQUIRY_CANCEL:
		printk("HCI_OP_INQUIRY_CANCEL");
		break;
	case HCI_OP_EXIT_PERIODIC_INQ:
		printk("HCI_OP_EXIT_PERIODIC_INQ");
		break;
	case HCI_OP_CREATE_CONN:
		printk("HCI_OP_CREATE_CONN");
		break;
	case HCI_OP_DISCONNECT:
		printk("HCI_OP_DISCONNECT");
		break;
	case HCI_OP_CREATE_CONN_CANCEL:
		printk("HCI_OP_CREATE_CONN_CANCEL");
		break;
	case HCI_OP_ACCEPT_CONN_REQ:
		printk("HCI_OP_ACCEPT_CONN_REQ");
		break;
	case HCI_OP_REJECT_CONN_REQ:
		printk("HCI_OP_REJECT_CONN_REQ");
		break;
	case HCI_OP_AUTH_REQUESTED:
		printk("HCI_OP_AUTH_REQUESTED");
		break;
	case HCI_OP_SET_CONN_ENCRYPT:
		printk("HCI_OP_SET_CONN_ENCRYPT");
		break;
	case HCI_OP_REMOTE_NAME_REQ:
		printk("HCI_OP_REMOTE_NAME_REQ");
		break;
	case HCI_OP_READ_REMOTE_FEATURES:
		printk("HCI_OP_READ_REMOTE_FEATURES");
		break;
	case HCI_OP_SNIFF_MODE:
		printk("HCI_OP_SNIFF_MODE");
		break;
	case HCI_OP_EXIT_SNIFF_MODE:
		printk("HCI_OP_EXIT_SNIFF_MODE");
		break;
	case HCI_OP_SWITCH_ROLE:
		printk("HCI_OP_SWITCH_ROLE");
		break;
	case HCI_OP_SNIFF_SUBRATE:
		printk("HCI_OP_SNIFF_SUBRATE");
		break;
	case HCI_OP_RESET:
		printk("HCI_OP_RESET");
		break;
	default:
		printk("CMD");
		break;
	}
	printk(":%04x,len:%d,", *opcode, paramLen);
	for (icount = 3; (icount < wlength) && (icount < 24); icount++) {
		printk("%02x ", *(cmd_data + icount));
	}
	printk("\n");

#endif
}

void print_event(struct sk_buff *skb)
{
#if PRINT_CMD_EVENT
	uint wlength = skb->len;
	uint icount = 0;
	u8 *opcode = (u8 *) (skb->data);
	u8 paramLen = *(opcode + 1);

	switch (*opcode) {
	case HCI_EV_INQUIRY_COMPLETE:
		printk("HCI_EV_INQUIRY_COMPLETE");
		break;
	case HCI_EV_INQUIRY_RESULT:
		printk("HCI_EV_INQUIRY_RESULT");
		break;
	case HCI_EV_CONN_COMPLETE:
		printk("HCI_EV_CONN_COMPLETE");
		break;
	case HCI_EV_CONN_REQUEST:
		printk("HCI_EV_CONN_REQUEST");
		break;
	case HCI_EV_DISCONN_COMPLETE:
		printk("HCI_EV_DISCONN_COMPLETE");
		break;
	case HCI_EV_AUTH_COMPLETE:
		printk("HCI_EV_AUTH_COMPLETE");
		break;
	case HCI_EV_REMOTE_NAME:
		printk("HCI_EV_REMOTE_NAME");
		break;
	case HCI_EV_ENCRYPT_CHANGE:
		printk("HCI_EV_ENCRYPT_CHANGE");
		break;
	case HCI_EV_CHANGE_LINK_KEY_COMPLETE:
		printk("HCI_EV_CHANGE_LINK_KEY_COMPLETE");
		break;
	case HCI_EV_REMOTE_FEATURES:
		printk("HCI_EV_REMOTE_FEATURES");
		break;
	case HCI_EV_REMOTE_VERSION:
		printk("HCI_EV_REMOTE_VERSION");
		break;
	case HCI_EV_QOS_SETUP_COMPLETE:
		printk("HCI_EV_QOS_SETUP_COMPLETE");
		break;
	case HCI_EV_CMD_COMPLETE:
		printk("HCI_EV_CMD_COMPLETE");
		break;
	case HCI_EV_CMD_STATUS:
		printk("HCI_EV_CMD_STATUS");
		break;
	case HCI_EV_ROLE_CHANGE:
		printk("HCI_EV_ROLE_CHANGE");
		break;
	case HCI_EV_NUM_COMP_PKTS:
		printk("HCI_EV_NUM_COMP_PKTS");
		break;
	case HCI_EV_MODE_CHANGE:
		printk("HCI_EV_MODE_CHANGE");
		break;
	case HCI_EV_PIN_CODE_REQ:
		printk("HCI_EV_PIN_CODE_REQ");
		break;
	case HCI_EV_LINK_KEY_REQ:
		printk("HCI_EV_LINK_KEY_REQ");
		break;
	case HCI_EV_LINK_KEY_NOTIFY:
		printk("HCI_EV_LINK_KEY_NOTIFY");
		break;
	case HCI_EV_CLOCK_OFFSET:
		printk("HCI_EV_CLOCK_OFFSET");
		break;
	case HCI_EV_PKT_TYPE_CHANGE:
		printk("HCI_EV_PKT_TYPE_CHANGE");
		break;
	case HCI_EV_PSCAN_REP_MODE:
		printk("HCI_EV_PSCAN_REP_MODE");
		break;
	case HCI_EV_INQUIRY_RESULT_WITH_RSSI:
		printk("HCI_EV_INQUIRY_RESULT_WITH_RSSI");
		break;
	case HCI_EV_REMOTE_EXT_FEATURES:
		printk("HCI_EV_REMOTE_EXT_FEATURES");
		break;
	case HCI_EV_SYNC_CONN_COMPLETE:
		printk("HCI_EV_SYNC_CONN_COMPLETE");
		break;
	case HCI_EV_SYNC_CONN_CHANGED:
		printk("HCI_EV_SYNC_CONN_CHANGED");
		break;
	case HCI_EV_SNIFF_SUBRATE:
		printk("HCI_EV_SNIFF_SUBRATE");
		break;
	case HCI_EV_EXTENDED_INQUIRY_RESULT:
		printk("HCI_EV_EXTENDED_INQUIRY_RESULT");
		break;
	case HCI_EV_IO_CAPA_REQUEST:
		printk("HCI_EV_IO_CAPA_REQUEST");
		break;
	case HCI_EV_SIMPLE_PAIR_COMPLETE:
		printk("HCI_EV_SIMPLE_PAIR_COMPLETE");
		break;
	case HCI_EV_REMOTE_HOST_FEATURES:
		printk("HCI_EV_REMOTE_HOST_FEATURES");
		break;
	default:
		printk("event");
		break;
	}
	printk(":%02x,len:%d,", *opcode, paramLen);
	for (icount = 2; (icount < wlength) && (icount < 24); icount++) {
		printk("%02x ", *(opcode + icount));
	}
	printk("\n");

#endif
}
