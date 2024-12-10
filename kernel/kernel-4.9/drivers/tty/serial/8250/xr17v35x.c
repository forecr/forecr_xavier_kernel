/*****************************************************************************/
/*
*      xr17v35x.c  -- MaxLinear multiport serial driver.
*
*      
*****************************************************************************
*                                        Copyright (c) 2010, MaxLinear, Inc.
*****************************************************************************
*
*      Based on Linux 2.6.37 Kernel's  drivers/serial/8250.c and /8250_pci.c
*
*      This program is free software; you can redistribute it and/or modify
*      it under the terms of the GNU General Public License as published by
*      the Free Software Foundation; either version 2 of the License, or
*      (at your option) any later version.
*
*      This program is distributed in the hope that it will be useful,
*      but WITHOUT ANY WARRANTY; without even the implied warranty of
*      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*      GNU General Public License for more details.
*
*      You should have received a copy of the GNU General Public License
*      along with this program; if not, write to the Free Software
*      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*
*	   Multiport Serial Driver for MaxLinear's PCI Family of UARTs (XR17V258/254/252/358/354/352/8358/4358/8354)
*								  (XR17D158/154/152)
*       ChangeLog:
*	   for    	: LINUX 2.6.32 and newer (Tested on various kernel versions from 2.6.32 to 4.15)
*	   date   	: July 2019
*	   version	: 2.6 
*	 Note: XR_17v35x_UART_RHR was not defined. Fixed  
*	Check Release Notes for information on what has changed in the new version.
*
*/

#include <linux/module.h>
#include <linux/tty.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/console.h>
#include <linux/sysrq.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/pci.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/export.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/sched.h>

#include <linux/file.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include <linux/tty_flip.h>
#include <linux/serial_reg.h>
#include <linux/serial.h>
#include <linux/serial_core.h>




#include <asm/io.h>
#include <asm/irq.h>
#include <asm/irq.h>
#include <asm/bitops.h>
#include <asm/byteorder.h>
#include <asm/serial.h>
#include <asm/io.h>
#include <asm/uaccess.h>


#include "linux/version.h"

#define _INLINE_ inline

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 0)
#define __devinitdata
#define __devinit
#define __devexit
#define __devexit_p
#endif

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 8, 0)
struct serial_uart_config {
	char	*name;
	int	dfl_xmit_fifo_size;
	int	flags;
};
#endif

//void tty_flip_buffer_push(struct tty_port *port);

/*
 * Definitions for PCI support.
 */
#define FL_BASE_MASK		0x0007
#define FL_BASE0		0x0000
#define FL_GET_BASE(x)		(x & FL_BASE_MASK)

#define NR_PORTS	256

#define XR_MAJOR       30
#define XR_MINOR       0
/*	
   Set this parameter to 1 to enable Debug mode
   The Driver enables the internal loopback under debug mode
   To disable internal loopback go to serialxr_set_termios
*/
#ifndef DEBUG
#define DEBUG 		0
#endif

/*
 * The special register set for XR17V35x UARTs.
 */
 
#define XR_17v35x_UART_RHR			0 
#define XR_17v35x_UART_THR			0
#define	XR_17V35X_UART_DLD	        2
#define	XR_17V35X_UART_MSR	        6
#define XR_17V35X_EXTENDED_FCTR		8
#define XR_17V35X_EXTENDED_EFR		9
#define XR_17V35X_TXFIFO_CNT		10
#define XR_17V35X_EXTENDED_TXTRG	10
#define XR_17V35X_RXFIFO_CNT		11
#define XR_17V35X_EXTENDED_RXTRG	11
#define XR_17V35X_UART_XOFF2      	13 
#define XR_17V35X_UART_XOFF1 		0xC0
#define XR_17V35X_UART_XON1			0xE0
#define XR_17V35X_FCTR_RTS_8DELAY	0x03
#define XR_17V35X_FCTR_TRGD			192
#define XR_17V35x_FCTR_RS485	    0x20

#define XR_17V35x_MPIOLVL_7_0       0x90
#define XR_17V35x_MPIO3T_7_0        0x91
#define XR_17V35x_MPIOSEL_7_0       0x93
#define XR_17V35x_MPIOLVL_15_8       0x96
#define XR_17V35x_MPIO3T_15_8        0x97
#define XR_17V35x_MPIOSEL_15_8       0x99


// Set this parameter to 1 to enable RS485 mode
#define ENABLE_RS485		        0
//Set this parameter to 1 to enable DTR RS-485 half duplex direction control
#define USE_DTR_RS485                    0
// Set this parameter to 1 to enabled internal loopback
#define ENABLE_INTERNAL_LOOPBACK      0

#define UART_17V35X_RX_OFFSET		0x100
#define UART_17V35X_TX_OFFSET 		0x100

#define	XR_17V35X_IER_RTSDTR	        0x40
#define XR_17V35X_IER_CTSDSR	        0x80

#define XR_17V35X_8XMODE	        0x88
#define XR_17V35X_4XMODE	        0x89

#define DIVISOR_CHANGED   0

#define PCI_NUM_BAR_RESOURCES	6
#define CAPTURE_SERIAL_INDEX    1

struct serial_private {
	struct pci_dev		*dev;
	unsigned int		nr;
	void __iomem		*remapped_bar[PCI_NUM_BAR_RESOURCES];
	struct pci_serial_quirk	*quirk;
	int    uart_index[NR_PORTS];
	int			line[0];
};

struct pciserial_board {
	unsigned int flags;
	unsigned int num_ports;
	unsigned int base_baud;
	unsigned int uart_offset;
	unsigned int reg_shift;
	unsigned int first_offset;
};

/*
 * init function returns:
 *  > 0 - number of ports
 *  = 0 - use board->num_ports
 *  < 0 - error
 */
struct pci_serial_quirk {
	u32	vendor;
	u32	device;
	u32	subvendor;
	u32	subdevice;
	int	(*init)(struct pci_dev *dev);
	int	(*setup)(struct serial_private *, 
			 const struct pciserial_board *,
			 struct uart_port *, int);
	void	(*exit)(struct pci_dev *dev);
};

/*
 * This is the configuration table for all of the PCI serial boards
 * which we support.  It is directly indexed by the xrpci_board_num_t enum
 * value, which is encoded in the pci_device_id PCI probe table's
 * driver_data member.
 *
 * The makeup of these names are:
 *  pbn_bn{_bt}_n_baud
 *
 *  bn   = PCI BAR number
 *  bt   = Index using PCI BARs
 *  n    = number of serial ports
 *  baud = baud rate
 */
enum xrpci_board_num_t {
	xr_8port = 0,
	xr_4port,
	xr_2port,
	xr_4354port,
	xr_8354port,
	xr_4358port,
	xr_8358port,
	xr_258port,
	xr_254port,
	xr_252port,
	xr_158port,
	xr_154port,
	xr_152port,
};

static struct pciserial_board xrpciserial_boards[] __devinitdata = {
	[xr_8port] = {
		.flags		= FL_BASE0,
		.num_ports	= 8,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	
	[xr_4port] = {
		.flags		= FL_BASE0,
		.num_ports	= 4,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_2port] = {
		.flags		= FL_BASE0,
		.num_ports	= 2,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_4354port] = {
		.flags		= FL_BASE0,
		.num_ports	= 8,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_8354port] = {
		.flags		= FL_BASE0,
		.num_ports	= 12,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_4358port] = {
		.flags		= FL_BASE0,
		.num_ports	= 12,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_8358port] = {
		.flags		= FL_BASE0,
		.num_ports	= 16,
		.base_baud	= 7812500*4,
		.uart_offset	= 0x400,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_258port] = {
		.flags		= FL_BASE0,
		.num_ports	= 8,
		.base_baud	= 1500000,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_254port] = {
		.flags		= FL_BASE0,
		.num_ports	= 4,
		.base_baud	= 1500000,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_252port] = {
		.flags		= FL_BASE0,
		.num_ports	= 2,
		.base_baud	= 1500000,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
    	[xr_158port] = {
		.flags		= FL_BASE0,
		.num_ports	= 8,
		.base_baud	= 921600,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_154port] = {
		.flags		= FL_BASE0,
		.num_ports	= 4,
		.base_baud	= 921600,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},
	[xr_152port] = {
		.flags		= FL_BASE0,
		.num_ports	= 2,
		.base_baud	= 921600,
		.uart_offset	= 0x200,
		.reg_shift	= 0,
		.first_offset	= 0,
	},

};

/*
 * Configuration:
 *   share_irqs - whether we pass SA_SHIRQ to request_irq().  This option
 *                is unsafe when used on edge-triggered interrupts.
 */
#define SERIALEXAR_SHARE_IRQS 1 
unsigned int share_irqs = SERIALEXAR_SHARE_IRQS;

/*
 * Debugging.
 */
#if 0
#define DEBUG_AUTOCONF(fmt...)	printk(fmt)
#else
#define DEBUG_AUTOCONF(fmt...)	do { } while (0)
#endif

#if DEBUG
#define DEBUG_INTR(fmt...)	printk(fmt)
#else
#define DEBUG_INTR(fmt...)	do { } while (0)
#endif

#define PASS_LIMIT	256

/*
 * We default to IRQ0 for the "no irq" hack.   Some
 * machine types want others as well - they're free
 * to redefine this in their header file.
 */
#define is_real_interrupt(irq)	((irq) != 0)
#define MSR_SAVE_FLAGS UART_MSR_ANY_DELTA

#define LSR_SAVE_FLAGS UART_LSR_BRK_ERROR_BITS
struct uart_xr_port {
	struct uart_port	port;
	struct timer_list	timer;		/* "no irq" timer */
	struct list_head	list;		/* ports on this IRQ */
	unsigned short		capabilities;	/* port capabilities */
	unsigned short		bugs;		/* port bugs */
	unsigned int		tx_loadsz;	/* transmit fifo load size */
	unsigned char		acr;
	unsigned char		ier;
	unsigned char		lcr;
	unsigned char		mcr;
	unsigned char		mcr_mask;	/* mask of user bits */
	unsigned char		mcr_force;	/* mask of forced bits */

	unsigned char		lsr_saved_flags;
	unsigned char		msr_saved_flags;
	
	unsigned short 		deviceid;
	unsigned char		channelnum;
	unsigned char       multidrop_address;
    unsigned char       multidrop_mode;
    unsigned char       is_match_address;
	/*
	 * We provide a per-port pm hook.
	 */
	void			(*pm)(struct uart_port *port,
					  unsigned int state, unsigned int old);
};

struct irq_info {
	struct			hlist_node node;
	int			irq;
	spinlock_t		lock;	/* Protects list not the hash */
	struct list_head	*head;
};

#define NR_IRQ_HASH		32	/* Can be adjusted later */
static struct hlist_head irq_lists[NR_IRQ_HASH];
static DEFINE_MUTEX(hash_mutex);	/* Used to walk the hash */

/*
 * Here we define the default xmit fifo size used for each type of UART.
 */
#define PORT_MAX_XR 2 
#define XRPCIe_TYPE 1 // the second entry that is [1] in the array
#define XRPCI25x_TYPE 2 // the third entry that is [2] in the array

static const struct serial_uart_config uart_config[PORT_MAX_XR+1] = {
	{ "Unknown",	1,	0 },
	{ "XR17v35x",	256,	0 },
	{ "XR17v25x",	64,	0 },
};

static int
setup_port(struct serial_private *priv, struct uart_port *port,
	   int bar, int offset, int regshift)
{
	struct pci_dev *dev = priv->dev;
	unsigned long base, len;

	if (bar >= PCI_NUM_BAR_RESOURCES)
		return -EINVAL;

	base = pci_resource_start(dev, bar);

	if (pci_resource_flags(dev, bar) & IORESOURCE_MEM) {
		len =  pci_resource_len(dev, bar);

		if (!priv->remapped_bar[bar])
			priv->remapped_bar[bar] = ioremap(base, len);
		if (!priv->remapped_bar[bar])
			return -ENOMEM;

		port->iotype = UPIO_MEM;
		port->iobase = 0;
		port->mapbase = base + offset;
		port->membase = priv->remapped_bar[bar] + offset;
		port->regshift = regshift;
	} else {
		//Exar's got to be memory mapped. some hardware reading error?
		return -EINVAL;
	}
	return 0;
}


static int
pci_default_setup(struct serial_private *priv,
		  const struct pciserial_board *board,
		  struct uart_port *port, int idx)
{
	unsigned int bar, offset = board->first_offset;

	bar = FL_GET_BASE(board->flags);	
	offset += idx * board->uart_offset;
	//printk(KERN_INFO "Exar PCIe device 0x%x\n", priv->dev->device);
	if((priv->dev->device == 0x4354 || priv->dev->device == 0x8354) && (idx >= 4))
	{
	    offset += 0x1000; // the ports on expansion device for 0x(4/8)354 sit at bar0+0x2000 offset. 
			      // So we need to add 0x1000 here as 4*0x400
	}

	return setup_port(priv, port, bar, offset, board->reg_shift);
}

/*
 * Master list of serial port init/setup/exit quirks.
 * This does not describe the general nature of the port.
 * (ie, baud base, number and location of ports, etc)
 *
 * This list is ordered alphabetically by vendor then device.
 * Specific entries must come before more generic entries.
 */
static struct pci_serial_quirk pci_serial_quirks[] = {	
	{
		.vendor		= 0x13a8,
		.device		= 0x358,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},
	
	{
		.vendor		= 0x13a8,
		.device		= 0x354,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x352,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x4354,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x8354,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x4358,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x8358,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x258,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},
	
	{
		.vendor		= 0x13a8,
		.device		= 0x254,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x252,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},
		{
		.vendor		= 0x13a8,
		.device		= 0x158,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},
	
	{
		.vendor		= 0x13a8,
		.device		= 0x154,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

	{
		.vendor		= 0x13a8,
		.device		= 0x152,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.setup		= pci_default_setup,	
	},

};

static inline int quirk_id_matches(u32 quirk_id, u32 dev_id)
{
	return quirk_id == PCI_ANY_ID || quirk_id == dev_id;
}

static struct pci_serial_quirk *find_quirk(struct pci_dev *dev)
{
	struct pci_serial_quirk *quirk;

	for (quirk = pci_serial_quirks; ; quirk++)
		if (quirk_id_matches(quirk->vendor, dev->vendor) &&
		    quirk_id_matches(quirk->device, dev->device))		    
		 	break;
	return quirk;
}

static _INLINE_ unsigned int serial_in(struct uart_xr_port *up, int offset)
{
	return readb(up->port.membase + offset);
}

static _INLINE_ void
serial_out(struct uart_xr_port *up, int offset, int value)
{
	writeb(value, up->port.membase + offset);
}

static void serialxr_stop_tx(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	int lcr;

	if (up->ier & UART_IER_THRI) {
		up->ier &= ~UART_IER_THRI;
		lcr = serial_in(up, UART_LCR);
		if (lcr & 0x80) {			
			printk(KERN_INFO "channelnum %d: serialxr stop tx - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
		}
		serial_out(up, UART_IER, up->ier);
	}
}

static void serialxr_start_tx(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	int lcr;

	if (!(up->ier & UART_IER_THRI)) {
		up->ier |= UART_IER_THRI;
		lcr = serial_in(up, UART_LCR);
		if (lcr & 0x80) {
			printk(KERN_INFO"channelnum %d: serialxr start tx - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
		}
		serial_out(up, UART_IER, up->ier);
	}
}

static void serialxr_stop_rx(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	int lcr;

	up->ier &= ~UART_IER_RLSI;
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr stop rx - LCR = 0x%x", up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	up->port.read_status_mask &= ~UART_LSR_DR;
	serial_out(up, UART_IER, up->ier);
}

static void serialxr_enable_ms(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	int lcr;

	up->ier |= UART_IER_MSI;
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr enable ms - LCR = 0x%x", up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_IER, up->ier);
}

static void
receive_chars(struct uart_xr_port *up, unsigned int *status)
{
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 9, 0)	
	struct uart_port *port = &up->port;
#else
	struct tty_struct *tty = up->port.state->port.tty;	
#endif	
	unsigned char ch[256], lsr = *status;
	char flag;
	int i, lcr, datasize_in_fifo, port_index;
	unsigned char tmp;		
	datasize_in_fifo = serial_in(up, XR_17V35X_RXFIFO_CNT);
	while(datasize_in_fifo!=serial_in(up, XR_17V35X_RXFIFO_CNT))
	/*Read Receive Fifo count until we get read same value twice*/
		datasize_in_fifo=serial_in(up, XR_17V35X_RXFIFO_CNT);

  	port_index = up->port.line;
	flag = TTY_NORMAL;
  
	if (unlikely(lsr & (UART_LSR_BI | UART_LSR_PE | UART_LSR_FE | UART_LSR_OE))) 
	{
		/*
		* Mask off conditions which should be ignored.
		*/
		lsr &= up->port.read_status_mask;
        if (lsr & UART_LSR_OE) 
		{
			printk("OverRun Happen....");
		} 
		if (lsr & UART_LSR_BI) 
		{
			DEBUG_INTR("handling break....");
			flag = TTY_BREAK;
		} 
		else if (lsr & UART_LSR_PE)
		{
			flag = TTY_PARITY;
			
			if(up->multidrop_mode == 1)
			{
				  //memcpy_fromio(ch, up->port.membase + UART_17V35X_RX_OFFSET, datasize_in_fifo);
					for(i=0;i<datasize_in_fifo;i++)
					{
						lcr = serial_in(up, UART_LCR);
						if (lcr & 0x80) {
							printk(KERN_INFO"channelnum %d: receive chars (multidrop mode) - LCR = 0x%x", up->channelnum, lcr);
							serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
						}
					  ch[i]= serial_in(up, XR_17v35x_UART_RHR);
					}
					up->port.icount.rx+=datasize_in_fifo;
		          //up->port.icount.rx+=datasize_in_fifo;
				  DEBUG_INTR("Receive address byte:%02x\n",ch[0]);
				  if(up->is_match_address == 0)
				  {
     				  if(ch[0] == up->multidrop_address)
     				  {
     				    DEBUG_INTR(" Enable the receiver\n");
     					//set EFR[4] = 1; enable the shaded bits 
     				    tmp = serial_in(up, XR_17V35X_EXTENDED_EFR);
     				    tmp |=0x10;
     				    serial_out(up, XR_17V35X_EXTENDED_EFR, tmp);
     				    serial_out(up,XR_17V35X_UART_MSR, 0);//Enable the receiver 
     				    //set EFR[4] = 0; disable the shaded bits 
     				    tmp = serial_in(up, XR_17V35X_EXTENDED_EFR);
     				    tmp &=~0x10;
     				    serial_out(up, XR_17V35X_EXTENDED_EFR, tmp); 
						up->is_match_address = 1;
     				  }
					  else
					  {
					    //do nothing
					  }
				  }
				  else
				  {
     				  if(ch[0] == up->multidrop_address)
     				  {
     				    //do dothing
     				  }
					  else
     				  {
     				    DEBUG_INTR(" Disable the receiver\n");
     					//set EFR[4] = 1; enable the shaded bits 
     				    tmp = serial_in(up, XR_17V35X_EXTENDED_EFR);
     				    tmp |=0x10;
     				    serial_out(up, XR_17V35X_EXTENDED_EFR, tmp);
     				    serial_out(up,XR_17V35X_UART_MSR, 0x04);//Disable the receiver 
     				     //set EFR[4] = 0; disable the shaded bits 
     				    tmp = serial_in(up, XR_17V35X_EXTENDED_EFR);
     				    tmp &=~0x10;
     				    serial_out(up, XR_17V35X_EXTENDED_EFR, tmp);
						up->is_match_address = 0;
     				  }
				  }
				  return;
			}
			if(up->multidrop_mode == 0)
			{
			    printk("handling port<%d> Parity error....(%d)\n",port_index,up->multidrop_mode);
			}
			
		}
		else if (lsr & UART_LSR_FE)
		{
		    DEBUG_INTR("handling Frame error....\n");	
			flag = TTY_FRAME;
		}
	}
	
    //memcpy_fromio(ch, up->port.membase + UART_17V35X_RX_OFFSET, datasize_in_fifo);
			
	//print_hex_dump(KERN_DEBUG,"R:",DUMP_PREFIX_NONE,16,1,ch,datasize_in_fifo,1);
    for(i=0;i<datasize_in_fifo;i++)
    {
		lcr = serial_in(up, UART_LCR);
		if (lcr & 0x80) {
			printk(KERN_INFO"channelnum %d: receive chars - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
		}
      ch[i]= serial_in(up, XR_17v35x_UART_RHR);
    }
	up->port.icount.rx+=datasize_in_fifo;
	
	for(i = 0; i < datasize_in_fifo; i++)
	{
		if (uart_handle_sysrq_char(&up->port, ch[i]))
			continue;
		uart_insert_char(&up->port, lsr, UART_LSR_OE, ch[i], flag);
	}
    spin_unlock(&up->port.lock);
#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 9, 0)
	tty_flip_buffer_push(&port->state->port);
#else
	tty_flip_buffer_push(tty);
#endif
	spin_lock(&up->port.lock);

	DEBUG_INTR(" LSR_DR...");
}

static void transmit_chars(struct uart_xr_port *up)
{
	struct circ_buf *xmit = &up->port.state->xmit;
	int count, bytes_in_fifo, tmp;
	int i, lcr;
	unsigned char *ch;
	if (up->port.x_char) {
		serial_out(up, UART_TX, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char = 0;
		return;
	}
	if (uart_tx_stopped(&up->port)) {
		serialxr_stop_tx(&up->port);
		return;
	}
	if (uart_circ_empty(xmit)) {
		serialxr_stop_tx(&up->port);
		return;
	}

	bytes_in_fifo = serial_in(up, XR_17V35X_TXFIFO_CNT);
	// read the fifo count untill we get the same value twice
	while (bytes_in_fifo != serial_in(up, XR_17V35X_TXFIFO_CNT))
		bytes_in_fifo = serial_in(up, XR_17V35X_TXFIFO_CNT);

	// how much buffer is availabe now to write?	
	count = up->port.fifosize - bytes_in_fifo;
	
	if (uart_circ_chars_pending(xmit) < count)
		count = uart_circ_chars_pending(xmit);
		
	do
	{	
		// if the count is more than (tail to end of the buffer), transmit only the rest here.
		// tail+tmp&(UART_XMIT_SIZE-1) will reset the tail to the starting of the circular buffer
		if( ((xmit->tail + count) & (UART_XMIT_SIZE-1)) < xmit->tail)
		{			
			tmp = UART_XMIT_SIZE - xmit->tail;
			//memcpy_toio(up->port.membase + UART_17V35X_TX_OFFSET, &(xmit->buf[xmit->tail]), tmp);
			ch = (unsigned char *)&(xmit->buf[xmit->tail]);
			//print_hex_dump(KERN_DEBUG,"T:",DUMP_PREFIX_NONE,16,1,&(xmit->buf[xmit->tail]),tmp,1);
			for(i=0;i<tmp;i++)
			{
				lcr = serial_in(up, UART_LCR);
				if (lcr & 0x80) {
					printk(KERN_INFO"channelnum %d: transmit_chars1 - LCR = 0x%x", up->channelnum, lcr);
					serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
				}				
			  serial_out(up, XR_17v35x_UART_THR, ch[i]);
			}
			xmit->tail += tmp;
			xmit->tail &= (UART_XMIT_SIZE-1);
			up->port.icount.tx += tmp;
			count	-= tmp;
		}
		else
		{	
		    ch = (unsigned char *)&(xmit->buf[xmit->tail]);
			//memcpy_toio(up->port.membase + UART_17V35X_TX_OFFSET, &(xmit->buf[xmit->tail]), count);	
			for(i=0;i < count;i++)
			{
				lcr = serial_in(up, UART_LCR);
				if (lcr & 0x80) {
					printk(KERN_INFO"channelnum %d: transmit_chars2 - LCR = 0x%x", up->channelnum, lcr);
					serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
				}
			  serial_out(up, XR_17v35x_UART_THR, ch[i]);
			}				    
			xmit->tail += count;
			xmit->tail &= UART_XMIT_SIZE - 1;
			up->port.icount.tx += count;
			count = 0;
		}

	}while (count > 0);
   	
	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);

	DEBUG_INTR("THRE...");

	if (uart_circ_empty(xmit))
		serialxr_stop_tx(&up->port);
}

static unsigned int check_modem_status(struct uart_xr_port *up)
{
	unsigned int status = serial_in(up, UART_MSR);

	status |= up->msr_saved_flags;
	up->msr_saved_flags = 0;
	if (status & UART_MSR_ANY_DELTA && up->ier & UART_IER_MSI &&
	    up->port.state != NULL) {
		if (status & UART_MSR_TERI)
			up->port.icount.rng++;
		if (status & UART_MSR_DDSR)
			up->port.icount.dsr++;
		if (status & UART_MSR_DDCD)
			uart_handle_dcd_change(&up->port, status & UART_MSR_DCD);
		if (status & UART_MSR_DCTS)
			uart_handle_cts_change(&up->port, status & UART_MSR_CTS);

		wake_up_interruptible(&up->port.state->port.delta_msr_wait);
	}

	return status;
}

/*
 * This handles the interrupt from one port.
 */
static void serialxr_handle_port(struct uart_xr_port *up)
{
	unsigned int status;
	unsigned long flags;

	spin_lock_irqsave(&up->port.lock, flags);

	status = serial_in(up, UART_LSR);

	DEBUG_INTR("status = %x...", status);

	if (status & (UART_LSR_DR | UART_LSR_BI))
		receive_chars(up, &status);
	check_modem_status(up);
	if (status & UART_LSR_THRE)
		transmit_chars(up);

	spin_unlock_irqrestore(&up->port.lock, flags);
}

/*
 * This is the serial driver's interrupt routine.
 *
 * Arjan thinks the old way was overly complex, so it got simplified.
 * Alan disagrees, saying that need the complexity to handle the weird
 * nature of ISA shared interrupts.  (This is a special exception.)
 *
 * In order to handle ISA shared interrupts properly, we need to check
 * that all ports have been serviced, and therefore the ISA interrupt
 * line has been de-asserted.
 *
 * This means we need to loop through all ports. checking that they
 * don't have an interrupt pending.
 */
static irqreturn_t serialxr_interrupt(int irq, void *dev_id)
{
	struct irq_info *i = dev_id;
	struct list_head *l, *end = NULL;
	int pass_counter = 0, handled = 0;

	DEBUG_INTR("serialxr_interrupt(%d)...", irq);

	spin_lock(&i->lock);

	l = i->head;
	do {
		struct uart_xr_port *up;
		unsigned int iir, lcr;

		up = list_entry(l, struct uart_xr_port, list);

		lcr = serial_in(up, UART_LCR);  // store value of LCR
		if (lcr & 0x80) {
			printk(KERN_INFO"channelnum %d: serialxr interrupt - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7F); // ensure LCR bit-7=0 before reading UART_IIR
		}		
		iir = serial_in(up, UART_IIR);
		if (!(iir & UART_IIR_NO_INT)) {
			serialxr_handle_port(up);

			handled = 1;

			end = NULL;
		} else if (end == NULL)
			end = l;

		serial_out(up, UART_LCR, lcr); // restore LCR
		l = l->next;
                /* add INT0 clear */
                 serial_in(up,0x80);
		if (l == i->head && pass_counter++ > 256) {
			/* If we hit this, we're dead. */
			printk(KERN_ERR "serialxr: too much work for "
				"irq%d\n", irq);
			break;
		}
	} while (l != end);

	spin_unlock(&i->lock);

	DEBUG_INTR("end.\n");
	return IRQ_RETVAL(handled);
}

/*
 * To support ISA shared interrupts, we need to have one interrupt
 * handler that ensures that the IRQ line has been deasserted
 * before returning.  Failing to do this will result in the IRQ
 * line being stuck active, and, since ISA irqs are edge triggered,
 * no more IRQs will be seen.
 */
static void serial_do_unlink(struct irq_info *i, struct uart_xr_port *up)
{
	spin_lock_irq(&i->lock);

	if (!list_empty(i->head)) {
		if (i->head == &up->list)
			i->head = i->head->next;
		list_del(&up->list);
	} else {
		BUG_ON(i->head != &up->list);
		i->head = NULL;
	}

	spin_unlock_irq(&i->lock);

	/* List empty so throw away the hash node */
	if (i->head == NULL) {
		hlist_del(&i->node);
		kfree(i);
	}
}

static int serial_link_irq_chain(struct uart_xr_port *up)
{
	struct hlist_head *h;
	struct hlist_node *n;
	struct irq_info *i;
	int ret, irq_flags = up->port.flags & UPF_SHARE_IRQ ? IRQF_SHARED : 0;

	mutex_lock(&hash_mutex);

	h = &irq_lists[up->port.irq % NR_IRQ_HASH];

	hlist_for_each(n, h) {
		i = hlist_entry(n, struct irq_info, node);
		if (i->irq == up->port.irq)
			break;
	}

	if (n == NULL) {
		i = kzalloc(sizeof(struct irq_info), GFP_KERNEL);
		if (i == NULL) {
			mutex_unlock(&hash_mutex);
			return -ENOMEM;
		}
		spin_lock_init(&i->lock);
		i->irq = up->port.irq;
		hlist_add_head(&i->node, h);
	}
	mutex_unlock(&hash_mutex);

	spin_lock_irq(&i->lock);

	if (i->head) {
		list_add_tail(&up->list, i->head);
		spin_unlock_irq(&i->lock);

		ret = 0;
	} else {
		INIT_LIST_HEAD(&up->list);
		i->head = &up->list;
		spin_unlock_irq(&i->lock);
		irq_flags |= up->port.irqflags;
		ret = request_irq(up->port.irq, serialxr_interrupt,
				  irq_flags, "xrserial", i);
		if (ret < 0)
			serial_do_unlink(i, up);
	}

	return ret;
}

static void serial_unlink_irq_chain(struct uart_xr_port *up)
{
	struct irq_info *i;
	struct hlist_node *n;
	struct hlist_head *h;

	mutex_lock(&hash_mutex);

	h = &irq_lists[up->port.irq % NR_IRQ_HASH];

	hlist_for_each(n, h) {
		i = hlist_entry(n, struct irq_info, node);
		if (i->irq == up->port.irq)
			break;
	}

	BUG_ON(n == NULL);
	BUG_ON(i->head == NULL);

	if (list_empty(i->head))
		free_irq(up->port.irq, i);

	serial_do_unlink(i, up);
	mutex_unlock(&hash_mutex);
}

static inline int poll_timeout(int timeout)
{
	return timeout > 6 ? (timeout / 2 - 2) : 1;
}

/*
 * This function is used to handle ports that do not have an
 * interrupt.  This doesn't work very well for 16450's, but gives
 * barely passable results for a 16550A.  (Although at the expense
 * of much CPU overhead).
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
static void serialxr_timeout(struct timer_list *p_tl)
#else
static void serialxr_timeout(unsigned long data)
#endif
{
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	struct uart_xr_port *up = from_timer(up, p_tl, timer);
	#else
	struct uart_xr_port *up = (struct uart_xr_port *)data;
	#endif

	unsigned int iir;
	int lcr;

	lcr = serial_in(up, UART_LCR);  // check value of LCR
	if (lcr & 0x80) {
		serial_out(up, UART_LCR, lcr & 0x7F);	// ensure LCR bit-7=0 before reading UART_IIR
	}
	iir = serial_in(up, UART_IIR);
	if (!(iir & UART_IIR_NO_INT))
		serialxr_handle_port(up);
	mod_timer(&up->timer, jiffies + poll_timeout(up->port.timeout));
}

#define BOTH_EMPTY (UART_LSR_TEMT | UART_LSR_THRE)

static unsigned int serialxr_tx_empty(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned long flags;
	unsigned int lsr;

	spin_lock_irqsave(&up->port.lock, flags);
	lsr = serial_in(up, UART_LSR);
	up->lsr_saved_flags |= lsr & LSR_SAVE_FLAGS;
	spin_unlock_irqrestore(&up->port.lock, flags);

	return (lsr & BOTH_EMPTY) == BOTH_EMPTY ? TIOCSER_TEMT : 0;
}

static unsigned int serialxr_get_mctrl(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned int status;
	unsigned int ret;

	status = check_modem_status(up);

	ret = 0;
	if (status & UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & UART_MSR_DSR)
		ret |= TIOCM_DSR;
	if (status & UART_MSR_CTS)
		ret |= TIOCM_CTS;
	return ret;
}

static void serialxr_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned char mcr = 0, efr;

	if (mctrl & TIOCM_RTS)
		mcr |= UART_MCR_RTS;
	if (mctrl & TIOCM_DTR)
		mcr |= UART_MCR_DTR;
	if (mctrl & TIOCM_OUT1)
		mcr |= UART_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= UART_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= UART_MCR_LOOP;

	mcr = (mcr & up->mcr_mask) | up->mcr_force | up->mcr;
	
	efr = serial_in(up, UART_EFR); 
	efr = efr & 0xEF;	// clear access to shaded registers so that write to MCR does not change from using DTR to RTS for RS-485 control
#if   USE_DTR_RS485
       mcr |= 0x04;
	printk(KERN_INFO "serialxr_set_mctrl mcr=%02x\n",mcr);
#endif
	serial_out(up, UART_EFR, efr);
	serial_out(up, UART_MCR, mcr);
}

static void serialxr_break_ctl(struct uart_port *port, int break_state)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned long flags;

	spin_lock_irqsave(&up->port.lock, flags);
	if (break_state == -1)
		up->lcr |= UART_LCR_SBC;
	else
		up->lcr &= ~UART_LCR_SBC;
	serial_out(up, UART_LCR, up->lcr);
	spin_unlock_irqrestore(&up->port.lock, flags);
}

static int serialxr_startup(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned long flags;
	unsigned int fctr_reg=0;
	int retval, lcr;
		
	up->capabilities = uart_config[up->port.type].flags;
 	serial_out(up, XR_17V35X_EXTENDED_EFR, UART_EFR_ECB);
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_IER, 0);

	/* Set the RX/TX trigger levels */
	/* These are some default values, the OEMs can change these values
		* according to their best case scenarios */
	
	if(up->deviceid > 0x258) // PCIe device
	{
		serial_out(up, XR_17V35X_EXTENDED_RXTRG, 32);
		serial_out(up, XR_17V35X_EXTENDED_TXTRG, 64);
	}
	else // for 25x
	{
		serial_out(up, XR_17V35X_EXTENDED_RXTRG, 32); // 25x
		serial_out(up, XR_17V35X_EXTENDED_TXTRG, 32);
	}

	/* Hysteresis level of 8, Enable Auto RS-485 Mode */
	fctr_reg=serial_in(up,XR_17V35X_EXTENDED_FCTR);
	DEBUG_INTR(KERN_INFO "serialxr_startup: FCTR=0x%x",fctr_reg);
#if ENABLE_RS485
	serial_out(up, XR_17V35X_EXTENDED_FCTR, fctr_reg|XR_17V35X_FCTR_TRGD | XR_17V35X_FCTR_RTS_8DELAY | XR_17V35x_FCTR_RS485);
#if USE_DTR_RS485	
	serial_out(up, UART_MCR, 0x04);  //use DTR for Auto RS-485 Control
#endif	
#else
	serial_out(up, XR_17V35X_EXTENDED_FCTR, (fctr_reg|XR_17V35X_FCTR_TRGD | XR_17V35X_FCTR_RTS_8DELAY)&0xDF);
#endif
	

	serial_out(up, UART_LCR, 0);

	/* Wake up and initialize UART */
	serial_out(up, XR_17V35X_EXTENDED_EFR, UART_EFR_ECB | 0x10/*Enable Shaded bits access*/);
	serial_out(up,XR_17V35X_UART_MSR, 0);
	serial_out(up, UART_LCR, 0);	// Do LCR first to avoid LCR bit-7=1 before writing to IER
	serial_out(up, UART_IER, 0);

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reeanbled in set_termios())
	 */
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO |
			UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT);
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_FCR, 0);
	
	/*
	 * Clear the interrupt registers.
	 */
	(void) serial_in(up, UART_LSR);
	(void) serial_in(up, UART_RX);
	(void) serial_in(up, UART_IIR);
	(void) serial_in(up, UART_MSR);
          /* add INT0 clear */
        serial_in(up,0x80);
if(port->irq) {
	retval = serial_link_irq_chain(up);
	if(retval)
		return retval;
}

	/*
	 * Now, initialize the UART
	 */
	serial_out(up, UART_LCR, UART_LCR_WLEN8);

	spin_lock_irqsave(&up->port.lock, flags);
		
	/*
	* Most PC uarts need OUT2 raised to enable interrupts.
	*/
	if (is_real_interrupt(up->port.irq))
		up->port.mctrl |= TIOCM_OUT2;
	//to enable intenal loop, uncomment the line below
	//up->port.mctrl |= TIOCM_LOOP;

	serialxr_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->port.lock, flags);

	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	up->ier = UART_IER_RLSI | UART_IER_RDI;
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr startup - LCR = 0x%x", up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_IER, up->ier);

	/*
	 * And clear the interrupt registers again for luck.
	 */
	(void) serial_in(up, UART_LSR);
	(void) serial_in(up, UART_RX);
	(void) serial_in(up, UART_IIR);
	(void) serial_in(up, UART_MSR);
         /* add INT0 clear */
        serial_in(up,0x80); 
	return 0;
}

static void serialxr_shutdown(struct uart_port *port)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	unsigned long flags;
	unsigned char lsr;
	int i = 0;
	int lcr;
	
	while(1)
	{
	   i++;
	   lsr = serial_in(up, UART_LSR);
	   if((lsr&0x60) != 0x60)
	      printk("serialxr_shutdown wait TXFIFO Empty %02x",lsr);
	   else
	   	  break;
	   msleep(1);
	   if(i>1000) break;
	   
	}  
	
	/*
	 * Disable interrupts from this port
	 */
	up->ier = 0;
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr_shutdown1 - LCR = 0x%x",	up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_IER, 0);

	spin_lock_irqsave(&up->port.lock, flags);
	
	up->port.mctrl &= ~TIOCM_OUT2;

	serialxr_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->port.lock, flags);

	/*
	 * Disable break condition and FIFOs
	 */
	serial_out(up, UART_LCR, serial_in(up, UART_LCR) & (~UART_LCR_SBC) & 0x7f);
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO |
				  UART_FCR_CLEAR_RCVR |
				  UART_FCR_CLEAR_XMIT);
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr_shutdown2 - LCR = 0x%x",	up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_FCR, 0);

	/*
	 * Read data port to reset things, and then unlink from
	 * the IRQ chain.
	 */
	(void) serial_in(up, UART_RX);
#if 1
if (port->irq)
	serial_unlink_irq_chain(up);
#endif
}

static int quot_coeff = 16 ;
static unsigned char low_baudrate_mode = 0;
static unsigned int uart_get_divisor_exar(struct uart_port *port, unsigned int baud)
{
	unsigned int quot;

	/*
	 * Old custom speed handling.
	 */
	if (baud == 38400 && (port->flags & UPF_SPD_MASK) == UPF_SPD_CUST)
		quot = port->custom_divisor;
	else
		{
			if(low_baudrate_mode)
			   quot = DIV_ROUND_CLOSEST(port->uartclk/4, quot_coeff * baud);
			else
			   quot = DIV_ROUND_CLOSEST(port->uartclk, quot_coeff * baud);	
		}
	
	

//	DEBUG_INTR(KERN_INFO "uart_get_divisor_exar:UartClk=%d QuotCoeff=0x%x",port->uartclk,quot_coeff);
	return quot;
}


static unsigned int serialxr_get_divisor(struct uart_port *port, unsigned int baud)
{
	unsigned int quot;

	quot = uart_get_divisor_exar(port, baud);

	return quot;
}

static void
serialxr_set_special_baudrate(struct uart_port *port,unsigned int special_baudrate)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	signed int baud, quot;
    signed int quot_fraction = 0;
	unsigned char val_4xmode;
	unsigned char val_8xmode;
	unsigned char lcr_bak;
	unsigned int reg_read;
	int port_index = up->channelnum;
	printk(KERN_INFO "Enter in serialxr_set non-standard baudrate:%d channelnum:%d\n",special_baudrate,up->channelnum);
	
	baud = special_baudrate/*uart_get_baud_rate(port, termios, old, 0, port->uartclk/4)*/;
    lcr_bak = serial_in(up, UART_LCR);
	val_4xmode = serial_in(up, XR_17V35X_4XMODE);
	val_8xmode = serial_in(up, XR_17V35X_8XMODE);
	    	
	if((port_index > 15)||(port_index < 0))
	{
	   return;
	}

	switch(up->deviceid)
	{
	  case 0x4354:
	  case 0x8354:
	  	if(port_index >= 4) port_index = port_index - 4;
	  	break;
	  case 0x4358:
	  case 0x8358:
	  	if(port_index >= 8) port_index = port_index - 8;
	  	break;
	 default:
	 	//Do nothing
	 	break;
	}
	
		
	if(baud < 12500000/16)
	{//using the 16x mode
	      val_4xmode &=~(1 << port_index);
	      val_8xmode &=~(1 << port_index);	
	      quot_coeff = 16;
	      printk(KERN_INFO "Using the 16x Mode\n");
	}
	else if((baud >= 12500000/16)&&(baud < 12500000/4))
	{//using the 8x mode
	      val_4xmode &=~(1 << port_index);
		  val_8xmode |=(1 << port_index);
		  quot_coeff = 8;
		  printk(KERN_INFO "Using the 8x Mode\n");
	}
	else 
	{//using the 4x mode
	   val_4xmode |=(1 << port_index);
	   val_8xmode &=~(1 << port_index);
	   quot_coeff = 4;
	   printk(KERN_INFO "Using the 4x Mode\n");
	}

	serial_out(up, XR_17V35X_8XMODE, val_8xmode);
	serial_out(up, XR_17V35X_4XMODE, val_4xmode);
	
	quot = serialxr_get_divisor(port, baud);
	if(!((up->deviceid == 0x152)||(up->deviceid == 0x154)||(up->deviceid == 0x158)))
	{
	    unsigned int quot_16;
	    DEBUG_INTR(KERN_INFO "XR_17V35X uartclk:%d Quot=0x%x\n",port->uartclk,quot);
	    if(quot_coeff == 16)
	    {
	        quot_16 = DIV_ROUND_CLOSEST(port->uartclk, baud);
		 quot_fraction = quot_16 & 0x0f;	
		 quot = (quot_16 >>4);
		
	    }
	    else if(quot_coeff == 8)
	    {
	         quot_16 = DIV_ROUND_CLOSEST(port->uartclk*2, baud);
		  quot_fraction = quot_16 & 0x0f;	
		  quot = (quot_16 >>4);
	    }
	    else if(quot_coeff == 4)
	    {
	         quot_16 = DIV_ROUND_CLOSEST(port->uartclk*4, baud);
		  quot_fraction = quot_16 & 0x0f;	
		  quot = (quot_16 >>4);
	    }
	    else
	    {
	       
	    }
       }
    serial_out(up, UART_LCR, lcr_bak | UART_LCR_DLAB);/* set DLAB */
    serial_out(up, UART_DLL, quot & 0xff);		/* LS of divisor */
    serial_out(up, UART_DLM, quot >> 8);		/* MS of divisor */
	//Fractional baud rate support
	if((up->deviceid == 0x152)||(up->deviceid == 0x154)||(up->deviceid == 0x158))
	{
	   //nothing to do , because these devices do not have support for the DLD register.
	}
	else
	{
	    reg_read=(serial_in(up, XR_17V35X_UART_DLD)&0xF0);
	    DEBUG_INTR(KERN_INFO "serialxr_set_special_baudrate: quot =0x%x quot_fraction=0x%x DLD_reg=0x%x\n",quot,quot_fraction,reg_read);		
	    serial_out(up, XR_17V35X_UART_DLD, quot_fraction | reg_read);
	    reg_read=serial_in(up, XR_17V35X_UART_DLD);
	 }
	 serial_out(up, UART_LCR, lcr_bak);		/* reset DLAB */

}

static void
serialxr_set_termios(struct uart_port *port, struct ktermios *termios,
		       struct ktermios *old)
{
struct uart_xr_port *up = (struct uart_xr_port *)port;
unsigned char cval;
unsigned long flags;
signed int baud, quot;
signed int quot_fraction = 0;
unsigned char val_4xmode;
unsigned char val_8xmode;
unsigned int reg_read;
unsigned char efr,mcr;
int lcr;

int port_index = up->channelnum;
switch (termios->c_cflag & CSIZE) 
{
case CS5:
		cval = 0x00;
		break;
case CS6:
		cval = 0x01;
		break;
case CS7:
		cval = 0x02;
		break;
default:
case CS8:
		cval = 0x03;
		break;
}

if (termios->c_cflag & CSTOPB)
		cval |= 0x04;
if (termios->c_cflag & PARENB)
		cval |= UART_LCR_PARITY;
if (!(termios->c_cflag & PARODD))
		cval |= UART_LCR_EPAR;
#ifdef CMSPAR
	if (termios->c_cflag & CMSPAR)
		cval |= UART_LCR_SPAR;
#endif

	/*
	 * Ask the core to calculate the divisor for us.
	 */
baud = uart_get_baud_rate(port, termios, old, 0, port->uartclk/4);

printk(KERN_INFO "\nserialxr_set_termios: Port Index:%d c_ispeed:%d c_ospeed:%d baud=%d",port_index,termios->c_ispeed,termios->c_ospeed,baud);

val_4xmode = serial_in(up, XR_17V35X_4XMODE);
val_8xmode = serial_in(up, XR_17V35X_8XMODE);
    	
if((port_index > 15)||(port_index < 0))
{
   return;
}

switch(up->deviceid)
{
  case 0x4354:
  case 0x8354:
	if(port_index >= 4) port_index = port_index - 4;
	break;
  case 0x4358:
  case 0x8358:
	if(port_index >= 8) port_index = port_index - 8;
	break;
 default:
	//Do nothing
	break;
}


if(baud < 120)
{
   //set EFR[4] = 1; enable the shaded bits 
   efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
   serial_out(up, XR_17V35X_EXTENDED_EFR, efr | 0x10);
   mcr=serial_in(up, UART_MCR);
   serial_out(up, UART_MCR, mcr | 0x80 );//set the prescaler (MCR bit-7 = 1, requires EFR bit-4 = 1) to divide the clock by 4.  
   //Restore the EFR Value
   serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
   low_baudrate_mode = 1;
}
else
{
   //set EFR[4] = 1; enable the shaded bits 
   efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
   serial_out(up, XR_17V35X_EXTENDED_EFR, efr | 0x10);
   mcr=serial_in(up, UART_MCR);
   serial_out(up, UART_MCR, mcr & 0x7f );//clr the prescaler (MCR bit-7 = 1, requires EFR bit-4 = 1) to divide the clock by 1.  
   //Restore the EFR Value
   serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
   low_baudrate_mode = 0;
}

	
if(baud < 12500000/16)
{//using the 16x mode
      val_4xmode &=~(1 << port_index);
      val_8xmode &=~(1 << port_index);	
      quot_coeff = 16;
      printk(KERN_INFO "Using the 16x Mode\n");
}
else if((baud >= 12500000/16)&&(baud < 12500000/4))
{//using the 8x mode
      val_4xmode &=~(1 << port_index);
	  val_8xmode |=(1 << port_index);
	  quot_coeff = 8;
	  printk(KERN_INFO "Using the 8x Mode\n");
}
else 
{//using the 4x mode
   val_4xmode |=(1 << port_index);
   val_8xmode &=~(1 << port_index);
   quot_coeff = 4;
   printk(KERN_INFO "Using the 4x Mode\n");
}
serial_out(up, XR_17V35X_8XMODE, val_8xmode);
serial_out(up, XR_17V35X_4XMODE, val_4xmode);
DEBUG_INTR(KERN_INFO "XR_17V35X_4XMODE:%d \n",serial_in(up, XR_17V35X_4XMODE));
DEBUG_INTR(KERN_INFO "XR_17V35X_8XMODE:%d \n",serial_in(up, XR_17V35X_8XMODE));
	
	quot = serialxr_get_divisor(port, baud);
	if(!((up->deviceid == 0x152)||(up->deviceid == 0x154)||(up->deviceid == 0x158)))
	 {
	    DEBUG_INTR(KERN_INFO "XR_17V35X uartclk:%d Quot=0x%x\n",port->uartclk,quot);
	    //#ifdef DIVISOR_CHANGED
	    if((port->uartclk/baud) > (quot_coeff*quot))
	    {	
		if(quot_coeff==16)  quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot));
		else if(quot_coeff==8) quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot))*2;
		else if(quot_coeff==4) quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot))*4;
	    }
	    else if(quot > 1)
	    {	
	       quot--;
	       if(quot_coeff==16)  quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot));
	       else if(quot_coeff==8) quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot))*2;
	       else if(quot_coeff==4) quot_fraction = ( (port->uartclk/baud) - (quot_coeff*quot))*4;

	    }
	    else
	    {
		quot_fraction = 0;
	    }

	    if(quot_fraction>=0x10) quot_fraction=0x0f;
	 }
//#endif	
	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	spin_lock_irqsave(&up->port.lock, flags);

	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_PE | UART_LSR_DR;
	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= UART_LSR_FE | UART_LSR_PE;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= UART_LSR_BI;

	/*
	 * Characteres to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= UART_LSR_PE | UART_LSR_FE;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= UART_LSR_OE;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		up->port.ignore_status_mask |= UART_LSR_DR;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier &= ~UART_IER_MSI;
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= UART_IER_MSI;

	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr_set_termios1 - LCR = 0x%x",	up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_IER, up->ier);
	reg_read=serial_in(up, XR_17V35X_EXTENDED_EFR);

	if(termios->c_cflag & CRTSCTS)
	{
	    serial_out(up, XR_17V35X_EXTENDED_EFR, reg_read|0xC0);
	    printk(KERN_INFO "Hardware Flow Control Enabled");	    
	}
	 else
	 {
	       serial_out(up, XR_17V35X_EXTENDED_EFR, reg_read & 0x3F);
	       printk(KERN_INFO "Hardware Flow Control Disabled\n");	    
	 }
	
	/*
	*	Auto XON/XOFF software flow control flags
	*/
	
	serial_out(up, XR_17V35X_UART_XON1,0x11); //Initializing XON1
	serial_out(up, XR_17V35X_UART_XOFF1,0x13); //Initializing XOFF1

	if(((termios->c_iflag) & IXOFF)&&((termios->c_iflag) & IXON))
	{
		serial_out(up, XR_17V35X_EXTENDED_EFR, (reg_read) | 0x0A );
		printk(KERN_INFO "Software Flow Control Enabled\n");
	}
	else 
	{
		serial_out(up, XR_17V35X_EXTENDED_EFR, (reg_read) & 0xF0 );
		printk(KERN_INFO "No Software Flow Control\n");
	}
	
	reg_read=serial_in(up, XR_17V35X_EXTENDED_EFR);
	
	if((termios->c_iflag) & IXANY)
	{
		serial_out(up, XR_17V35X_EXTENDED_EFR, ((termios->c_iflag) & IXOFF)||((termios->c_iflag) & IXON)?((reg_read) | 0x1A):((reg_read) | 0x10));
		reg_read=serial_in(up, UART_MCR);
		serial_out(up, UART_MCR, (reg_read) | 0x20 );
		serial_out(up, XR_17V35X_EXTENDED_EFR, (reg_read) & 0xEF );
		printk(KERN_INFO "AUTO XANY Enabled\n");
	}
	else 
	{
		serial_out(up, XR_17V35X_EXTENDED_EFR, (reg_read) | 0x10 );
		reg_read=serial_in(up, UART_MCR);
		serial_out(up, UART_MCR, (reg_read) & 0xDF );
		reg_read=serial_in(up,XR_17V35X_EXTENDED_EFR);
		serial_out(up, XR_17V35X_EXTENDED_EFR, (reg_read) & 0xEF );
		printk(KERN_INFO "AUTO XANY NOT Enabled\n");
	}
	
//---------------------------------------------------------------------------//
	
	serial_out(up, UART_LCR, cval | UART_LCR_DLAB);/* set DLAB */
	
	serial_out(up, UART_DLL, quot & 0xff);		/* LS of divisor */
	serial_out(up, UART_DLM, quot >> 8);		/* MS of divisor */
	//Fractional baud rate support
	if((up->deviceid == 0x152)||(up->deviceid == 0x154)||(up->deviceid == 0x158))
	{
		//nothing to do , because these devices do not have support for the DLD register.
	}
	else
	{
	  reg_read=(serial_in(up, XR_17V35X_UART_DLD)&0xF0);
	  DEBUG_INTR(KERN_INFO "serialxr_set_termios: quot =0x%x quot_fraction=0x%x DLD_reg=0x%x\n",quot,quot_fraction,reg_read);		
	  serial_out(up, XR_17V35X_UART_DLD, quot_fraction | reg_read);
	  reg_read=serial_in(up, XR_17V35X_UART_DLD);
	}
	serial_out(up, UART_LCR, cval);		/* reset DLAB */
	up->lcr = cval;						/* Save LCR */
	
	lcr = serial_in(up, UART_LCR);
	if (lcr & 0x80) {
		printk(KERN_INFO"channelnum %d: serialxr_set_termios2 - LCR = 0x%x", up->channelnum, lcr);
		serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
	}
	serial_out(up, UART_FCR, UART_FCR_ENABLE_FIFO);/* set fcr */
	/*
		Configuring MPIO as inputs
	*/
	if((up->deviceid == 0x354)||(up->deviceid == 0x4354)||(up->deviceid == 0x8354))
	{
		serial_out(up, XR_17V35x_MPIOSEL_7_0,0x0FF); //0x0ff= ALL INPUTS	
	}
	else if((up->deviceid == 0x358)||(up->deviceid == 0x4358)||(up->deviceid == 0x8358))
	{
		serial_out(up, XR_17V35x_MPIOSEL_7_0,0x0FF); //0x0ff= ALL INPUTS
		serial_out(up, XR_17V35x_MPIOSEL_15_8,0x0FF); //0x0ff= ALL INPUTS
	}
	
	serialxr_set_mctrl(&up->port, up->port.mctrl);
#if ENABLE_INTERNAL_LOOPBACK
	reg_read=serial_in(up, UART_MCR);
	serial_out(up, UART_MCR, (reg_read) | 0x10);
	printk(KERN_INFO "Enabling Internal Loopback\n");
#endif
	spin_unlock_irqrestore(&up->port.lock, flags);
}

/*
 *      EXAR ioctls
 */
//#define 	FIOQSIZE		0x5460 
#define		EXAR_READ_REG      	(FIOQSIZE + 1)
#define 	EXAR_WRITE_REG     	(FIOQSIZE + 2)

#define 	EXAR_SET_MULTIDROP_MODE_NORMAL   (FIOQSIZE + 3)
#define 	EXAR_SET_MULTIDROP_MODE_AUTO     (FIOQSIZE + 4)
#define 	EXAR_SET_REMOVE_MULTIDROP_MODE   (FIOQSIZE + 5)
#define 	EXAR_SET_NON_STANDARD_BAUDRATE   (FIOQSIZE + 6)



struct xrioctl_rw_reg {
	unsigned char reg;
	unsigned char regvalue;
};
/*
 * This function is used to handle Exar Device specific ioctl calls
 * The user level application should have defined the above ioctl
 * commands with the above values to access these ioctls and the 
 * input parameters for these ioctls should be struct xrioctl_rw_reg
 * The Ioctl functioning is pretty much self explanatory here in the code,
 * and the register values should be between 0 to XR_17V35X_EXTENDED_RXTRG
 */

static int
serialxr_ioctl(struct uart_port *port, unsigned int cmd, unsigned long arg)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	int ret = -ENOIOCTLCMD;
	struct xrioctl_rw_reg ioctlrwarg;
    unsigned char address;
	unsigned char tmp,lcr_bak,dld,efr;
	unsigned int any_baudrate = 0;
	switch (cmd)
	{
		case EXAR_READ_REG:
		if (copy_from_user(&ioctlrwarg, (void *)arg, sizeof(ioctlrwarg)))
			return -EFAULT;
		ioctlrwarg.regvalue = serial_in(up, ioctlrwarg.reg);
		if (copy_to_user((void *)arg, &ioctlrwarg, sizeof(ioctlrwarg)))
			return -EFAULT;
		DEBUG_INTR(KERN_INFO "serialxr_ioctl read reg[0x%02x]=0x%02x \n",ioctlrwarg.reg,ioctlrwarg.regvalue);
		ret = 0;
		break;
		
		case EXAR_WRITE_REG:
		if (copy_from_user(&ioctlrwarg, (void *)arg, sizeof(ioctlrwarg)))
			return -EFAULT;
		serial_out(up, ioctlrwarg.reg, ioctlrwarg.regvalue);
		DEBUG_INTR(KERN_INFO "serialxr_ioctl write reg[0x%02x]=0x%02x \n",ioctlrwarg.reg,ioctlrwarg.regvalue);
		ret = 0;
		break;
		case EXAR_SET_MULTIDROP_MODE_NORMAL:
		if (copy_from_user(&address, (void *)arg, 1))
			return -EFAULT;
		   
		    up->multidrop_address = address;
			//set EFR[4] = 1; enable the shaded bits 
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr |=0x10;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
			
			serial_out(up,XR_17V35X_UART_MSR, 0x04);//Disable the receiver with mode=0
            //set EFR[4] =0; disable the shaded bits 
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr &=~0x10;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr); 
			
			
			lcr_bak = serial_in(up, UART_LCR);
			tmp = 0x80 | lcr_bak | 0x38; //LCR[7]=1 for access DLD  LCR[5:3] = '111'  for Forced parity to space "0"
			serial_out(up, UART_LCR, tmp);
			//set the DLD[6] = 1 enable Multidrop mode
			dld = serial_in(up, XR_17V35X_UART_DLD);
			dld |= 0x40;
			serial_out(up, XR_17V35X_UART_DLD, dld);
			
			//set EFR[5] = 0; disable the special char Select
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr &=~0x20;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
			
		    lcr_bak = serial_in(up, UART_LCR);//set LCR[7]=0
			lcr_bak &=~0x80;
			serial_out(up, UART_LCR, lcr_bak);
			
			ret = 0;
			up->multidrop_mode = 1;//for enable multidrop normal mode 
			up->is_match_address = 0;
			DEBUG_INTR(KERN_INFO "User request EXAR_SET_MULTIDROP_MODE_NORMAL addr:%d \n",up->multidrop_address);
		    break;
		
		case EXAR_SET_MULTIDROP_MODE_AUTO:
		if (copy_from_user(&address, (void *)arg, 1))
			return -EFAULT;
		    up->multidrop_address = address;
			
			serial_out(up, XR_17V35X_UART_XOFF2,address);
			
            //set EFR[4] = 1; enable the shaded bits 
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr |=0x10;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
			
			serial_out(up,XR_17V35X_UART_MSR, 0x04);//Disable the receiver with mode=0
			 //set EFR[4] =0; disable the shaded bits 
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr &=~0x10;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr); 
			
		    lcr_bak = serial_in(up, UART_LCR);
			tmp = 0x80 | lcr_bak | 0x38; //LCR[7]=1 for access DLD  LCR[5:3] = '111'  for Forced parity to space "0"
			serial_out(up, UART_LCR, tmp);
			
			//set the DLD[6] = 1 enable Multidrop mode
			dld = serial_in(up, XR_17V35X_UART_DLD);
			dld |= 0x40;
			serial_out(up, XR_17V35X_UART_DLD, dld);
			
			//set EFR[5] = 1; enable the special char Select
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr |=0x20;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);
			//printk(KERN_INFO "UART_EFR=0x%02x\n",serial_in(up, XR_17V35X_EXTENDED_EFR));
						
			up->multidrop_mode = 2;//for enable multidrop auto mode 

			lcr_bak = serial_in(up, UART_LCR);//set LCR[7]=0
			lcr_bak &=~0x80;
			serial_out(up, UART_LCR, lcr_bak);
			
			DEBUG_INTR(KERN_INFO "User request EXAR_SET_MULTIDROP_MODE_AUTO addr:%d \n",up->multidrop_address);
			ret = 0;	
			break;
		case EXAR_SET_REMOVE_MULTIDROP_MODE:
			//set the DLD[6] = 0 disable Multidrop mode
			lcr_bak = serial_in(up, UART_LCR);
			tmp = 0x80 | lcr_bak; //LCR[7]=1 for access DLD
			
			dld = serial_in(up, XR_17V35X_UART_DLD);
			dld &=~0x40;//Disable Multidrop mode
			serial_out(up, XR_17V35X_UART_DLD, tmp);
					
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr &=~0x20;//disable the special char select
			efr |= 0x10; //enable the shaded bits 
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);

			
			serial_out(up,XR_17V35X_UART_MSR, 0x00);//Enable the receiver with mode=0
			
             //set EFR[4] =0; disable the shaded bits 
			efr = serial_in(up, XR_17V35X_EXTENDED_EFR);
			efr &=~0x10;
			serial_out(up, XR_17V35X_EXTENDED_EFR, efr);  
			
            lcr_bak &=~0x38;//LCR[5:3] = '000'  
            lcr_bak &=~0x80;//Set LCR[7] = 0 
			serial_out(up, UART_LCR, lcr_bak);
			up->multidrop_mode = 0;
			up->is_match_address = 0;
		    ret = 0;	
		   
		break;
		case EXAR_SET_NON_STANDARD_BAUDRATE:
		if (copy_from_user(&any_baudrate, (void *)arg, sizeof(unsigned int)))
		{
		   return -EFAULT;	
		}
		serialxr_set_special_baudrate(port,any_baudrate);
		break;
		
	}
	
	return ret;
}
	      
static void
serialxr_pm(struct uart_port *port, unsigned int state,
	      unsigned int oldstate)
{
	int lcr;	
	struct uart_xr_port *up = (struct uart_xr_port *)port;
	if (state) {
		/* sleep */
		serial_out(up, XR_17V35X_EXTENDED_EFR, UART_EFR_ECB);
		lcr = serial_in(up, UART_LCR);
		if (lcr & 0x80) {
			printk(KERN_INFO"channelnum %d: serialxr_pm sleep - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
		}
		serial_out(up, UART_IER, UART_IERX_SLEEP);
		serial_out(up, XR_17V35X_EXTENDED_EFR, 0);
	} else {
		/* wake */
		serial_out(up, XR_17V35X_EXTENDED_EFR, UART_EFR_ECB);
		lcr = serial_in(up, UART_LCR);
		if (lcr & 0x80) {
			printk(KERN_INFO"channelnum %d: serialxr_pm wake - LCR = 0x%x", up->channelnum, lcr);
			serial_out(up, UART_LCR, lcr & 0x7f);	// Set LCR bit-7=0 when accessing RHR/THR/IER/ISR to avoid incorrect register access
		}
		serial_out(up, UART_IER, 0);
		serial_out(up, XR_17V35X_EXTENDED_EFR, 0);
	}

	if (up->pm)
		up->pm(port, state, oldstate);
}

static void serialxr_release_port(struct uart_port *port)
{	
}

static int serialxr_request_port(struct uart_port *port)
{
	return 0;
}

static void serialxr_config_port(struct uart_port *port, int flags)
{
	struct uart_xr_port *up = (struct uart_xr_port *)port;	

	if (flags & UART_CONFIG_TYPE)
	{	
		if(up->deviceid > 0x258) // PCIe device
		{
			up->port.type = XRPCIe_TYPE;
		}
		else
		{
			up->port.type = XRPCI25x_TYPE;
		}
		up->port.fifosize = uart_config[up->port.type].dfl_xmit_fifo_size;
		up->capabilities = uart_config[up->port.type].flags;	
	}
}

static const char *
serialxr_type(struct uart_port *port)
{
	int type = port->type;
	
	if (type >= ARRAY_SIZE(uart_config))
		type = 0;
	return uart_config[type].name;
}

static struct uart_ops serialxr_pops = {
	.tx_empty	= serialxr_tx_empty,
	.set_mctrl	= serialxr_set_mctrl,
	.get_mctrl	= serialxr_get_mctrl,
	.stop_tx	= serialxr_stop_tx,
	.start_tx	= serialxr_start_tx,
	.stop_rx	= serialxr_stop_rx,
	.enable_ms	= serialxr_enable_ms,
	.break_ctl	= serialxr_break_ctl,
	.startup	= serialxr_startup,
	.shutdown	= serialxr_shutdown,
	.set_termios	= serialxr_set_termios,
	.pm		= serialxr_pm,
	.type		= serialxr_type,
	.release_port	= serialxr_release_port,
	.request_port	= serialxr_request_port,
	.config_port	= serialxr_config_port,
	.ioctl		= serialxr_ioctl,
};

static DEFINE_MUTEX(serial_mutex);

static struct uart_xr_port serialxr_ports[NR_PORTS];

#define SERIALXR_CONSOLE	NULL

static struct uart_driver xr_uart_driver = {
	.owner			= THIS_MODULE,
	.driver_name		= "xrserial",
	.dev_name		= "ttyXR",
	.major			= XR_MAJOR,
	.minor			= XR_MINOR,
	.nr			= NR_PORTS,
	.cons			= SERIALXR_CONSOLE,
};

static struct uart_xr_port *serialxr_find_match_or_unused(struct uart_port *port)
{
	int i;

	/*
	 * First, find a port entry which matches.
	 */
	for (i = 0; i < NR_PORTS; i++)
		if (uart_match_port(&serialxr_ports[i].port, port))
			return &serialxr_ports[i];

	/*
	 * We didn't find a matching entry, so look for the first
	 * free entry.  We look for one which hasn't been previously
	 * used (indicated by zero iobase).
	 */
	for (i = 0; i < NR_PORTS; i++)
		if (serialxr_ports[i].port.type == PORT_UNKNOWN &&
		    serialxr_ports[i].port.iobase == 0)
		{
			port->line = i;
			return &serialxr_ports[i];
		}

	/*
	 * That also failed.  Last resort is to find any entry which
	 * doesn't have a real port associated with it.
	 */
	for (i = 0; i < NR_PORTS; i++)
		if (serialxr_ports[i].port.type == PORT_UNKNOWN)
			return &serialxr_ports[i];

	return NULL;
}


/*
 *	serialxr_register_port - register a serial port
 *	@port: serial port template
 *
 *	Configure the serial port specified by the request. If the
 *	port exists and is in use, it is hung up and unregistered
 *	first.
 *
 *	The port is then probed and if necessary the IRQ is autodetected
 *	If this fails an error is returned.
 *
 *	On success the port is ready to use and the line number is returned.
 */
int serialxr_register_port(struct uart_port *port, unsigned short deviceid, unsigned char channelnum)
{
	struct uart_xr_port *uart;
	int ret = -ENOSPC;

	if (port->uartclk == 0)
		return -EINVAL;

	mutex_lock(&serial_mutex);
	uart = serialxr_find_match_or_unused(port);
	if (uart) {
		uart->port.iobase   = port->iobase;
		uart->port.membase  = port->membase;
		uart->port.irq      = port->irq;
		uart->port.uartclk  = port->uartclk;
		uart->port.fifosize = port->fifosize;
		uart->port.regshift = port->regshift;
		uart->port.iotype   = port->iotype;
		uart->port.flags    = port->flags | UPF_BOOT_AUTOCONF;
		uart->port.mapbase  = port->mapbase;
		if (port->dev)
			uart->port.dev = port->dev;

		uart->deviceid = deviceid;
		uart->channelnum = channelnum;
		uart->port.line = port->line;
		spin_lock_init(&uart->port.lock);

		#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
		timer_setup(&uart->timer, serialxr_timeout, 0);
		#else
		init_timer(&uart->timer);
		uart->timer.function = serialxr_timeout;
		#endif

		/*
		 * ALPHA_KLUDGE_MCR needs to be killed.
		 */
		uart->mcr_mask = ~(0x0); //~ALPHA_KLUDGE_MCR;
		uart->mcr_force = 0; // ALPHA_KLUDGE_MCR;

		uart->port.ops = &serialxr_pops;		
		
		ret = uart_add_one_port(&xr_uart_driver, &uart->port);
#if 0
		if (ret == 0)
		{
			ret = uart->port.line;

			if (is_real_interrupt(uart->port.irq)) {
				serial_link_irq_chain(uart);
			}
		}
#endif
	}
	mutex_unlock(&serial_mutex);

	return ret;
}

/*
 * Probe one serial board.  Unfortunately, there is no rhyme nor reason
 * to the arrangement of serial ports on a PCI card.
 */
static int __devinit
init_one_xrpciserialcard(struct pci_dev *dev, const struct pci_device_id *ent)
{
	struct serial_private *priv;
	struct pciserial_board *board;
	struct pci_serial_quirk *quirk;
	struct uart_port serial_port;
	int rc, nr_ports, i;
			
	if (ent->driver_data >= ARRAY_SIZE(xrpciserial_boards)) {
		printk(KERN_INFO "pci_init_one: invalid driver_data: %ld\n",
			ent->driver_data);
		return -EINVAL;
	}

	board = &xrpciserial_boards[ent->driver_data];

	rc = pci_enable_device(dev);
	if (rc)
		return rc;
	
	nr_ports = board->num_ports;

	/*
	 * Find an init and setup quirks.
	 */
	quirk = find_quirk(dev);

	/*
	 * Run the new-style initialization function.
	 * The initialization function returns:
	 *  <0  - error
	 *   0  - use board->num_ports
	 *  >0  - number of ports
	 */
	if (quirk->init) {
		rc = quirk->init(dev);
		if (rc < 0)
			goto disable;
		if (rc)
			nr_ports = rc;
	}

	priv = kmalloc(sizeof(struct serial_private) +
		       sizeof(unsigned int) * nr_ports,
		       GFP_KERNEL);
	if (!priv) {
		rc = -ENOMEM;
		goto deinit;
	}

	memset(priv, 0, sizeof(struct serial_private) +
			sizeof(unsigned int) * nr_ports);

	priv->dev = dev;
	priv->quirk = quirk;

	memset(&serial_port, 0, sizeof(struct uart_port));
	serial_port.flags = UPF_SKIP_TEST | UPF_BOOT_AUTOCONF | UPF_SHARE_IRQ;

	if((priv->dev->device == 0x152)	||(priv->dev->device == 0x154)||(priv->dev->device == 0x158))
		serial_port.uartclk = board->base_baud * 16;
	else
		serial_port.uartclk = board->base_baud * 4;
	serial_port.irq = dev->irq;
	serial_port.dev = &dev->dev;
	for (i = 0; i < nr_ports; i++) {
		if (quirk->setup(priv, board, &serial_port, i))
			break;

		// setup the uartclock for the devices on expansion slot
		switch(priv->dev->device)
		{
		    case 0x4354:	      
		    case 0x8354:
		      if(i >= 4)
			serial_port.uartclk = 62500000; // half the clock speed of the main chip (which is 125MHz)
		      break;

		    case 0x4358:	      
		    case 0x8358:
		      if(i >= 8) // epansions slot ports
			serial_port.uartclk = 62500000; // half the clock speed of the main chip (which is 125MHz)
		      break;

		    default: //0x358/354/352/258/254/252
		    break;
		}

		rc = serialxr_register_port(&serial_port, dev->device,i);
		if (rc < 0) {
			printk(KERN_WARNING "Couldn't register serial port %s: %d\n", pci_name(dev), i);
			break;
		}
				
		printk(KERN_WARNING "init_one_xrpciserialcard line:%d\n",serial_port.line);
		priv->uart_index[i] = serial_port.line;
		priv->line[i] = rc;
		
		
	}

	priv->nr = i;

	if (!IS_ERR(priv)) {
		pci_set_drvdata(dev, priv);
		return 0;
	}

 deinit:
	if (quirk->exit)
		quirk->exit(dev);
 disable:
	pci_disable_device(dev);
	return rc;
}

/*
 *	serialxr_unregister_port - remove a serial port at runtime
 *	@line: serial line number
 *
 *	Remove one serial port.  This may not be called from interrupt
 *	context.  We hand the port back to the our control.
 */
void serialxr_unregister_port(int line)
{
	struct uart_xr_port *uart = &serialxr_ports[line];

	mutex_lock(&serial_mutex);
#if 0
	if (is_real_interrupt(uart->port.irq))
	  serial_unlink_irq_chain(uart);
#endif
	uart_remove_one_port(&xr_uart_driver, &uart->port);
	uart->port.dev = NULL;	
	mutex_unlock(&serial_mutex);
}

void pciserial_remove_ports(struct serial_private *priv)
{
	struct pci_serial_quirk *quirk;
	int i;

	for (i = 0; i < priv->nr; i++)
	{
	  	printk(KERN_WARNING "pciserial_remove_ports dev:%p port_num:%d\n",priv->dev,priv->uart_index[i]);
		//serialxr_unregister_port(priv->line[i]);
		serialxr_unregister_port(priv->uart_index[i]);
		
	}	

	for (i = 0; i < PCI_NUM_BAR_RESOURCES; i++) {
		if (priv->remapped_bar[i])
			iounmap(priv->remapped_bar[i]);
		priv->remapped_bar[i] = NULL;
	}

	/*
	 * Find the exit quirks.
	 */
	quirk = find_quirk(priv->dev);
	if (quirk->exit)
		quirk->exit(priv->dev);

	kfree(priv);
}

static void __devexit remove_one_xrpciserialcard(struct pci_dev *dev)
{
	struct serial_private *priv = pci_get_drvdata(dev);

	pci_set_drvdata(dev, NULL);

	pciserial_remove_ports(priv);

	pci_disable_device(dev);
}


static struct pci_device_id xrserial_pci_tbl[] = {
	{	0x13a8, 0x358,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_8port },
	{	0x13a8, 0x354,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_4port },
	{	0x13a8, 0x352,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_2port },
	{	0x13a8, 0x4354,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_4354port },
	{	0x13a8, 0x8354,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_8354port },
	{	0x13a8, 0x4358,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_4358port },
	{	0x13a8, 0x8358,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_8358port },
	{	0x13a8, 0x258,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_258port },
	{	0x13a8, 0x254,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_254port },
	{	0x13a8, 0x252,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_252port },
	{	0x13a8, 0x158,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_158port },
	{	0x13a8, 0x154,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_154port },
	{	0x13a8, 0x152,
		PCI_ANY_ID, PCI_ANY_ID,
		0, 0, xr_152port },

	{ 0, }
};

static struct pci_driver xrserial_pci_driver = {
	.name		= "xrserial",
	.probe		= init_one_xrpciserialcard,
	.remove		= __devexit_p(remove_one_xrpciserialcard),
	.id_table	= xrserial_pci_tbl,
};

static int __init serialxr_init(void)
{
	int ret;

	printk(KERN_INFO "Exar PCIe (XR17V35x) serial driver Revision: 2.6\n");

	ret = uart_register_driver(&xr_uart_driver);
	if (ret)
		return ret;

	ret = pci_register_driver(&xrserial_pci_driver);

	if (ret < 0)
		uart_unregister_driver(&xr_uart_driver);

	return ret;	
}

static void __exit serialxr_exit(void)
{
	pci_unregister_driver(&xrserial_pci_driver);
	uart_unregister_driver(&xr_uart_driver);
}

module_init(serialxr_init);
module_exit(serialxr_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Exar PCIe specific serial driver for XR17V35x- Revision: 2.6");
