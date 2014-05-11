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
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/errno.h>
#include <linux/cpu_pm.h>

#include <asm/smp.h>
#include <asm/io.h>
#include <asm/smp_scu.h>
#include <asm/hardware/gic.h>

#include <mach/platform.h>
#ifdef CONFIG_HOTPLUG
#include "sunxi-hotplug.h"
#endif

#ifdef CONFIG_SMP

#define get_nr_cores()					\
	({						\
		unsigned int __val;			\
		asm("mrc	p15, 1, %0, c9, c0, 2"	\
		    : "=r" (__val)			\
		    :					\
		    : "cc");				\
		((__val>>24) & 0x03) + 1;		\
	})

extern void sunxi_secondary_startup(void);
static DEFINE_SPINLOCK(boot_lock);
#ifdef CONFIG_ARCH_SUN8I
#include <mach/sun8i/platsmp.h>
#elif defined CONFIG_ARCH_SUN9I
#include <mach/sun9i/platsmp.h>
#endif

/*
 * Setup the set of possible CPUs (via set_cpu_possible)
 */
void __init smp_init_cpus(void)
{
	unsigned int i, ncores;

	ncores = get_nr_cores();
	pr_debug("[%s] ncores=%d\n", __func__, ncores);

	/*
	 * sanity check, the cr_cpu_ids is configured form CONFIG_NR_CPUS
	 */
	if (ncores > nr_cpu_ids) {
	        pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
				ncores, nr_cpu_ids);
	        ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++) {
	    set_cpu_possible(i, true);
	}

	set_smp_cross_call(gic_raise_softirq);
}
void __init platform_smp_prepare_cpus(unsigned int max_cpus)
{
    return sunxi_smp_prepare_cpus(max_cpus);
}
/*
 * Perform platform specific initialisation of the specified CPU.
 */
void __cpuinit platform_secondary_init(unsigned int cpu)
{
	/*
	 * if any interrupts are already enabled for the primary
	 * core (e.g. timer irq), then they will not have been enabled
	 * for us: do so
	 */
	gic_secondary_init(0);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

/*
 * Boot a secondary CPU, and assign it the specified idle task.
 * This also gives us the initial stack to use for this CPU.
 */
int __cpuinit boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	pr_debug("[%s] enter\n", __func__);
	spin_lock(&boot_lock);

	enable_cpu(cpu);

	/*
	 * Now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);
	return 0;
}

#endif

static void sunxi_cpu_restore(void)
{
#if  defined (CONFIG_ARCH_SUN9I)
	/* modify for sun9i platform porting,
	 * maybe sunxi should have a small framework to 
	 * deal with this platfrom different.
	 * by sunny at 2013-7-9 16:28:44.
	 */
	writel(virt_to_phys(sunxi_secondary_startup),
			(void *)(SUNXI_R_PRCM_VBASE + PRIVATE_REG0));
	return;
#else
	writel(virt_to_phys(sunxi_secondary_startup),
			(void *)(SUNXI_R_CPUCFG_VBASE + SUNXI_CPUCFG_P_REG0));
	return;
#endif
}

static int sunxi_cpususpend_notifier(struct notifier_block *self, unsigned long cmd,	void *v)
{
	switch (cmd) {
	case CPU_PM_ENTER:
		break;
	case CPU_PM_ENTER_FAILED:
	case CPU_PM_EXIT:
		sunxi_cpu_restore();
		break;
	case CPU_CLUSTER_PM_ENTER:
	case CPU_CLUSTER_PM_ENTER_FAILED:
	case CPU_CLUSTER_PM_EXIT:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block sunxi_cpususpend_notifier_block = {
	.notifier_call = sunxi_cpususpend_notifier,
};

static int __init register_cpususpend_notifier(void)
{
	return cpu_pm_register_notifier(&sunxi_cpususpend_notifier_block);
}
core_initcall(register_cpususpend_notifier);

