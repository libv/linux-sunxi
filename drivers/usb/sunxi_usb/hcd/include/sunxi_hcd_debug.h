/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hcd_debug.h
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
#ifndef  __SUNXI_HCD_DEBUG_H__
#define  __SUNXI_HCD_DEBUG_H__

#include  "sunxi_hcd_core.h"

void print_sunxi_hcd_config(struct sunxi_hcd_config *config, char *str);
void print_sunxi_hcd_list(struct list_head *list_head, char *str);
void print_urb_list(struct usb_host_endpoint *hep, char *str);

#endif   //__SUNXI_HCD_DEBUG_H__

