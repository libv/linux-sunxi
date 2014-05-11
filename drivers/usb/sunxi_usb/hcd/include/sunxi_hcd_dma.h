/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hcd_dma.h
*
* Author 		: javen
*
* Description 	: dma☠☠
*
* Notes         :
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	  2010-12-20           1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_HCD_DMA_H__
#define  __SUNXI_HCD_DMA_H__

//---------------------------------------------------------------
//  ☠ ☠☠
//---------------------------------------------------------------
#if 1
#define  is_hcd_support_dma(usbc_no)   0
#else
#define  is_hcd_support_dma(usbc_no)    (usbc_no == 0)
#endif

/* 使☠DMA☠☠☠: 1☠☠☠☠☠  2☠DMA☠☠ 3☠☠ep0 */
#define  is_sunxi_hcd_dma_capable(usbc_no, len, maxpacket, epnum)	(is_hcd_support_dma(usbc_no) \
        	                                             		 && (len > maxpacket) \
        	                                             		 && epnum)

//---------------------------------------------------------------
//  ☠锟捷结构 ☠☠
//---------------------------------------------------------------
typedef struct sunxi_hcd_dma{
	char name[32];
	//struct sunxi_dma_client dma_client;

	int dma_hdle;	/* dma ☠锟 */
}sunxi_hcd_dma_t;

//---------------------------------------------------------------
//  ☠☠ ☠☠
//---------------------------------------------------------------
void sunxi_hcd_switch_bus_to_dma(struct sunxi_hcd_qh *qh, u32 is_in);
void sunxi_hcd_switch_bus_to_pio(struct sunxi_hcd_qh *qh, __u32 is_in);

void sunxi_hcd_dma_set_config(struct sunxi_hcd_qh *qh, __u32 buff_addr, __u32 len);
__u32 sunxi_hcd_dma_is_busy(struct sunxi_hcd_qh *qh);

void sunxi_hcd_dma_start(struct sunxi_hcd_qh *qh, __u32 fifo, __u32 buffer, __u32 len);
void sunxi_hcd_dma_stop(struct sunxi_hcd_qh *qh);
__u32 sunxi_hcd_dma_transmit_length(struct sunxi_hcd_qh *qh, __u32 is_in, __u32 buffer_addr);

__s32 sunxi_hcd_dma_probe(struct sunxi_hcd *sunxi_hcd);
__s32 sunxi_hcd_dma_remove(struct sunxi_hcd *sunxi_hcd);

#endif   //__SUNXI_HCD_DMA_H__


