/*
 *  arch/arm/mach-sun6i/arisc/arisc_i.h
 *
 * Copyright (c) 2012 Allwinner.
 * sunny (sunny@allwinnertech.com)
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

#ifndef	__ARISC_I_H__
#define	__ARISC_I_H__

#include "./include/arisc_includes.h"
#include <linux/arisc/arisc.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <mach/sys_config.h>
#include <linux/pinctrl/consumer.h>
#include <linux/pinctrl/pinconf-sunxi.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/clk/sunxi_name.h>
#include <asm/atomic.h>
#include <linux/delay.h>

#define DRV_NAME	"sunxi-arisc"
#define DEV_NAME	"sunxi-arisc"

#if defined CONFIG_ARCH_SUN8IW1P1
#define DRV_VERSION "1.00"
#elif defined CONFIG_ARCH_SUN8IW3P1
#define DRV_VERSION "1.50"
#elif defined CONFIG_ARCH_SUN9IW1P1
#define DRV_VERSION "2.00"
#else
#error "please select a platform\n"
#endif

//local functions
extern int arisc_config_dram_paras(void);
extern int arisc_suspend_flag_query(void);

#endif	//__ARISC_I_H__
