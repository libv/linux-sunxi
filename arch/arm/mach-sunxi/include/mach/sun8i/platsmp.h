/*
 * arch/arm/mach-sunxi/include/mach/sun8i/platsmp.h
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sun8i smp ops header file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef __SUN8I_PLAT_SMP_H
#define __SUN8I_PLAT_SMP_H

static inline void enable_cpu(int cpu)
{
	u32 pwr_reg;

	/* step1: Assert nCOREPORESET LOW and hold L1RSTDISABLE LOW.
	          Ensure DBGPWRDUP is held LOW to prevent any external
	          debug access to the processor.
	*/
	/* assert cpu core reset */
	writel(0, (void *)(SUNXI_R_CPUCFG_VBASE + CPUX_RESET_CTL(cpu)));
	/* L1RSTDISABLE hold low */
	pwr_reg = readl((void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_GENCTL));
	pwr_reg &= ~(1<<cpu);
	writel(pwr_reg, (void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_GENCTL));

#ifdef CONFIG_ARCH_SUN8IW1
	/* step2: release power clamp */
	//write bit3, bit4 to 0
	writel(0xe7, (void *)(SUNXI_R_PRCM_VBASE + SUNXI_CPUX_PWR_CLAMP(cpu)));
	while((0xe7) != readl((void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUX_PWR_CLAMP_STATUS(cpu))))
	        ;
	//write 012567 bit to 0
	writel(0x00, (void *)(SUNXI_R_PRCM_VBASE + SUNXI_CPUX_PWR_CLAMP(cpu)));
	while((0x00) != readl((void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUX_PWR_CLAMP_STATUS(cpu))))
	        ;
	mdelay(2);
#endif

	/* step3: clear power-off gating */
	pwr_reg = readl((void *)(SUNXI_R_PRCM_VBASE + SUNXI_CPU_PWROFF_REG));
	pwr_reg &= ~(0x00000001<<cpu);
	writel(pwr_reg, (void *)(SUNXI_R_PRCM_VBASE + SUNXI_CPU_PWROFF_REG));
	mdelay(1);

	/* step4: de-assert core reset */
	writel(3, (void *)(SUNXI_R_CPUCFG_VBASE + CPUX_RESET_CTL(cpu)));
}

/*
 * Initialize cpu_possible map, and enable coherency
 */
void sunxi_smp_prepare_cpus(unsigned int max_cpus)
{
	pr_debug("[%s] enter\n", __func__);

	writel(virt_to_phys(sunxi_secondary_startup),
			(void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_P_REG0));
}

#endif /* __SUN8I_PLAT_SMP_H */
