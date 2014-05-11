/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_usb_board.h
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
#ifndef  __SUNXI_USB_BOARD_H__
#define  __SUNXI_USB_BOARD_H__
#include  <mach/sys_config.h>
#include  <linux/gpio.h>

//----------------------------------------------------------
//
//----------------------------------------------------------
#define  SET_USB_PARA				"usb_para"
#define  SET_USB0					"usbc0"
#define  SET_USB1					"usbc1"
#define  SET_USB2					"usbc2"

#define  KEY_USB_GLOBAL_ENABLE		"usb_global_enable"
#define  KEY_USBC_NUM				"usbc_num"

#define  KEY_USB_ENABLE				"usb_used"
#define  KEY_USB_PORT_TYPE			"usb_port_type"
#define  KEY_USB_DETECT_TYPE		"usb_detect_type"
#define  KEY_USB_ID_GPIO			"usb_id_gpio"
#define  KEY_USB_DETVBUS_GPIO		"usb_det_vbus_gpio"
#define  KEY_USB_DRVVBUS_GPIO		"usb_drv_vbus_gpio"
#define  KEY_USB_RESTRICT_GPIO		"usb_restrict_gpio"

#define  KEY_USB_HOST_INIT_STATE    "usb_host_init_state"
#define  KEY_USB_USB_RESTRICT_FLAG  "usb_restric_flag"
#define  KEY_USB_USB_RESTRICT_VOLTAGE   "usb_restric_voltage"
#define  KEY_USB_USB_RESTRICT_CAPACITY  "usb_restric_capacity"


//---------------------------------------------------
//
//  USB  ☠☠☠息
//
//---------------------------------------------------
enum usb_gpio_group_type{
    GPIO_GROUP_TYPE_PIO = 0,
    GPIO_GROUP_TYPE_POWER,
};

/* 0: device only; 1: host only; 2: otg */
enum usb_port_type{
    USB_PORT_TYPE_DEVICE = 0,
    USB_PORT_TYPE_HOST,
    USB_PORT_TYPE_OTG,
};

/* 0: dp/dm☠猓 1: vbus/id☠锟 */
enum usb_detect_type{
    USB_DETECT_TYPE_DP_DM = 0,
    USB_DETECT_TYPE_VBUS_ID,
};

enum usb_det_vbus_type{
	USB_DET_VBUS_TYPE_NULL = 0,
    USB_DET_VBUS_TYPE_GIPO,
    USB_DET_VBUS_TYPE_AXP,
};


/* pio☠息 */
typedef struct usb_gpio{
	__u32 valid;          	/* pio锟角凤拷☠谩锟 0:☠效, !0:☠效	*/
	script_item_u gpio_set;
}usb_gpio_t;

typedef struct usb_port_info{
	__u32 enable;          				/* port锟角凤拷☠锟			*/

	__u32 port_no;						/* usb锟剿口猴拷			*/
	enum usb_port_type port_type;    	/* usb锟剿匡拷☠☠			*/
	enum usb_detect_type detect_type; 	/* usb☠夥绞			*/
	enum usb_det_vbus_type det_vbus_type;

	usb_gpio_t id;						/* usb id pin☠息 		*/
	usb_gpio_t det_vbus;				/* usb vbus pin☠息 	*/
	usb_gpio_t drv_vbus;				/* usb drv_vbus pin☠息	*/
	usb_gpio_t restrict_gpio_set;				/* usb drv_vbus pin☠息	*/
	__u32 usb_restrict_flag;						/* usb锟剿口猴拷			*/
	__u32 voltage;						/* usb锟剿口猴拷			*/
	__u32 capacity;						/* usb锟剿口猴拷			*/
	__u32 host_init_state;				/* usb ☠☠☠锟侥筹拷始☠状态☠0 : ☠☠☠. 1 : ☠☠ */
}usb_port_info_t;

typedef struct usb_cfg{
	u32 usb_global_enable;
	u32 usbc_num;

	struct usb_port_info port[USBC_MAX_CTL_NUM];
}usb_cfg_t;

#endif   //__SUNXI_USB_BOARD_H__

