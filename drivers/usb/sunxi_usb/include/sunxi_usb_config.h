/*
*************************************************************************************
*                         			      Linux
*					           USB Device Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_usb_config.h
*
* Author 		: javen
*
* Description 	:
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2011-4-14            1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_USB_CONFIG_H__
#define  __SUNXI_USB_CONFIG_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/errno.h>

#include  "sunxi_usb_typedef.h"
#include  "sunxi_usb_debug.h"
#include  "sunxi_usb_bsp.h"
#include  "sunxi_sys_reg.h"



#include  "sunxi_usb_board.h"
#include  "sunxi_udc.h"
#include  "sunxi_hcd.h"
#include  <mach/sys_config.h>
#include  <linux/gpio.h>

#if defined (CONFIG_FPGA_V4_PLATFORM) || defined (CONFIG_FPGA_V7_PLATFORM)
#define   SUNXI_USB_FPGA
#endif


#endif   //__SUNXI_USB_CONFIG_H__

