/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hcd_dma.c
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

#include  "../include/sunxi_hcd_core.h"
#include  "../include/sunxi_hcd_dma.h"
#include <asm/cacheflush.h>

/*
*******************************************************************************
*                     sunxi_hcd_switch_bus_to_dma
*
* Description:
*    锟叫伙拷 USB ☠锟竭革拷 DMA
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
void sunxi_hcd_switch_bus_to_dma(struct sunxi_hcd_qh *qh, u32 is_in)
{
	DMSG_DBG_DMA("sunxi_hcd_switch_bus_to_dma\n");

    return;
}
EXPORT_SYMBOL(sunxi_hcd_switch_bus_to_dma);

/*
*******************************************************************************
*                     sunxi_hcd_switch_bus_to_pio
*
* Description:
*    锟叫伙拷 USB ☠锟竭革拷 PIO
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
void sunxi_hcd_switch_bus_to_pio(struct sunxi_hcd_qh *qh, __u32 is_in)
{
    return;
}
EXPORT_SYMBOL(sunxi_hcd_switch_bus_to_pio);

/*
*******************************************************************************
*                     sunxi_hcd_enable_dma_channel_irq
*
* Description:
*    使☠ DMA channel 锟叫讹拷
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
void sunxi_hcd_enable_dma_channel_irq(struct sunxi_hcd_qh *qh)
{
	DMSG_DBG_DMA("sunxi_hcd_enable_dma_channel_irq\n");

    return;
}
EXPORT_SYMBOL(sunxi_hcd_enable_dma_channel_irq);

/*
*******************************************************************************
*                     sunxi_hcd_disable_dma_channel_irq
*
* Description:
*    ☠止 DMA channel 锟叫讹拷
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
void sunxi_hcd_disable_dma_channel_irq(struct sunxi_hcd_qh *qh)
{
	DMSG_DBG_DMA("sunxi_hcd_disable_dma_channel_irq\n");

    return;
}
EXPORT_SYMBOL(sunxi_hcd_disable_dma_channel_irq);

/*
*******************************************************************************
*                     sunxi_hcd_dma_set_config
*
* Description:
*    ☠☠ DMA
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
void sunxi_hcd_dma_set_config(struct sunxi_hcd_qh *qh, __u32 buff_addr, __u32 len)
{

    return;
}
EXPORT_SYMBOL(sunxi_hcd_dma_set_config);

/*
*******************************************************************************
*                     sunxi_hcd_dma_start
*
* Description:
*    ☠始 DMA ☠☠
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
void sunxi_hcd_dma_start(struct sunxi_hcd_qh *qh, __u32 fifo, __u32 buffer, __u32 len)
{

    return;
}
EXPORT_SYMBOL(sunxi_hcd_dma_start);

/*
*******************************************************************************
*                     sunxi_hcd_dma_stop
*
* Description:
*    ☠止 DMA ☠☠
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
void sunxi_hcd_dma_stop(struct sunxi_hcd_qh *qh)
{
    return;
}
EXPORT_SYMBOL(sunxi_hcd_dma_stop);

/*
*******************************************************************************
*                     sunxi_hcd_dma_transmit_length
*
* Description:
*    ☠询 DMA 锟窖撅拷☠☠某☠锟
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
static __u32 sunxi_hcd_dma_left_length(struct sunxi_hcd_qh *qh, __u32 is_in, __u32 buffer_addr)
{

    return 0;
}

__u32 sunxi_hcd_dma_transmit_length(struct sunxi_hcd_qh *qh, __u32 is_in, __u32 buffer_addr)
{
	return 0;

}

#else
__u32 sunxi_hcd_dma_transmit_length(struct sunxi_hcd_qh *qh, __u32 is_in, __u32 buffer_addr)
{

	return 0;
}
#endif

EXPORT_SYMBOL(sunxi_hcd_dma_transmit_length);



/*
*******************************************************************************
*                     sunxi_hcd_dma_probe
*
* Description:
*    DMA ☠始☠
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
__u32 sunxi_hcd_dma_is_busy(struct sunxi_hcd_qh *qh)
{
	return 0;
}
EXPORT_SYMBOL(sunxi_hcd_dma_is_busy);

/*
*******************************************************************************
*                     sunxi_hcd_dma_probe
*
* Description:
*    DMA ☠始☠
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
__s32 sunxi_hcd_dma_probe(struct sunxi_hcd *sunxi_hcd)
{

    return 0;
}
EXPORT_SYMBOL(sunxi_hcd_dma_probe);

/*
*******************************************************************************
*                     sunxi_hcd_dma_remove
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
__s32 sunxi_hcd_dma_remove(struct sunxi_hcd *sunxi_hcd)
{

	return 0;
}
EXPORT_SYMBOL(sunxi_hcd_dma_remove);





