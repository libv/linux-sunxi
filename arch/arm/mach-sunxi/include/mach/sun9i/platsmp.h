/*
 * arch/arm/mach-sunxi/include/mach/sun9i/platsmp.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sun9i smp ops header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUN9I_PLAT_SMP_H
#define __SUN9I_PLAT_SMP_H

static inline void enable_cpu(int cpu)
{
#ifdef CONFIG_FPGA_V4_PLATFORM
	u32 val;

	early_printk("[%s] need change######\n", __func__);
	/* assert cpu core reset */
	val = readl((void *)(SUNXI_R_CPUCFG_VBASE + C0_RST_CTRL));
	val &= ~(1<<cpu);
	writel(val, (void *)(SUNXI_R_CPUCFG_VBASE + C0_RST_CTRL));
	/* L1RSTDISABLE hold low */
/*	pwr_reg = readl(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_GENCTL);
	pwr_reg &= ~(1<<cpu);
	writel(pwr_reg, SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_GENCTL);
*/
	/* clear power-off gating */
	val = readl((void *)(SUNXI_R_PRCM_VBASE + C0_CPU_PWROFF_GATING));
	val &= ~(1<<cpu);
	writel(val, (void *)(SUNXI_R_PRCM_VBASE + C0_CPU_PWROFF_GATING));
	mdelay(1);

	/* de-assert core reset */
	val = readl((void *)(SUNXI_R_CPUCFG_VBASE + C0_RST_CTRL));
	val |= (1<<cpu);
	writel(val, (void *)(SUNXI_R_CPUCFG_VBASE + C0_RST_CTRL));
#else
	early_printk("[%s] todo######\n", __func__);
#endif
}

/*
 * Initialize cpu_possible map, and enable coherency
 */
static inline void sunxi_smp_prepare_cpus(unsigned int max_cpus)
{
#ifdef CONFIG_FPGA_V4_PLATFORM
	early_printk("[%s] for sun9i fpga\n", __func__);
	writel(virt_to_phys(sunxi_secondary_startup),
			(void *)(SUNXI_R_PRCM_VBASE + PRIVATE_REG0)); /* only for a7 */
#else
	early_printk("[%s] todo######\n", __func__);
#endif
}

#endif /* __SUN9I_PLAT_SMP_H */
