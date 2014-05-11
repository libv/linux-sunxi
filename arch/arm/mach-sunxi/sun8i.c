/*
 * arch/arm/mach-sunxi/sun8i.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sun8i platform file
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/io.h>
#include <linux/sunxi_timer.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/param.h>
#include <linux/memblock.h>
#include <linux/arisc/arisc.h>
#include <linux/dma-mapping.h>

#include <asm/pmu.h>
#include <asm/hardware/gic.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach-types.h>
#include <asm/setup.h>
#include <asm/arch_timer.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#ifdef CONFIG_SMP
extern struct smp_operations sunxi_smp_ops;
#endif

/* plat memory info, maybe from boot, so we need bkup for future use */
unsigned int mem_start = PLAT_PHYS_OFFSET;
unsigned int mem_size = PLAT_MEM_SIZE;

#ifndef CONFIG_OF
#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
static struct plat_serial8250_port serial_platform_data[] = {
	{
		.membase        = (void __iomem *)(SUNXI_UART0_VBASE),
		.mapbase        = (resource_size_t)SUNXI_UART0_PBASE,
		.irq            = SUNXI_IRQ_UART0,
		.flags          = UPF_BOOT_AUTOCONF|UPF_IOREMAP,
		.iotype         = UPIO_MEM32,
		.regshift       = 2,
		.uartclk        = 24000000,
	}, {
		.flags          = 0,
	}
 };

static struct platform_device serial_dev = {
	.name = "serial8250",
	.id = PLAT8250_DEV_PLATFORM,
	.dev = {
		.platform_data = &serial_platform_data[0],
	}
};
#endif

#if defined(CONFIG_CPU_HAS_PMU)
/* cpu performance support */
static struct resource sunxi_pmu_res = {
#if defined (CONFIG_ARCH_SUN8IW1) || defined (CONFIG_ARCH_SUN8IW3)
	.start  = SUNXI_IRQ_PMU0,
	.end    = SUNXI_IRQ_PMU3,
#else
	.start  = SUNXI_IRQ_PMU,
	.end    = SUNXI_IRQ_PMU,
#endif
	.flags  = IORESOURCE_IRQ,
};

static struct platform_device sunxi_pmu_dev = {
	.name   = "arm-pmu",
	.id     = ARM_PMU_DEVICE_CPU,
	.num_resources = 1,
	.resource = &sunxi_pmu_res,
};
#endif

static struct platform_device *sunxi_dev[] __initdata = {
#if defined(CONFIG_SERIAL_8250) || defined(CONFIG_SERIAL_8250_MODULE)
	&serial_dev,
#endif
#if defined(CONFIG_CPU_HAS_PMU)
	&sunxi_pmu_dev,
#endif
};
#endif

#if defined(CONFIG_ION) || defined(CONFIG_ION_MODULE)
struct tag_mem32 ion_mem = {
	.start = ION_CARVEOUT_MEM_BASE,
	.size  = ION_CARVEOUT_MEM_SIZE,
};

/*
 * Pick out the ion memory size.  We look for ion_reserve=size@start,
 * where start and size are "size[KkMm]"
 */
static int __init early_ion_reserve(char *p)
{
	char *endp;

	ion_mem.size  = memparse(p, &endp);
	if (*endp == '@')
		ion_mem.start = memparse(endp + 1, NULL);
	else /* set ion memory to end */
		ion_mem.start = mem_start + mem_size - ion_mem.size;

	pr_debug("[%s]: ION memory reserve: [0x%016x - 0x%016x]\n",
			__func__, ion_mem.start, ion_mem.size);

	return 0;
}
early_param("ion_reserve", early_ion_reserve);
#endif

static void sun8i_restart(char mode, const char *cmd)
{
	writel(0, (void __iomem *)(SUNXI_R_WDOG_VBASE + R_WDOG_IRQ_EN_REG));
	writel(0x01, (void __iomem *)(SUNXI_R_WDOG_VBASE + R_WDOG_CFG_REG));
	writel((0x1 << 5) | 0x01, (void __iomem *)(SUNXI_R_WDOG_VBASE + R_WDOG_MODE_REG));
	while(1);
}

static struct map_desc sunxi_io_desc[] __initdata = {
	{(u32)SUNXI_IO_VBASE,      __phys_to_pfn(SUNXI_IO_PBASE),      SUNXI_IO_SIZE,      MT_DEVICE},
	{(u32)SUNXI_SRAM_A1_VBASE, __phys_to_pfn(SUNXI_SRAM_A1_PBASE), SUNXI_SRAM_A1_SIZE, MT_MEMORY_ITCM},
	{(u32)SUNXI_SRAM_A2_VBASE, __phys_to_pfn(SUNXI_SRAM_A2_PBASE), SUNXI_SRAM_A2_SIZE, MT_DEVICE_NONSHARED},
#ifdef CONFIG_ARCH_SUN8IW3P1
	{(u32)SUNXI_SRAM_VE_VBASE, __phys_to_pfn(SUNXI_SRAM_VE_PBASE), SUNXI_SRAM_VE_SIZE, MT_DEVICE},
#endif
	{(u32)SUNXI_BROM_VBASE,    __phys_to_pfn(SUNXI_BROM_PBASE),    SUNXI_BROM_SIZE,    MT_DEVICE},
};

static void __init sun8i_fixup(struct tag *tags, char **from,
			       struct meminfo *meminfo)
{
#ifdef CONFIG_EVB_PLATFORM
	struct tag *t;

	for (t = tags; t->hdr.size; t = tag_next(t)) {
		if (t->hdr.tag == ATAG_MEM && t->u.mem.size) {
			early_printk("[%s]: From boot, get meminfo:\n"
					"\tStart:\t0x%08x\n"
					"\tSize:\t%dMB\n",
					__func__,
					t->u.mem.start,
					t->u.mem.size >> 20);
			mem_start = t->u.mem.start;
			mem_size = t->u.mem.size;
			return;
		}
	}
#endif

	early_printk("[%s] enter\n", __func__);

	meminfo->bank[0].start = PLAT_PHYS_OFFSET;
	meminfo->bank[0].size = PLAT_MEM_SIZE;
	meminfo->nr_banks = 1;

	early_printk("nr_banks: %d, bank.start: 0x%08x, bank.size: 0x%08x\n",
			meminfo->nr_banks, meminfo->bank[0].start,
			(unsigned int)meminfo->bank[0].size);
}

void __init sun8i_reserve(void)
{
	/* reserve for sys_config */
	memblock_reserve(SYS_CONFIG_MEMBASE, SYS_CONFIG_MEMSIZE);

	/* reserve for standby */
	memblock_reserve(SUPER_STANDBY_MEM_BASE, SUPER_STANDBY_MEM_SIZE);

#if defined(CONFIG_ION) || defined(CONFIG_ION_MODULE)
	/* fix "page fault when ION_IOC_SYNC" bug in mali driver */
	//memblock_remove(ion_mem.start, ion_mem.size);
	memblock_reserve(ion_mem.start, ion_mem.size);
#endif
}

#ifndef CONFIG_OF
static void __init sun8i_gic_init(void)
{
	gic_init(0, 29, (void __iomem *)SUNXI_GIC_DIST_VBASE, (void __iomem *)SUNXI_GIC_CPU_VBASE);
}
#endif

void __init sunxi_map_io(void)
{
	iotable_init(sunxi_io_desc, ARRAY_SIZE(sunxi_io_desc));
}

static void __init sunxi_dev_init(void)
{
#ifdef CONFIG_OF
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
#else
	platform_add_devices(sunxi_dev, ARRAY_SIZE(sunxi_dev));
#endif
}

#ifdef CONFIG_ARM_ARCH_TIMER
extern void __init sunxi_init_clocks(void);
struct arch_timer sun8i_arch_timer __initdata = {
	.res[0] = {
		.start = 29,
		.end = 29,
		.flags = IORESOURCE_IRQ,
	},
	.res[1] = {
		.start = 30,
		.end = 30,
		.flags = IORESOURCE_IRQ,
	},
};

void sunxi_timer_init(void);
static void __init sun8i_timer_init(void)
{
	sunxi_init_clocks();
	arch_timer_register(&sun8i_arch_timer);
	arch_timer_sched_clock_init();
}

struct sys_timer sunxi_timer __initdata = {
	.init = sun8i_timer_init,
};
#endif
unsigned int _hex2dec(unsigned int hex)
{
    unsigned int dec;

    switch (hex) {
        case 48: dec = 0; break;
        case 49: dec = 1; break;
        case 50: dec = 2; break;
        case 51: dec = 3; break;
        case 52: dec = 4; break;
        case 53: dec = 5; break;
        case 54: dec = 6; break;
        case 55: dec = 7; break;
        case 56: dec = 8; break;
        case 57: dec = 9; break;
        default:
            pr_err("something wrong in chip id\n");
            dec = 0;
    }

    return dec;
}

static uint8_t sunxi_chip_id[16];
int get_chip_id(uint8_t *chip_id)
{
#ifdef CONFIG_SUNXI_ARISC
	static int read_flag = 0;

	if(!read_flag) {
		arisc_axp_get_chip_id(sunxi_chip_id);
		read_flag = 1;
	}
#endif

	memcpy(chip_id, sunxi_chip_id, 16);

	return 0;
}
int get_chip_id2(uint32_t *chip_id)
{
	uint32_t sid_rkey[4];
#ifdef CONFIG_SUNXI_ARISC
	static int read_flag = 0;
	if(!read_flag) {
		arisc_axp_get_chip_id(sunxi_chip_id);
		read_flag = 1;
	}
#endif
	memcpy(sid_rkey, sunxi_chip_id, 16);

    if (sid_rkey[0] == 0 && sid_rkey[1] == 0 &&
        sid_rkey[2] == 0 && sid_rkey[3] == 0) {
        chip_id[0] = 0;
        chip_id[1] = 0;
        chip_id[2] = 0;
        chip_id[3] = 0;
        return 0;
    }
    chip_id[0] = sid_rkey[3];


    chip_id[1] = (sid_rkey[0] >> 24) & 0xff;
    chip_id[1] |= (sid_rkey[1] & 0xff) << 8;
    chip_id[1] |= ((sid_rkey[1] >> 8) & 0xff) << 16;
	chip_id[1] |= _hex2dec((sid_rkey[1] >> 16) & 0xff) << 24;
    chip_id[1] |= _hex2dec((sid_rkey[1] >> 24) & 0xff) << 28;

    chip_id[2] |= (sid_rkey[2]&0xff000000) >> 20;
	chip_id[2] |= _hex2dec(sid_rkey[2]&0xff);
	return 0;
}
void __init sunxi_init_early(void)
{
#ifdef CONFIG_SUNXI_CONSISTENT_DMA_SIZE
	init_consistent_dma_size(CONFIG_SUNXI_CONSISTENT_DMA_SIZE << 20);
#endif
}

MACHINE_START(SUNXI, "sun8i")
	.atag_offset	= 0x100,
	.init_machine	= sunxi_dev_init,
	.init_early     = sunxi_init_early,
	.map_io		= sunxi_map_io,
#ifndef CONFIG_OF
	.init_irq	= sun8i_gic_init,
#endif
	.handle_irq	= gic_handle_irq,
	.restart	= sun8i_restart,
	.timer		= &sunxi_timer,
	.dt_compat	= NULL,
	.reserve	= sun8i_reserve,
	.fixup		= sun8i_fixup,
	.nr_irqs	= NR_IRQS,
MACHINE_END
