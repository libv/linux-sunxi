/* extended_standby.h
 *
 * Copyright (C) 2013-2014 allwinner.
 *
 *  By      : liming
 *  Version : v1.0
 *  Date    : 2013-7-10 09:08
 */

#ifndef _SUN8I_EXTENDED_STANDBY_H
#define _SUN8I_EXTENDED_STANDBY_H

#if defined CONFIG_ARCH_SUN8IW1P1
static extended_standby_t talk_standby = {
	.id           	= TALKING_STANDBY_FLAGS,
	.pwr_dm_en      = 0xf3,      //mean gpu, cpu is powered off.
	.osc_en         = 0xf,
	.init_pll_dis   = (~(0x12)), //mean pll2 is on. pll5 is shundowned by dram driver.
	.exit_pll_en    = 0x21,      //mean enable pll1 and pll6
	.pll_change     = 0x0,
	.bus_change     = 0x0,
};

static extended_standby_t usb_standby = {
	.id  		= USB_STANDBY_FLAGS,
	.pwr_dm_en      = 0xf3,      //mean gpu, cpu is powered off.
	.osc_en         = 0xf ,      //mean all osc is powered on.
	.init_pll_dis   = (~(0x30)), //mean pll6 is on.pll5 is shundowned by dram driver.
	.exit_pll_en    = 0x1,       //mean enable pll1 and pll6
	.pll_change     = 0x20,
	.pll_factor[5]  = {0x18,0x1,0x1,0},
	.bus_change     = 0x4,
	.bus_factor[2]  = {0x8,0,0x3,0,0},
};
#elif defined CONFIG_ARCH_SUN8IW3P1
static extended_standby_t talk_standby = {
	.id           	= TALKING_STANDBY_FLAGS,
	.pwr_dm_en      = 0xf7,      //mean cpu is powered off.
	.osc_en         = 0xf,
	.init_pll_dis   = (~(0x12)), //mean pll2 is on. pll5 is shundowned by dram driver.
	.exit_pll_en    = 0x21,      //mean enable pll1 and pll6
	.pll_change     = 0x0,
	.bus_change     = 0x0,
};

static extended_standby_t usb_standby = {
	.id  		= USB_STANDBY_FLAGS,
	.pwr_dm_en      = 0xf7,      //mean cpu is powered off.
	.osc_en         = 0xf ,      //mean all osc is powered on.
	.init_pll_dis   = (~(0x30)), //mean pll6 is on.pll5 is shundowned by dram driver.
	.exit_pll_en    = 0x1,       //mean enable pll1 and pll6
	.pll_change     = 0x20,
	.pll_factor[5]  = {0x18,0x1,0x1,0},
	.bus_change     = 0x4,
	.bus_factor[2]  = {0x8,0,0x3,0,0},
};
#endif

#endif /*_EXTENDED_STANDBY_H */