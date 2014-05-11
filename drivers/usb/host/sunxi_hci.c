/*
*************************************************************************************
*                         			      Linux
*					           USB Host Controller Driver
*
*				        (c) Copyright 2006-2010, All winners Co,Ld.
*							       All Rights Reserved
*
* File Name 	: sunxi_hci_sun6i.c
*
* Author 		: javen
*
* Description 	: Include file for AW1623 HCI Host Controller Driver
*
* Notes         :
*
* History 		:
*      <author>    		<time>       	<version >    		<desc>
*    yangnaitian      2011-5-24            1.0          create this file
*    javen            2011-7-18            1.1          ☠☠锟绞憋拷涌☠睾凸☠缈拷锟
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
#include <linux/clk/sunxi_name.h>
#include <linux/gpio.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/dma-mapping.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <mach/irqs.h>
#include <mach/platform.h>
#include <mach/sys_config.h>

#include  "sunxi_hci.h"

#ifndef  SUNXI_USB_FPGA
static char* usbc_name[4] 			= {"usbc0", "usbc1", "usbc2", "usbc3"};
#endif

static char* usbc_ahb_ehci_name[3]  = {"", USBEHCI_CLK, USBEHCI_CLK};
static char* usbc_ahb_ohci_name[4]  = {"", USBOHCI_CLK, USBOHCI_CLK, USBOHCI_CLK};
static char* usbc_phy_name[3] 	= {USBPHY0_CLK, USBPHY1_CLK, USBPHY1_CLK};
static u32 usbc_base[4] 			= {(u32)SUNXI_USB_OTG_VBASE, (u32)SUNXI_USB_HCI0_VBASE, (u32)SUNXI_USB_HCI0_VBASE, (u32)SUNXI_USB_HCI0_VBASE};
static u32 ehci_irq_no[3] 			= {0, SUNXI_IRQ_USBEHCI0, SUNXI_IRQ_USBEHCI0};
static u32 ohci_irq_no[4] 			= {0, SUNXI_IRQ_USBOHCI0, SUNXI_IRQ_USBOHCI0, SUNXI_IRQ_USBOHCI0};

static u32 usb1_set_vbus_cnt = 0;
static u32 usb2_set_vbus_cnt = 0;
static u32 usb3_set_vbus_cnt = 0;

static u32 usb1_enable_passly_cnt = 0;
static u32 usb2_enable_passly_cnt = 0;
static u32 usb3_enable_passly_cnt = 0;

static void sunxi_usb_3g_config(struct sunxi_hci_hcd *sunxi_hci)
{
#ifndef  SUNXI_USB_FPGA
    script_item_value_type_e type = 0;
	script_item_u item_temp;
    u32 usb_3g_used      = 0;
    u32 usb_3g_usbc_num  = 0;
    u32 usb_3g_usbc_type = 0;

    /* 3g_used */
    type = script_get_item("3g_para", "3g_used", &item_temp);
    if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
        usb_3g_used = item_temp.val;
    }else{
        //DMSG_PANIC("WRN: script_parser_fetch usb_3g_used failed\n");
        return;
    }

    /* 3g_usbc_num */
    type = script_get_item("3g_para", "3g_usbc_num", &item_temp);
    if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
        usb_3g_usbc_num = item_temp.val;
    }else{
        //DMSG_PANIC("WRN: script_parser_fetch usb_3g_usbc_num failed\n");
        return;
    }

    /* 3g_usbc_type */
    type = script_get_item("3g_para", "3g_usbc_type", &item_temp);
    if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
        usb_3g_usbc_type = item_temp.val;
    }else{
        //DMSG_PANIC("WRN: script_parser_fetch usb_3g_usbc_type failed\n");
        return;
    }

    /* 只☠3G使锟矫碉拷锟角革拷☠☠☠ */
    if(sunxi_hci->usbc_no == usb_3g_usbc_num){
        sunxi_hci->used = 0;
        if(sunxi_hci->usbc_type == usb_3g_usbc_type){
            sunxi_hci->used = 1;
        }
    }
#endif
    return;
}

/*
*******************************************************************************
*                     get_usb_cfg
*
* Description:
*    void
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
static s32 get_usb_cfg(struct sunxi_hci_hcd *sunxi_hci)
{
#ifndef  SUNXI_USB_FPGA

    script_item_value_type_e type = 0;
	script_item_u item_temp;

	/* usbc enable */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_used", &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->used = item_temp.val;
	}else{
		DMSG_INFO("ERR: get %s usbc enable failed\n" ,sunxi_hci->hci_name);
		sunxi_hci->used = 0;
	}

	/* usbc restrict_gpio */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_restrict_gpio", &sunxi_hci->restrict_gpio_set);
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		sunxi_hci->usb_restrict_valid = 1;
	}else{
		DMSG_INFO("ERR: %s(restrict_gpio) is invalid\n", sunxi_hci->hci_name);
		sunxi_hci->usb_restrict_valid = 0;
	}

	/* usbc drv_vbus */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_drv_vbus_gpio", &sunxi_hci->drv_vbus_gpio_set);
	if(type == SCIRPT_ITEM_VALUE_TYPE_PIO){
		sunxi_hci->drv_vbus_gpio_valid = 1;
	}else{
		DMSG_INFO("ERR: %s(drv vbus) is invalid\n", sunxi_hci->hci_name);
		sunxi_hci->drv_vbus_gpio_valid = 0;
	}

	/* host_init_state */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_host_init_state", &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->host_init_state = item_temp.val;
	}else{
		DMSG_INFO("ERR: script_parser_fetch host_init_state failed\n");
		sunxi_hci->host_init_state = 1;
	}


	/* get usb_restrict_flag */
	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_restric_flag", &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->usb_restrict_flag = item_temp.val;
	}else{
		DMSG_INFO("ERR: get usb_restrict_flag failed\n");
		sunxi_hci->usb_restrict_flag = 0;
	}

	type = script_get_item(usbc_name[sunxi_hci->usbc_no], "usb_not_suspend", &item_temp);
	if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
		sunxi_hci->not_suspend = item_temp.val;
	}else{
		DMSG_INFO("ERR: get usb_restrict_flag failed\n");
		sunxi_hci->not_suspend = 0;
	}

	sunxi_usb_3g_config(sunxi_hci);

	/* wifi_used */
	if(sunxi_hci->host_init_state == 0){
	    u32 usb_wifi_used      = 0;
	    u32 usb_wifi_usbc_num  = 0;
	    u32 usb_wifi_usbc_type = 0;

		type = script_get_item("wifi_para", "wifi_used", &item_temp);
		if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
			usb_wifi_used = item_temp.val;
		}else{
			DMSG_PANIC("ERR: script_parser_fetch wifi_used failed\n");
			usb_wifi_used = 0;
		}

	    if(usb_wifi_used){
	        /* wifi_usbc_num */
			type = script_get_item("wifi_para", "wifi_usbc_id", &item_temp);
			if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
				usb_wifi_usbc_num = item_temp.val;
			}else{
				DMSG_PANIC("ERR: script_parser_fetch wifi_usbc_id failed\n");
				usb_wifi_usbc_num = 0;
			}

	        /* wifi_usbc_type */
			type = script_get_item("wifi_para", "wifi_usbc_type", &item_temp);
			if(type == SCIRPT_ITEM_VALUE_TYPE_INT){
				usb_wifi_usbc_type = item_temp.val;
			}else{
				DMSG_PANIC("ERR: script_parser_fetch wifi_usbc_type failed\n");
				usb_wifi_usbc_type = 0;
			}

	        /* 只☠wifi使锟矫碉拷锟角革拷模☠ */
	        if(sunxi_hci->usbc_no == usb_wifi_usbc_num){
	            sunxi_hci->used = 0;
	            if(sunxi_hci->usbc_type == usb_wifi_usbc_type){
	                sunxi_hci->used = 1;
	            }
	        }
	    }
	}
#else
	sunxi_hci->used = 1;
	sunxi_hci->host_init_state = 1;
#endif

	return 0;
}

/*
*******************************************************************************
*                     USBC_Phy_GetCsr
*
* Description:
*    void
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
static __u32 USBC_Phy_GetCsr(__u32 usbc_no)
{
	__u32 val = 0x0;

	switch(usbc_no){
		case 0:
			val = (u32)SUNXI_USB_OTG_VBASE + 0x404;
		break;

		case 1:
			val = (u32)SUNXI_USB_OTG_VBASE + 0x404;
		break;

		case 2:
			val = (u32)SUNXI_USB_OTG_VBASE + 0x404;
		break;

		default:
		break;
	}

	return val;
}

/*
*******************************************************************************
*                     USBC_Phy_TpRead
*
* Description:
*    void
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
static __u32 USBC_Phy_TpRead(__u32 usbc_no, __u32 addr, __u32 len)
{
	__u32 temp = 0, ret = 0;
	__u32 i=0;
	__u32 j=0;

	for(j = len; j > 0; j--)
	{
		/* set  the bit address to be read */
		temp = USBC_Readl(USBC_Phy_GetCsr(usbc_no));
		temp &= ~(0xff << 8);
		temp |= ((addr + j -1) << 8);
		USBC_Writel(temp, USBC_Phy_GetCsr(usbc_no));

		for(i = 0; i < 0x4; i++);

		temp = USBC_Readl(USBC_Phy_GetCsr(usbc_no));
		ret <<= 1;
		ret |= ((temp >> (16 + usbc_no)) & 0x1);
	}

	return ret;
}
#endif

/*
*******************************************************************************
*                     USBC_Phy_TpWrite
*
* Description:
*    void
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
static __u32 USBC_Phy_TpWrite(__u32 usbc_no, __u32 addr, __u32 data, __u32 len)
{
	__u32 temp = 0, dtmp = 0;
//	__u32 i=0;
	__u32 j=0;

	dtmp = data;
	for(j = 0; j < len; j++)
	{
		/* set  the bit address to be write */
		temp = USBC_Readl(USBC_Phy_GetCsr(usbc_no));
		temp &= ~(0xff << 8);
		temp |= ((addr + j) << 8);
		USBC_Writel(temp, USBC_Phy_GetCsr(usbc_no));

		temp = USBC_Readb(USBC_Phy_GetCsr(usbc_no));
		temp &= ~(0x1 << 7);
		temp |= (dtmp & 0x1) << 7;
		temp &= ~(0x1 << (usbc_no << 1));
		USBC_Writeb(temp, USBC_Phy_GetCsr(usbc_no));

		temp = USBC_Readb(USBC_Phy_GetCsr(usbc_no));
		temp |= (0x1 << (usbc_no << 1));
		USBC_Writeb( temp, USBC_Phy_GetCsr(usbc_no));

		temp = USBC_Readb(USBC_Phy_GetCsr(usbc_no));
		temp &= ~(0x1 << (usbc_no <<1 ));
		USBC_Writeb(temp, USBC_Phy_GetCsr(usbc_no));
		dtmp >>= 1;
	}

	return data;
}

/*
*******************************************************************************
*                     USBC_Phy_Read
*
* Description:
*    void
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
static __u32 USBC_Phy_Read(__u32 usbc_no, __u32 addr, __u32 len)
{
	return USBC_Phy_TpRead(usbc_no, addr, len);
}
#endif

/*
*******************************************************************************
*                     USBC_Phy_Write
*
* Description:
*    void
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
static __u32 USBC_Phy_Write(__u32 usbc_no, __u32 addr, __u32 data, __u32 len)
{
	return USBC_Phy_TpWrite(usbc_no, addr, data, len);
}


/*
*******************************************************************************
*                     UsbPhyInit
*
* Description:
*    void
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
static void UsbPhyInit(__u32 usbc_no)
{
//	DMSG_INFO("csr1: usbc%d: 0x%x\n", usbc_no, (u32)USBC_Readl(USBC_Phy_GetCsr(usbc_no)));

    /* ☠☠45欧锟借抗 */
	if(usbc_no == 0){
	    USBC_Phy_Write(usbc_no, 0x0c, 0x01, 1);
	}

//	DMSG_INFO("csr2-0: usbc%d: 0x%x\n", usbc_no, (u32)USBC_Phy_Read(usbc_no, 0x0c, 1));

    /* ☠☠ USB0 PHY 锟侥凤拷锟饺猴拷☠☠ */
	USBC_Phy_Write(usbc_no, 0x20, 0x14, 5);

//	DMSG_INFO("csr2-1: usbc%d: 0x%x\n", usbc_no, (u32)USBC_Phy_Read(usbc_no, 0x20, 5));

    /* ☠☠ disconnect ☠值 */
	USBC_Phy_Write(usbc_no, 0x2a, 3, 2);    /*by wangjx*/

//	DMSG_INFO("csr2: usbc%d: 0x%x\n", usbc_no, (u32)USBC_Phy_Read(usbc_no, 0x2a, 2));
//	DMSG_INFO("csr3: usbc%d: 0x%x\n", usbc_no, (u32)USBC_Readl(USBC_Phy_GetCsr(usbc_no)));

	return;
}

/*
*******************************************************************************
*                     clock_init
*
* Description:
*    void
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
static s32 clock_init(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
    if(ohci){  /* ohci */
    	sunxi_hci->ahb = clk_get(NULL, usbc_ahb_ohci_name[sunxi_hci->usbc_no]);
    	if (IS_ERR(sunxi_hci->ahb)){
    		DMSG_PANIC("ERR: get ohci%d abh clk failed.\n", (sunxi_hci->usbc_no - 1));
    		goto failed;
    	}
	}else{  /* ehci */
    	sunxi_hci->ahb = clk_get(NULL, usbc_ahb_ehci_name[sunxi_hci->usbc_no]);
    	if (IS_ERR(sunxi_hci->ahb)){
    		DMSG_PANIC("ERR: get ehci%d abh clk failed.\n", (sunxi_hci->usbc_no - 1));
    		goto failed;
    	}
	}

	sunxi_hci->mod_usbphy = clk_get(NULL, usbc_phy_name[sunxi_hci->usbc_no]);
	if (IS_ERR(sunxi_hci->mod_usbphy)){
		DMSG_PANIC("ERR: get usb%d mod_usbphy failed.\n", sunxi_hci->usbc_no);
		goto failed;
	}

	return 0;

failed:
	if(IS_ERR(sunxi_hci->ahb)){
		//clk_put(sunxi_hci->ahb);
		sunxi_hci->ahb = NULL;
	}

	if(IS_ERR(sunxi_hci->mod_usbphy)){
		//clk_put(sunxi_hci->mod_usbphy);
		sunxi_hci->mod_usbphy = NULL;
	}

	return -1;
}

/*
*******************************************************************************
*                     clock_exit
*
* Description:
*    void
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
static s32 clock_exit(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
	if(!IS_ERR(sunxi_hci->ahb)){
		clk_put(sunxi_hci->ahb);
		sunxi_hci->ahb = NULL;
	}

	if(!IS_ERR(sunxi_hci->mod_usbphy)){
		clk_put(sunxi_hci->mod_usbphy);
		sunxi_hci->mod_usbphy = NULL;
	}

	return 0;
}
#ifndef  SUNXI_USB_FPGA
/*
*******************************************************************************
*                     open_clock
*
* Description:
*    void
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
static int open_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
 	DMSG_INFO("[%s]: open clock\n", sunxi_hci->hci_name);

    if(sunxi_hci->ahb && sunxi_hci->mod_usbphy && !sunxi_hci->clk_is_open){
    sunxi_hci->clk_is_open = 1;
	if(clk_prepare_enable(sunxi_hci->ahb)){
		DMSG_PANIC("ERR:try to prepare_enable %s_ahb failed!\n", sunxi_hci->hci_name);
	}
	mdelay(10);
	
#ifdef  CONFIG_USB_SUNXI_HSIC
		u32 reg_value = 0;
		u32 i = 0;
		u32 ccmu_base = (u32)SUNXI_CCM_VBASE;
		reg_value = USBC_Readl(ccmu_base + 0xcc);
		reg_value |= (0x01<<10);
		reg_value |= (0x01<<11);
		USBC_Writel(reg_value, (ccmu_base + 0xcc));

		for(i=0; i < 0x100; i++);

		reg_value |= (0x01 << 2);
		USBC_Writel(reg_value, (ccmu_base + 0xcc));
#else
		if(clk_prepare_enable(sunxi_hci->mod_usbphy)){
			DMSG_PANIC("ERR:try to prepare_enable %s_usbphy failed!\n", sunxi_hci->hci_name);
		}
		mdelay(10);
#endif

	UsbPhyInit(sunxi_hci->usbc_no);
	}else{
		DMSG_PANIC("[%s]: wrn: open clock failed, (0x%p, 0x%p, %d, 0x%p)\n",
		      sunxi_hci->hci_name,
		      sunxi_hci->ahb, sunxi_hci->mod_usbphy, sunxi_hci->clk_is_open,
		      sunxi_hci->mod_usb);
	}
	return 0;
}

/*
*******************************************************************************
*                     close_clock
*
* Description:
*    void
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
static int close_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
 	DMSG_INFO("[%s]: close clock\n", sunxi_hci->hci_name);

    if(sunxi_hci->ahb && sunxi_hci->mod_usbphy && sunxi_hci->clk_is_open){
    	sunxi_hci->clk_is_open = 0;

#ifdef  CONFIG_USB_SUNXI_HSIC
		u32 reg_value = 0;
		u32 i = 0;
		u32 ccmu_base = (u32)SUNXI_CCM_VBASE;
		reg_value = USBC_Readl(ccmu_base + 0xcc);
		reg_value &= ~ (0x01<<10);
		reg_value &= ~ (0x01<<11);
		USBC_Writel(reg_value, (ccmu_base + 0xcc));

		for(i=0; i < 0x100; i++);

		reg_value &= ~ (0x01 << 2);
		USBC_Writel(reg_value, (ccmu_base + 0xcc));
#else
		clk_disable_unprepare(sunxi_hci->mod_usbphy);
#endif
		clk_disable_unprepare(sunxi_hci->ahb);
		mdelay(10);

    }else{
		DMSG_PANIC("[%s]: wrn: open clock failed, (0x%p, 0x%p, %d, 0x%p)\n",
			      sunxi_hci->hci_name,sunxi_hci->ahb,
			      sunxi_hci->mod_usbphy, sunxi_hci->clk_is_open,
			      sunxi_hci->mod_usb);
	}

	return 0;
}
#else

/*
*******************************************************************************
*                     open_clock
*
* Description:
*    void
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
static int open_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
	u32 reg_value = 0;
	u32 ccmu_base = (u32)SUNXI_CCM_VBASE;
	
 	DMSG_INFO("[%s]: open clock\n", sunxi_hci->hci_name);

	//Gating AHB clock for USB_phy1
	reg_value = USBC_Readl(ccmu_base + 0x60);
	reg_value |= (1 << 26);	            /* AHB clock gate usb0 */
	reg_value |= (1 << 29);	            /* AHB clock gate usb0 */
	USBC_Writel(reg_value, (ccmu_base + 0x60));
	
	reg_value = USBC_Readl(ccmu_base + 0x2c0);
	reg_value |= (1 << 26);	            /* AHB clock gate usb0 */
	reg_value |= (1 << 29);	            /* AHB clock gate usb0 */
	USBC_Writel(reg_value, (ccmu_base + 0x2c0));

	//delay to wati SIE stable
	reg_value = 10000;
	while(reg_value--);

	//Enable module clock for USB phy1
	reg_value = USBC_Readl(ccmu_base + 0xcc);
	reg_value |= (1 << 16);
	reg_value |= (1 << 9);
	reg_value |= (1 << 8);
	reg_value |= (1 << 1);
	reg_value |= (1 << 0);          //disable reset
	USBC_Writel(reg_value, (ccmu_base + 0xcc));

	//delay some time
	reg_value = 10000;
	while(reg_value--);
	
	UsbPhyInit(sunxi_hci->usbc_no);
	
	printk("open_clock 0x60(0x%x), 0xcc(0x%x),0x2c0(0x%x)\n",
				  (u32)USBC_Readl(ccmu_base + 0x60),
				  (u32)USBC_Readl(ccmu_base + 0xcc),
				  (u32)USBC_Readl(ccmu_base + 0x2c0));

	return 0;
}

/*
*******************************************************************************
*                     close_clock
*
* Description:
*    void
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
static int close_clock(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
	u32 reg_value = 0;
	u32 ccmu_base = (u32)SUNXI_CCM_VBASE;

 	DMSG_INFO("[%s]: close clock\n", sunxi_hci->hci_name);
		//Gating AHB clock for USB_phy1
	reg_value = USBC_Readl(ccmu_base + 0x60);
	reg_value &= (1 << 26);	            /* AHB clock gate usb0 */
	reg_value &= (1 << 29);	            /* AHB clock gate usb0 */
	USBC_Writel(reg_value, (ccmu_base + 0x60));

	reg_value = USBC_Readl(ccmu_base + 0x2c0);
	reg_value &= (1 << 26);	            /* AHB clock gate usb0 */
	reg_value &= (1 << 29);	            /* AHB clock gate usb0 */
	USBC_Writel(reg_value, (ccmu_base + 0x2c0));

	//delay to wati SIE stable
	reg_value = 10000;
	while(reg_value--);

	//Enable module clock for USB phy1
	reg_value = USBC_Readl(ccmu_base + 0xcc);
	reg_value &= (1 << 16);
	reg_value &= (1 << 9);
	reg_value &= (1 << 8);
	reg_value &= (1 << 1);
	reg_value &= (1 << 0);          //disable reset
	USBC_Writel(reg_value, (ccmu_base + 0xcc));
	return 0;
}
#endif

/*
*******************************************************************************
*                     enable_usb_passby
*
* Description:
*    void
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
static void usb_passby(struct sunxi_hci_hcd *sunxi_hci, u32 enable)
{
	unsigned long reg_value = 0;
	spinlock_t lock;
	unsigned long flags = 0;

	spin_lock_init(&lock);
	spin_lock_irqsave(&lock, flags);

	/*enable passby*/
	if(sunxi_hci->usbc_no == 1){
		reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE);
		if(enable && usb1_enable_passly_cnt == 0){
	    	reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
	    	reg_value |= (1 << 9);     	/* AHB Master interface burst type INCR4 enable */
	    	reg_value |= (1 << 8);     	/* AHB Master interface INCRX align enable */
#ifndef  SUNXI_USB_FPGA
			reg_value |= (1 << 0);		/* ULPI bypass enable */
#else
			reg_value &= ~(1 << 0); 	/* ULPI bypass enable	   */
#endif		
		}else if(!enable && usb1_enable_passly_cnt == 1){
	    	reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
	    	reg_value &= ~(1 << 9);     /* AHB Master interface burst type INCR4 disable */
	    	reg_value &= ~(1 << 8);     /* AHB Master interface INCRX align disable */
	    	reg_value &= ~(1 << 0);     /* ULPI bypass disable */
		}
        USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

        if(enable){
            usb1_enable_passly_cnt++;
        }else{
            usb1_enable_passly_cnt--;
        }
	}else if(sunxi_hci->usbc_no == 2){
		reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE);
		if(enable && usb2_enable_passly_cnt == 0){
	    	reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
	    	reg_value |= (1 << 9);     	/* AHB Master interface burst type INCR4 enable */
	    	reg_value |= (1 << 8);     	/* AHB Master interface INCRX align enable */
	    	reg_value |= (1 << 0);     	/* ULPI by
	    	pass enable */
		}else if(!enable && usb2_enable_passly_cnt == 1){
	    	reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
	    	reg_value &= ~(1 << 9);     /* AHB Master interface burst type INCR4 disable */
	    	reg_value &= ~(1 << 8);     /* AHB Master interface INCRX align disable */
	    	reg_value &= ~(1 << 0);     /* ULPI bypass disable */
		}
	    USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

        if(enable){
            usb2_enable_passly_cnt++;
        }else{
            usb2_enable_passly_cnt--;
        }
    }else if(sunxi_hci->usbc_no == 3){
		reg_value = USBC_Readl(sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE);
		if(enable && usb3_enable_passly_cnt == 0){
	    	reg_value |= (1 << 10);		/* AHB Master interface INCR8 enable */
	    	reg_value |= (1 << 9);     	/* AHB Master interface burst type INCR4 enable */
	    	reg_value |= (1 << 8);     	/* AHB Master interface INCRX align enable */
	    	reg_value |= (1 << 0);     	/* ULPI bypass enable */
		}else if(!enable && usb3_enable_passly_cnt == 1){
	    	reg_value &= ~(1 << 10);	/* AHB Master interface INCR8 disable */
	    	reg_value &= ~(1 << 9);     /* AHB Master interface burst type INCR4 disable */
	    	reg_value &= ~(1 << 8);     /* AHB Master interface INCRX align disable */
	    	reg_value &= ~(1 << 0);     /* ULPI bypass disable */
		}
	    USBC_Writel(reg_value, (sunxi_hci->usb_vbase + SUNXI_USB_PMU_IRQ_ENABLE));

        if(enable){
            usb3_enable_passly_cnt++;
        }else{
            usb3_enable_passly_cnt--;
        }
    }else{
		DMSG_PANIC("EER: unkown usbc_no(%d)\n", sunxi_hci->usbc_no);
		return;
	}

	spin_unlock_irqrestore(&lock, flags);

    return;
}

/*
*******************************************************************************
*                     hci_port_configure
*
* Description:
*    void
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
static void hci_port_configure(struct sunxi_hci_hcd *sunxi_hci, u32 enable)
{
	return;
}

#ifndef  SUNXI_USB_FPGA

/*
*******************************************************************************
*                     pin_init
*
* Description:
*    void
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
static int alloc_pin(struct sunxi_hci_hcd *sunxi_ehci)
{
    u32 ret = 1;

    if(sunxi_ehci->drv_vbus_gpio_valid){
    	ret = gpio_request(sunxi_ehci->drv_vbus_gpio_set.gpio.gpio, NULL);
    	if(ret != 0){
    		DMSG_PANIC("ERR: gpio_request failed\n");
    		sunxi_ehci->drv_vbus_gpio_valid = 0;
    	}else{
			gpio_direction_output(sunxi_ehci->drv_vbus_gpio_set.gpio.gpio, 0);			
    	}
	}

    if(sunxi_ehci->usb_restrict_valid){
		ret = gpio_request(sunxi_ehci->restrict_gpio_set.gpio.gpio, NULL);
		if(ret != 0){
			DMSG_PANIC("ERR: gpio_request failed\n");
			sunxi_ehci->usb_restrict_valid = 0;
		}else{
	        gpio_direction_output(sunxi_ehci->restrict_gpio_set.gpio.gpio, 0);		
		}
	}
	if(sunxi_ehci->usb_restrict_valid){
		if(sunxi_ehci->usb_restrict_flag){
			 __gpio_set_value(sunxi_ehci->restrict_gpio_set.gpio.gpio, 0);
		}else{
			 __gpio_set_value(sunxi_ehci->restrict_gpio_set.gpio.gpio, 1);
		}
	}

	return 0;
}

/*
*******************************************************************************
*                     pin_exit
*
* Description:
*    void
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
static void free_pin(struct sunxi_hci_hcd *sunxi_ehci)
{
    if(sunxi_ehci->drv_vbus_gpio_valid){
        gpio_free(sunxi_ehci->drv_vbus_gpio_set.gpio.gpio);
        sunxi_ehci->drv_vbus_gpio_valid = 0;
    }

	if(sunxi_ehci->usb_restrict_valid){
        gpio_free(sunxi_ehci->restrict_gpio_set.gpio.gpio);
        sunxi_ehci->drv_vbus_gpio_valid = 0;
    }

	return;
}

/*
*******************************************************************************
*                     __sunxi_set_vbus
*
* Description:
*    void
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
static void __sunxi_set_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{
    u32 on_off = 0;

	DMSG_INFO("[%s]: Set USB Power %s\n", sunxi_hci->hci_name, (is_on ? "ON" : "OFF"));

    /* set power flag */
	sunxi_hci->power_flag = is_on;

    /* set power */
    if(sunxi_hci->drv_vbus_gpio_set.gpio.data == 0){
        on_off = is_on ? 1 : 0;
    }else{
        on_off = is_on ? 0 : 1;
    }
	if(sunxi_hci->drv_vbus_gpio_valid){
   		__gpio_set_value(sunxi_hci->drv_vbus_gpio_set.gpio.gpio, on_off);
	}

	return;
}

#else

/*
*******************************************************************************
*                     pin_init
*
* Description:
*    void
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
static int alloc_pin(struct sunxi_hci_hcd *sunxi_ehci)
{
	return 0;
}

/*
*******************************************************************************
*                     pin_exit
*
* Description:
*    void
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
static void free_pin(struct sunxi_hci_hcd *sunxi_ehci)
{
	return;
}

static void __sunxi_set_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{
	return;
}

#endif
/*
*******************************************************************************
*                     sunxi_set_vbus
*
* Description:
*    void
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
static void sunxi_set_vbus(struct sunxi_hci_hcd *sunxi_hci, int is_on)
{
    DMSG_DEBUG("[%s]: sunxi_set_vbus cnt %d\n",
              sunxi_hci->hci_name,
              (sunxi_hci->usbc_no == 1) ? usb1_set_vbus_cnt : usb2_set_vbus_cnt);

    if(sunxi_hci->usbc_no == 1){
        if(is_on && usb1_set_vbus_cnt == 0){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power on */
        }else if(!is_on && usb1_set_vbus_cnt == 1){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power off */
        }

        if(is_on){
            usb1_set_vbus_cnt++;
        }else{
            usb1_set_vbus_cnt--;
        }
    }else if(sunxi_hci->usbc_no == 2){
        if(is_on && usb2_set_vbus_cnt == 0){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power on */
        }else if(!is_on && usb2_set_vbus_cnt == 1){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power off */
        }

        if(is_on){
            usb2_set_vbus_cnt++;
        }else{
            usb2_set_vbus_cnt--;
        }
    }else{
        if(is_on && usb3_set_vbus_cnt == 0){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power on */
        }else if(!is_on && usb3_set_vbus_cnt == 1){
            __sunxi_set_vbus(sunxi_hci, is_on);  /* power off */
        }

        if(is_on){
            usb3_set_vbus_cnt++;
        }else{
            usb3_set_vbus_cnt--;
        }
    }

	return;
}

//---------------------------------------------------------------
//  EHCI
//---------------------------------------------------------------

#define  SUNXI_EHCI_NAME		"sunxi-ehci"
static const char ehci_name[] = SUNXI_EHCI_NAME;

static struct sunxi_hci_hcd sunxi_ehci0;
static struct sunxi_hci_hcd sunxi_ehci1;

static u64 sunxi_ehci_dmamask = DMA_BIT_MASK(32);

static struct platform_device sunxi_usb_ehci_device[] = {
	[0] = {
		.name		= ehci_name,
		.id			= 1,
		.dev 		= {
			.dma_mask			= &sunxi_ehci_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &sunxi_ehci0,
		},
	},

	[1] = {
		.name		= ehci_name,
		.id			= 2,
		.dev 		= {
			.dma_mask			= &sunxi_ehci_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &sunxi_ehci1,
		},
	},
};

//---------------------------------------------------------------
//  OHCI
//---------------------------------------------------------------
#define  SUNXI_OHCI_NAME		"sunxi-ohci"
static const char ohci_name[] = SUNXI_OHCI_NAME;

static struct sunxi_hci_hcd sunxi_ohci0;
static struct sunxi_hci_hcd sunxi_ohci1;
static struct sunxi_hci_hcd sunxi_ohci2;


static u64 sunxi_ohci_dmamask = DMA_BIT_MASK(32);

static struct platform_device sunxi_usb_ohci_device[] = {
	[0] = {
		.name		= ohci_name,
		.id			= 1,
		.dev 		= {
			.dma_mask			= &sunxi_ohci_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &sunxi_ohci0,
		},
	},

	[1] = {
		.name		= ohci_name,
		.id			= 2,
		.dev 		= {
			.dma_mask			= &sunxi_ohci_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &sunxi_ohci1,
		},
	},

	[2] = {
		.name		= ohci_name,
		.id			= 3,
		.dev 		= {
			.dma_mask			= &sunxi_ohci_dmamask,
			.coherent_dma_mask	= DMA_BIT_MASK(32),
			.platform_data		= &sunxi_ohci2,
		},
	},
};

/*
*******************************************************************************
*                     print_sunxi_hci
*
* Description:
*    void
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
static void print_sunxi_hci(struct sunxi_hci_hcd *sunxi_hci)
{
	DMSG_DEBUG("\n------%s config------\n", sunxi_hci->hci_name);
	DMSG_DEBUG("hci_name             = %s\n", sunxi_hci->hci_name);
	DMSG_DEBUG("irq_no               = %d\n", sunxi_hci->irq_no);
	DMSG_DEBUG("usbc_no              = %d\n", sunxi_hci->usbc_no);

	DMSG_DEBUG("usb_vbase            = 0x%p\n", sunxi_hci->usb_vbase);
	DMSG_DEBUG("sram_vbase           = 0x%p\n", sunxi_hci->sram_vbase);
	DMSG_DEBUG("clock_vbase          = 0x%p\n", sunxi_hci->clock_vbase);
	DMSG_DEBUG("sdram_vbase          = 0x%p\n", sunxi_hci->sdram_vbase);

	DMSG_DEBUG("used                 = %d\n", sunxi_hci->used);
	DMSG_DEBUG("host_init_state      = %d\n", sunxi_hci->host_init_state);

	DMSG_DEBUG("gpio_name            = %s\n", sunxi_hci->drv_vbus_gpio_set.gpio_name);
	DMSG_DEBUG("port                 = %d\n", sunxi_hci->drv_vbus_gpio_set.port);
	DMSG_DEBUG("port_num             = %d\n", sunxi_hci->drv_vbus_gpio_set.port_num);
	DMSG_DEBUG("mul_sel              = %d\n", sunxi_hci->drv_vbus_gpio_set.mul_sel);
	DMSG_DEBUG("pull                 = %d\n", sunxi_hci->drv_vbus_gpio_set.pull);
	DMSG_DEBUG("drv_level            = %d\n", sunxi_hci->drv_vbus_gpio_set.drv_level);
	DMSG_DEBUG("data                 = %d\n", sunxi_hci->drv_vbus_gpio_set.data);

	DMSG_DEBUG("\n--------------------------\n");

    return;
}

/*
*******************************************************************************
*                     init_sunxi_hci
*
* Description:
*    void
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
static int init_sunxi_hci(struct sunxi_hci_hcd *sunxi_hci, u32 usbc_no, u32 ohci, const char *hci_name)
{
    s32 ret = 0;
	
    memset(sunxi_hci, 0, sizeof(struct sunxi_hci_hcd));

    sunxi_hci->usbc_no         = usbc_no;
    sunxi_hci->usbc_type = ohci ? SUNXI_USB_OHCI : SUNXI_USB_EHCI;

    if(ohci){
        sunxi_hci->irq_no = ohci_irq_no[sunxi_hci->usbc_no];
    }else{
        sunxi_hci->irq_no = ehci_irq_no[sunxi_hci->usbc_no];
    }

    sprintf(sunxi_hci->hci_name, "%s%d", hci_name, sunxi_hci->usbc_no);

	sunxi_hci->usb_vbase		= (void __iomem	*)usbc_base[sunxi_hci->usbc_no];
	sunxi_hci->sram_vbase		= (void __iomem	*)SUNXI_SRAMCTRL_VBASE;
	sunxi_hci->clock_vbase     = (void __iomem	*)SUNXI_CCM_VBASE;
	sunxi_hci->gpio_vbase		= (void __iomem	*)SUNXI_PIO_VBASE;
	sunxi_hci->sdram_vbase     = (void __iomem	*)SUNXI_SDMMC1_VBASE;

	get_usb_cfg(sunxi_hci);
	sunxi_hci->open_clock          = open_clock;
	sunxi_hci->close_clock         = close_clock;
	sunxi_hci->set_power           = sunxi_set_vbus;
	sunxi_hci->usb_passby          = usb_passby;
    sunxi_hci->port_configure      = hci_port_configure;

#ifdef  SUNXI_USB_FPGA
{
    u32 reg_value = 0;

	reg_value = USBC_Readl(sunxi_hci->sram_vbase+ 0x04);
	reg_value &= ~(0x01);
	USBC_Writel(reg_value, (sunxi_hci->sram_vbase+ 0x04));

}
#endif

#ifdef  CONFIG_USB_SUNXI_HSIC
	u32 reg_value = 0;
	reg_value = USBC_Readl(sunxi_hci->sram_vbase+ SUNXI_USB_PMU_IRQ_ENABLE);
	reg_value |= (1 << 1);
	reg_value |= (1 << 20);
	reg_value |= (1 << 17);
	USBC_Writel(reg_value, (sunxi_hci->sram_vbase+ SUNXI_USB_PMU_IRQ_ENABLE));
#endif



    ret = clock_init(sunxi_hci, ohci);
    if(ret != 0){
        DMSG_PANIC("ERR: clock_init failed\n");
        goto failed1;
    }

    print_sunxi_hci(sunxi_hci);

    return 0;

failed1:

    return -1;
}

/*
*******************************************************************************
*                     exit_sunxi_hci
*
* Description:
*    void
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
static int exit_sunxi_hci(struct sunxi_hci_hcd *sunxi_hci, u32 ohci)
{
    clock_exit(sunxi_hci, ohci);

    return 0;
}

/*
*******************************************************************************
*                     sunxi_hci_init
*
* Description:
*    void
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

static int __init sunxi_hci_init(void)
{
    /* USB1 */
    init_sunxi_hci(&sunxi_ehci0, 1, 0, ehci_name);
    init_sunxi_hci(&sunxi_ohci0, 1, 1, ohci_name);
    alloc_pin(&sunxi_ehci0);

    /* USB2 */
    //init_sunxi_hci(&sunxi_ehci1, 2, 0, ehci_name);
    //init_sunxi_hci(&sunxi_ohci1, 2, 1, ohci_name);
    //alloc_pin(&sunxi_ehci1);

	/* USB3 */
    //init_sunxi_hci(&sunxi_ohci2, 3, 1, ohci_name);
    //alloc_pin(&sunxi_ohci2);


#ifdef  CONFIG_USB_SUNXI_EHCI0
    if(sunxi_ehci0.used){
    	platform_device_register(&sunxi_usb_ehci_device[0]);
    }else{
		DMSG_INFO("ERR: usb%d %s is not enable\n", sunxi_ehci0.usbc_no, sunxi_ehci0.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI0
    if(sunxi_ohci0.used){
  	    platform_device_register(&sunxi_usb_ohci_device[0]);
    }else{
		DMSG_INFO("ERR: usb%d %s is not enable\n", sunxi_ohci0.usbc_no, sunxi_ohci0.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_EHCI1
    if(sunxi_ehci1.used){
     	platform_device_register(&sunxi_usb_ehci_device[1]);
    }else{
		DMSG_INFO("ERR: usb%d %s is not enable\n", sunxi_ehci1.usbc_no, sunxi_ehci1.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI1
    if(sunxi_ohci1.used){
     	platform_device_register(&sunxi_usb_ohci_device[1]);
    }else{
		DMSG_INFO("ERR: usb%d %s is not enable\n", sunxi_ohci1.usbc_no, sunxi_ohci1.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI2
		if(sunxi_ohci2.used){
			platform_device_register(&sunxi_usb_ohci_device[2]);
		}else{
			DMSG_INFO("ERR: usb%d %s is not enable\n", sunxi_ohci2.usbc_no, sunxi_ohci2.hci_name);
		}
#endif

    return 0;
}

/*
*******************************************************************************
*                     sunxi_hci_exit
*
* Description:
*    void
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
static void __exit sunxi_hci_exit(void)
{

#ifdef  CONFIG_USB_SUNXI_EHCI0
    if(sunxi_ehci0.used){
    	platform_device_unregister(&sunxi_usb_ehci_device[0]);
    }else{
		DMSG_PANIC("ERR: usb%d %s is disable\n", sunxi_ehci0.usbc_no, sunxi_ehci0.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI0
    if(sunxi_ohci0.used){
  	    platform_device_unregister(&sunxi_usb_ohci_device[0]);
    }else{
		DMSG_PANIC("ERR: usb%d %s is disable\n", sunxi_ohci0.usbc_no, sunxi_ohci0.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_EHCI1
    if(sunxi_ehci1.used){
     	platform_device_unregister(&sunxi_usb_ehci_device[1]);
    }else{
		DMSG_PANIC("ERR: usb%d %s is disable\n", sunxi_ehci1.usbc_no, sunxi_ehci1.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI1
    if(sunxi_ohci1.used){
     	platform_device_unregister(&sunxi_usb_ohci_device[1]);
    }else{
		DMSG_PANIC("ERR: usb%d %s is disable\n", sunxi_ohci1.usbc_no, sunxi_ohci1.hci_name);
    }
#endif

#ifdef  CONFIG_USB_SUNXI_OHCI2
	if(sunxi_ohci2.used){
		platform_device_unregister(&sunxi_usb_ohci_device[2]);
	}else{
		DMSG_PANIC("ERR: usb%d %s is disable\n", sunxi_ohci2.usbc_no, sunxi_ohci2.hci_name);
	}
#endif

    /* USB1 */
    exit_sunxi_hci(&sunxi_ehci0, 0);
    exit_sunxi_hci(&sunxi_ohci0, 1);
   	free_pin(&sunxi_ehci0);

    /* USB2 */
    //exit_sunxi_hci(&sunxi_ehci1, 0);
    //exit_sunxi_hci(&sunxi_ohci1, 1);
   	//free_pin(&sunxi_ehci1);

    /*USB3 */
    //exit_sunxi_hci(&sunxi_ohci2, 1);
   	//free_pin(&sunxi_ohci2);

    return ;
}

fs_initcall(sunxi_hci_init);
//module_init(sunxi_hci_init);
module_exit(sunxi_hci_exit);

