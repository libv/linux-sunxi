/*
*************************************************************************************
*                         			      Linux
*					           USB Device Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_udc_dma.c
*
* Author 		: javen
*
* Description 	: DMA 锟斤拷锟斤拷锟斤拷
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*       javen     	   2010-3-3            1.0          create this file
*
*************************************************************************************
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#ifdef CONFIG_DMA_ENGINE
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/dma/sunxi-dma.h>
#endif
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <asm/cacheflush.h>
//#include <mach/dma.h>

#include  "sunxi_udc_config.h"
#include  "sunxi_udc_board.h"
#include  "sunxi_udc_dma.h"



extern void sunxi_udc_dma_completion(struct sunxi_udc *dev, struct sunxi_udc_ep *ep, struct sunxi_udc_request *req);
/*
*******************************************************************************
*                     sunxi_udc_switch_bus_to_dma
*
* Description:
*    锟叫伙拷 USB 锟斤拷锟竭革拷 DMA
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_switch_bus_to_dma(struct sunxi_udc_ep *ep, u32 is_tx)
{
    return;
}

/*
*******************************************************************************
*                     sunxi_udc_switch_bus_to_pio
*
* Description:
*    锟叫伙拷 USB 锟斤拷锟竭革拷 PIO
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_switch_bus_to_pio(struct sunxi_udc_ep *ep, __u32 is_tx)
{
    return;
}

/*
*******************************************************************************
*                     sunxi_udc_enable_dma_channel_irq
*
* Description:
*    使锟斤拷 DMA channel 锟叫讹拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_enable_dma_channel_irq(struct sunxi_udc_ep *ep)
{
	DMSG_DBG_DMA("sunxi_udc_enable_dma_channel_irq\n");

    return;
}

/*
*******************************************************************************
*                     sunxi_udc_disable_dma_channel_irq
*
* Description:
*    锟斤拷止 DMA channel 锟叫讹拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_disable_dma_channel_irq(struct sunxi_udc_ep *ep)
{
    return;
}
/*
*******************************************************************************
*                     sunxi_udc_dma_callback
*
* Description:
*    DMA 初始化
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
static void sunxi_udc_dma_callback1(void *parg)//enum dma_cb_cause_e cause)
{
	struct sunxi_udc *dev = NULL;
	struct sunxi_udc_request *req = NULL;
	struct sunxi_udc_ep *ep = NULL;
	//printk("line:%d, %s\n", __LINE__, __func__);
	dev = (struct sunxi_udc *)parg;
	if(dev == NULL) {
		DMSG_PANIC("ERR: sw_udc_dma_callback failed\n");
		return ;
	}

    /* find ep */
	ep = &dev->ep[1];
	if(ep){
        /* find req */
		if(likely (!list_empty(&ep->queue))){
			req = list_entry(ep->queue.next, struct sunxi_udc_request, queue);
		}else{
			req = NULL;
		}

        /* call back */
		if(req){
		    sunxi_udc_dma_completion(dev, ep, req);
		}
	}else{
		DMSG_PANIC("ERR: sw_udc_dma_callback: dma is remove, but dma irq is happened\n");
	}

	return ;
}
static void sunxi_udc_dma_callback2(void *parg)//enum dma_cb_cause_e cause)
{
	struct sunxi_udc *dev = NULL;
	struct sunxi_udc_request *req = NULL;
	struct sunxi_udc_ep *ep = NULL;
	//printk("line:%d, %s\n", __LINE__, __func__);
	dev = (struct sunxi_udc *)parg;
	if(dev == NULL) {
		DMSG_PANIC("ERR: sw_udc_dma_callback failed\n");
		return ;
	}

    /* find ep */
	ep = &dev->ep[2];
	if(ep){
        /* find req */
		if(likely (!list_empty(&ep->queue))){
			req = list_entry(ep->queue.next, struct sunxi_udc_request, queue);
		}else{
			req = NULL;
		}

        /* call back */
		if(req){
		    sunxi_udc_dma_completion(dev, ep, req);
		}
	}else{
		DMSG_PANIC("ERR: sw_udc_dma_callback: dma is remove, but dma irq is happened\n");
	}

	return ;
}
static void sunxi_udc_dma_callback3(void *parg)//enum dma_cb_cause_e cause)
{
	struct sunxi_udc *dev = NULL;
	struct sunxi_udc_request *req = NULL;
	struct sunxi_udc_ep *ep = NULL;
	//printk("line:%d, %s\n", __LINE__, __func__);
	dev = (struct sunxi_udc *)parg;
	if(dev == NULL) {
		DMSG_PANIC("ERR: sw_udc_dma_callback failed\n");
		return ;
	}

    /* find ep */
	ep = &dev->ep[3];
	if(ep){
        /* find req */
		if(likely (!list_empty(&ep->queue))){
			req = list_entry(ep->queue.next, struct sunxi_udc_request, queue);
		}else{
			req = NULL;
		}

        /* call back */
		if(req){
		    sunxi_udc_dma_completion(dev, ep, req);
		}
	}else{
		DMSG_PANIC("ERR: sw_udc_dma_callback: dma is remove, but dma irq is happened\n");
	}

	return ;
}
static void sunxi_udc_dma_callback4(void *parg)//enum dma_cb_cause_e cause)
{
	struct sunxi_udc *dev = NULL;
	struct sunxi_udc_request *req = NULL;
	struct sunxi_udc_ep *ep = NULL;
	//printk("line:%d, %s\n", __LINE__, __func__);	
	dev = (struct sunxi_udc *)parg;
	if(dev == NULL) {
		DMSG_PANIC("ERR: sw_udc_dma_callback failed\n");
		return ;
	}

    /* find ep */
	ep = &dev->ep[4];
	if(ep){
        /* find req */
		if(likely (!list_empty(&ep->queue))){
			req = list_entry(ep->queue.next, struct sunxi_udc_request, queue);
		}else{
			req = NULL;
		}

        /* call back */
		if(req){
		    sunxi_udc_dma_completion(dev, ep, req);
		}
	}else{
		DMSG_PANIC("ERR: sw_udc_dma_callback: dma is remove, but dma irq is happened\n");
	}

	return ;
}
static void sunxi_udc_dma_callback5(void *parg)//enum dma_cb_cause_e cause)
{
	struct sunxi_udc *dev = NULL;
	struct sunxi_udc_request *req = NULL;
	struct sunxi_udc_ep *ep = NULL;
	//printk("line:%d, %s\n", __LINE__, __func__);
	dev = (struct sunxi_udc *)parg;
	if(dev == NULL) {
		DMSG_PANIC("ERR: sw_udc_dma_callback failed\n");
		return ;
	}

    /* find ep */
	ep = &dev->ep[5];
	if(ep){
        /* find req */
		if(likely (!list_empty(&ep->queue))){
			req = list_entry(ep->queue.next, struct sunxi_udc_request, queue);
		}else{
			req = NULL;
		}

        /* call back */
		if(req){
		    sunxi_udc_dma_completion(dev, ep, req);
		}
	}else{
		DMSG_PANIC("ERR: sw_udc_dma_callback: dma is remove, but dma irq is happened\n");
	}

	return ;
}
/*
*******************************************************************************
*                     sunxi_udc_dma_request
*
* Description:
*    DMA 锟斤拷始锟斤拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
dm_hdl_t sunxi_udc_dma_request(void)
{
	return 0;
}

int sunxi_udc_dma_release(dm_hdl_t dma_hdl)
{
	return 0;
}


/*
*******************************************************************************
*                     sunxi_udc_dma_set_config
*
* Description:
*    锟斤拷锟斤拷 DMA
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_dma_set_config(struct sunxi_udc_ep *ep, struct sunxi_udc_request *req, __u32 buff_addr, __u32 len)
{
	__u32 is_tx				= 0;
	//__u32 para				= 0;
	__u32 fifo_addr         = 0;
	//int ret 				= 0;
	unsigned int usbc_no = 0;
 	struct dma_slave_config slave_config;
	if(ep->dev->sunxi_udc_dma[ep->num].chan ==  NULL){
		DMSG_PANIC("udc_dma start error,DMA is NULL.\n");
		return ;	
	}
	//struct dma_async_tx_descriptor *dma_desc = NULL;
	memset(&slave_config, 0, sizeof(slave_config));
	is_tx = is_tx_ep(ep);
	
	fifo_addr = USBC_REG_EPFIFOx((u32)ep->dev->sunxi_udc_io->usb_vbase, ep->num);
    switch(ep->num){
		case 1:
			usbc_no = DRQSRC_OTG_EP1;
		break;

		case 2:
			usbc_no = DRQSRC_OTG_EP2;
		break;

		case 3:
			usbc_no = DRQSRC_OTG_EP3;
		break;

		case 4:
			usbc_no = DRQSRC_OTG_EP4;
		break;

		case 5:
			usbc_no = DRQSRC_OTG_EP5;
		break;

		default:
			usbc_no = 0;
	}

	if(!is_tx){ /* ep in, rx*/
		slave_config.direction = DMA_DEV_TO_MEM;
		slave_config.src_addr = fifo_addr & 0xfffffff;
		slave_config.dst_addr = buff_addr;
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.src_maxburst = 1;
		slave_config.dst_maxburst = 1;
		slave_config.slave_id = sunxi_slave_id(DRQDST_SDRAM, usbc_no);
		dmaengine_slave_config(ep->dev->sunxi_udc_dma[ep->num].chan, &slave_config);	
	}else{ /* ep out, tx*/
		slave_config.direction = DMA_MEM_TO_DEV;
		slave_config.src_addr = buff_addr;
		slave_config.dst_addr = fifo_addr & 0xfffffff;
		slave_config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		slave_config.src_maxburst = 1;
		slave_config.dst_maxburst = 1;
		slave_config.slave_id = sunxi_slave_id(usbc_no, DRQSRC_SDRAM);
		dmaengine_slave_config(ep->dev->sunxi_udc_dma[ep->num].chan, &slave_config);
	}


    return;
}

/*
*******************************************************************************
*                     sunxi_udc_dma_start
*
* Description:
*    锟斤拷始 DMA 锟斤拷锟斤拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_dma_start(struct sunxi_udc_ep *ep, __u32 fifo, dma_addr_t buffer, __u32 len)
{

	struct dma_async_tx_descriptor *dma_desc = NULL;
	__u32 is_tx	= 0;

	is_tx = is_tx_ep(ep);	
	if(ep->dev->sunxi_udc_dma[ep->num].chan ==  NULL){
		DMSG_PANIC("udc_dma start error,DMA is NULL.\n");
		return ;	
	}
	//if(!ep->dev->sunxi_udc_dma[ep->num].is_start){
		ep->dev->sunxi_udc_dma[ep->num].is_start = 1;
		if(is_tx){
			dma_desc = dmaengine_prep_slave_single(ep->dev->sunxi_udc_dma[ep->num].chan,buffer, len, 
						DMA_MEM_TO_DEV, DMA_PREP_INTERRUPT|DMA_CTRL_ACK);
			if (!dma_desc) {
				DMSG_PANIC("[sunxi-udc-%d]dmaengine_prep_slave_sg() failed!\n", ep->num);
		//		dma_unmap_sg(&sspi->pdev->dev, sspi->dma_rx.sg, sspi->dma_rx.nents, DMA_FROM_DEVICE);
				return;
			}
		}else{
			dma_desc = dmaengine_prep_slave_single(ep->dev->sunxi_udc_dma[ep->num].chan, buffer, len, 
						DMA_DEV_TO_MEM, DMA_PREP_INTERRUPT|DMA_CTRL_ACK);
			if (!dma_desc) {
				DMSG_PANIC("[sunxi-udc-%d]dmaengine_prep_slave_sg() failed!\n", ep->num);
		//		dma_unmap_sg(&sspi->pdev->dev, sspi->dma_rx.sg, sspi->dma_rx.nents, DMA_FROM_DEVICE);
				return ;
			}
		}	

		switch(ep->num){
		case 1:
			dma_desc->callback = sunxi_udc_dma_callback1;
		break;

		case 2:
			dma_desc->callback = sunxi_udc_dma_callback2;
		break;

		case 3:
			dma_desc->callback = sunxi_udc_dma_callback3;
		break;

		case 4:
			dma_desc->callback = sunxi_udc_dma_callback4;
		break;

		case 5:
			dma_desc->callback = sunxi_udc_dma_callback5;
		break;

		default:
			break;
		}
	
		//dma_desc->callback = sunxi_udc_dma_callback;
		dma_desc->callback_param = (void *)ep->dev;
		dmaengine_submit(dma_desc);
		dma_async_issue_pending(ep->dev->sunxi_udc_dma[ep->num].chan);
//	}

    return;
}   
/*
*******************************************************************************
*                     sunxi_udc_dma_stop
*
* Description:
*    锟斤拷止 DMA 锟斤拷锟斤拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
void sunxi_udc_dma_stop(struct sunxi_udc_ep *ep)
{
	int ret = 0;
	DMSG_DBG_DMA("line:%d, %s\n", __LINE__, __func__);

	ret = dmaengine_terminate_all(ep->dev->sunxi_udc_dma[ep->num].chan);
	if(ret != 0) {
		DMSG_PANIC("ERR: sunxi_dma_ctl stop  failed\n");
		return;
	}

    return;
}

/*
*******************************************************************************
*                     sunxi_udc_dma_transmit_length
*
* Description:
*    锟斤拷询 DMA 锟窖撅拷锟斤拷锟斤拷某锟斤拷锟
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
#if 0
__u32 sunxi_udc_dma_left_length(struct sunxi_udc_ep *ep, __u32 is_in, __u32 buffer_addr)
{
    dma_addr_t src = 0;
    dma_addr_t dst = 0;
	__u32 dma_buffer = 0;
	__u32 left_len = 0;

	DMSG_DBG_DMA("sunxi_udc_dma_transmit_length\n");

	sunxi_dma_getposition((dm_hdl_t)ep->dev->sunxi_udc_dma[ep->num].dma_hdle, &src, &dst);
	if(is_in){	/* tx */
		dma_buffer = (__u32)src;
	}else{	/* rx */
		dma_buffer = (__u32)dst;
	}

	left_len = buffer_addr - (u32)phys_to_virt(dma_buffer);

    DMSG_DBG_DMA("dma transfer lenght, buffer_addr(0x%x), dma_buffer(0x%x), left_len(%d), want(%d)\n",
		      buffer_addr, dma_buffer, left_len, ep->dma_transfer_len);

    return left_len;
}

__u32 sunxi_udc_dma_transmit_length(struct sunxi_udc_ep *ep, __u32 is_in, __u32 buffer_addr)
{
    if(ep->dma_transfer_len){
		return (ep->dma_transfer_len - sunxi_udc_dma_left_length(ep, is_in, buffer_addr));
	}

	return ep->dma_transfer_len;
}

#else
__u32 sunxi_udc_dma_transmit_length(struct sunxi_udc_ep *ep, __u32 is_in, __u32 buffer_addr)
{
	return ep->dma_transfer_len;;
}
#endif

/*
*******************************************************************************
*                     sunxi_udc_dma_is_busy
*
* Description:
*    DMA 锟斤拷始锟斤拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
__u32 sunxi_udc_dma_is_busy(struct sunxi_udc_ep *ep)
{
	return ep->dma_working;
}
/*
*******************************************************************************
*                     sunxi_udc_dma_probe
*
* Description:
*    DMA 锟斤拷始锟斤拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
__s32 sunxi_udc_dma_probe(struct sunxi_udc *dev)
{
	dma_cap_mask_t mask;
	int i = 0;
	printk("sunxi_udc_dma_probe version77..\n");
	/* Try to acquire a generic DMA engine slave channel */
	for(i = 1; i <= (USBC_MAX_EP_NUM - 1); i++){
		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		if (dev->sunxi_udc_dma[i].chan == NULL) {
			dev->sunxi_udc_dma[i].chan = dma_request_channel(mask, NULL, NULL);
		    if ( dev->sunxi_udc_dma[i].chan== NULL) {
		        printk("Request DMA(EP%d) failed!\n", i);
		        return -EINVAL;
		    }
		}
	}	

    return 0;
}

/*
*******************************************************************************
*                     sunxi_udc_dma_remove
*
* Description:
*    DMA 锟狡筹拷
*
* Parameters:
*    void
*
* Return value:
*    void
*
* note:
*    void
*
*******************************************************************************
*/
__s32 sunxi_udc_dma_remove(struct sunxi_udc *dev)
{
	//int i = 0;
	//int ret = 0;
	DMSG_DBG_DMA("line:%d, %s\n", __LINE__, __func__);
	#if 0
	for(i = 1; i <= (USBC_MAX_EP_NUM - 1); i++){
		if(dev->sunxi_udc_dma[i].chan!= NULL) {
			ret = dmaengine_terminate_all(dev->sunxi_udc_dma[i].chan);
			if(ret != 0) {
				DMSG_PANIC("ERR: sunxi_udc_dma_remove: stop failed\n");
			}
			dma_release_channel(dev->sunxi_udc_dma[i].chan);

			//dev->sunxi_udc_dma[i].dma_hdle = 0;
			dev->sunxi_udc_dma[i].is_start = 0;
			dev->ep[i].dma_working = 0;
			dev->ep[i].dma_transfer_len = 0;
		}
	}
	#endif
	return 0;
}

