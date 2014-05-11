/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/clk-private.h>
#include <linux/clk-provider.h>
#include <linux/clkdev.h>
#include <linux/clk/sunxi.h>
#include <mach/sys_config.h>
#include "clk-sunxi.h"
#include "clk-factors.h"
#include "clk-periph.h"
#include "clk-sun8iw1.h"


void sunxi_clk_default_source(void)
{
    script_item_u   script_item;
	struct clk *clk = NULL;
	struct clk *parent_clk = NULL;
    unsigned int org_rate;
#ifdef CONFIG_SUNXI_CLK_AHB_FROM_PLL6
    clk = clk_get(NULL,"ahb1");
    if(!clk || IS_ERR(clk))
        printk("Error not get clk ahb1\n");
    else
    {
        parent_clk = clk_get(NULL,"pll6ahb1");
        if(!parent_clk || IS_ERR(parent_clk))
        {
            clk_put(clk);
            printk("Error not get clk pll6ahb1");
        }
        else
        {
            printk("try to set ahb clk source to pll6ahb1\n");
			clk_set_parent(clk,parent_clk);
            printk("set ahb clk source to pll6ahb1\n");
            clk_put(parent_clk);
            clk_put(clk);
        }
    }
#else
 if((script_get_item("clock", "ahb1_parent", &script_item) == SCIRPT_ITEM_VALUE_TYPE_STR) && script_item.str)
  {
    clk = clk_get(NULL,"ahb1");
    if(!clk || IS_ERR(clk))
        printk("Error not get clk ahb1\n");
    else
    {
        parent_clk = clk_get(NULL,script_item.str);
        if(!parent_clk || IS_ERR(parent_clk))
        {
            clk_put(clk);
            printk("Error not get clk %s\n",script_item.str);
        }
        else
        {
			clk_set_parent(clk,parent_clk);
            printk("set ahb1 clk source to %s\n",script_item.str);
            clk_put(parent_clk);
            clk_put(clk);
        }
    }
  }
#endif

 if((script_get_item("clock", "apb2_parent", &script_item) == SCIRPT_ITEM_VALUE_TYPE_STR) && script_item.str)
  {
    clk = clk_get(NULL,"apb2");
    if(!clk || IS_ERR(clk))
        printk("Error not get clk apb2\n");
    else
    {
        parent_clk = clk_get(NULL,script_item.str);
        if(!parent_clk || IS_ERR(parent_clk))
        {
            clk_put(clk);
            printk("Error not get clk %s\n",script_item.str);
        }
        else
        {
            // switch to 1.2 M
            org_rate = clk_get_rate(clk);
            printk(KERN_INFO "set apb2 to low freq 1.2 Mhz\n");
            clk_set_rate(clk, 1200000);
			clk_set_parent(clk,parent_clk);
            printk("set ahb clk source to %s\n",script_item.str);
            printk(KERN_INFO "recove apb2 to pre freq %d\n",org_rate);
            clk_set_rate(clk, org_rate);
            clk_put(parent_clk);
            clk_put(clk);
        }
    }
  }
}


#ifdef CONFIG_SUNXI_CLK_DEFAULT_INIT
static char *init_clks[] = {"pll3", "pll4","pll6","pll7","pll8","pll9","pll10"};
static int __init sunxi_clk_default_plls(void)
{
    int     i;
    script_item_u   script_item;
	struct clk *clk = NULL;
	struct clk *parent_clk = NULL;

    for(i=0;i < ARRAY_SIZE(init_clks);i++)
    {
        if(script_get_item("clock", init_clks[i], &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT)
        {
            if(script_item.val) {
                    clk = clk_get(NULL,init_clks[i]);
                    if(!clk || IS_ERR(clk))
                    {
                        clk = NULL;
                        printk("Error not get clk %s\n",init_clks[i]);
                        continue;
                    }
                    printk(KERN_INFO "script config %s to %d Mhz\n", init_clks[i],script_item.val);
                    clk_set_rate(clk, script_item.val*1000000);
                    clk_put(clk);
                    clk=NULL;
            }
        }
        else
           printk("Warning Not Found clk %s in script \n",init_clks[i]);
    }
    printk("sunxi_default_clk_init\n");

#if defined(CONFIG_PLL6AHB1_CLK_DFT_VALUE) && (CONFIG_PLL6AHB1_CLK_DFT_VALUE > 0)
                    printk("try to set pll6ahb1 to %d\n",CONFIG_PLL6AHB1_CLK_DFT_VALUE);
                    clk = clk_get(NULL,"pll6ahb1");
                    clk_set_rate(clk,CONFIG_PLL6AHB1_CLK_DFT_VALUE);
                    clk_put(clk);
#endif
}


static int __init sunxi_clk_default_devices(void)
{
    script_item_u   script_item;
	struct clk *clk = NULL;
	struct clk *parent_clk = NULL;

#if defined(CONFIG_AHB1_CLK_DFT_VALUE) && (CONFIG_AHB1_CLK_DFT_VALUE > 0)
                    printk("try to set ahb1 to %d\n",CONFIG_AHB1_CLK_DFT_VALUE);
                    clk = clk_get(NULL,"ahb1");
                    clk_set_rate(clk,CONFIG_AHB1_CLK_DFT_VALUE);
                    clk_put(clk);
#endif
#if defined(CONFIG_APB1_CLK_DFT_VALUE) && (CONFIG_APB1_CLK_DFT_VALUE > 0)
                    printk("try to set apb1 to %d\n",CONFIG_APB1_CLK_DFT_VALUE);
                    clk = clk_get(NULL,"apb1");
                    clk_set_rate(clk,CONFIG_APB1_CLK_DFT_VALUE);
                    clk_put(clk);
#endif

  if((script_get_item("clock", "apb2", &script_item) == SCIRPT_ITEM_VALUE_TYPE_INT) && script_item.val)
  {
        clk = clk_get(NULL,"apb2");
        if(!clk || IS_ERR(clk))
        {
            clk = NULL;
            printk("Error not get clk %s\n","apb2");
        }
        else
        {
            printk(KERN_INFO "script config apb2 to %d Mhz\n",script_item.val);
            clk_set_rate(clk, script_item.val*1000000);
            clk_put(clk);
            clk=NULL;
        }
  }
    return 0;

}
static int __init sunx_clk_default_value(void)
{
    sunxi_clk_default_plls();
    sunxi_clk_default_source();
    sunxi_clk_default_devices();
}
arch_initcall(sunx_clk_default_value);
#endif
