/*
*************************************************************************************
*                         			      Linux
*					                 USB Host Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_usb_debug.h
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
#ifndef  __SUNXI_USB_DEBUG_H__
#define  __SUNXI_USB_DEBUG_H__


#ifdef  CONFIG_USB_SUNXI_USB_DEBUG

#define  DMSG_PRINT(stuff...)		printk(stuff)

#define  DMSG_INFO_UDC(...)			(DMSG_PRINT("[sw_udc]: "), DMSG_PRINT(__VA_ARGS__))
#define  DMSG_INFO_HCD0(...)		(DMSG_PRINT("[sw_hcd0]: "), DMSG_PRINT(__VA_ARGS__))
#define  DMSG_INFO_HCD1(...)		(DMSG_PRINT("[sw_hcd1]: "), DMSG_PRINT(__VA_ARGS__))
#define  DMSG_INFO_HCD2(...)		(DMSG_PRINT("[sw_hcd2]: "), DMSG_PRINT(__VA_ARGS__))
#define  DMSG_INFO_MANAGER(...)		(DMSG_PRINT("[usb_manager]: "), DMSG_PRINT(__VA_ARGS__))

#else

#define  DMSG_PRINT(...)
#define  DMSG_INFO_UDC(...)
#define  DMSG_INFO_HCD0(...)
#define  DMSG_INFO_HCD1(...)
#define  DMSG_INFO_HCD2(...)
#define  DMSG_INFO_MANAGER(...)

#endif

#define  DMSG_PRINT_EX(stuff...)		printk(stuff)

#define  DMSG_ERR(...)        		(DMSG_PRINT_EX("WRN:L%d(%s):", __LINE__, __FILE__), DMSG_PRINT_EX(__VA_ARGS__))


/* ☠☠ */
#if  0
    #define DMSG_TEST         			DMSG_PRINT
#else
    #define DMSG_TEST(...)
#endif

/* ☠☠☠锟 */
#if  0
    #define DMSG_MANAGER_DEBUG          DMSG_PRINT
#else
    #define DMSG_MANAGER_DEBUG(...)
#endif

#if  0
    #define DMSG_DEBUG        			DMSG_PRINT
#else
    #define DMSG_DEBUG(...)
#endif

/* ☠通☠息☠印 */
#if  1
    #define DMSG_INFO         			DMSG_PRINT
#else
    #define DMSG_INFO(...)
#endif

/* ☠锟截撅拷☠ */
#if	1
    #define DMSG_PANIC        			DMSG_ERR
#else
    #define DMSG_PANIC(...)
#endif

/* ☠通☠☠ */
#if	0
    #define DMSG_WRN        			DMSG_ERR
#else
    #define DMSG_WRN(...)
#endif

/* dma ☠锟皆达拷印 */
#if	0
    #define DMSG_DBG_DMA     			DMSG_PRINT
#else
    #define DMSG_DBG_DMA(...)
#endif


//---------------------------------------------------------------
//  ☠☠ ☠☠
//---------------------------------------------------------------
void print_usb_reg_by_ep(spinlock_t *lock, __u32 usbc_base, __s32 ep_index, char *str);
void print_all_usb_reg(spinlock_t *lock, __u32 usbc_base, __s32 ep_start, __u32 ep_end, char *str);

void clear_usb_reg(__u32 usb_base);
void fpga_config_use_otg(__u32 sram_vbase);

#endif   //__SUNXI_USB_DEBUG_H__

