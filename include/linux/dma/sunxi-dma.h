/*
 * include/linux/dma/sunxi-dma.h
 *
 * Copyright (C) 2013-2015 Allwinnertech Co., Ltd
 *
 * Author: Sugar <shuge@allwinnertech.com>
 *
 * Sunxi DMA controller driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

/*
 * The source DRQ type and port corresponding relation
 *
 */
#define DRQSRC_SRAM		0
#define DRQSRC_SDRAM		0
#define DRQSRC_SPDIFRX		2
#define DRQSRC_DAUDIO_0_RX	3

#ifndef CONFIG_ARCH_SUN9I
#define DRQSRC_DAUDIO_1_RX	4
#define DRQSRC_NAND0		5
#endif

#define DRQSRC_UART0RX		6
#define DRQSRC_UART1RX 		7
#define DRQSRC_UART2RX		8
#define DRQSRC_UART3RX		9
#define DRQSRC_UART4RX		10
/* #define DRQSRC_RESEVER		11 */
/* #define DRQSRC_RESEVER		12 */

#ifndef CONFIG_ARCH_SUN9I
#define DRQSRC_HDMI_DDC		13
#define DRQSRC_HDMI_AUDIO	14
#define DRQSRC_AUDIO_CODEC	15
#define DRQSRC_SS		16
#define DRQSRC_OTG_EP1		17
#define DRQSRC_OTG_EP2		18
#define DRQSRC_OTG_EP3		19
#define DRQSRC_OTG_EP4		20
#define DRQSRC_OTG_EP5		21
#else
#define DRQSRC_AC97		18
#endif

#define DRQSRC_UART5RX		22
#define DRQSRC_SPI0RX		23
#define DRQSRC_SPI1RX		24
#define DRQSRC_SPI2RX		25
#define DRQSRC_SPI3RX		26

#ifndef CONFIG_ARCH_SUN9I
#define DRQSRC_TP		27
#define DRQSRC_NAND1		28
#define DRQSRC_MTC_ACC		29
#define DRQSRC_DIGITAL_MIC	30
#endif

/*
 * The destination DRQ type and port corresponding relation
 *
 */
#define DRQDST_SRAM		0
#define DRQDST_SDRAM		0
#define DRQDST_SPDIFRX		2
#define DRQDST_DAUDIO_0_TX	3
#define DRQDST_DAUDIO_1_TX	4

#ifndef CONFIG_ARCH_SUN9I
#define DRQDST_NAND0		5
#endif

#define DRQDST_UART0TX		6
#define DRQDST_UART1TX 		7
#define DRQDST_UART2TX		8
#define DRQDST_UART3TX		9
#define DRQDST_UART4TX		10

#ifdef CONFIG_ARCH_SUN8IW3
#define DRQSRC_TCON0		12
#endif

#ifndef CONFIG_ARCH_SUN9I
#define DRQDST_HDMI_DDC		13
#define DRQDST_HDMI_AUDIO	14
#define DRQDST_AUDIO_CODEC	15
#else
#define DRQDST_CIR_TX		15
#endif

#ifndef CONFIG_ARCH_SUN9I
#define DRQDST_SS		16
#define DRQDST_OTG_EP1		17
#define DRQDST_OTG_EP2		18
#define DRQDST_OTG_EP3		19
#define DRQDST_OTG_EP4		20
#define DRQDST_OTG_EP5		21
#else
#define DRQDST_AC97		18
#endif

#define DRQDST_UART5TX		22
#define DRQDST_SPI0TX		23
#define DRQDST_SPI1TX		24
#define DRQDST_SPI2TX		25
#define DRQDST_SPI3TX		26

#ifndef CONFIG_ARCH_SUN9I
/* #define DRQDST_RESEVER		27 */
#define DRQDST_NAND1		28
#define DRQDST_MTC_ACC		29
#define DRQDST_DIGITAL_MIC	30
#endif

#define sunxi_slave_id(d, s) (((d)<<16) | (s))
#define GET_SRC_DRQ(x)	((x) & 0x000000ff)
#define GET_DST_DRQ(x)	((x) & 0x00ff0000)

