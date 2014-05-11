/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable factor-based clock implementation
 */
#ifndef __MACH_SUNXI_CLK_SUNXI_H
#define __MACH_SUNXI_CLK_SUNXI_H

#define to_clk_factor(_hw) container_of(_hw, struct sunxi_clk_factors, hw)
//#define SETMASK(width, shift)   (((-1U) >> (31-width))  << (shift))
#define SETMASK(width, shift)   ((width?((-1U) >> (32-width)):0)  << (shift))
#define CLRMASK(width, shift)   (~(SETMASK(width, shift)))
#define GET_BITS(shift, width, reg)     \
            (((reg) & SETMASK(width, shift)) >> (shift))
#define SET_BITS(shift, width, reg, val) \
            (((reg) & CLRMASK(width, shift)) | (val << (shift)))

#define __SUNXI_ALL_CLK_IGNORE_UNUSED__  1
extern void sunx_clk_default_source(void);
#endif
