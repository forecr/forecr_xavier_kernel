/*
 *
 *  Realtek Bluetooth USB driver
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
#include <linux/reboot.h>
#include <net/sock.h>
#include <linux/version.h>
#include <linux/rcupdate.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
#include <asm/unaligned.h>
#else
#include <linux/unaligned.h>
#endif
#include "linux/vmalloc.h"

#include "btrtl_usb.h"
#include "btrtl_misc.h"

#define VERSION "3.1.30b8810.20260528-131345"

#ifdef BTCOEX
#include "btrtl_coex.h"
#endif

static DEFINE_MUTEX(sync_lock);
static DECLARE_WAIT_QUEUE_HEAD(sync_wq);

#ifdef RTKBT_SWITCH_PATCH
#include <linux/semaphore.h>
#include <net/bluetooth/hci_core.h>
static DEFINE_SEMAPHORE(switch_sem);
#endif

#ifndef HCI_ISODATA_PKT
#define HCI_ISODATA_PKT		0x05
#endif
#ifndef HCI_ISO_HDR_SIZE
#define HCI_ISO_HDR_SIZE     4
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 7, 1)
static bool reset = true;
#endif

static struct usb_driver btusb_driver;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static u16 iso_min_conn_handle = 0x1b;
#endif

static const struct usb_device_id btusb_table[] = {
	/* Generic Bluetooth USB device */
	{ USB_DEVICE_INFO(0xe0, 0x01, 0x01) },

	/* Generic Bluetooth USB interface */
	{ USB_INTERFACE_INFO(0xe0, 0x01, 0x01) },

	{}
};

static const struct usb_device_id blacklist_table[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0bda,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x13d3,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0489,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x1358,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x04ca,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x2ff8,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0b05,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0930,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x10ec,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x04c5,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0cb5,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x0cb8,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x04b8,
	}, {
		.match_flags = USB_DEVICE_ID_MATCH_VENDOR,
		.idVendor = 0x04e8,
	}, { }
};

static void rtk_free(struct btusb_data *data)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 1)
	kfree(data);
#endif
	return;
}

static struct btusb_data *rtk_alloc(struct usb_interface *intf)
{
	struct btusb_data *data;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 1)
	data = kzalloc(sizeof(*data), GFP_KERNEL);
#else
	data = devm_kzalloc(&intf->dev, sizeof(*data), GFP_KERNEL);
#endif
	return data;
}

MODULE_DEVICE_TABLE(usb, btusb_table);

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
static inline void btusb_free_frags(struct btusb_data *data)
{
	unsigned long flags;

	spin_lock_irqsave(&data->rxlock, flags);

	kfree_skb(data->evt_skb);
	data->evt_skb = NULL;

	kfree_skb(data->acl_skb);
	data->acl_skb = NULL;

	kfree_skb(data->sco_skb);
	data->sco_skb = NULL;

	spin_unlock_irqrestore(&data->rxlock, flags);
}

static int btusb_recv_intr(struct btusb_data *data, void *buffer, int count)
{
	struct sk_buff *skb;
	int err = 0;

	spin_lock(&data->rxlock);
	skb = data->evt_skb;

	while (count) {
		int len;

		if (!skb) {
			skb = bt_skb_alloc(HCI_MAX_EVENT_SIZE, GFP_ATOMIC);
			if (!skb) {
				err = -ENOMEM;
				break;
			}

			bt_cb(skb)->pkt_type = HCI_EVENT_PKT;
			bt_cb(skb)->expect = HCI_EVENT_HDR_SIZE;
		}

		len = min_t(uint, bt_cb(skb)->expect, count);
#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
		skb_put_data(skb, buffer, len);
#else
		memcpy(skb_put(skb, len), buffer, len);
#endif

		count -= len;
		buffer += len;
		bt_cb(skb)->expect -= len;

		if (skb->len == HCI_EVENT_HDR_SIZE) {
			/* Complete event header */
			bt_cb(skb)->expect = hci_event_hdr(skb)->plen;

			if (skb_tailroom(skb) < bt_cb(skb)->expect) {
				kfree_skb(skb);
				skb = NULL;

				err = -EILSEQ;
				break;
			}
		}

		if (bt_cb(skb)->expect == 0) {
			/* Complete frame */
			hci_recv_frame(data->hdev, skb);
			skb = NULL;
		}
	}

	data->evt_skb = skb;
	spin_unlock(&data->rxlock);

	return err;
}

#define PKT_START	0x01
#define PKT_HDR		0x02
#define PKT_DATA	0x03

static int btusb_recv_bulk_mode(struct btusb_data *data, void *buffer,
				int count)
{
	struct sk_buff *skb;
	int err = 0;
	u8 *tbuf;
	u8 *buf = buffer;
	int i = 0;
	u16 tlen;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	u16 handle;
#endif

	spin_lock(&data->rxlock);
	skb = data->acl_skb;

	while (count) {
		if (!skb) {
			skb = bt_skb_alloc(HCI_MAX_FRAME_SIZE, GFP_ATOMIC);
			if (!skb) {
				err = -ENOMEM;
				break;
			}

			bt_cb(skb)->pkt_type = PKT_START;
		}

		switch (bt_cb(skb)->pkt_type) {
		case PKT_START:
			tbuf = skb_push(skb, 1);
			*tbuf = buf[i];
			count--;
			i++;
			switch (*tbuf) {
			case HCI_EVENT_PKT:
				bt_cb(skb)->expect = HCI_EVENT_HDR_SIZE + 1;
				break;
			case HCI_ACLDATA_PKT:
				bt_cb(skb)->expect = HCI_ACL_HDR_SIZE + 1;
				break;
			case HCI_SCODATA_PKT:
				bt_cb(skb)->expect = HCI_SCO_HDR_SIZE + 1;
				break;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
			case HCI_ISODATA_PKT:
				bt_cb(skb)->expect = HCI_ISO_HDR_SIZE + 1;
				break;
#endif
			default:
				RTKBT_ERR("%s: Unknown pkt type 0x%02x",
					  __func__, *tbuf);
				/* Continue search until we find the right pkt
				 * type
				 */
				continue;
			}
			bt_cb(skb)->pkt_type = PKT_HDR;
			break;
		case PKT_HDR:
			memcpy(skb_put(skb, 1), &buf[i], 1);
			count--;
			i++;
			if (skb->len == bt_cb(skb)->expect) {
				u16 plen = 0;

				tbuf = skb->data;
				switch (*tbuf) {
				case HCI_EVENT_PKT:
					plen = tbuf[2];
					break;
				case HCI_ACLDATA_PKT:
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
					handle = get_unaligned_le16(&tbuf[1]) & 0xfff;
					if(handle >= iso_min_conn_handle) {
						*tbuf = HCI_ISODATA_PKT;
					}
#endif
					plen = get_unaligned_le16(&tbuf[3]);
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
					if (*tbuf == HCI_ISODATA_PKT)
						plen &= 0x3fff;
#endif
					break;
				case HCI_SCODATA_PKT:
					plen = tbuf[3];
					break;
				case HCI_ISODATA_PKT:
					plen = get_unaligned_le16(&tbuf[3]);
					plen &= 0x3fff;
					break;
				}

				/* Empty payload */
				if (!plen) {
					bt_cb(skb)->pkt_type = *tbuf;
					bt_cb(skb)->expect = 0;
					skb_pull(skb, 1);
					hci_recv_frame(data->hdev, skb);
					skb = NULL;
					/* go back to the outer while */
					break;
				}

				bt_cb(skb)->expect += plen;
				bt_cb(skb)->pkt_type = PKT_DATA;
			}
			break;
		case PKT_DATA:
			if (skb->len + count < bt_cb(skb)->expect)
				tlen = count;
			else
				tlen = bt_cb(skb)->expect - skb->len;
			tbuf = skb_put(skb, tlen);
			memcpy(tbuf, buf + i, tlen);
			count -= tlen;
			i += tlen;
			if (skb->len == bt_cb(skb)->expect) {
				tbuf = skb->data;
				bt_cb(skb)->pkt_type = *tbuf;
				bt_cb(skb)->expect = 0;
				skb_pull(skb, 1);
#ifdef BTCOEX
				switch (*tbuf) {
				case HCI_EVENT_PKT:
					rtk_btcoex_parse_event(skb->data,
							       skb->len);
					break;
				case HCI_ACLDATA_PKT:
					rtk_btcoex_parse_l2cap_data_rx(skb->data,
								       skb->len);
					break;
				}
#endif
				hci_recv_frame(data->hdev, skb);
				skb = NULL;
			}
			break;
		default:
			RTKBT_ERR("%s: Unknown rx state %u", __func__,
				  bt_cb(skb)->pkt_type);
			count--;
			i++;
			break;
		}
	}

	data->acl_skb = skb;
	spin_unlock(&data->rxlock);

	return err;
}

static int btusb_recv_bulk(struct btusb_data *data, void *buffer, int count)
{
	struct sk_buff *skb;
	int err = 0;

	if (test_bit(BTUSB_BULK_MODE, &data->flags))
		return btusb_recv_bulk_mode(data, buffer, count);

	spin_lock(&data->rxlock);
	skb = data->acl_skb;

	while (count) {
		int len;

		if (!skb) {
			skb = bt_skb_alloc(HCI_MAX_FRAME_SIZE, GFP_ATOMIC);
			if (!skb) {
				err = -ENOMEM;
				break;
			}

			bt_cb(skb)->pkt_type = HCI_ACLDATA_PKT;
			bt_cb(skb)->expect = HCI_ACL_HDR_SIZE;
		}

		len = min_t(uint, bt_cb(skb)->expect, count);
#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
		skb_put_data(skb, buffer, len);
#else
		memcpy(skb_put(skb, len), buffer, len);
#endif

		count -= len;
		buffer += len;
		bt_cb(skb)->expect -= len;

		if (skb->len == HCI_ACL_HDR_SIZE) {
			struct hci_acl_hdr *h = hci_acl_hdr(skb);
			__le16 dlen = h->dlen;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
			__u16 handle = __le16_to_cpu(h->handle) & 0xfff;

			if(handle >= iso_min_conn_handle) {
				bt_cb(skb)->pkt_type = HCI_ISODATA_PKT;
			}
#endif
			/* Complete ACL header */
			bt_cb(skb)->expect = __le16_to_cpu(dlen);

			if (skb_tailroom(skb) < bt_cb(skb)->expect) {
				kfree_skb(skb);
				skb = NULL;

				err = -EILSEQ;
				break;
			}
		}

		if (bt_cb(skb)->expect == 0) {
			/* Complete frame */
			hci_recv_frame(data->hdev, skb);
			skb = NULL;
		}
	}

	data->acl_skb = skb;
	spin_unlock(&data->rxlock);

	return err;
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
static int btrtl_usb_recv_isoc(u16 pos, u8 *data, u8 *p, int len,
			u16 wMaxPacketSize)
{
	u8 *prev;

	if (pos >= HCI_SCO_HDR_SIZE && pos >= wMaxPacketSize &&
	    len == wMaxPacketSize && !(pos % wMaxPacketSize) &&
	    wMaxPacketSize >= 10 && p[0] == data[0] && p[1] == data[1]) {

		prev = data + (pos - wMaxPacketSize);

		/* Detect the sco data of usb isoc pkt duplication. */
		if (!memcmp(p + 2, prev + 2, 8))
			return -EILSEQ;

		if (wMaxPacketSize >= 12 &&
		    p[2] == prev[6] && p[3] == prev[7] &&
		    p[4] == prev[4] && p[5] == prev[5] &&
		    p[6] == prev[10] && p[7] == prev[11] &&
		    p[8] == prev[8] && p[9] == prev[9]) {
			return -EILSEQ;
		}
	}

	return 0;
}
#endif

static int btusb_recv_isoc(struct btusb_data *data, void *buffer, int count)
{
	struct sk_buff *skb;
	int err = 0;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
	u16 wMaxPacketSize = le16_to_cpu(data->isoc_rx_ep->wMaxPacketSize);
#endif

	spin_lock(&data->rxlock);
	skb = data->sco_skb;

	while (count) {
		int len;

		if (!skb) {
			skb = bt_skb_alloc(HCI_MAX_SCO_SIZE, GFP_ATOMIC);
			if (!skb) {
				err = -ENOMEM;
				break;
			}

			bt_cb(skb)->pkt_type = HCI_SCODATA_PKT;
			bt_cb(skb)->expect = HCI_SCO_HDR_SIZE;
		}

		len = min_t(uint, bt_cb(skb)->expect, count);

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		/* Gaps in audio could be heard while streaming WBS using USB
		 * alt settings 3 on some platforms.
		 * Add the function to detect it.
		 */
		if (test_bit(BTUSB_USE_ALT3_FOR_WBS, &data->flags)) {
			err = btrtl_usb_recv_isoc(skb->len, skb->data, buffer,
					len, wMaxPacketSize);
			if (err)
				break;
		}
#endif
#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
		skb_put_data(skb, buffer, len);
#else
		memcpy(skb_put(skb, len), buffer, len);
#endif

		count -= len;
		buffer += len;
		bt_cb(skb)->expect -= len;

		if (skb->len == HCI_SCO_HDR_SIZE) {
			/* Complete SCO header */
			bt_cb(skb)->expect = hci_sco_hdr(skb)->dlen;

			if (skb_tailroom(skb) < bt_cb(skb)->expect) {
				kfree_skb(skb);
				skb = NULL;

				err = -EILSEQ;
				break;
			}
		}

		if (bt_cb(skb)->expect == 0) {
			/* Complete frame */
			hci_recv_frame(data->hdev, skb);
			skb = NULL;
		}
	}

	data->sco_skb = skb;
	spin_unlock(&data->rxlock);

	return err;
}
#else
static int inc_tx(struct btusb_data *data)
{
	unsigned long flags;
	int rv;

	spin_lock_irqsave(&data->txlock, flags);
	rv = test_bit(BTUSB_SUSPENDING, &data->flags);
	if (!rv)
		data->tx_in_flight++;
	spin_unlock_irqrestore(&data->txlock, flags);

	return rv;
}

#endif

static void btusb_intr_complete(struct urb *urb)
{
	struct hci_dev *hdev = urb->context;
	struct btusb_data *data = GET_DRV_DATA(hdev);
	int err;

	//RTKBT_DBG("%s: urb %p status %d count %d ", __func__,
	//urb, urb->status, urb->actual_length);

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return;

	if (urb->status == 0) {
		hdev->stat.byte_rx += urb->actual_length;

#ifdef BTCOEX
		rtk_btcoex_parse_event(urb->transfer_buffer,
				urb->actual_length);
#endif
#if HCI_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
		if (hci_recv_fragment(hdev, HCI_EVENT_PKT,
				      urb->transfer_buffer,
				      urb->actual_length) < 0) {
			RTKBT_ERR("%s: Corrupted event packet", __func__);
			hdev->stat.err_rx++;
		}
#else
		if (btusb_recv_intr(data, urb->transfer_buffer,
				    urb->actual_length) < 0) {
			RTKBT_ERR("%s corrupted event packet", hdev->name);
			hdev->stat.err_rx++;
		}
#endif
	}
	/* Avoid suspend failed when usb_kill_urb */
	else if (urb->status == -ENOENT) {
		return;
	}

	if (!test_bit(BTUSB_INTR_RUNNING, &data->flags))
		return;

	usb_mark_last_busy(data->udev);
	usb_anchor_urb(urb, &data->intr_anchor);

	err = usb_submit_urb(urb, GFP_ATOMIC);
	if (err < 0) {
		/* -EPERM: urb is being killed;
		 * -ENODEV: device got disconnected */
		if (err != -EPERM && err != -ENODEV)
			RTKBT_ERR("%s: Failed to re-submit urb %p, err %d",
				  __func__, urb, err);
		usb_unanchor_urb(urb);
	}
}

static int btusb_submit_intr_urb(struct hci_dev *hdev, gfp_t mem_flags)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	struct urb *urb;
	unsigned char *buf;
	unsigned int pipe;
	int err, size;

	//RTKBT_DBG("%s", hdev->name);

	if (!data->intr_ep)
		return -ENODEV;

	urb = usb_alloc_urb(0, mem_flags);
	if (!urb)
		return -ENOMEM;

	size = le16_to_cpu(data->intr_ep->wMaxPacketSize);

	buf = kmalloc(size, mem_flags);
	if (!buf) {
		usb_free_urb(urb);
		return -ENOMEM;
	}

	pipe = usb_rcvintpipe(data->udev, data->intr_ep->bEndpointAddress);

	usb_fill_int_urb(urb, data->udev, pipe, buf, size,
			 btusb_intr_complete, hdev, data->intr_ep->bInterval);

	urb->transfer_flags |= URB_FREE_BUFFER;

	usb_anchor_urb(urb, &data->intr_anchor);

	err = usb_submit_urb(urb, mem_flags);
	if (err < 0) {
		RTKBT_ERR
		    ("btusb_submit_intr_urb %s urb %p submission failed (%d)",
		     hdev->name, urb, -err);
		usb_unanchor_urb(urb);
	}

	usb_free_urb(urb);

	return err;
}

static void btusb_bulk_complete(struct urb *urb)
{
	struct hci_dev *hdev = urb->context;
	struct btusb_data *data = GET_DRV_DATA(hdev);
	int err;

	//RTKBT_DBG("%s: urb %p status %d count %d",
	//__func__, urb, urb->status, urb->actual_length);

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return;

#ifdef BTCOEX
	if (urb->status == 0)
		rtk_btcoex_parse_l2cap_data_rx(urb->transfer_buffer,
				urb->actual_length);
#endif

	if (urb->status == 0) {
		hdev->stat.byte_rx += urb->actual_length;

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
		if (hci_recv_fragment(hdev, HCI_ACLDATA_PKT,
				      urb->transfer_buffer,
				      urb->actual_length) < 0) {
			RTKBT_ERR("%s: Corrupted ACL packet", __func__);
			hdev->stat.err_rx++;
		}
#else
		if (data->recv_bulk(data, urb->transfer_buffer,
				    urb->actual_length) < 0) {
			RTKBT_ERR("%s corrupted ACL packet", hdev->name);
			hdev->stat.err_rx++;
		}
#endif
	}
	/* Avoid suspend failed when usb_kill_urb */
	else if (urb->status == -ENOENT) {
		return;
	}

	if (!test_bit(BTUSB_BULK_RUNNING, &data->flags))
		return;

	usb_anchor_urb(urb, &data->bulk_anchor);
	usb_mark_last_busy(data->udev);

	err = usb_submit_urb(urb, GFP_ATOMIC);
	if (err < 0) {
		/* -EPERM: urb is being killed;
		 * -ENODEV: device got disconnected */
		if (err != -EPERM && err != -ENODEV)
			RTKBT_ERR
			    ("btusb_bulk_complete %s urb %p failed to resubmit (%d)",
			     hdev->name, urb, -err);
		usb_unanchor_urb(urb);
	}
}

static int btusb_submit_bulk_urb(struct hci_dev *hdev, gfp_t mem_flags)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	struct urb *urb;
	unsigned char *buf;
	unsigned int pipe;
	int err, size = HCI_MAX_FRAME_SIZE;

	//RTKBT_DBG("%s: hdev name %s", __func__, hdev->name);

	if (!data->bulk_rx_ep)
		return -ENODEV;

	urb = usb_alloc_urb(0, mem_flags);
	if (!urb)
		return -ENOMEM;

	buf = kmalloc(size, mem_flags);
	if (!buf) {
		usb_free_urb(urb);
		return -ENOMEM;
	}

	pipe = usb_rcvbulkpipe(data->udev, data->bulk_rx_ep->bEndpointAddress);

	usb_fill_bulk_urb(urb, data->udev, pipe,
			  buf, size, btusb_bulk_complete, hdev);

	urb->transfer_flags |= URB_FREE_BUFFER;

	usb_mark_last_busy(data->udev);
	usb_anchor_urb(urb, &data->bulk_anchor);

	err = usb_submit_urb(urb, mem_flags);
	if (err < 0) {
		RTKBT_ERR("%s: Failed to submit urb %p, err %d", __func__, urb,
			  err);
		usb_unanchor_urb(urb);
	}

	usb_free_urb(urb);

	return err;
}

static void btusb_isoc_complete(struct urb *urb)
{
	struct hci_dev *hdev = urb->context;
	struct btusb_data *data = GET_DRV_DATA(hdev);
	int i, err;

	/*
	   RTKBT_DBG("%s urb %p status %d count %d", hdev->name,
	   urb, urb->status, urb->actual_length);
	 */
	if (!test_bit(HCI_RUNNING, &hdev->flags))
		return;

	if (urb->status == 0) {
		for (i = 0; i < urb->number_of_packets; i++) {
			unsigned int offset = urb->iso_frame_desc[i].offset;
			unsigned int length =
			    urb->iso_frame_desc[i].actual_length;

			if (urb->iso_frame_desc[i].status)
				continue;

			hdev->stat.byte_rx += length;

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
			if (hci_recv_fragment(hdev, HCI_SCODATA_PKT,
					      urb->transfer_buffer + offset,
					      length) < 0) {
				RTKBT_ERR("%s: Corrupted SCO packet", __func__);
				hdev->stat.err_rx++;
			}
#else
			if (btusb_recv_isoc(data, urb->transfer_buffer + offset,
					    length) < 0) {
				RTKBT_ERR("%s corrupted SCO packet",
					  hdev->name);
				hdev->stat.err_rx++;
			}
#endif
		}
	}
	/* Avoid suspend failed when usb_kill_urb */
	else if (urb->status == -ENOENT) {
		return;
	}

	if (!test_bit(BTUSB_ISOC_RUNNING, &data->flags))
		return;

	usb_anchor_urb(urb, &data->isoc_anchor);
	i = 0;
retry:
	err = usb_submit_urb(urb, GFP_ATOMIC);
	if (err < 0) {
		/* -EPERM: urb is being killed;
		 * -ENODEV: device got disconnected */
		if (err != -EPERM && err != -ENODEV)
			RTKBT_ERR
			    ("%s: Failed to re-sumbit urb %p, retry %d, err %d",
			     __func__, urb, i, err);
		if (i < 10) {
			i++;
			mdelay(1);
			goto retry;
		}

		usb_unanchor_urb(urb);
	}
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
static inline void __fill_isoc_descriptor_msbc(struct urb *urb, int len,
					       int mtu, struct btusb_data *data)
{
	int i = 0, offset = 0;
	unsigned int interval;

	BT_DBG("len %d mtu %d", len, mtu);

	/* For mSBC ALT 6 settings some Realtek chips need to transmit the data
	 * continuously without the zero length of USB packets.
	 */
	if (btrealtek_test_flag(data->hdev, REALTEK_ALT6_CONTINUOUS_TX_CHIP))
		goto ignore_usb_alt6_packet_flow;

	/* For mSBC ALT 6 setting the host will send the packet at continuous
	 * flow. As per core spec 5, vol 4, part B, table 2.1. For ALT setting
	 * 6 the HCI PACKET INTERVAL should be 7.5ms for every usb packets.
	 * To maintain the rate we send 63bytes of usb packets alternatively for
	 * 7ms and 8ms to maintain the rate as 7.5ms.
	 */
	if (data->usb_alt6_packet_flow) {
		interval = 7;
		data->usb_alt6_packet_flow = false;
	} else {
		interval = 6;
		data->usb_alt6_packet_flow = true;
	}

	for (i = 0; i < interval; i++) {
		urb->iso_frame_desc[i].offset = offset;
		urb->iso_frame_desc[i].length = offset;
	}

ignore_usb_alt6_packet_flow:
	if (len && i < BTUSB_MAX_ISOC_FRAMES) {
		urb->iso_frame_desc[i].offset = offset;
		urb->iso_frame_desc[i].length = len;
		i++;
	}

	urb->number_of_packets = i;
}
#endif

static inline void __fill_isoc_descriptor(struct urb *urb, int len, int mtu)
{
	int i, offset = 0;

	//RTKBT_DBG("len %d mtu %d", len, mtu);

	for (i = 0; i < BTUSB_MAX_ISOC_FRAMES && len >= mtu;
	     i++, offset += mtu, len -= mtu) {
		urb->iso_frame_desc[i].offset = offset;
		urb->iso_frame_desc[i].length = mtu;
	}

	if (len && i < BTUSB_MAX_ISOC_FRAMES) {
		urb->iso_frame_desc[i].offset = offset;
		urb->iso_frame_desc[i].length = len;
		i++;
	}

	urb->number_of_packets = i;
}

static int btusb_submit_isoc_urb(struct hci_dev *hdev, gfp_t mem_flags)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	struct urb *urb;
	unsigned char *buf;
	unsigned int pipe;
	int err, size;

	//RTKBT_DBG("%s", hdev->name);

	if (!data->isoc_rx_ep)
		return -ENODEV;

	urb = usb_alloc_urb(BTUSB_MAX_ISOC_FRAMES, mem_flags);
	if (!urb)
		return -ENOMEM;

	size = le16_to_cpu(data->isoc_rx_ep->wMaxPacketSize) *
	    BTUSB_MAX_ISOC_FRAMES;

	buf = kmalloc(size, mem_flags);
	if (!buf) {
		usb_free_urb(urb);
		return -ENOMEM;
	}

	pipe = usb_rcvisocpipe(data->udev, data->isoc_rx_ep->bEndpointAddress);

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 2, 14)
	usb_fill_int_urb(urb, data->udev, pipe, buf, size, btusb_isoc_complete,
			 hdev, data->isoc_rx_ep->bInterval);

	urb->transfer_flags = URB_FREE_BUFFER | URB_ISO_ASAP;
#else
	urb->dev = data->udev;
	urb->pipe = pipe;
	urb->context = hdev;
	urb->complete = btusb_isoc_complete;
	urb->interval = data->isoc_rx_ep->bInterval;

	urb->transfer_flags = URB_FREE_BUFFER | URB_ISO_ASAP;
	urb->transfer_buffer = buf;
	urb->transfer_buffer_length = size;
#endif

	__fill_isoc_descriptor(urb, size,
			       le16_to_cpu(data->isoc_rx_ep->wMaxPacketSize));

	usb_anchor_urb(urb, &data->isoc_anchor);

	err = usb_submit_urb(urb, mem_flags);
	if (err < 0) {
		RTKBT_ERR("%s %s urb %p submission failed (%d)",
			  __func__, hdev->name, urb, err);
		usb_unanchor_urb(urb);
	}

	usb_free_urb(urb);

	return err;
}

static void btusb_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct hci_dev *hdev = (struct hci_dev *)skb->dev;
	struct btusb_data *data = GET_DRV_DATA(hdev);

//      RTKBT_DBG("btusb_tx_complete %s urb %p status %d count %d", hdev->name,
//                                      urb, urb->status, urb->actual_length);

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		goto done;

	if (!urb->status)
		hdev->stat.byte_tx += urb->transfer_buffer_length;
	else
		hdev->stat.err_tx++;

done:
	spin_lock(&data->txlock);
	data->tx_in_flight--;
	spin_unlock(&data->txlock);

	kfree(urb->setup_packet);

	kfree_skb(skb);
}

static void btusb_isoc_tx_complete(struct urb *urb)
{
	struct sk_buff *skb = urb->context;
	struct hci_dev *hdev = (struct hci_dev *)skb->dev;

	RTKBT_DBG("%s: urb %p status %d count %d", __func__,
			urb, urb->status, urb->actual_length);

	if (!test_bit(HCI_RUNNING, &hdev->flags))
		goto done;

	if (!urb->status)
		hdev->stat.byte_tx += urb->transfer_buffer_length;
	else
		hdev->stat.err_tx++;

done:
	kfree(urb->setup_packet);

	kfree_skb(skb);
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static int rtl_read_iso_handle_range(struct hci_dev *hdev)
{
	struct sk_buff *skb;
	struct __rp {
		u8 status;
		u8 min_handle[2];
	} *rp;
	int ret = -EIO;

	skb = __hci_cmd_sync(hdev, 0xfdab, 0, NULL, HCI_CMD_TIMEOUT);
	if (IS_ERR(skb)) {
		return PTR_ERR(skb);
	}

	/* FIXME: if the return status is not zero, __hci_cmd_sync() would
	 * return an error and we would not reach here.
	 */
	if (skb->data[0]) {
		RTKBT_ERR("%s: Read failed, status %0x", hdev->name,
			  skb->data[0]);
		goto err;
	}

	if (skb->len < sizeof(*rp)) {
		RTKBT_WARN("%s: The len %u of rp is too short", __func__,
			  skb->len);
		goto err;
	}

	rp = (void *)skb->data;
	iso_min_conn_handle = (u16)rp->min_handle[1] << 8 | rp->min_handle[0];
	RTKBT_DBG("ISO handle range (handle >= %04x)", iso_min_conn_handle);

	kfree_skb(skb);

	return 0;
err:
	kfree_skb(skb);
	return ret;
}
#endif

static int download_patch_wrapper(struct usb_interface *intf)
{
	struct btusb_data *data;
	int ret = 0;

	RTKBT_INFO("download_patch_wrapper");

	mutex_lock(&sync_lock);

	data = usb_get_intfdata(intf);
	if (!data) {
		RTKBT_ERR("No btdata");
		ret = -ENODATA;
		goto done;
	}

	if (test_bit(BTUSB_DETACHED, &data->flags)) {
		RTKBT_ERR("BT dev has been detached");
		ret = -ENODEV;
		goto unlock;
	}

	/* Maybe the caller has been set the state to loading but it's harmless.
	 */
	atomic_set(&data->sync_state, DEVICE_STATE_LOADING);
	mutex_unlock(&sync_lock);

	/* From now on, it's safe to access the interface */

	mutex_lock(&data->dl_lock);
	ret = download_patch(intf);

	/* ret_val > 0 means fw has been loaded, ret_val = 0 means fw download
	 * succeeded.
	 */
	if (ret < 0)
		atomic_set(&data->sync_state, DEVICE_STATE_ONLINE);
	else
		atomic_set(&data->sync_state, DEVICE_STATE_LOADED);
	mutex_unlock(&data->dl_lock);

	/* Maybe btusb_disconnect() is waiting for downloading completed. */
	wake_up_interruptible(&sync_wq);

	return ret;
unlock:
	/* Make sure that the detach process ends properly */
	atomic_set(&data->sync_state, DEVICE_STATE_ONLINE);
done:
	/* Maybe btusb_disconnect() is waiting for downloading completed. */
	wake_up_interruptible(&sync_wq);

	mutex_unlock(&sync_lock);

	return ret;
}

int rtlbt_download_patch(void)
{
	struct hci_dev *ctrl;
	int ret = -EINVAL;
	struct usb_interface *intf = NULL;
	int state = DEVICE_STATE_OFFLINE;
	struct btusb_data *data = NULL;

	mutex_lock(&sync_lock);
	rcu_read_lock();
	ctrl = rcu_dereference_protected(controller,
					 lockdep_is_held(&sync_lock));
	if (likely(ctrl)) {
		data = hci_get_drvdata(ctrl);
		intf = data->intf;
		if (test_bit(BTUSB_DETACHED, &data->flags)) {
			rcu_read_unlock();
			ret = -ENODEV;
			goto unlock;
		}
		state = atomic_read(&data->sync_state);
	}
	rcu_read_unlock();
	ret = 0;
	switch(state) {
	case DEVICE_STATE_OFFLINE:
		ret = -ENODEV;
		break;
	case DEVICE_STATE_LOADING:
		ret = -EINPROGRESS;
		break;
	case DEVICE_STATE_LOADED:
		RTKBT_INFO("%s: fw has been loaded", __func__);
		ret = 1;
		break;
	}
	if (ret) {
		if (ret > 0)
			ret = 0;
		goto unlock;
	}
	/* online -> loading */
	atomic_set(&data->sync_state, DEVICE_STATE_LOADING);
	/* from now on, it's safe to access the intf */
	mutex_unlock(&sync_lock);

	ret = -EINVAL;
	if (intf) {
		/* sync_state will be changed in the below func */
		ret = download_patch_wrapper(intf);
	} else {
		RTKBT_ERR("this should never happen");
	}

	if (ret < 0)
		atomic_set(&data->sync_state, DEVICE_STATE_ONLINE);
	wake_up_interruptible(&sync_wq);
	return ret;

unlock:
	mutex_unlock(&sync_lock);
	wake_up_interruptible(&sync_wq);
	return ret;
}
EXPORT_SYMBOL_GPL(rtlbt_download_patch);

static int btusb_check_mode(struct btusb_data *data)
{
	struct usb_endpoint_descriptor *epout_desc = NULL;
	struct usb_endpoint_descriptor *epin_desc = NULL;
	struct usb_endpoint_descriptor *ep_desc;
	struct usb_interface *intf;
	struct usb_device *udev;
	/* unsigned int pipe;
	 * u8 *buf;
	 */
	int err;
	int i;

	if (!data)
		return -EINVAL;
	udev = data->udev;
	intf = data->intf;

	if (intf->num_altsetting < 2)
		return 0;

	if (intf->cur_altsetting->desc.bAlternateSetting != 0)
		return 0;

	for (i = 0; i < intf->altsetting[1].desc.bNumEndpoints; i++) {
		ep_desc  = &intf->altsetting[1].endpoint[i].desc;
		if (usb_endpoint_is_bulk_out(ep_desc))
			epout_desc = ep_desc;
		else if (usb_endpoint_is_bulk_in(ep_desc))
			epin_desc = ep_desc;
		if (epin_desc && epout_desc)
			break;
	}

	if (!epout_desc || !epin_desc)
		return -ENODEV;

	err = usb_set_interface(udev, 0, 1);
	if (!err) {
		usb_kill_anchored_urbs(&data->intr_anchor);
		usb_kill_anchored_urbs(&data->bulk_anchor);
		usb_kill_anchored_urbs(&data->tx_anchor);
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
		btusb_free_frags(data);
#endif
		data->bulk_tx_ep = epout_desc;
		data->bulk_rx_ep = epin_desc;

		set_bit(BTUSB_BULK_MODE, &data->flags);
	}
	RTKBT_INFO("%s: err (%d)", __func__, err);

	/* buf = kmalloc(sizeof(int), GFP_KERNEL);
	 * memset(buf, 0, sizeof(int));
	 * pipe = usb_rcvctrlpipe(udev, 0);
	 * err = usb_control_msg(udev, pipe, 1, USB_TYPE_CLASS | USB_DIR_IN, 0, 0,
	 * 		      buf, 2, 30);
	 * RTKBT_INFO("%s: get protocol, 0x%02x 0x%02x or err (%d)", __func__,
	 * 	   *buf, *(buf + 1), err);
	 * if (err > 0 && (*(buf + 1) & (1 << 1)) && *buf != 1) {
	 * 	pipe = usb_sndctrlpipe(udev, 0);
	 * 	err = usb_control_msg(udev, pipe, 2, USB_TYPE_CLASS, 1, 0,
	 * 			      NULL, 0, 20);
	 * 	RTKBT_INFO("%s: set protocol, ret (%d)", __func__, err);
	 * 	if (err >= 0)
	 * 		set_bit(BTUSB_BULK_MODE, &data->flags);
	 * }
	 */

	return 0;
}

static int btusb_open(struct hci_dev *hdev)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	int err;

	err = usb_autopm_get_interface(data->intf);
	if (err < 0)
		return err;

	data->intf->needs_remote_wakeup = 1;
	RTKBT_DBG("%s start", __func__);

	/*******************************/
	if (0 == atomic_read(&hdev->promisc)) {
		RTKBT_DBG("btusb_open hdev->promisc ==0");
		//err = -1;
		//goto failed;
	}

	btusb_check_mode(data);
	err = download_patch_wrapper(data->intf);
	if (err < 0)
		goto failed;
	/*******************************/

	err = setup_btrealtek_flag(data->intf, hdev);
	if (err < 0)
		RTKBT_WARN("setup_btrealtek_flag incorrect!");

	RTKBT_INFO("%s set HCI UP RUNNING", __func__);
	if (test_and_set_bit(HCI_UP, &hdev->flags))
		goto done;

	if (test_and_set_bit(HCI_RUNNING, &hdev->flags))
		goto done;

	if (test_and_set_bit(BTUSB_INTR_RUNNING, &data->flags))
		goto done;

	if (!test_bit(BTUSB_BULK_MODE, &data->flags)) {
		err = btusb_submit_intr_urb(hdev, GFP_KERNEL);
		if (err < 0)
			goto failed;
	}

	err = btusb_submit_bulk_urb(hdev, GFP_KERNEL);
	if (err < 0) {
		mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
		usb_kill_anchored_urbs(&data->intr_anchor);
		goto failed;
	}

	set_bit(BTUSB_BULK_RUNNING, &data->flags);
	btusb_submit_bulk_urb(hdev, GFP_KERNEL);

done:
	usb_autopm_put_interface(data->intf);

#ifdef BTCOEX
	rtk_btcoex_open(hdev);
#endif
	RTKBT_DBG("%s end", __FUNCTION__);

	return 0;

failed:
	clear_bit(BTUSB_INTR_RUNNING, &data->flags);
	clear_bit(HCI_RUNNING, &hdev->flags);
	usb_autopm_put_interface(data->intf);
	RTKBT_ERR("%s failed", __FUNCTION__);
	return err;
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static int btusb_setup(struct hci_dev *hdev)
{
	rtl_read_iso_handle_range(hdev);
	return 0;
}
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
static int btusb_shutdown(struct hci_dev *hdev)
{
	struct sk_buff *skb;
	int ret = 0;
	int locked = 0;

	/* In some kernel versions, the caller does not hold the hdev->req_lock.
	 * But the __hci_cmd_sync() requires the caller holds the hdev->req_lock
	 * to avoid race on some variables in hdev.
	 * So make sure the req_lock is locked before calling __hci_cmd_sync().
	 */
	locked = mutex_is_locked(&hdev->req_lock);
	RTKBT_INFO("req_lock state %d", locked);

	if (!locked)
		mutex_lock(&hdev->req_lock);

	skb = __hci_cmd_sync(hdev, HCI_OP_RESET, 0, NULL, HCI_INIT_TIMEOUT);
	if (IS_ERR(skb)) {
		ret = PTR_ERR(skb);
		bt_dev_err(hdev, "HCI reset during shutdown failed");
		goto done;
	}
	kfree_skb(skb);

done:
	if (!locked)
		mutex_unlock(&hdev->req_lock);
	return ret;
}
#endif

static void btusb_stop_traffic(struct btusb_data *data)
{
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_kill_anchored_urbs(&data->intr_anchor);
	usb_kill_anchored_urbs(&data->bulk_anchor);
	usb_kill_anchored_urbs(&data->isoc_anchor);
}

static int btusb_close(struct hci_dev *hdev)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	int err;

#if HCI_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
	int i;
#endif

	/* When in kernel 4.4.0 and greater, the HCI_RUNNING bit is
	 * cleared in hci_dev_do_close(). */
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	if (!test_and_clear_bit(HCI_RUNNING, &hdev->flags))
		return 0;
#else
	if (test_bit(HCI_RUNNING, &hdev->flags)) {
		RTKBT_ERR("HCI_RUNNING is not cleared before.");
		return -1;
	}
#endif

	RTKBT_DBG("btusb_close");
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 1, 0)
	/*******************************/
	for (i = 0; i < NUM_REASSEMBLY; i++) {
		if (hdev->reassembly[i]) {
			kfree_skb(hdev->reassembly[i]);
			hdev->reassembly[i] = NULL;
			RTKBT_DBG("%s free ressembly i=%d", __FUNCTION__, i);
		}
	}
	/*******************************/
#endif
	cancel_work_sync(&data->work);
	cancel_work_sync(&data->waker);

	clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
	clear_bit(BTUSB_BULK_RUNNING, &data->flags);
	clear_bit(BTUSB_INTR_RUNNING, &data->flags);

	btusb_stop_traffic(data);
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
	btusb_free_frags(data);
#endif

	err = usb_autopm_get_interface(data->intf);
	if (err < 0)
		goto failed;

	data->intf->needs_remote_wakeup = 0;
	usb_autopm_put_interface(data->intf);

#ifdef BTCOEX
	rtk_btcoex_close();
#endif

failed:
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_scuttle_anchored_urbs(&data->deferred);

#ifdef RTKBT_SWITCH_PATCH
	down(&switch_sem);
	if (data->context) {
		struct api_context *ctx = data->context;

		if (ctx->flags & RTLBT_CLOSE) {
			ctx->flags &= ~RTLBT_CLOSE;
			ctx->status = 0;
			complete(&ctx->done);
		}
	}
	up(&switch_sem);
#endif

	return 0;
}

static int btusb_flush(struct hci_dev *hdev)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);

	RTKBT_DBG("%s add delay ", __FUNCTION__);
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_kill_anchored_urbs(&data->tx_anchor);
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
	btusb_free_frags(data);
#endif

	return 0;
}

static const char pkt_ind[][8] = {
	[HCI_COMMAND_PKT] = "cmd",
	[HCI_ACLDATA_PKT] = "acl",
	[HCI_SCODATA_PKT] = "sco",
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	[HCI_ISODATA_PKT] = "iso",
#endif
};

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
static struct urb *alloc_ctrl_urb(struct hci_dev *hdev, struct sk_buff *skb)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	struct usb_ctrlrequest *dr;
	struct urb *urb;
	unsigned int pipe;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return ERR_PTR(-ENOMEM);

	dr = kmalloc(sizeof(*dr), GFP_KERNEL);
	if (!dr) {
		usb_free_urb(urb);
		return ERR_PTR(-ENOMEM);
	}

	dr->bRequestType = data->cmdreq_type;
	dr->bRequest     = 0;
	dr->wIndex       = 0;
	dr->wValue       = 0;
	dr->wLength      = __cpu_to_le16(skb->len);

	pipe = usb_sndctrlpipe(data->udev, 0x00);

	usb_fill_control_urb(urb, data->udev, pipe, (void *)dr,
			     skb->data, skb->len, btusb_tx_complete, skb);

	skb->dev = (void *)hdev;

	return urb;
}

static struct urb *alloc_bulk_urb(struct hci_dev *hdev, struct sk_buff *skb)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	struct urb *urb;
	unsigned int pipe;

	if (!data->bulk_tx_ep)
		return ERR_PTR(-ENODEV);

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return ERR_PTR(-ENOMEM);

	pipe = usb_sndbulkpipe(data->udev, data->bulk_tx_ep->bEndpointAddress);

	usb_fill_bulk_urb(urb, data->udev, pipe,
			  skb->data, skb->len, btusb_tx_complete, skb);

	skb->dev = (void *)hdev;

	return urb;
}

static struct urb *alloc_isoc_urb(struct hci_dev *hdev, struct sk_buff *skb)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	struct urb *urb;
	unsigned int pipe;

	if (!data->isoc_tx_ep || !data->isoc_tx_ep->wMaxPacketSize)
		return ERR_PTR(-ENODEV);

	urb = usb_alloc_urb(BTUSB_MAX_ISOC_FRAMES, GFP_KERNEL);
	if (!urb)
		return ERR_PTR(-ENOMEM);

	pipe = usb_sndisocpipe(data->udev, data->isoc_tx_ep->bEndpointAddress);

	usb_fill_int_urb(urb, data->udev, pipe,
			 skb->data, skb->len, btusb_isoc_tx_complete,
			 skb, data->isoc_tx_ep->bInterval);

	urb->transfer_flags  = URB_ISO_ASAP;

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
	if (data->isoc_altsetting == 6)
		__fill_isoc_descriptor_msbc(urb, skb->len,
				le16_to_cpu(data->isoc_tx_ep->wMaxPacketSize),
				data);
	else
		__fill_isoc_descriptor(urb, skb->len,
				le16_to_cpu(data->isoc_tx_ep->wMaxPacketSize));
#else
	__fill_isoc_descriptor(urb, skb->len,
				le16_to_cpu(data->isoc_tx_ep->wMaxPacketSize));
#endif

	skb->dev = (void *)hdev;

	return urb;
}

static int submit_tx_urb(struct hci_dev *hdev, struct urb *urb)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	int err;

	usb_anchor_urb(urb, &data->tx_anchor);

	err = usb_submit_urb(urb, GFP_KERNEL);
	if (err < 0) {
		if (err != -EPERM && err != -ENODEV)
			RTKBT_ERR("%s urb %p submission failed (%d)",
				   hdev->name, urb, -err);
		kfree(urb->setup_packet);
		usb_unanchor_urb(urb);
	} else {
		usb_mark_last_busy(data->udev);
	}

	usb_free_urb(urb);
	return err;
}

static int submit_or_queue_tx_urb(struct hci_dev *hdev, struct urb *urb)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	unsigned long flags;
	bool suspending;

	spin_lock_irqsave(&data->txlock, flags);
	suspending = test_bit(BTUSB_SUSPENDING, &data->flags);
	if (!suspending)
		data->tx_in_flight++;
	spin_unlock_irqrestore(&data->txlock, flags);

	if (!suspending)
		return submit_tx_urb(hdev, urb);

	usb_anchor_urb(urb, &data->deferred);
	schedule_work(&data->waker);

	usb_free_urb(urb);
	return 0;
}

#endif

#ifdef CONFIG_BTRTL_LE_ADV_ENABLE_DEFER
void btusb_send_delay_check(struct sk_buff *skb)
{
	struct hci_command_hdr *hdr;
	u16 opcode;

	hdr = (struct hci_command_hdr *)skb->data;
	opcode = le16_to_cpu(hdr->opcode);
	switch (opcode) {
	case 0x200a:
	case 0x2039:
	case 0x200c:
	case 0x2042:
               /* Avoid adv enabling and disabling too frequent.
                * Because high frequency switch might cause fw crash.
                */
		if (skb->len > sizeof(*hdr) && skb->data[sizeof(*hdr)] == 0x01)
			msleep(10);
		break;
	default:
		break;
	}
}
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
static int btusb_send_bulk_mode(u8 pkt_type, struct hci_dev *hdev,
				struct sk_buff *skb)
{
	struct urb *urb;
	u8 *buf;

	switch (pkt_type) {
	case HCI_COMMAND_PKT:
		buf = skb_push(skb, 1);
		*buf = HCI_COMMAND_PKT;
		hdev->stat.cmd_tx++;
		break;
	case HCI_ACLDATA_PKT:
		buf = skb_push(skb, 1);
		*buf = HCI_ACLDATA_PKT;
		hdev->stat.acl_tx++;
		break;
	case HCI_SCODATA_PKT:
		buf = skb_push(skb, 1);
		*buf = HCI_SCODATA_PKT;
		hdev->stat.sco_tx++;
		break;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	case HCI_ISODATA_PKT:
		buf = skb_push(skb, 1);
		*buf = HCI_ISODATA_PKT;
		hdev->stat.acl_tx++;
		break;
#endif
	default:
		return -EILSEQ;
	}

	urb = alloc_bulk_urb(hdev, skb);
	if (IS_ERR(urb))
		return PTR_ERR(urb);

	return submit_or_queue_tx_urb(hdev, urb);
}
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
int btusb_send_frame(struct hci_dev *hdev, struct sk_buff *skb)
{
#else
int btusb_send_frame(struct sk_buff *skb)
{
	struct hci_dev *hdev = (struct hci_dev *)skb->dev;
#endif

	struct urb *urb;
#if HCI_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	struct btusb_data *data = GET_DRV_DATA(hdev);
	struct usb_ctrlrequest *dr;
	unsigned int pipe;
	int err;
#else
	struct btusb_data *data = hci_get_drvdata(hdev);
#endif

//	RTKBT_DBG("%s", hdev->name);

	/* After Kernel version 4.4.0, move the check into the
	 * hci_send_frame function before calling hdev->send
	 */
#if HCI_VERSION_CODE < KERNEL_VERSION(4, 4, 0)
	if (!test_bit(HCI_RUNNING, &hdev->flags)) {
		/* If the parameter is wrong, the hdev isn't the correct
		 * one. Then no HCI commands can be sent.
		 * This issue is related to the wrong HCI_VERSION_CODE set */
		RTKBT_ERR("HCI is not running");
		return -EBUSY;
	}
#endif

	/* Before kernel/hci version 3.13.0, the skb->dev is set before
	 * entering btusb_send_frame(). So there is no need to set it here.
	 *
	 * The skb->dev will be used in the callbacks when urb transfer
	 * completes. See btusb_tx_complete() and btusb_isoc_tx_complete() */
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 13, 0) && \
    HCI_VERSION_CODE < KERNEL_VERSION(3, 18, 0)
	skb->dev = (void *)hdev;
#endif

	switch (bt_cb(skb)->pkt_type) {
	case HCI_COMMAND_PKT:
		print_command(skb);
#ifdef CONFIG_BTRTL_LE_ADV_ENABLE_DEFER
		btusb_send_delay_check(skb);
#endif
#ifdef BTCOEX
		rtk_btcoex_parse_cmd(skb->data, skb->len);
#endif
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
		if (test_bit(BTUSB_BULK_MODE, &data->flags))
			return btusb_send_bulk_mode(HCI_COMMAND_PKT, hdev, skb);
		urb = alloc_ctrl_urb(hdev, skb);
		if (IS_ERR(urb))
			return PTR_ERR(urb);

		hdev->stat.cmd_tx++;
		return submit_or_queue_tx_urb(hdev, urb);
#else
		urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!urb)
			return -ENOMEM;

		dr = kmalloc(sizeof(*dr), GFP_ATOMIC);
		if (!dr) {
			usb_free_urb(urb);
			return -ENOMEM;
		}

		dr->bRequestType = data->cmdreq_type;
		dr->bRequest = 0;
		dr->wIndex = 0;
		dr->wValue = 0;
		dr->wLength = __cpu_to_le16(skb->len);

		pipe = usb_sndctrlpipe(data->udev, 0x00);

		usb_fill_control_urb(urb, data->udev, pipe, (void *)dr,
				     skb->data, skb->len, btusb_tx_complete,
				     skb);

		hdev->stat.cmd_tx++;
		break;

#endif
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	case HCI_ISODATA_PKT:
#endif
	case HCI_ACLDATA_PKT:
		print_acl(skb, 1);
#ifdef BTCOEX
		if(bt_cb(skb)->pkt_type == HCI_ACLDATA_PKT)
			rtk_btcoex_parse_l2cap_data_tx(skb->data, skb->len);
#endif
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
		if (test_bit(BTUSB_BULK_MODE, &data->flags)) {
			if (bt_cb(skb)->pkt_type == HCI_ACLDATA_PKT)
				return btusb_send_bulk_mode(HCI_ACLDATA_PKT,
							     hdev, skb);
			else
				return btusb_send_bulk_mode(HCI_ISODATA_PKT,
							     hdev, skb);
		}
		urb = alloc_bulk_urb(hdev, skb);
		if (IS_ERR(urb))
			return PTR_ERR(urb);

		hdev->stat.acl_tx++;
		return submit_or_queue_tx_urb(hdev, urb);
#else
		if (!data->bulk_tx_ep)
			return -ENODEV;

		urb = usb_alloc_urb(0, GFP_ATOMIC);
		if (!urb)
			return -ENOMEM;

		pipe = usb_sndbulkpipe(data->udev,
				       data->bulk_tx_ep->bEndpointAddress);

		usb_fill_bulk_urb(urb, data->udev, pipe,
				  skb->data, skb->len, btusb_tx_complete, skb);

		hdev->stat.acl_tx++;
		break;

#endif
	case HCI_SCODATA_PKT:
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
		if (hci_conn_num(hdev, SCO_LINK) < 1)
			return -ENODEV;

		if (test_bit(BTUSB_BULK_MODE, &data->flags))
			return btusb_send_bulk_mode(HCI_SCODATA_PKT, hdev, skb);

		urb = alloc_isoc_urb(hdev, skb);
		if (IS_ERR(urb))
			return PTR_ERR(urb);

		hdev->stat.sco_tx++;
		return submit_tx_urb(hdev, urb);
	}

	return -EILSEQ;
#else
		if (!data->isoc_tx_ep || !data->isoc_tx_ep->wMaxPacketSize ||
		    SCO_NUM < 1)
			return -ENODEV;

		urb = usb_alloc_urb(BTUSB_MAX_ISOC_FRAMES, GFP_ATOMIC);
		if (!urb)
			return -ENOMEM;

		pipe = usb_sndisocpipe(data->udev,
				       data->isoc_tx_ep->bEndpointAddress);

		usb_fill_int_urb(urb, data->udev, pipe,
				 skb->data, skb->len, btusb_isoc_tx_complete,
				 skb, data->isoc_tx_ep->bInterval);

		urb->transfer_flags = URB_ISO_ASAP;

		__fill_isoc_descriptor(urb, skb->len,
				       le16_to_cpu(data->isoc_tx_ep->
						   wMaxPacketSize));

		hdev->stat.sco_tx++;
		goto skip_waking;

	default:
		return -EILSEQ;

	}

	err = inc_tx(data);
	if (err) {
		usb_anchor_urb(urb, &data->deferred);
		schedule_work(&data->waker);
		err = 0;
		goto done;
	}

skip_waking:
	usb_anchor_urb(urb, &data->tx_anchor);
	err = usb_submit_urb(urb, GFP_ATOMIC);
	if (err < 0) {
		RTKBT_ERR("%s %s urb %p submission for %s failed, err %d",
			  __func__, hdev->name, urb,
			  pkt_ind[bt_cb(skb)->pkt_type], err);
		kfree(urb->setup_packet);
		usb_unanchor_urb(urb);
	} else {
		usb_mark_last_busy(data->udev);
	}

done:
	usb_free_urb(urb);
	return err;
#endif
}


#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
static void btusb_destruct(struct hci_dev *hdev)
{
	RTKBT_DBG("btusb_destruct %s", hdev->name);
	hci_free_dev(hdev);
}
#endif

static void btusb_notify(struct hci_dev *hdev, unsigned int evt)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);

	RTKBT_DBG("%s: %s evt %d", __func__, hdev->name, evt);

	if (SCO_NUM != data->sco_num) {
		data->sco_num = SCO_NUM;
		RTKBT_DBG("%s: Update sco num %d", __func__, data->sco_num);
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		data->air_mode = evt;
#endif
		schedule_work(&data->work);
	}
}

static inline int __set_isoc_interface(struct hci_dev *hdev, int altsetting)
{
	struct btusb_data *data = GET_DRV_DATA(hdev);
	struct usb_interface *intf = data->isoc;
	struct usb_endpoint_descriptor *ep_desc;
	int i, err;

	if (!data->isoc)
		return -ENODEV;

	RTKBT_INFO("set isoc interface: alt %d", altsetting);

	err = usb_set_interface(data->udev, 1, altsetting);
	if (err < 0) {
		RTKBT_ERR("%s setting interface failed (%d)", hdev->name, -err);
		return err;
	}

	data->isoc_altsetting = altsetting;

	data->isoc_tx_ep = NULL;
	data->isoc_rx_ep = NULL;

	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		ep_desc = &intf->cur_altsetting->endpoint[i].desc;

		if (!data->isoc_tx_ep && usb_endpoint_is_isoc_out(ep_desc)) {
			data->isoc_tx_ep = ep_desc;
			continue;
		}

		if (!data->isoc_rx_ep && usb_endpoint_is_isoc_in(ep_desc)) {
			data->isoc_rx_ep = ep_desc;
			continue;
		}
	}

	if (!data->isoc_tx_ep || !data->isoc_rx_ep) {
		RTKBT_ERR("%s invalid SCO descriptors", hdev->name);
		return -ENODEV;
	}

	return 0;
}

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
static int btusb_switch_alt_setting(struct hci_dev *hdev, int new_alts)
{
	struct btusb_data *data = hci_get_drvdata(hdev);
	int err;

	if (data->isoc_altsetting != new_alts) {
		unsigned long flags;

		clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
		usb_kill_anchored_urbs(&data->isoc_anchor);

		/* When isochronous alternate setting needs to be
		 * changed, because SCO connection has been added
		 * or removed, a packet fragment may be left in the
		 * reassembling state. This could lead to wrongly
		 * assembled fragments.
		 *
		 * Clear outstanding fragment when selecting a new
		 * alternate setting.
		 */
		spin_lock_irqsave(&data->rxlock, flags);
		kfree_skb(data->sco_skb);
		data->sco_skb = NULL;
		spin_unlock_irqrestore(&data->rxlock, flags);

		err = __set_isoc_interface(hdev, new_alts);
		if (err < 0)
			return err;
	}

	if (!test_and_set_bit(BTUSB_ISOC_RUNNING, &data->flags)) {
		if (btusb_submit_isoc_urb(hdev, GFP_KERNEL) < 0)
			clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
		else
			btusb_submit_isoc_urb(hdev, GFP_KERNEL);
	}

	return 0;
}

static struct usb_host_interface *btusb_find_altsetting(struct btusb_data *data,
							int alt)
{
	struct usb_interface *intf = data->isoc;
	int i;

	BT_DBG("Looking for Alt no :%d", alt);

	if (!intf)
		return NULL;

	for (i = 0; i < intf->num_altsetting; i++) {
		if (intf->altsetting[i].desc.bAlternateSetting == alt)
			return &intf->altsetting[i];
	}

	return NULL;
}
#endif

static void btusb_work(struct work_struct *work)
{
	struct btusb_data *data = container_of(work, struct btusb_data, work);
	struct hci_dev *hdev = data->hdev;
	int err;
	int new_alts = 0;

	if (test_bit(BTUSB_BULK_MODE, &data->flags))
		return;

	RTKBT_DBG("%s: sco num %d", __func__, data->sco_num);
	if (data->sco_num > 0) {
		if (!test_bit(BTUSB_DID_ISO_RESUME, &data->flags)) {
			err =
			    usb_autopm_get_interface(data->isoc ? data->
						     isoc : data->intf);
			if (err < 0) {
				clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
				mdelay(URB_CANCELING_DELAY_MS);
				usb_kill_anchored_urbs(&data->isoc_anchor);
				return;
			}

			set_bit(BTUSB_DID_ISO_RESUME, &data->flags);
		}
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
		if (data->air_mode == HCI_NOTIFY_ENABLE_SCO_CVSD) {
			if (hdev->voice_setting & 0x0020) {
				static const int alts[3] = { 2, 4, 5 };
				new_alts = alts[data->sco_num - 1];
			} else {
				new_alts = data->sco_num;
			}
		} else if (data->air_mode == HCI_NOTIFY_ENABLE_SCO_TRANSP) {
			if (btusb_find_altsetting(data, 6))
				new_alts = 6;
			else if (btusb_find_altsetting(data, 3) &&
				 hdev->sco_mtu >= 72 &&
				 test_bit(BTUSB_USE_ALT3_FOR_WBS, &data->flags))
				new_alts = 3;
			else
				new_alts = 1;
		}

		if (btusb_switch_alt_setting(hdev, new_alts) < 0)
				RTKBT_ERR("set USB alt:(%d) failed!", new_alts);
#else
#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
		if (hdev->voice_setting & 0x0020) {
			static const int alts[3] = { 2, 4, 5 };
			new_alts = alts[data->sco_num - 1];
		} else {
			new_alts = data->sco_num;
		}
		if (data->isoc_altsetting != new_alts) {
#else
		if (data->isoc_altsetting != 2) {
			new_alts = 2;
#endif

			clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
			mdelay(URB_CANCELING_DELAY_MS);
			usb_kill_anchored_urbs(&data->isoc_anchor);

			if (__set_isoc_interface(hdev, new_alts) < 0)
				return;
		}

		if (!test_and_set_bit(BTUSB_ISOC_RUNNING, &data->flags)) {
			RTKBT_INFO("submit SCO RX urb.");
			if (btusb_submit_isoc_urb(hdev, GFP_KERNEL) < 0)
				clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
			else
				btusb_submit_isoc_urb(hdev, GFP_KERNEL);
		}
#endif
	} else {
		clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
		mdelay(URB_CANCELING_DELAY_MS);
		usb_kill_anchored_urbs(&data->isoc_anchor);

		__set_isoc_interface(hdev, 0);
		if (test_and_clear_bit(BTUSB_DID_ISO_RESUME, &data->flags))
			usb_autopm_put_interface(data->isoc ? data->
						 isoc : data->intf);
	}
}

static void btusb_waker(struct work_struct *work)
{
	struct btusb_data *data = container_of(work, struct btusb_data, waker);
	int err;

	err = usb_autopm_get_interface(data->intf);
	RTKBT_DBG("%s start", __FUNCTION__);
	if (err < 0)
		return;

	usb_autopm_put_interface(data->intf);
	RTKBT_DBG("%s end", __FUNCTION__);
}

#ifdef RTKBT_TV_POWERON_WHITELIST
static int rtkbt_lookup_le_device_poweron_whitelist(struct hci_dev *hdev,
						struct usb_device *udev)
{
	struct hci_conn_params *p;
	u8 *cmd;
	int result = 0;

	hci_dev_lock(hdev);
	list_for_each_entry(p, &hdev->le_conn_params, list) {
#if 0 // for debug message
		RTKBT_DBG("%s(): auto_connect = %d", __FUNCTION__, p->auto_connect);
		RTKBT_DBG("%s(): addr_type = 0x%02x", __FUNCTION__, p->addr_type);
		RTKBT_DBG("%s(): addr=%02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__,
                                p->addr.b[5], p->addr.b[4], p->addr.b[3],
                                p->addr.b[2], p->addr.b[1], p->addr.b[0]);
#endif
		if ( p->auto_connect == HCI_AUTO_CONN_ALWAYS &&
			p->addr_type == ADDR_LE_DEV_PUBLIC ) {

			RTKBT_DBG("%s(): Set RTKBT LE Power-on Whitelist for "
				"%02x:%02x:%02x:%02x:%02x:%02x", __FUNCTION__,
                                p->addr.b[5], p->addr.b[4], p->addr.b[3],
                                p->addr.b[2], p->addr.b[1], p->addr.b[0]);

			cmd = kzalloc(16, GFP_ATOMIC);
			if (!cmd) {
				RTKBT_ERR("Can't allocate memory for cmd");
				return -ENOMEM;
			}
			cmd[0] = 0x7b;
			cmd[1] = 0xfc;
			cmd[2] = 0x07;
			cmd[3] = 0x00;
			cmd[4] = p->addr.b[0];
			cmd[5] = p->addr.b[1];
			cmd[6] = p->addr.b[2];
			cmd[7] = p->addr.b[3];
			cmd[8] = p->addr.b[4];
			cmd[9] = p->addr.b[5];

			result = __rtk_send_hci_cmd(udev, cmd, 10);
			kfree(cmd);
		}
	}
	hci_dev_unlock(hdev);

	return result;
}
#endif

#if defined(CONFIG_BTRTL_APCF)
static int rtkbt_send_cmd(struct btusb_data *data, u8 *cmd, u16 len, u32 timeout)
{
	struct sk_buff *skb = NULL;
	struct usb_device *udev;
	u16 opcode;
	u8 param0 = 0;
	int ret;

	if (!data || !data->udev || !cmd || !len)
		return -EINVAL;
	udev = data->udev;
	opcode = get_unaligned_le16(cmd);
	if (test_bit(BTUSB_INTR_RUNNING, &data->flags)) {
		ret = __rtk_send_hci_cmd(udev, cmd, cmd[2] + 3);
		/* wait for the command complete event.
		 * The cmd complete will be read by the urb that has been
		 * submitted.
		 */
		msleep(10);
	} else {
		skb = rtk_hci_cmd_sync(udev, opcode, cmd[2], cmd + 3, timeout);
		if (IS_ERR(skb) || !skb) {
			if (len >= 4)
				param0 = cmd[3];
			RTKBT_ERR("%s: failed to issue %04x (%u), %ld",
				  __func__, opcode, param0, PTR_ERR(skb));
			return -EIO;
		} else {
			kfree_skb(skb);
		}
	}

	return 0;
}
#endif

#define APCF_BROADCASTER_ADDRESS	0x02
#define APCF_SERVICE_UUID		0x03
#define APCF_SERVICE_SOLICIT_UUID	0x04
#define APCF_LOCAL_NAME			0x05
#define APCF_MANUFACTURER_DATA		0x06
#define APCF_SERVICE_DATA		0x07
#define APCF_TRANSPORT_DISC_SERVICE	0x08
#define APCF_AD_TYPE_FILTER		0x09
#define APCF_FILTER_OP_TAG		0xb0
#define APCF_OP_FILTER_ADD		0x00
#define APCF_OP_FILTER_DEL		0x01
#define APCF_OP_FILTER_CLR		0x02

#define MAX_APCF_FILTERS	3

#define APCF_WORK_DEFER_TIME	250

#ifndef hci_opcode_pack
#define hci_opcode_pack(ogf, ocf)	((u16) ((ocf & 0x03ff)|(ogf << 10)))
#endif

struct apcf_enable_cp {
	u8  subcmd;
	u8  apcf_enable;
} __attribute__((packed));

struct apcf_set_filter_params_cp {
	u8  subcmd;
	u8  action;
	u8  flt_index;
	u16 feat_sel;
	u16 list_logic_type;
	u8  filter_logic_type;
	u8  rssi_high_thresh;
	u8  delivery_mode;
	u16 onfound_timeout;
	u8  onfound_timeout_cnt;
	u8  rssi_low_thresh;
	u16 onlost_timeout;
	u16 num_of_tracking_entries;
} __attribute__((packed));

struct apcf_set_manf_data_cp {
	u8  subcmd;
	u8  action;
	u8  flt_index;
} __attribute__((packed));

struct apcf_set_wakeup_cp {
	u8  flt_index;
	u8  enable;
	u8  pulse_unit;
	u8  pulse_format[4];
	u8  timer;
} __attribute__((packed));

struct apcf_filter {
	u8	*data;
	u16	len;
	u8      type;
	u8      wakeup;
	u8	pulse_unit; /* unit ms */
	u8	pulse_format[4];
	u8	timer; /* sec */
};

struct apcf_struct {
	struct apcf_filter filters[MAX_APCF_FILTERS];
};

#define WAKE_DATA_ADV_LEN_MAX		50

struct hci_wakeup_adv_info_rp {
	u8 event_type[2];
	u8 address_type;
	u8 address[6];
	u8 adv_len;
	u8 data[WAKE_DATA_ADV_LEN_MAX];
} __attribute__((packed));

struct wakedata_struct {
	u8 event_type;
	u8 len;
	u8 peer[6];
	u8 adv_len;
	u8 adv_data[0];
} __attribute__((packed));

#if defined(CONFIG_BTRTL_APCF)

static struct apcf_struct apcf;
static DEFINE_MUTEX(apcf_lock);
static LIST_HEAD(apcf_cfg_filter);
static LIST_HEAD(apcf_cfg_wakeup);

static void apcf_clear_driver_filters(void)
{
	u8 n = 0;

	for (n = 0; n < MAX_APCF_FILTERS; n++) {
		if (apcf.filters[n].data)
			kfree(apcf.filters[n].data);
	}
	memset(&apcf, 0, sizeof(apcf));
}

/* This function requires the caller holds apcf_lock */
static ssize_t __apcf_filter_op(const char *buf, size_t count, const u8 b[6])
{
	int ret = -EINVAL;
	u8 *ptr;
	u8 *data = NULL;
	u8 *mem = NULL;
	size_t i;
	u8 len = 0;
	unsigned long num;
	u8 filter_op;
	u8 index = 0;
	u8 type;
	char str[3] = { 0 };
	size_t bdaddr_s = 0;

	RTKBT_INFO("%s: count %zu", __func__, count);

	if (!buf || !count)
		return -EINVAL;
	if (buf[0] == 0x23) /* '#' */
		return -EINVAL;

	/* Plus 6-byte address */
	mem = kzalloc(count / 2 + 6, GFP_KERNEL);
	if (!mem) {
		RTKBT_ERR("allocate mem for apcf filter error");
		return -ENOMEM;
	}
	data = mem;

	len = 0;
	for (i = 0; i < count - 1; i += 2) {
		/* It is not allowed that the first byte is '#' */
		if (buf[i] == 0x23 && !bdaddr_s) {
			bdaddr_s = i;
			continue;
		}
		if (bdaddr_s) {
			if (buf[i] != 0x23)
				continue;
			memcpy(&data[len], b, 6);
			len += 6;
			bdaddr_s = 0;
			continue;
		}
		memcpy(str, buf + i, 2);
		ret = kstrtoul(str, 16, &num);
		if (ret)
			goto err;
		data[len++] = (u8)num;
	}
	if (len < 3) {
		ret = -EINVAL;
		goto err;
	}

	print_hex_dump(KERN_INFO, "btrtl_usb: ", DUMP_PREFIX_ADDRESS,
		       16, 1, data, len, true);
	ptr = data;
	if (ptr[0] < 2 || ptr[1] != APCF_FILTER_OP_TAG) {
		RTKBT_ERR("No filter tag");
		ret = -EINVAL;
		goto err;
	}
	switch (ptr[2]) {
	case APCF_OP_FILTER_ADD:
	case APCF_OP_FILTER_DEL:
		if (*ptr < 3) {
			RTKBT_ERR("invalid len of filter operation");
			ret = -EINVAL;
			goto err;
		}
		filter_op = ptr[2];
		index = ptr[3];
		if (index >= MAX_APCF_FILTERS) {
			RTKBT_ERR("invalid filter index 0x%02x", index);
			ret = -EINVAL;
			goto err;
		}
		if (filter_op == APCF_OP_FILTER_DEL) {
			ret = count;
			kfree(apcf.filters[index].data);
			memset(&apcf.filters[index], 0,
			       sizeof(apcf.filters[0]));
			goto err;
		}
		break;
	case APCF_OP_FILTER_CLR:
		ret = count;
		apcf_clear_driver_filters();
		goto err;
	default:
		RTKBT_ERR("unsupported filter operation 0x%02x", ptr[2]);
		ret = -EINVAL;
		goto err;
	}
	len -= (1 + *ptr);
	if (len < 3) {
		RTKBT_ERR("invalid apcf filter data");
		ret = -EINVAL;
		goto err;
	}
	data += (1 + *ptr);

	for (ptr = data; ptr < data + len && *ptr; ptr += *ptr + 1) {
		if (ptr + 1 + *ptr > data + len)
			break;
	}
	len = ptr - data;
	if (!len || len < 3) {
		RTKBT_ERR("len is too small (inc zero)");
		ret = -ENODATA;
		goto err;
	}

	RTKBT_INFO("%s: len %u, %02x%02x%02x", __func__, len,
		   data[2], data[1], data[0]);

	ptr = data;
	if (*ptr < 2) {
		RTKBT_ERR("data size is too small");
		ret = -ENODATA;
		goto err;
	}

	switch (*(ptr + 1)) {
	case APCF_MANUFACTURER_DATA:
		type = APCF_MANUFACTURER_DATA;
		if (*ptr - 1 > 2 * 29) {
			RTKBT_ERR("manufacturer data size exceeds %u", *ptr);
			ret = -EINVAL;
			goto err;
		}
		break;
	default:
		RTKBT_ERR("unsupported data type 0x%02x", *(ptr + 1));
		ret = -EINVAL;
		goto err;
	}

	len = *ptr - 1;
	ptr = kzalloc(len, GFP_KERNEL);
	if (!ptr) {
		ret = -ENOMEM;
		goto err;
	}

	memcpy(ptr, data + 2, len);
	if (apcf.filters[index].data)
		kfree(apcf.filters[index].data);
	apcf.filters[index].data = ptr;
	apcf.filters[index].len = len;
	apcf.filters[index].type = type;

	kfree(mem);
	return (ssize_t)count;
err:
	if (mem)
		kfree(mem);
	return ret ? ret : count;
}

/* This function requires the caller holds apcf_lock */
static ssize_t __apcf_wakeup_store(const char *buf, size_t count)
{
	int ret = -EINVAL;
	u8 index;
	char str[3] = { 0 };
	u8 *data = NULL;
	u8 *ptr = NULL;
	u16 len;
	u8 i;
	unsigned long num;

	data = kzalloc(count / 2, GFP_KERNEL);
	if (!data) {
		RTKBT_ERR("%s: Can not alloc mem for apcf wakeup", __func__);
		return -ENOMEM;
	}

	len = 0;
	for (i = 0; i < count - 1; i += 2) {
		memcpy(str, buf + i, 2);
		ret = kstrtoul(str, 16, &num);
		if (ret)
			goto done;
		data[len++] = (u8)num;
	}

	print_hex_dump(KERN_INFO, "btrtl_usb: ", DUMP_PREFIX_ADDRESS,
		       16, 1, data, len, true);

	if (len < sizeof(struct apcf_set_wakeup_cp)) {
		ret = -EINVAL;
		RTKBT_ERR("%s: invalid data, len %u", __func__, len);
		goto done;
	}

	index = data[0];
	if (index >= MAX_APCF_FILTERS) {
		ret = -EINVAL;
		RTKBT_ERR("%s: invalid index %u", __func__, index);
		goto done;
	}

	ptr = data + 1;
	apcf.filters[index].wakeup = !!*ptr++;
	apcf.filters[index].pulse_unit = *ptr++;
	memcpy(apcf.filters[index].pulse_format, ptr, 4);
	ptr += 4;
	apcf.filters[index].timer = *ptr++;
	ret = (int)count;
done:
	kfree(data);
	return ret;
}

static int rtkbt_apcf_init_default(u8 *bdaddr)
{
	const char *def_filter0 = "03b000001906"
		"5d0003000107##000000000000##"
		"ffffffff0000ffffffffffff";
	const char *def_wakeup0 = "00010af0ffff0003";
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct cfg_apcf_item *c = NULL;

	config_file_proc(APCF_CONFIG_FILTER, CFG_TYPE_APCF_FILTER);
	config_file_proc(APCF_CONFIG_WAKEUP, CFG_TYPE_APCF_WAKEUP);

	mutex_lock(&apcf_lock);
	drain_apcf_cfg(&apcf_cfg_filter, CFG_TYPE_APCF_FILTER);
	drain_apcf_cfg(&apcf_cfg_wakeup, CFG_TYPE_APCF_WAKEUP);
	mutex_unlock(&apcf_lock);

	mutex_lock(&apcf_lock);
	if (!list_empty(&apcf_cfg_filter)) {
		list_for_each_safe(pos, next, &apcf_cfg_filter) {
			c = list_entry(pos, struct cfg_apcf_item, list);
			list_del(&c->list);
			__apcf_filter_op(c->data, c->len, bdaddr);
			vfree(c);
		}
	} else {
		__apcf_filter_op(def_filter0, strlen(def_filter0) + 1, bdaddr);
	}
	mutex_unlock(&apcf_lock);

	mutex_lock(&apcf_lock);
	if (!list_empty(&apcf_cfg_wakeup)) {
		list_for_each_safe(pos, next, &apcf_cfg_wakeup) {
			c = list_entry(pos, struct cfg_apcf_item, list);
			list_del(&c->list);
			__apcf_wakeup_store(c->data, c->len);
			vfree(c);
		}
	} else {
		__apcf_wakeup_store(def_wakeup0, strlen(def_wakeup0) + 1);
	}
	mutex_unlock(&apcf_lock);

	return 0;
}

static void rtkbt_apcf_deinit(void)
{
	u8 n;
	struct list_head *pos = NULL;
	struct list_head *next = NULL;
	struct list_head *heads[2];
	struct cfg_apcf_item *c;

	drain_apcf_cfg(NULL, CFG_TYPE_APCF_FILTER | CFG_TYPE_APCF_WAKEUP);

	mutex_lock(&apcf_lock);

	for (n = 0; n < MAX_APCF_FILTERS; n++)
		if (apcf.filters[n].data)
			kfree(apcf.filters[n].data);
	memset(&apcf, 0, sizeof(apcf));

	heads[0] = &apcf_cfg_filter;
	heads[1] = &apcf_cfg_wakeup;
	for (n = 0; n < 2; n++) {
		list_for_each_safe(pos, next, heads[n]) {
			c = list_entry(pos, struct cfg_apcf_item, list);
			list_del(&c->list);
			vfree(c);
		}
	}

	mutex_unlock(&apcf_lock);

	RTKBT_INFO("%s", __func__);
}

static void apcf_work_func(struct work_struct *work)
{
	struct btusb_data *data;
	struct hci_dev *hdev;
	u8 *bdaddr;
	static u8 sched_times = 0;
	static unsigned int defer_time = APCF_WORK_DEFER_TIME;

	data = container_of(work, struct btusb_data, apcf_work.work);
	hdev = data->hdev;
	bdaddr = hdev->bdaddr.b;
	if (!bacmp(&hdev->bdaddr, BDADDR_ANY)) {
		if (++sched_times > 10)
			return;
		defer_time *= 2;
		schedule_delayed_work(&data->apcf_work,
				      msecs_to_jiffies(defer_time));
		return;
	}
	RTKBT_INFO("%s: bdaddr %02x:%02x:%02x:%02x:%02x:%02x, times %u",
		   __func__, bdaddr[5], bdaddr[4], bdaddr[3], bdaddr[2],
		   bdaddr[1], bdaddr[0], sched_times);
	sched_times = 0;
	defer_time = APCF_WORK_DEFER_TIME;
	rtkbt_apcf_init_default(bdaddr);
}

static int rtkbt_set_apcf(struct btusb_data *btusb_data)
{
	u8 *cmd = NULL;
	int ret = 0;
	u16 opcode;
	u8 i = 0;
	u8 *data;
	u16 len;
	struct apcf_set_filter_params_cp *set_flt_cp;
	struct apcf_enable_cp *enable_cp;
	struct apcf_set_manf_data_cp *manf_data_cp;
	struct apcf_set_wakeup_cp *wakeup_cp;
	struct usb_device *udev;

	RTKBT_INFO("%s", __func__);

	if (!btusb_data || !btusb_data->udev)
		return -EINVAL;
	udev = btusb_data->udev;

	cmd = kzalloc(256, GFP_ATOMIC);
	if (!cmd) {
		RTKBT_ERR("%s: failed to alloc cmd memory", __func__);
		return -ENOMEM;
	}

	opcode = hci_opcode_pack(0x3f, 0x157);
	cmd[0] = opcode & 0xff;
	cmd[1] = opcode >> 8;
	enable_cp = (void *)(cmd + 3);
	memset(enable_cp, 0, sizeof(*enable_cp));
	enable_cp->subcmd = 0x00;
	enable_cp->apcf_enable = 0x01;
	cmd[2] = sizeof(*enable_cp);
	rtkbt_send_cmd(btusb_data, cmd, cmd[2] + 3, 200);

	opcode = hci_opcode_pack(0x3f, 0x157);
	cmd[0] = opcode & 0xff;
	cmd[1] = opcode >> 8;
	set_flt_cp = (void *)(cmd + 3);
	memset(set_flt_cp, 0, sizeof(*set_flt_cp));
	set_flt_cp->subcmd = 0x01;
	/* Clear all the filters and associated entries in other tables */
	set_flt_cp->action = 0x02;
	put_unaligned_le16(0x01ff, &set_flt_cp->feat_sel);
	set_flt_cp->rssi_high_thresh = 0x80;
	cmd[2] = sizeof(*set_flt_cp);
	rtkbt_send_cmd(btusb_data, cmd, cmd[2] + 3, 200);

	mutex_lock(&apcf_lock);
	for (i = 0; i < MAX_APCF_FILTERS; i++) {
		data = apcf.filters[i].data;
		len = apcf.filters[i].len;
		if (!data || !len)
			continue;
		switch (apcf.filters[i].type) {
		case APCF_MANUFACTURER_DATA:
			opcode = hci_opcode_pack(0x3f, 0x157);
			cmd[0] = opcode & 0xff;
			cmd[1] = opcode >> 8;
			manf_data_cp = (void *)(cmd + 3);
			memset(manf_data_cp, 0, sizeof(*manf_data_cp));
			manf_data_cp->subcmd = 0x06;
			manf_data_cp->action = 0x00;
			manf_data_cp->flt_index = i;
			memcpy(cmd + 3 + sizeof(*manf_data_cp), data, len);
			cmd[2] = sizeof(*manf_data_cp) + len;
			mutex_unlock(&apcf_lock);

			rtkbt_send_cmd(btusb_data, cmd, cmd[2] + 3, 200);

			mutex_lock(&apcf_lock);

			opcode = hci_opcode_pack(0x3f, 0x157);
			cmd[0] = opcode & 0xff;
			cmd[1] = opcode >> 8;
			set_flt_cp = (void *)(cmd + 3);
			memset(set_flt_cp, 0, sizeof(*set_flt_cp));
			set_flt_cp->subcmd = 0x01;
			set_flt_cp->action = 0x00;
			set_flt_cp->flt_index = i;
			put_unaligned_le16(0x0020, &set_flt_cp->feat_sel);
			set_flt_cp->rssi_high_thresh = 0x80;
			cmd[2] = sizeof(*set_flt_cp);
			rtkbt_send_cmd(btusb_data, cmd, cmd[2] + 3, 200);
			break;
		default:
			RTKBT_ERR("%s: unsupported filter 0x%02x", __func__,
				  apcf.filters[i].type);
			continue;
		}

		if (apcf.filters[i].wakeup) {
			opcode = hci_opcode_pack(0x3f, 0x1b4);
			cmd[0] = opcode & 0xff;
			cmd[1] = opcode >> 8;
			wakeup_cp = (void *)(cmd + 3);
			memset(wakeup_cp, 0, sizeof(*wakeup_cp));
			wakeup_cp->flt_index = i;
			wakeup_cp->enable = 1;
			wakeup_cp->pulse_unit = apcf.filters[i].pulse_unit;
			memcpy(wakeup_cp->pulse_format,
			       apcf.filters[i].pulse_format,
			       sizeof(wakeup_cp->pulse_format));
			wakeup_cp->timer = apcf.filters[i].timer;
			cmd[2] = sizeof(*wakeup_cp);
			rtkbt_send_cmd(btusb_data, cmd, cmd[2] + 3, 200);
		}
	}
	mutex_unlock(&apcf_lock);

	kfree(cmd);
	return ret >= 0 ? 0 : ret;
}

static ssize_t apcf_filter_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	u8 i;
	u8 n;
	ssize_t ret = 0;
	u8 *data = NULL;
	u16 len;

	mutex_lock(&apcf_lock);
	for (n = 0; n < MAX_APCF_FILTERS; n++) {
		data = apcf.filters[n].data;
		len = apcf.filters[n].len;
		ret += sprintf(buf + ret, "[%02x,%02x,%u]", n,
			       apcf.filters[n].type, len);
		if (!data || !len) {
			ret += sprintf(buf + ret, "\n");
			continue;
		}
		for (i = 0; i < len; i++)
			ret += sprintf(buf + ret, "%02x", data[i]);
		ret += sprintf(buf + ret, "\n");
	}
	mutex_unlock(&apcf_lock);

	return ret;
}

static ssize_t apcf_filter_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	ssize_t ret;
	struct hci_dev *hdev = container_of(dev, struct hci_dev, dev);
	u8 b[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

	mutex_lock(&apcf_lock);
	ret = __apcf_filter_op(buf, count, hdev ? hdev->bdaddr.b : b);
	mutex_unlock(&apcf_lock);
	return ret ? ret : count;
}

static ssize_t apcf_wakeup_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	u8 n;
	ssize_t ret = 0;
	struct apcf_filter *flt;

	mutex_lock(&apcf_lock);
	for (n = 0; n < MAX_APCF_FILTERS; n++) {
		flt = &apcf.filters[n];
		ret += sprintf(buf + ret, "[%02x,%02x]", n,
			       flt->type);
		ret += sprintf(buf + ret, "%02x", flt->pulse_unit);
		ret += sprintf(buf + ret, "%02x%02x%02x%02x",
			       flt->pulse_format[0],
			       flt->pulse_format[1],
			       flt->pulse_format[2],
			       flt->pulse_format[3]);
		ret += sprintf(buf + ret, "%02x", flt->timer);
		ret += sprintf(buf + ret, "\n");
	}
	mutex_unlock(&apcf_lock);

	return ret;
}

static ssize_t apcf_wakeup_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	ssize_t ret;

	mutex_lock(&apcf_lock);
	ret = __apcf_wakeup_store(buf, count);
	mutex_unlock(&apcf_lock);
	return ret ? ret : count;
}

static int rtkbt_set_apcf_off(struct usb_device *udev)
{
	u8 *cmd = NULL;
	int ret = 0;
	u16 opcode;
	struct apcf_enable_cp *enable_cp;

	RTKBT_INFO("%s", __func__);

	cmd = kzalloc(256, GFP_ATOMIC);
	if (!cmd) {
		RTKBT_ERR("%s: failed to alloc cmd memory", __func__);
		return -ENOMEM;
	}

	opcode = hci_opcode_pack(0x3f, 0x157);
	cmd[0] = opcode & 0xff;
	cmd[1] = opcode >> 8;
	enable_cp = (void *)(cmd + 3);
	memset(enable_cp, 0, sizeof(*enable_cp));
	enable_cp->subcmd = 0x00;
	enable_cp->apcf_enable = 0x00;
	cmd[2] = sizeof(*enable_cp);
	ret = __rtk_send_hci_cmd(udev, cmd, cmd[2] + 3);
	msleep(10); /* wait for the command complete event */

	kfree(cmd);
	return ret >= 0 ? 0 : ret;
}

static DEVICE_ATTR_RW(apcf_filter);
static DEVICE_ATTR_RW(apcf_wakeup);

#endif /* end of CONFIG_BTRTL_APCF */

#if CONFIG_BTRTL_WAKEUP_REASON
static u8 *wakeup_reason;
static DEFINE_MUTEX(wakeup_reason_lock);

static ssize_t wakeup_reason_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t ret = 0;
	u8 *ptr;
	u8 *data;
	u8 len;
	u8 n;
	struct wakedata_struct *wd = NULL;
	struct hci_wakeup_adv_info_rp *rp;
	u16 event_type;
	struct {
		u8  type_legacy;
		u16 type_extended;
	} event_type_to_legacy[] = {
		{ 0x00, 0x0013 }, /* adv_ind */
		{ 0x01, 0x0015 }, /* adv_direct_ind */
		{ 0x02, 0x0012 }, /* adv_scan_ind */
		{ 0x03, 0x0010 }, /* adv_nonconn_ind */
		{ 0x04, 0x001b }, /* scan_rsp to an adv_ind */
		{ 0x04, 0x001a }, /* scan_rsp to an adv_scan_ind */
	};

	wd = kzalloc(sizeof(*wd) + WAKE_DATA_ADV_LEN_MAX, GFP_KERNEL);
	if (!wd)
		return -EINVAL;

	mutex_lock(&wakeup_reason_lock);
	if (!wakeup_reason)
		goto done;

	if (!wakeup_reason[0]) {
		ret += sprintf(buf + ret, "%02x", 0);
		ret += sprintf(buf + ret, "\n");
		goto done;
	}

	rp = (struct hci_wakeup_adv_info_rp *)&wakeup_reason[1];
	if (rp->adv_len > sizeof(rp->data)) {
		ret = -EOVERFLOW;
		goto done;
	}

	wd->event_type = 0;
	event_type = get_unaligned_le16(rp->event_type);
	for (n = 0; n < ARRAY_SIZE(event_type_to_legacy); n++) {
		if (event_type == event_type_to_legacy[n].type_extended) {
			wd->event_type = event_type_to_legacy[n].type_legacy;
			break;
		}
	}
	memcpy(wd->peer, rp->address, sizeof(wd->peer));
	memcpy(wd->adv_data, rp->data, rp->adv_len);

	data = wd->adv_data;
	for (ptr = data; ptr < data + rp->adv_len && *ptr; ptr += *ptr + 1) {
		if (ptr + 1 + *ptr > data + rp->adv_len)
			break;
	}
	/* Adjust for actual length. */
	len = ptr - data;
	wd->adv_len = len;

	len += sizeof(*wd);
	wd->len = len - 2;

	data = (u8 *)wd;
	for (n = 0; n < len; n++)
		ret += sprintf(buf + ret, "%02x", data[n]);
	ret += sprintf(buf + ret, "\n");

done:
	mutex_unlock(&wakeup_reason_lock);

	kfree(wd);

	return ret;
}

static DEVICE_ATTR_RO(wakeup_reason);

struct hci_rp_read_wakeup_reason {
	__u8 status;
	__u8 adv_len;
	__u8 adv_data[0];
} __attribute__((packed));

static int btusb_read_wakeup_reason(struct btusb_data *data)
{
	struct hci_rp_read_wakeup_reason *rp;
	struct sk_buff *skb;
	int ret;
	struct hci_dev *hdev;
	struct usb_device *udev;

	hdev = data->hdev;
	udev = data->udev;

	if (!test_bit(BTUSB_INTR_RUNNING, &data->flags))
		skb = rtk_hci_cmd_sync(udev, 0xfd87, 0, NULL, 1000);
	else
		skb = __hci_cmd_sync(hdev, 0xfd87, 0, NULL, HCI_CMD_TIMEOUT);
	if (IS_ERR(skb) || !skb) {
		ret = PTR_ERR(skb);
		bt_dev_err(hdev, "HCI read wakeup reason failed (%d)", ret);
		return ret;
	}

	rp = (void *)skb->data;
	if (rp->status) {
		bt_dev_err(hdev, "rp status 0x%02x", rp->status);
		goto err;
	}
	if (rp->adv_len + 2 > skb->len) {
		bt_dev_err(hdev, "len mismatch (0x%02x, 0x%02x)",
			   rp->adv_len + 2, skb->len);
		goto err;
	}
	bt_dev_info(hdev, "wakeup reason 0x%02x", rp->adv_len);

	mutex_lock(&wakeup_reason_lock);
	if (wakeup_reason) {
		u8 *tmp = wakeup_reason;;

		tmp[0] = rp->adv_len;
		if (tmp[0])
			memcpy(tmp + 1, rp->adv_data, tmp[0]);
	} else {
		bt_dev_err(hdev, "No mem for wakeup reason");
	}
	mutex_unlock(&wakeup_reason_lock);
	kfree_skb(skb);
	return 0;
err:
	kfree_skb(skb);
	return -EIO;
}
#endif /* end of CONFIG_BTRTL_WAKEUP_REASON */

static int rtkbt_pm_notify(struct notifier_block *notifier,
		    ulong pm_event, void *unused)
{
	struct btusb_data *data;
	struct usb_device *udev;
	struct usb_interface *intf;
	struct hci_dev *hdev;
	/* int err; */
#if defined RTKBT_SWITCH_PATCH || defined RTKBT_TV_POWERON_WHITELIST
	int result = 0;
#endif
#ifdef RTKBT_SWITCH_PATCH
	u8 *cmd;
	static u8 hci_state = 0;
	struct api_context ctx;
#endif

	data = container_of(notifier, struct btusb_data, pm_notifier);
	udev = data->udev;
	intf = data->intf;
	hdev = data->hdev;

	RTKBT_DBG("%s: pm_event %ld", __func__, pm_event);
	switch (pm_event) {
	case PM_SUSPEND_PREPARE:
	case PM_HIBERNATION_PREPARE:
		if (udev->state == USB_STATE_SUSPENDED)
			pm_runtime_resume(&udev->dev);
		/* No need to load firmware because the download firmware
		 * process is deprecated in resume.
		 * We use rebind after resume instead */
		/* err = usb_autopm_get_interface(data->intf);
		 * if (err < 0)
		 * 	return err;
		 * patch_entry->fw_len =
		 *     load_firmware(dev_entry, &patch_entry->fw_cache);
		 * usb_autopm_put_interface(data->intf);
		 * if (patch_entry->fw_len <= 0) {
		 * 	RTKBT_DBG("rtkbt_pm_notify return NOTIFY_BAD");
		 * 	return NOTIFY_BAD;
		 * } */

		RTKBT_DBG("%s: suspend prepare", __func__);

		if (!device_may_wakeup(&udev->dev)) {
#ifdef CONFIG_NEEDS_BINDING
			intf->needs_binding = 1;
			RTKBT_DBG("Remote wakeup not support, set "
				  "intf->needs_binding = 1");
#else
			RTKBT_DBG("Remote wakeup not support, no needs binding");
#endif
		}

#ifdef RTKBT_SWITCH_PATCH
		if (test_bit(HCI_UP, &hdev->flags)) {
			unsigned long expire;

			init_completion(&ctx.done);
			hci_state = 1;

			down(&switch_sem);
			data->context = &ctx;
			ctx.flags = RTLBT_CLOSE;
			queue_work(hdev->req_workqueue, &hdev->power_off.work);
			up(&switch_sem);

			expire = msecs_to_jiffies(1000);
			if (!wait_for_completion_timeout(&ctx.done, expire))
				RTKBT_ERR("hdev close timeout");

			down(&switch_sem);
			data->context = NULL;
			up(&switch_sem);
		}

		cmd = kzalloc(16, GFP_ATOMIC);
		if (!cmd) {
			RTKBT_ERR("Can't allocate memory for cmd");
			return -ENOMEM;
		}

		/* Clear patch */
		cmd[0] = 0x66;
		cmd[1] = 0xfc;
		cmd[2] = 0x00;

		result = __rtk_send_hci_cmd(udev, cmd, 3);
		kfree(cmd);
		msleep(100); /* From FW colleague's recommendation */
		result = download_special_patch(intf, "lps_");
#endif

#ifdef RTKBT_TV_POWERON_WHITELIST
		result = rtkbt_lookup_le_device_poweron_whitelist(hdev, udev);
		if (result < 0) {
			RTKBT_ERR("rtkbt_lookup_le_device_poweron_whitelist error: %d", result);
		}
#endif

#if defined(CONFIG_BTRTL_APCF)
		if (rtkbt_set_apcf(data))
			RTKBT_ERR("%s: set apcf error", __func__);
#endif

#if defined RTKBT_SUSPEND_WAKEUP || defined RTKBT_SWITCH_PATCH
#ifdef RTKBT_POWERKEY_WAKEUP
		/* Tell the controller to wake up host if received special
		 * advertising packet
		 */
		set_scan(intf);
#endif
		/* Send special vendor commands */
#endif

		break;

	case PM_POST_SUSPEND:
	case PM_POST_HIBERNATION:
	case PM_POST_RESTORE:
		/* if (patch_entry->fw_len > 0) {
		 * 	kfree(patch_entry->fw_cache);
		 * 	patch_entry->fw_cache = NULL;
		 * 	patch_entry->fw_len = 0;
		 * } */

#ifdef RTKBT_SWITCH_PATCH
		cmd = kzalloc(16, GFP_ATOMIC);
		if (!cmd) {
			RTKBT_ERR("Can't allocate memory for cmd");
			return -ENOMEM;
		}

		/* Clear patch */
		cmd[0] = 0x66;
		cmd[1] = 0xfc;
		cmd[2] = 0x00;

		result = __rtk_send_hci_cmd(udev, cmd, 3);
		kfree(cmd);
		msleep(100); /* From FW colleague's recommendation */
		result = download_patch(intf);
		if (hci_state) {
			hci_state = 0;
			queue_work(hdev->req_workqueue, &hdev->power_on);
		}
#endif

#ifdef BTUSB_RPM
		RTKBT_DBG("%s: Re-enable autosuspend", __func__);
		/* pm_runtime_use_autosuspend(&udev->dev);
		 * pm_runtime_set_autosuspend_delay(&udev->dev, 2000);
		 * pm_runtime_set_active(&udev->dev);
		 * pm_runtime_allow(&udev->dev);
		 * pm_runtime_mark_last_busy(&udev->dev);
		 * pm_runtime_autosuspend(&udev->dev);
		 * pm_runtime_put_autosuspend(&udev->dev);
		 * usb_disable_autosuspend(udev); */
		/* FIXME: usb_enable_autosuspend(udev) is useless here.
		 * Because it is always enabled after enabled in btusb_probe()
		 */
		usb_enable_autosuspend(udev);
		pm_runtime_mark_last_busy(&udev->dev);
#endif

#if defined(CONFIG_BTRTL_APCF)
		if (rtkbt_set_apcf_off(udev))
			RTKBT_ERR("%s: set apcf off error", __func__);

#endif
#if CONFIG_BTRTL_WAKEUP_REASON
		btusb_read_wakeup_reason(data);
#endif
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

static int rtkbt_shutdown_notify(struct notifier_block *notifier,
		    ulong pm_event, void *unused)
{
	struct btusb_data *data;
	struct usb_device *udev;
	struct usb_interface *intf;
	struct hci_dev *hdev;
	/* int err; */

	data = container_of(notifier, struct btusb_data, shutdown_notifier);
	udev = data->udev;
	intf = data->intf;
	hdev = data->hdev;

	RTKBT_DBG("%s: pm_event %ld", __func__, pm_event);
	switch (pm_event) {
	case SYS_POWER_OFF:
	case SYS_RESTART:
#ifdef RTKBT_SHUTDOWN_WAKEUP
		RTKBT_DBG("%s: power off", __func__);
		set_scan(intf);
#endif
		break;

	default:
		break;
	}

	return NOTIFY_DONE;
}

static int btusb_probe(struct usb_interface *intf,
		       const struct usb_device_id *id)
{
	struct usb_endpoint_descriptor *ep_desc;
	struct btusb_data *data;
	struct hci_dev *hdev;
	int i, err, flag1, flag2;
	struct usb_device *udev;
	struct hci_dev *ctrl = NULL;

	udev = interface_to_usbdev(intf);

	RTKBT_INFO("btusb_probe intf->cur_altsetting->desc.bInterfaceNumber %d",
		  intf->cur_altsetting->desc.bInterfaceNumber);

	/* interface numbers are hardcoded in the spec */
	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return -ENODEV;

	if (!id->driver_info) {
		const struct usb_device_id *match;

		match = usb_match_id(intf, blacklist_table);
		if (match)
			id = match;
		else
			return -ENODEV;
	}

	/*******************************/
	flag1 = device_can_wakeup(&udev->dev);
	flag2 = device_may_wakeup(&udev->dev);
	RTKBT_DBG("btusb_probe can_wakeup %x, may wakeup %x", flag1, flag2);
#ifdef BTUSB_WAKEUP_HOST
	device_wakeup_enable(&udev->dev);
#endif
	//device_wakeup_enable(&udev->dev);
	/*device_wakeup_disable(&udev->dev);
	   flag1=device_can_wakeup(&udev->dev);
	   flag2=device_may_wakeup(&udev->dev);
	   RTKBT_DBG("btusb_probe can_wakeup=%x  flag2=%x",flag1,flag2);
	 */
	err = patch_add(intf);
	if (err < 0)
		return -1;
	/*******************************/

	data = rtk_alloc(intf);
	if (!data)
		return -ENOMEM;

	for (i = 0; i < intf->cur_altsetting->desc.bNumEndpoints; i++) {
		ep_desc = &intf->cur_altsetting->endpoint[i].desc;
		if (!data->intr_ep && usb_endpoint_is_bulk_in(ep_desc) && (ep_desc->bEndpointAddress == 0x81)) {
			data->intr_ep = ep_desc;
			continue;
		}

		if (!data->intr_ep && usb_endpoint_is_int_in(ep_desc)) {
			data->intr_ep = ep_desc;
			continue;
		}

		if (!data->bulk_tx_ep && usb_endpoint_is_bulk_out(ep_desc)) {
			data->bulk_tx_ep = ep_desc;
			continue;
		}

		if (!data->bulk_rx_ep && usb_endpoint_is_bulk_in(ep_desc)) {
			data->bulk_rx_ep = ep_desc;
			continue;
		}
	}

	if (!data->intr_ep || !data->bulk_tx_ep || !data->bulk_rx_ep) {
		rtk_free(data);
		return -ENODEV;
	}

	data->cmdreq_type = USB_TYPE_CLASS;

	data->udev = interface_to_usbdev(intf);
	data->intf = intf;

	spin_lock_init(&data->lock);

	INIT_WORK(&data->work, btusb_work);
	INIT_WORK(&data->waker, btusb_waker);
	spin_lock_init(&data->txlock);

	init_usb_anchor(&data->tx_anchor);
	init_usb_anchor(&data->intr_anchor);
	init_usb_anchor(&data->bulk_anchor);
	init_usb_anchor(&data->isoc_anchor);
	init_usb_anchor(&data->deferred);

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
	spin_lock_init(&data->rxlock);
	data->recv_bulk = btusb_recv_bulk;
#endif

	hdev = hci_alloc_dev();
	if (!hdev) {
		rtk_free(data);
		return -ENOMEM;
	}

	HDEV_BUS = HCI_USB;

	data->hdev = hdev;

	SET_HCIDEV_DEV(hdev, &intf->dev);

	hdev->open = btusb_open;
	hdev->close = btusb_close;
	hdev->flush = btusb_flush;
	hdev->send = btusb_send_frame;
	hdev->notify = btusb_notify;
#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
	hdev->setup = btusb_setup;
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
	hdev->shutdown = btusb_shutdown;
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
	hci_set_drvdata(hdev, data);
#else
	hdev->driver_data = data;
	hdev->destruct = btusb_destruct;
	hdev->owner = THIS_MODULE;
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(5, 8, 0)
	set_bit(BTUSB_USE_ALT3_FOR_WBS, &data->flags);
#if HCI_VERSION_CODE < KERNEL_VERSION(6, 15, 8)
	set_bit(HCI_QUIRK_WIDEBAND_SPEECH_SUPPORTED, &hdev->quirks);
#else
	hci_set_quirk(hdev, HCI_QUIRK_WIDEBAND_SPEECH_SUPPORTED);
#endif
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 7, 1)
	if (!reset)
#if HCI_VERSION_CODE < KERNEL_VERSION(6, 15, 8)
		set_bit(HCI_QUIRK_RESET_ON_CLOSE, &hdev->quirks);
#else
		hci_set_quirk(hdev, HCI_QUIRK_RESET_ON_CLOSE);
#endif
#endif

	/* Interface numbers are hardcoded in the specification */
	data->isoc = usb_ifnum_to_if(data->udev, 1);

	if (data->isoc) {
		err = usb_driver_claim_interface(&btusb_driver,
						 data->isoc, data);
		if (err < 0) {
			hci_free_dev(hdev);
			rtk_free(data);
			return err;
		}
	}

#if HCI_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)
#if HCI_VERSION_CODE < KERNEL_VERSION(6, 15, 8)
	set_bit(HCI_QUIRK_SIMULTANEOUS_DISCOVERY, &hdev->quirks);
#else
	hci_set_quirk(hdev, HCI_QUIRK_SIMULTANEOUS_DISCOVERY);
#endif
#endif

	usb_set_intfdata(intf, data);

	atomic_set(&data->sync_state, DEVICE_STATE_ONLINE);
	mutex_init(&data->dl_lock);
	mutex_lock(&sync_lock);
	ctrl = rcu_dereference(controller);
	if (!ctrl)
		rcu_assign_pointer(controller, hdev);
	else
		RTKBT_ERR("controller has been set, %p, %p", ctrl, hdev);
	mutex_unlock(&sync_lock);

	err = hci_register_dev(hdev);
	if (err < 0) {
		hci_free_dev(hdev);
		rtk_free(data);
		return err;
	}

#if CONFIG_BTRTL_WAKEUP_REASON
	mutex_lock(&wakeup_reason_lock);
	if (!wakeup_reason)
		wakeup_reason = kzalloc(256, GFP_KERNEL);
	if (!wakeup_reason)
		RTKBT_WARN("%s: alloc mem for wakeup reason failed", __func__);
	mutex_unlock(&wakeup_reason_lock);
	device_create_file(&hdev->dev, &dev_attr_wakeup_reason);
#endif

#if defined(CONFIG_BTRTL_APCF)
	device_create_file(&hdev->dev, &dev_attr_apcf_filter);
	device_create_file(&hdev->dev, &dev_attr_apcf_wakeup);
	INIT_DELAYED_WORK(&data->apcf_work, (void *)apcf_work_func);
	schedule_delayed_work(&data->apcf_work,
			      msecs_to_jiffies(APCF_WORK_DEFER_TIME));
#endif

	/* Register PM notifier */
	data->pm_notifier.notifier_call = rtkbt_pm_notify;
	register_pm_notifier(&data->pm_notifier);

	/* Register POWER-OFF notifier */
	data->shutdown_notifier.notifier_call = rtkbt_shutdown_notify;
	register_reboot_notifier(&data->shutdown_notifier);
#ifdef BTCOEX
	rtk_btcoex_probe(hdev);
#endif

	RTKBT_DBG("%s: done", __func__);

	return 0;
}

static void rtlbt_detach_dev(struct usb_interface *intf)
{
	DECLARE_WAITQUEUE(wait, current);
	struct btusb_data *data = usb_get_intfdata(intf);
	struct hci_dev *hdev = data->hdev;
	struct hci_dev *ctrl = rcu_dereference(controller);
	int state;

	/* FIXME: currently only single controller is supported. */
	if (!hdev || hdev != ctrl)
		return;

	add_wait_queue(&sync_wq, &wait);

	mutex_lock(&sync_lock);

	usb_set_intfdata(intf, NULL);
	set_bit(BTUSB_DETACHED, &data->flags);

	while (1) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		state = atomic_read(&data->sync_state);
		if (state != DEVICE_STATE_LOADING)
			break;
		mutex_unlock(&sync_lock);
		schedule();
		mutex_lock(&sync_lock);
	}
	set_current_state(TASK_RUNNING);
	atomic_set(&data->sync_state, DEVICE_STATE_OFFLINE);
	rcu_assign_pointer(controller, NULL);
	synchronize_rcu();
	mutex_unlock(&sync_lock);
	remove_wait_queue(&sync_wq, &wait);
}

static void btusb_disconnect(struct usb_interface *intf)
{
	struct btusb_data *data = usb_get_intfdata(intf);
	struct hci_dev *hdev;
	struct usb_device *udev;

	udev = interface_to_usbdev(intf);

	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return;

	if (!data)
		return;

	RTKBT_DBG("btusb_disconnect");

	hdev = data->hdev;

	/* Un-register PM notifier */
	unregister_pm_notifier(&data->pm_notifier);
	unregister_reboot_notifier(&data->shutdown_notifier);

	/*******************************/
	patch_remove(intf);
	/*******************************/

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
	__hci_dev_hold(hdev);
#endif

	rtlbt_detach_dev(intf);

	if (data->isoc)
		usb_set_intfdata(data->isoc, NULL);

#ifdef CONFIG_BTRTL_APCF
	cancel_delayed_work_sync(&data->apcf_work);
	device_remove_file(&hdev->dev, &dev_attr_apcf_filter);
	device_remove_file(&hdev->dev, &dev_attr_apcf_wakeup);
	rtkbt_apcf_deinit();
#endif
#if CONFIG_BTRTL_WAKEUP_REASON
	device_remove_file(&hdev->dev, &dev_attr_wakeup_reason);
	mutex_lock(&wakeup_reason_lock);
	if (wakeup_reason)
		kfree(wakeup_reason);
	wakeup_reason = NULL;
	mutex_unlock(&wakeup_reason_lock);
#endif

	hci_unregister_dev(hdev);

	if (intf == data->isoc)
		usb_driver_release_interface(&btusb_driver, data->intf);
	else if (data->isoc)
		usb_driver_release_interface(&btusb_driver, data->isoc);

#if HCI_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
	__hci_dev_put(hdev);
#endif

#if HCI_VERSION_CODE >= KERNEL_VERSION(3, 18, 0)
	btusb_free_frags(data);
#endif

	hci_free_dev(hdev);
	rtk_free(data);
}

#ifdef CONFIG_PM
static int btusb_suspend(struct usb_interface *intf, pm_message_t message)
{
	struct btusb_data *data = usb_get_intfdata(intf);

	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return 0;

	/*******************************/
	RTKBT_DBG("btusb_suspend message.event 0x%x, data->suspend_count %d",
		  message.event, data->suspend_count);
	if (!test_bit(HCI_RUNNING, &data->hdev->flags)) {
		RTKBT_INFO("%s: hdev is not HCI_RUNNING", __func__);
		/* set_scan(data->intf); */
	}
	/*******************************/

	if (data->suspend_count++)
		return 0;

	spin_lock_irq(&data->txlock);
	if (!((message.event & PM_EVENT_AUTO) && data->tx_in_flight)) {
		set_bit(BTUSB_SUSPENDING, &data->flags);
		spin_unlock_irq(&data->txlock);
		RTKBT_INFO("%s: suspending...", __func__);
	} else {
		spin_unlock_irq(&data->txlock);
		data->suspend_count--;
		return -EBUSY;
	}

	cancel_work_sync(&data->work);

	btusb_stop_traffic(data);
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_kill_anchored_urbs(&data->tx_anchor);

	return 0;
}

static void play_deferred(struct btusb_data *data)
{
	struct urb *urb;
	int err;

	while ((urb = usb_get_from_anchor(&data->deferred))) {
	    /************************************/
		usb_anchor_urb(urb, &data->tx_anchor);
		err = usb_submit_urb(urb, GFP_ATOMIC);
		if (err < 0) {
			RTKBT_ERR("play_deferred urb %p submission failed",
				  urb);
			kfree(urb->setup_packet);
			usb_unanchor_urb(urb);
		} else {
			usb_mark_last_busy(data->udev);
		}
		usb_free_urb(urb);
		/************************************/
		data->tx_in_flight++;
	}
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_scuttle_anchored_urbs(&data->deferred);
}

static int btusb_resume(struct usb_interface *intf)
{
	struct btusb_data *data = usb_get_intfdata(intf);
	struct hci_dev *hdev = data->hdev;
	int err = 0;

	if (intf->cur_altsetting->desc.bInterfaceNumber != 0)
		return 0;

	/*******************************/
	RTKBT_DBG("%s: data->suspend_count %d", __func__, data->suspend_count);

	/* if intf->needs_binding is set, driver will be rebind.
	 * The probe will be called instead of resume */
	/* if (!test_bit(HCI_RUNNING, &hdev->flags)) {
	 * 	RTKBT_DBG("btusb_resume-----bt is off,download patch");
	 * 	download_patch(intf);
	 * } else
	 * 	RTKBT_DBG("btusb_resume,----bt is on");
	 */
	/*******************************/
	if (--data->suspend_count)
		return 0;

	if (test_bit(BTUSB_INTR_RUNNING, &data->flags)) {
		err = btusb_submit_intr_urb(hdev, GFP_NOIO);
		if (err < 0) {
			clear_bit(BTUSB_INTR_RUNNING, &data->flags);
			goto failed;
		}
	}

	if (test_bit(BTUSB_BULK_RUNNING, &data->flags)) {
		err = btusb_submit_bulk_urb(hdev, GFP_NOIO);
		if (err < 0) {
			clear_bit(BTUSB_BULK_RUNNING, &data->flags);
			goto failed;
		}

		btusb_submit_bulk_urb(hdev, GFP_NOIO);
	}

	if (test_bit(BTUSB_ISOC_RUNNING, &data->flags)) {
		if (btusb_submit_isoc_urb(hdev, GFP_NOIO) < 0)
			clear_bit(BTUSB_ISOC_RUNNING, &data->flags);
		else
			btusb_submit_isoc_urb(hdev, GFP_NOIO);
	}

	spin_lock_irq(&data->txlock);
	play_deferred(data);
	clear_bit(BTUSB_SUSPENDING, &data->flags);
	spin_unlock_irq(&data->txlock);
	schedule_work(&data->work);

	RTKBT_DBG("%s: data->suspend_count %d, done", __func__,
		  data->suspend_count);

	return 0;

failed:
	mdelay(URB_CANCELING_DELAY_MS);	// Added by Realtek
	usb_scuttle_anchored_urbs(&data->deferred);
//done:
	spin_lock_irq(&data->txlock);
	clear_bit(BTUSB_SUSPENDING, &data->flags);
	spin_unlock_irq(&data->txlock);
	RTKBT_DBG("%s: data->suspend_count %d, fail", __func__,
		  data->suspend_count);

	return err;
}
#endif

static struct usb_driver btusb_driver = {
	.name = "btrtl_usb",
	.probe = btusb_probe,
	.disconnect = btusb_disconnect,
#ifdef CONFIG_PM
	.suspend = btusb_suspend,
	.resume = btusb_resume,
#if defined RTKBT_SWITCH_PATCH || defined RTKBT_SUSPEND_WAKEUP || defined \
	RTKBT_SHUTDOWN_WAKEUP
	.reset_resume = btusb_resume,
#endif
#endif
	.id_table = btusb_table,
	.supports_autosuspend = 1,
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 7, 1)
	.disable_hub_initiated_lpm = 1,
#endif
};

static int __init btusb_init(void)
{
	RTKBT_DBG("Realtek Bluetooth USB driver ver %s", VERSION);
#ifdef BTCOEX
	rtk_btcoex_init();
#endif
	return usb_register(&btusb_driver);
}

static void __exit btusb_exit(void)
{
	RTKBT_DBG("btrtl_usb: btusb_exit");
	usb_deregister(&btusb_driver);

#ifdef BTCOEX
	rtk_btcoex_exit();
#endif
}

module_init(btusb_init);
module_exit(btusb_exit);

MODULE_AUTHOR("");
MODULE_DESCRIPTION("Realtek Bluetooth USB driver ver " VERSION);
MODULE_VERSION(VERSION);
MODULE_LICENSE("GPL");
