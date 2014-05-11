/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hcd_config.h
*
* Author 		: javen
*
* Description 	:
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2010-12-20           1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_HCD_CONFIG_H__
#define  __SUNXI_HCD_CONFIG_H__

#include <linux/slab.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/errno.h>

#include  "../../include/sunxi_usb_config.h"
#include  "sunxi_hcd_debug.h"

//-------------------------------------------------------
//
//-------------------------------------------------------
//#define        XUSB_DEBUG    /* ☠锟皆匡拷☠ */

/* xusb hcd ☠锟皆达拷印 */
#if	0
    #define DMSG_DBG_HCD     			DMSG_PRINT
#else
    #define DMSG_DBG_HCD(...)
#endif

#endif   //__SUNXI_HCD_CONFIG_H__

