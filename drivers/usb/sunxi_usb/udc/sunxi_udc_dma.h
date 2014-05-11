/*
*************************************************************************************
*                         			      Linux
*					           USB Device Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_udc_dma.h
*
* Author 		: javen
*
* Description 	: DMA ☠☠☠
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	   2010-3-3            1.0          create this file
*
*************************************************************************************
*/
#ifndef  __SUNXI_UDC_DMA_H__
#define  __SUNXI_UDC_DMA_H__

//---------------------------------------------------------------
//  ☠ ☠☠
//---------------------------------------------------------------
#ifdef  SW_UDC_DMA
#define  is_udc_support_dma()       1
#else
#define  is_udc_support_dma()       0
#endif

/* dma channel total */
#define DMA_CHAN_TOTAL		(8)
typedef void * dm_hdl_t;

/* define dma channel struct */
typedef struct {
	u32		used;     	/* 1 used, 0 unuse */
	u32		channel_num;
	u32		ep_num;
	u32		reg_base;	/* regs base addr */
	spinlock_t 	lock;		/* dma channel lock */
}dma_channel_t;

/*
 * dma config information
 */
struct dma_config_t {
	spinlock_t		lock;		/* dma channel lock */
	u32		dma_num;
	u32		dma_working;
	u32		dma_en;
	u32		dma_bst_len;
	u32		dma_dir;
	u32		dma_for_ep;
	u32		dma_sdram_str_addr;
	u32		dma_bc;
	u32		dma_residual_bc;
};

//---------------------------------------------------------------
//  ☠☠ ☠☠
//---------------------------------------------------------------
void sunxi_udc_switch_bus_to_dma(struct sunxi_udc_ep *ep, u32 is_tx);
void sunxi_udc_switch_bus_to_pio(struct sunxi_udc_ep *ep, __u32 is_tx);

void sunxi_udc_enable_dma_channel_irq(struct sunxi_udc_ep *ep);
void sunxi_udc_disable_dma_channel_irq(struct sunxi_udc_ep *ep);
void sunxi_dma_set_config(dm_hdl_t dma_hdl, struct dma_config_t *pcfg);
dm_hdl_t sunxi_udc_dma_request(void);
int sunxi_udc_dma_release(dm_hdl_t dma_hdl);
void sunxi_udc_dma_set_config(struct sunxi_udc_ep *ep, struct sunxi_udc_request *req, __u32 buff_addr, __u32 len);
void sunxi_udc_dma_start(struct sunxi_udc_ep *ep, __u32 fifo, __u32 buffer, __u32 len);
void sunxi_udc_dma_stop(struct sunxi_udc_ep *ep);
__u32 sunxi_udc_dma_transmit_length(struct sunxi_udc_ep *ep, __u32 is_in, __u32 buffer_addr);
__u32 sunxi_udc_dma_is_busy(struct sunxi_udc_ep *ep);

__s32 sunxi_udc_dma_probe(struct sunxi_udc *dev);
__s32 sunxi_udc_dma_remove(struct sunxi_udc *dev);



#endif   //__SUNXI_UDC_DMA_H__

