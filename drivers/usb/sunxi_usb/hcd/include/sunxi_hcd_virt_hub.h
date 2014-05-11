/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hcd_virt_hub.h
*
* Author 		: javen
*
* Description 	: ☠☠ hub
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2010-12-20           1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_HCD_VIRT_HUB_H__
#define  __SUNXI_HCD_VIRT_HUB_H__

void sunxi_hcd_root_disconnect(struct sunxi_hcd *sunxi_hcd);
int sunxi_hcd_hub_status_data(struct usb_hcd *hcd, char *buf);
int sunxi_hcd_hub_control(struct usb_hcd *hcd,
                     u16 typeReq,
                     u16 wValue,
                     u16 wIndex,
                     char *buf,
                     u16 wLength);

void sunxi_hcd_port_suspend_ex(struct sunxi_hcd *sunxi_hcd);
void sunxi_hcd_port_resume_ex(struct sunxi_hcd *sunxi_hcd);
void sunxi_hcd_port_reset_ex(struct sunxi_hcd *sunxi_hcd);

#endif   //__SUNXI_HCD_VIRT_HUB_H__

