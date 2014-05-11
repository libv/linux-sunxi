/*
 * sound\soc\sunxi\spdif\sunxi_spdif.c
 * (C) Copyright 2010-2016
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * chenpailin <chenpailin@Reuuimllatech.com>
 *
 * some simple description for this code
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/io.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>

#include <asm/dma.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>
#include <linux/pinctrl/consumer.h>
#include "sunxi_spdma.h"
#include "sunxi_spdif.h"

static int regsave[9];
static int spdif_used = 0;
struct sunxi_spdif_info sunxi_spdif;
//static struct clk *spdif_apbclk 	= NULL;
static struct clk *spdif_pll2clk	= NULL;
static struct clk *spdif_pllx8		= NULL;
static struct clk *spdif_moduleclk	= NULL;

static struct sunxi_dma_params sunxi_spdif_stereo_out = {
	.name		= "spdif_out",	
	.dma_addr 	=	SUNXI_SPDIFBASE + SUNXI_SPDIF_TXFIFO,	
};

static struct sunxi_dma_params sunxi_spdif_stereo_in = {
	.name		= "spdif_in",
	.dma_addr 	=	SUNXI_SPDIFBASE + SUNXI_SPDIF_RXFIFO,
};

void sunxi_snd_txctrl(struct snd_pcm_substream *substream, int on)
{
	u32 reg_val;
	
	if (substream->runtime->channels == 1) {
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_SINGLEMOD;
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
	}
	 
	/*soft reset SPDIF*/
	writel(0x1, sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	
	/*flush TX FIFO*/
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIF_FCTL_FTX;	
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	
	/*clear interrupt status*/
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_ISTA);
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_ISTA);
	
	/*clear TX counter*/
	writel(0, sunxi_spdif.regs + SUNXI_SPDIF_TXCNT);

	if (on) {
		/*SPDIF TX ENBALE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_TXEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		
		/*DRQ ENABLE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_INT);
		reg_val |= SUNXI_SPDIF_INT_TXDRQEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_INT);
		
		/*global enable*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
		reg_val |= SUNXI_SPDIF_CTL_GEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);		
	} else {
		/*SPDIF TX DISABALE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		reg_val &= ~SUNXI_SPDIF_TXCFG_TXEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		
		/*DRQ DISABLE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_INT);
		reg_val &= ~SUNXI_SPDIF_INT_TXDRQEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_INT);
		
		/*global disable*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
		reg_val &= ~SUNXI_SPDIF_CTL_GEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	}
}

void sunxi_snd_rxctrl(struct snd_pcm_substream *substream, int on)
{
	u32 reg_val;

	/*flush RX FIFO*/
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	reg_val |= SUNXI_SPDIF_FCTL_FRX;	
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	
	/*clear interrupt status*/
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_ISTA);
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_ISTA);
	
	/*clear RX counter*/
	writel(0, sunxi_spdif.regs + SUNXI_SPDIF_RXCNT);

	if (on) {
		/*SPDIF RX ENBALE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
		reg_val |= SUNXI_SPDIF_RXCFG_RXEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
		
		/*DRQ ENABLE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_INT);
		reg_val |= SUNXI_SPDIF_INT_RXDRQEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_INT);
		
		/*global enable*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
		reg_val |= SUNXI_SPDIF_CTL_GEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);		
	} else {
		/*SPDIF TX DISABALE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
		reg_val &= ~SUNXI_SPDIF_RXCFG_RXEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
		
		/*DRQ DISABLE*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_INT);
		reg_val &= ~SUNXI_SPDIF_INT_RXDRQEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_INT);
		
		/*global disable*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
		reg_val &= ~SUNXI_SPDIF_CTL_GEN;	
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	}
}

static inline int sunxi_snd_is_clkmaster(void)
{
	return 0;
}

static int sunxi_spdif_set_fmt(struct snd_soc_dai *cpu_dai, unsigned int fmt)
{
	u32 reg_val;
		
	reg_val = 0;
	reg_val &= ~SUNXI_SPDIF_TXCFG_SINGLEMOD;
	reg_val |= SUNXI_SPDIF_TXCFG_ASS;
	reg_val &= ~SUNXI_SPDIF_TXCFG_NONAUDIO;
	reg_val |= SUNXI_SPDIF_TXCFG_FMT16BIT;
	reg_val |= SUNXI_SPDIF_TXCFG_CHSTMODE;
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
	
	reg_val = 0;
	reg_val &= ~SUNXI_SPDIF_FCTL_FIFOSRC;
	reg_val |= SUNXI_SPDIF_FCTL_TXTL(4);
	reg_val |= SUNXI_SPDIF_FCTL_RXTL(15);
	reg_val |= SUNXI_SPDIF_FCTL_TXIM(1);
	reg_val |= SUNXI_SPDIF_FCTL_RXOM(3);
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	
	if (!fmt) {/*PCM*/
		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA0_CHNUM(2));
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
		
		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA1_SAMWORDLEN(1));
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
	} else {  /*non PCM*/
		reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		reg_val |= SUNXI_SPDIF_TXCFG_NONAUDIO;
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
		
		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA0_CHNUM(2));
		reg_val |= SUNXI_SPDIF_TXCHSTA0_AUDIO;
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
		
		reg_val = 0;
		reg_val |= (SUNXI_SPDIF_TXCHSTA1_SAMWORDLEN(1));
		writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
	}
	
	return 0;
}

static int sunxi_spdif_hw_params(struct snd_pcm_substream *substream,
																struct snd_pcm_hw_params *params,
																struct snd_soc_dai *dai)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct sunxi_dma_params *dma_data;
	
	/* play or record */
	if(substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		dma_data = &sunxi_spdif_stereo_out;
	else
		dma_data = &sunxi_spdif_stereo_in;
		
	snd_soc_dai_set_dma_data(rtd->cpu_dai, substream, dma_data);
	
	return 0;
}						

static int sunxi_spdif_trigger(struct snd_pcm_substream *substream,
                              int cmd, struct snd_soc_dai *dai)
{
	int ret = 0;

	switch (cmd) {
		case SNDRV_PCM_TRIGGER_START:
		case SNDRV_PCM_TRIGGER_RESUME:
		case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				sunxi_snd_rxctrl(substream, 1);
			} else {
				sunxi_snd_txctrl(substream, 1);
				printk("%s, line:%d\n", __func__, __LINE__);
			}
			break;
		case SNDRV_PCM_TRIGGER_STOP:
		case SNDRV_PCM_TRIGGER_SUSPEND:
		case SNDRV_PCM_TRIGGER_PAUSE_PUSH:			
			if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
				sunxi_snd_rxctrl(substream, 0);
			} else {
			  sunxi_snd_txctrl(substream, 0);
			}
			break;
		default:
			ret = -EINVAL;
			break;
	}

		return ret;
}					

/*freq:   1: 22.5792MHz   0: 24.576MHz  */
static int sunxi_spdif_set_sysclk(struct snd_soc_dai *cpu_dai, int clk_id, 
                                 unsigned int freq, int dir)
{
	if (!freq) {
		if (clk_set_rate(spdif_pll2clk, 24576000)) {
			printk("try to set the spdif_pll2clk rate failed!\n");
		}
	} else {
		if (clk_set_rate(spdif_pll2clk, 22579200)) {
			printk("try to set the spdif_pll2clk rate failed!\n");
		}
	}

	return 0;
}		

static int sunxi_spdif_set_clkdiv(struct snd_soc_dai *cpu_dai, int div_id, int div)
{
	u32 reg_val = 0;

	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xf));
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);

	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xf));
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
		
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xf));
  	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
	reg_val &= ~(SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xf));
  	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);

	
	switch(div_id) {
		case SUNXI_DIV_MCLK:
		{
			reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
			reg_val &= ~(SUNXI_SPDIF_TXCFG_TXRATIO(0x1F));	
			reg_val |= SUNXI_SPDIF_TXCFG_TXRATIO(div-1);
			writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
			
			if(clk_get_rate(spdif_pll2clk) == 24576000)
			{
				switch(div)
				{
					/*24KHZ*/
					case 8:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x6));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x9));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x6));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x9));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);

						break;
						
					/*32KHZ*/
					case 6:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x3));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xC));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x3));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xC));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
						
					/*48KHZ*/
					case 4:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x2));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xD));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);	

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x2));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xD));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);							
						break;
						
					/*96KHZ*/
					case 2:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xA));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x5));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xA));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x5));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
						
					/*192KHZ*/
					case 1:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xE));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);	
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xE));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);	
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
						
					default:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
				}
			}else{  /*22.5792MHz*/		
				switch(div)
				{
					/*22.05khz*/
					case 8:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x4));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xb));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);	

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x4));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xb));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);							
						break;
						
					/*44.1KHZ*/
					case 4:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0xF));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);	

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0xF));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);							
						break;
						
					/*88.2khz*/
					case 2:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0x8));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x7));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0x8));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x7));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
			
					/*176.4KHZ*/
					case 1:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(0xC));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0x3));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(0xC));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
					
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0x3));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
						
					default:
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
						reg_val |= (SUNXI_SPDIF_TXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
						reg_val |= (SUNXI_SPDIF_TXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);

						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
						reg_val |= (SUNXI_SPDIF_RXCHSTA0_SAMFREQ(1));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
				
						reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
						reg_val |= (SUNXI_SPDIF_RXCHSTA1_ORISAMFREQ(0));
						writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);						
						break;
				}
			}			
		}
		break;
		case SUNXI_DIV_BCLK:
		break;
			
		default:
			return -EINVAL;
	}

	return 0;
}

u32 sunxi_spdif_get_clockrate(void)
{
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_spdif_get_clockrate);

static int sunxi_spdif_dai_probe(struct snd_soc_dai *dai)
{			
	return 0;
}
static int sunxi_spdif_dai_remove(struct snd_soc_dai *dai)
{
	return 0;
}

static void spdifregsave(void)
{
	regsave[0] = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	regsave[1] = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
	regsave[2] = readl(sunxi_spdif.regs + SUNXI_SPDIF_FCTL) | (0x3<<16);
	regsave[3] = readl(sunxi_spdif.regs + SUNXI_SPDIF_INT);
	regsave[4] = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
	regsave[5] = readl(sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
	regsave[6] = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
	regsave[7] = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
	regsave[8] = readl(sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);	
}

static void spdifregrestore(void)
{
	writel(regsave[0], sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	writel(regsave[1], sunxi_spdif.regs + SUNXI_SPDIF_TXCFG);
	writel(regsave[2], sunxi_spdif.regs + SUNXI_SPDIF_FCTL);
	writel(regsave[3], sunxi_spdif.regs + SUNXI_SPDIF_INT);
	writel(regsave[4], sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA0);
	writel(regsave[5], sunxi_spdif.regs + SUNXI_SPDIF_TXCHSTA1);
	writel(regsave[6], sunxi_spdif.regs + SUNXI_SPDIF_RXCFG);
	writel(regsave[7], sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA0);
	writel(regsave[8], sunxi_spdif.regs + SUNXI_SPDIF_RXCHSTA1);
}

static int sunxi_spdif_suspend(struct snd_soc_dai *cpu_dai)
{
	u32 reg_val;
	printk("[SPDIF]Enter %s\n", __func__);
	
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	reg_val &= ~SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);

	spdifregsave();
	if ((NULL == spdif_moduleclk) ||(IS_ERR(spdif_moduleclk))) {
		printk("spdif_moduleclk handle is invalid, just return\n");
		return -EFAULT;
	} else {
		/*disable the module clock*/
		clk_disable(spdif_moduleclk);
	}
//	if ((NULL == spdif_apbclk) ||(IS_ERR(spdif_apbclk))) {
//		printk("spdif_apbclk handle is invalid, just return\n");
//		return -EFAULT;
//	} else {	
//		clk_disable(spdif_apbclk);
//	}
	return 0;
}

static int sunxi_spdif_resume(struct snd_soc_dai *cpu_dai)
{
	u32 reg_val;
	printk("[SPDIF]Enter %s\n", __func__);

	/*enable the module clock*/
//	if (clk_enable(spdif_apbclk)) {
//		printk("try to enable spdif_apbclk output failed!\n");
//	}

	/*enable the module clock*/
	if (clk_enable(spdif_moduleclk)) {
		printk("try to enable spdif_moduleclk output failed!\n");
	}
	
	spdifregrestore();
	
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	reg_val |= SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	
	return 0;
}

#define SUNXI_SPDIF_RATES (SNDRV_PCM_RATE_8000_192000 | SNDRV_PCM_RATE_KNOT)
static struct snd_soc_dai_ops sunxi_spdif_dai_ops = {
	.trigger 	= sunxi_spdif_trigger,
	.hw_params 	= sunxi_spdif_hw_params,
	.set_fmt 	= sunxi_spdif_set_fmt,
	.set_clkdiv = sunxi_spdif_set_clkdiv,
	.set_sysclk = sunxi_spdif_set_sysclk, 
};
static struct snd_soc_dai_driver sunxi_spdif_dai = {
	.probe 		= sunxi_spdif_dai_probe,
	.suspend 	= sunxi_spdif_suspend,
	.resume 	= sunxi_spdif_resume,
	.remove 	= sunxi_spdif_dai_remove,
	.playback = {
		.channels_min = 1,
		.channels_max = 4,
		.rates = SUNXI_SPDIF_RATES,
	.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.channels_min = 1,
		.channels_max = 4,
		.rates = SUNXI_SPDIF_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = &sunxi_spdif_dai_ops,
};		
static struct pinctrl *spdif_pinctrl;

static int __init sunxi_spdif_dev_probe(struct platform_device *pdev)
{
	int reg_val = 0;
	int ret = 0;

	sunxi_spdif.regs = ioremap(SUNXI_SPDIFBASE, 0x100);
	if (sunxi_spdif.regs == NULL) {
		return -ENXIO;
	}
	printk("%s, line:%d, dev_name(&pdev->dev):%s\n", __func__, __LINE__, dev_name(&pdev->dev));
	spdif_pinctrl = devm_pinctrl_get_select_default(&pdev->dev);
	if (IS_ERR_OR_NULL(spdif_pinctrl)) {
		dev_warn(&pdev->dev,
			"pins are not configured from the driver\n");
	}

//	/*spdif apbclk*/
//	spdif_apbclk = clk_get(NULL, CLK_APB_SPDIF);
//	if ((!spdif_apbclk)||(IS_ERR(spdif_apbclk))) {
//		printk("try to get spdif_apbclk failed\n");
//	}
//	if (clk_enable(spdif_apbclk)) {
//		printk("spdif_apbclk failed! line = %d\n", __LINE__);
//	}

	spdif_pllx8 = clk_get(NULL, "pll2x8");
	if ((!spdif_pllx8)||(IS_ERR(spdif_pllx8))) {
		printk("try to get spdif_pllx8 failed\n");
	}
	if (clk_prepare_enable(spdif_pllx8)) {
		printk("enable spdif_pll2clk failed; \n");
	}

/********spdif pll2clk can be remove****************/
	/*spdif pll2clk*/
	spdif_pll2clk = clk_get(NULL, "pll2");
	if ((!spdif_pll2clk)||(IS_ERR(spdif_pll2clk))) {
		printk("try to get spdif_pll2clk failed\n");
	}
	if (clk_prepare_enable(spdif_pll2clk)) {
		printk("enable spdif_pll2clk failed; \n");
	}
	/*spdif module clk*/
	spdif_moduleclk = clk_get(NULL, "spdif");
	if ((!spdif_moduleclk)||(IS_ERR(spdif_moduleclk))) {
		printk("try to get spdif_moduleclk failed\n");
	}

/*******clk_set_parent can be remove*******/
	if (clk_set_parent(spdif_moduleclk, spdif_pll2clk)) {
		printk("try to set parent of spdif_moduleclk to spdif_pll2ck failed! line = %d\n",__LINE__);
	}
	if (clk_set_rate(spdif_moduleclk, 24576000/8)) {
		printk("set spdif_moduleclk clock freq to 24576000 failed! line = %d\n", __LINE__);
	}
	if (clk_prepare_enable(spdif_moduleclk)) {
		printk("open spdif_moduleclk failed! line = %d\n", __LINE__);
	}
//	if (clk_reset(spdif_moduleclk, AW_CCU_CLK_NRESET)) {
//		printk("try to NRESET spdif module clk failed!\n");
//	}

	/*global enbale*/
	reg_val = readl(sunxi_spdif.regs + SUNXI_SPDIF_CTL);
	reg_val |= SUNXI_SPDIF_CTL_GEN;
	writel(reg_val, sunxi_spdif.regs + SUNXI_SPDIF_CTL);

	ret = snd_soc_register_dai(&pdev->dev, &sunxi_spdif_dai);
	return 0;
}

static int __exit sunxi_spdif_dev_remove(struct platform_device *pdev)
{
	if (spdif_used) {
		spdif_used = 0;
		if ((NULL == spdif_moduleclk) ||(IS_ERR(spdif_moduleclk))) {
			printk("spdif_moduleclk handle is invalid, just return\n");
			return -EFAULT;
		} else {
			/*release the module clock*/
			clk_disable(spdif_moduleclk);
		}
		if ((NULL == spdif_pllx8) ||(IS_ERR(spdif_pllx8))) {
			printk("spdif_pllx8 handle is invalid, just return\n");
			return -EFAULT;
		} else {
			/*release pllx8clk*/
			clk_put(spdif_pllx8);
		}
		if ((NULL == spdif_pll2clk) ||(IS_ERR(spdif_pll2clk))) {
			printk("spdif_pll2clk handle is invalid, just return\n");
			return -EFAULT;
		} else {
			/*release pll2clk*/
			clk_put(spdif_pll2clk);
		}
//		if ((NULL == spdif_apbclk) ||(IS_ERR(spdif_apbclk))) {
//			printk("spdif_apbclk handle is invalid, just return\n");
//			return -EFAULT;
//		} else {
//			/*release apbclk*/
//			clk_put(spdif_apbclk);
//		}
		devm_pinctrl_put(spdif_pinctrl);

		snd_soc_unregister_dai(&pdev->dev);
		platform_set_drvdata(pdev, NULL);
	}
	
	return 0;
}

static struct platform_device sunxi_spdif_device = {
	.name = "spdif0",
	.id = PLATFORM_DEVID_NONE,
};

static struct platform_driver sunxi_spdif_driver = {
	.probe = sunxi_spdif_dev_probe,
	.remove = __exit_p(sunxi_spdif_dev_remove),
	.driver = {
		.name = "spdif0",
		.owner = THIS_MODULE,
	},	
};

static int __init sunxi_spdif_init(void)
{
	int err = 0;
	script_item_u val;
	script_item_value_type_e  type;

	type = script_get_item("spdif0", "spdif_used", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        printk("[SPDIF] type err!\n");
    }

	spdif_used = val.val;
	printk("%s, line:%d, spdif_used:%d\n", __func__, __LINE__, spdif_used);
 	if (spdif_used) {
		if((platform_device_register(&sunxi_spdif_device))<0)
			return err;

		if ((err = platform_driver_register(&sunxi_spdif_driver)) < 0)
			return err;
	} else {
        printk("[SPDIF]sunxi-spdif cannot find any using configuration for controllers, return directly!\n");
        return 0;
    }
 
	return 0;
}
module_init(sunxi_spdif_init);

static void __exit sunxi_spdif_exit(void)
{	
	platform_driver_unregister(&sunxi_spdif_driver);
}
module_exit(sunxi_spdif_exit);

/* Module information */
MODULE_AUTHOR("REUUIMLLA");
MODULE_DESCRIPTION("sunxi SPDIF SoC Interface");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sunxi-spdif");
