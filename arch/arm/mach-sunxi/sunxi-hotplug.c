/*
 * Sunxi platform smp source file.
 * It contains platform specific fucntions needed for the linux smp kernel.
 *
 * Copyright (c) Allwinner.  All rights reserved.
 * Sugar (shuge@allwinnertech.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/device.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <asm/cacheflush.h>
#include <asm/smp_plat.h>
#include <asm/smp.h>
#include <asm/smp_scu.h>
#include <asm/hardware/gic.h>
#include <mach/platform.h>


#if defined(CONFIG_HOTPLUG_CPU) && defined(CONFIG_SMP)

static cpumask_t dead_cpus;

#ifndef CONFIG_FPGA_V4_PLATFORM
#define IS_WFI_MODE(cpu)    (readl(IO_ADDRESS(SUNXI_R_CPUCFG_PBASE) + CPUX_STATUS(cpu)) & (1<<2))
#endif

int platform_cpu_kill(unsigned int cpu)
{
	int k;
	u32 pwr_reg;

	int tmp_cpu = get_cpu();
	put_cpu();
	pr_info("[hotplug]: cpu(%d) try to kill cpu(%d)\n", tmp_cpu, cpu);

	for (k = 0; k < 1000; k++) {
		if (cpumask_test_cpu(cpu, &dead_cpus)
#ifndef CONFIG_FPGA_V4_PLATFORM
				&& IS_WFI_MODE(cpu)
#endif
		   ) {

			writel(0, (void *)(SUNXI_R_CPUCFG_VBASE + CPUX_RESET_CTL(cpu)));

			/* step9: set up power-off signal */
			pwr_reg = readl(IO_ADDRESS(SUNXI_R_PRCM_PBASE) + SUNXI_CPU_PWROFF_REG);
			pwr_reg |= (1<<cpu);
			writel(pwr_reg, IO_ADDRESS(SUNXI_R_PRCM_PBASE) + SUNXI_CPU_PWROFF_REG);
			usleep_range(1000, 1000);

#ifdef CONFIG_ARCH_SUN8IW1
			/* step10: active the power output clamp */
			writel(0xff, IO_ADDRESS(SUNXI_R_PRCM_PBASE) + SUNXI_CPUX_PWR_CLAMP(cpu));
#endif
			pr_info("[hotplug]: cpu%d is killed! .\n", cpu);

			return 1;
		}

		msleep(1);
	}

	pr_err("[hotplug]: try to kill cpu:%d failed!\n", cpu);
	return 0;
}

void platform_cpu_die(unsigned int cpu)
{
	unsigned long actlr;

	gic_cpu_exit(0);

	/* notify platform_cpu_kill() that hardware shutdown is finished */
	cpumask_set_cpu(cpu, &dead_cpus);

	/* step1: disable cache */
	asm("mrc    p15, 0, %0, c1, c0, 0" : "=r" (actlr) );
	actlr &= ~(1<<2);
	asm("mcr    p15, 0, %0, c1, c0, 0\n" : : "r" (actlr));

	/* step2: clean and ivalidate L1 cache */
	flush_cache_all();

	/* step3: execute a CLREX instruction */
	asm("clrex" : : : "memory", "cc");

	/* step4: switch cpu from SMP mode to AMP mode, aim is to disable cache coherency */
	asm("mrc    p15, 0, %0, c1, c0, 1" : "=r" (actlr) );
	actlr &= ~(1<<6);
	asm("mcr    p15, 0, %0, c1, c0, 1\n" : : "r" (actlr));

	/* step5: execute an ISB instruction */
	isb();
	/* step6: execute a DSB instruction  */
	dsb();

	/* step7: execute a WFI instruction */
	while(1) {
		asm("wfi" : : : "memory", "cc");
	}
}

int  platform_cpu_disable(unsigned int cpu)
{
	cpumask_clear_cpu(cpu, &dead_cpus);
	/*
	 * we don't allow CPU 0 to be shutdown (it is still too special
	 * e.g. clock tick interrupts)
	 */
	return cpu == 0 ? -EPERM : 0;
}
#endif

