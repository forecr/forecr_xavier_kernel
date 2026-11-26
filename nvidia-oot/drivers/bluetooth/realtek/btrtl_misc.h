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
#include <linux/version.h>
#include <linux/pm_runtime.h>
#include <linux/usb.h>
#include <linux/suspend.h>

#define CONFIG_BTUSB_AUTOSUSPEND	0

/* Download LPS patch when host suspends or power off
 *   LPS patch name:  lps_rtl8xxx_fw
 *   LPS config name: lps_rtl8xxx_config
 * Download normal patch when host resume or power on */
/* #define RTKBT_SWITCH_PATCH */

/* RTKBT Power-on for sideband wake-up by LE Advertising from Remote. */
/* Note that it's necessary to apply TV FW Patch. */
/* #define RTKBT_SUSPEND_WAKEUP */
/* #define RTKBT_SHUTDOWN_WAKEUP */
#define RTKBT_POWERKEY_WAKEUP

/* RTKBT Power-on Whitelist for sideband wake-up by LE Advertising from Remote.
 * Note that it's necessary to apply TV FW Patch. */
/* #define RTKBT_TV_POWERON_WHITELIST */

#if 1
#define RTKBT_DBG(fmt, arg...) printk(KERN_DEBUG "btrtl_usb: " fmt "\n" , ## arg)
#define RTKBT_INFO(fmt, arg...) printk(KERN_INFO "btrtl_usb: " fmt "\n" , ## arg)
#define RTKBT_WARN(fmt, arg...) printk(KERN_WARNING "btrtl_usb: " fmt "\n", ## arg)
#else
#define RTKBT_DBG(fmt, arg...)
#endif

#if 1
#define RTKBT_ERR(fmt, arg...) printk(KERN_ERR "btrtl_usb: " fmt "\n" , ## arg)
#else
#define RTKBT_ERR(fmt, arg...)
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 33)
#define USB_RPM		1
#else
#define USB_RPM		0
#endif

#define CONFIG_NEEDS_BINDING

/* If module is still powered when kernel suspended, there is no re-binding. */
#ifdef RTKBT_SWITCH_PATCH
#undef CONFIG_NEEDS_BINDING
#endif

/* USB SS */
#if (CONFIG_BTUSB_AUTOSUSPEND && USB_RPM)
#define BTUSB_RPM
#endif

#define PRINT_CMD_EVENT			0
#define PRINT_ACL_DATA			0

#define CFG_TYPE_BT		0x01
#define CFG_TYPE_APCF_FILTER	0x02
#define CFG_TYPE_APCF_WAKEUP	0x04

#define APCF_CONFIG_FILTER	"/opt/rtk_btapcf_filter"
#define APCF_CONFIG_WAKEUP	"/opt/rtk_btapcf_wakeup"

struct cfg_apcf_item {
	struct list_head list;
	u32 len;
	u8 data[0];
};

#define DEVICE_STATE_OFFLINE	0
#define DEVICE_STATE_ONLINE	1
#define DEVICE_STATE_LOADING	2
#define DEVICE_STATE_LOADED	3
extern struct hci_dev __rcu *controller;

typedef struct {
	uint16_t prod_id;
	uint16_t lmp_sub;
	char *	 mp_patch_name;
	char *	 patch_name;
	char *	 config_name;
	u8       chip_type;
} patch_info;

typedef struct {
	struct list_head list_node;
	struct usb_interface *intf;
	struct usb_device *udev;
	patch_info *patch_entry;
	u32 opcode;
	u8 fw_type;
	u8 enh_dl_enabled;
	u8 *cfg_buf;
	u16 cfg_len;
} dev_data;

extern int patch_add(struct usb_interface *intf);
extern void patch_remove(struct usb_interface *intf);
extern int download_patch(struct usb_interface *intf);
extern void print_event(struct sk_buff *skb);
extern void print_command(struct sk_buff *skb);
extern void print_acl(struct sk_buff *skb, int dataOut);
extern void config_file_proc(const char *path, int type);
extern void drain_apcf_cfg(struct list_head *head, int type);
extern struct sk_buff *rtk_hci_cmd_sync(dev_data *dev_entry, u16 opcode,
					u8 plen, u8 *param, u32 timeout);

int __rtk_send_hci_cmd(struct usb_device *udev, u8 *buf, u16 size);

#ifdef RTKBT_SWITCH_PATCH
#define RTLBT_CLOSE	(1 << 0)
struct api_context {
	u32			flags;
	struct completion	done;
	int			status;
};

int download_special_patch(struct usb_interface *intf, const char *special_name);
#endif

int setup_btrealtek_flag(struct usb_interface *intf, struct hci_dev *hdev);

enum {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
	REALTEK_ALT6_CONTINUOUS_TX_CHIP,
#endif

	__REALTEK_NUM_FLAGS,
};

struct btrealtek_data {
	DECLARE_BITMAP(flags, __REALTEK_NUM_FLAGS);
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0)
static inline void *hci_get_priv(struct hci_dev *hdev)
{
	return (char *)hdev + sizeof(*hdev);
}
#endif

#define btrealtek_set_flag(hdev, nr)					\
	do {								\
		struct btrealtek_data *realtek = hci_get_priv((hdev));	\
		set_bit((nr), realtek->flags);				\
	} while (0)

#define btrealtek_get_flag(hdev)						\
	(((struct btrealtek_data *)hci_get_priv(hdev))->flags)

#define btrealtek_test_flag(hdev, nr)	test_bit((nr), btrealtek_get_flag(hdev))

#if defined RTKBT_SUSPEND_WAKEUP || defined RTKBT_SHUTDOWN_WAKEUP || defined RTKBT_SWITCH_PATCH
int set_scan(struct usb_interface *intf);
#endif
