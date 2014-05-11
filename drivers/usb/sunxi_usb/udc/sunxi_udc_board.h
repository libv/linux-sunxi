/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_udc_board.h
*
* Author 		: javen
*
* Description 	: 锟藉级☠☠
*
* Notes         :
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2010-12-20           1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_UDC_BOARD_H__
#define  __SUNXI_UDC_BOARD_H__

u32 open_usb_clock(sunxi_udc_io_t *sunxi_udc_io);
u32 close_usb_clock(sunxi_udc_io_t *sunxi_udc_io);

__s32 sunxi_udc_io_init(__u32 usbc_no, struct platform_device *pdev, sunxi_udc_io_t *sunxi_udc_io);
__s32 sunxi_udc_io_exit(__u32 usbc_no, struct platform_device *pdev, sunxi_udc_io_t *sunxi_udc_io);
__s32 sunxi_udc_bsp_init(__u32 usbc_no, sunxi_udc_io_t *sunxi_udc_io);
#endif   //__SUNXI_UDC_BOARD_H__


