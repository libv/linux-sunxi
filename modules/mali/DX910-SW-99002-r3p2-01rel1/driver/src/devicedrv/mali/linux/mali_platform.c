/*
 * Copyright (C) 2010-2012 ARM Limited. All rights reserved.
 * 
 * This program is free software and is provided to you under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation, and any use by you of this program is subject to the terms of such GNU licence.
 * 
 * A copy of the licence is included with the program, and can also be obtained from Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

/**
 * @file mali_platform.c
 * Platform specific Mali driver functions for a default platform
 */
#include "mali_kernel_common.h"
#include "mali_osk.h"
#include <linux/mali/mali_utgard.h>
#include <linux/platform_device.h>
#include <linux/version.h>


#include <linux/module.h>  
#include <linux/clk.h>
#include <mach/irqs.h>
//#include <mach/clock.h>
#include <mach/sys_config.h>
#include <linux/pm_runtime.h>

#define SW_PA_MALI_IO_BASE   (0x01c40000)                                       
#define AW_IRQ_GPU_GP        (52)
#define AW_IRQ_GPU_GPMMU     (53)
#define AW_IRQ_GPU_PP0       (54)
#define AW_IRQ_GPU_PPMMU0    (55)
#define GIC_SRC_GPUPMU       (56)//not used?
#define AW_IRQ_GPU_PP1       (57)
#define AW_IRQ_GPU_PPMMU1    (58)

#define REG_RD32(reg)    (*((volatile u32 *)(reg)))         
#define REG_WR32(reg, value) (*((volatile u32 *)(reg))  = (value))  

struct __fb_addr_para {
    unsigned int fb_paddr;
    unsigned int fb_size;
};

void sunxi_get_fb_addr_para(struct __fb_addr_para *fb_addr_para);

static void mali_platform_device_release(struct device *device);
void mali_gpu_utilization_handler(u32 utilization);

typedef enum mali_power_mode_tag
{
	MALI_POWER_MODE_ON,
	MALI_POWER_MODE_LIGHT_SLEEP,
	MALI_POWER_MODE_DEEP_SLEEP,
} mali_power_mode;

static struct resource mali_gpu_resources[]=
{
    MALI_GPU_RESOURCES_MALI400_MP2_PMU(SW_PA_MALI_IO_BASE, AW_IRQ_GPU_GP, AW_IRQ_GPU_GPMMU, \
                                        AW_IRQ_GPU_PP0, AW_IRQ_GPU_PPMMU0, AW_IRQ_GPU_PP1, AW_IRQ_GPU_PPMMU1)
};

static struct platform_device mali_gpu_device =
{
    .name = MALI_GPU_NAME_UTGARD,
    .id = 0,
    .dev.release = mali_platform_device_release,
};

static struct mali_gpu_device_data mali_gpu_data = 
{
//    .dedicated_mem_start = SW_GPU_MEM_BASE,
//    .dedicated_mem_size = SW_GPU_MEM_SIZE,
    .shared_mem_size = 512*1024*1024,
//    .fb_start = SW_FB_MEM_BASE,
//    .fb_size = SW_FB_MEM_SIZE,
    .utilization_interval = 1000,
    .utilization_handler = mali_gpu_utilization_handler,
};

int mali_clk_div = 3;
module_param(mali_clk_div, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mali_clk_div, "Clock divisor for mali");

struct clk *h_ahb_mali, *h_mali_clk, *h_ve_pll;
int mali_clk_flag=0;

static void mali_platform_device_release(struct device *device)
{
    MALI_DEBUG_PRINT(2,("mali_platform_device_release() called\n"));
}

_mali_osk_errcode_t mali_platform_init(void)
{
	int reg_val;

	REG_WR32(0xf1c201a0, 0x80000003);

	reg_val = REG_RD32(0xf1c20064);
	reg_val |= 1<<20;
	REG_WR32(0xf1c20064, reg_val);
	
/*	unsigned long rate;
	int clk_div;
	int mali_used = 0;
	
	//get mali ahb clock
	h_ahb_mali = clk_get(NULL, "ahb_mali");
	if(!h_ahb_mali){
		MALI_PRINT(("try to get ahb mali clock failed!\n"));
	}
	//get mali clk
	h_mali_clk = clk_get(NULL, "mali");
	if(!h_mali_clk){
		MALI_PRINT(("try to get mali clock failed!\n"));
	}

	h_ve_pll = clk_get(NULL, "ve_pll");
	if(!h_ve_pll){
		MALI_PRINT(("try to get ve pll clock failed!\n"));
	}

	//set mali parent clock
	if(clk_set_parent(h_mali_clk, h_ve_pll)){
		MALI_PRINT(("try to set mali clock source failed!\n"));
	}
	
	//set mali clock
	rate = clk_get_rate(h_ve_pll);

	if(!script_parser_fetch("mali_para", "mali_used", &mali_used, 1)) {
		if (mali_used == 1) {
			if (!script_parser_fetch("mali_para", "mali_clkdiv", &clk_div, 1)) {
				if (clk_div > 0) {
					//pr_info("mali: use config clk_div %d\n", clk_div);
					mali_clk_div = clk_div;
				}
			}
		}
	}

	//pr_info("mali: clk_div %d\n", mali_clk_div);
	rate /= mali_clk_div;

	if(clk_set_rate(h_mali_clk, rate)){
		MALI_PRINT(("try to set mali clock failed!\n"));
	}
	
	if(clk_reset(h_mali_clk,0)){
		MALI_PRINT(("try to reset release failed!\n"));
	}
	
if(mali_clk_flag == 0)//jshwang add 2012-8-23 16:05:50
	{
		//printk(KERN_WARNING "enable mali clock\n");
		MALI_PRINT(("enable mali clock\n"));
		mali_clk_flag = 1;
	       if(clk_enable(h_ahb_mali))
	       {
		     MALI_PRINT(("try to enable mali ahb failed!\n"));
	       }
	       if(clk_enable(h_mali_clk))
	       {
		       MALI_PRINT(("try to enable mali clock failed!\n"));
	        }
	}	*/
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
    /*close mali axi/apb clock*/
//    if(mali_clk_flag == 1)
//    {
//    	//MALI_PRINT(("disable mali clock\n"));
//    	mali_clk_flag = 0;
//       clk_disable(h_mali_clk);
//       clk_disable(h_ahb_mali);
//    }
    MALI_SUCCESS;
}
/*
_mali_osk_errcode_t mali_platform_power_mode_change(mali_power_mode power_mode)
{
	if(power_mode == MALI_POWER_MODE_ON)
    {
    	if(mali_clk_flag == 0)
	{
		//printk(KERN_WARNING "enable mali clock\n");
		//MALI_PRINT(("enable mali clock\n"));
		mali_clk_flag = 1;
	       if(clk_enable(h_ahb_mali))
	       {
		     MALI_PRINT(("try to enable mali ahb failed!\n"));
	       }
	       if(clk_enable(h_mali_clk))
	       {
		       MALI_PRINT(("try to enable mali clock failed!\n"));
	        }
	}
    }
    else if(power_mode == MALI_POWER_MODE_LIGHT_SLEEP)
    {
    	//close mali axi/apb clock/
	if(mali_clk_flag == 1)
	{
		//MALI_PRINT(("disable mali clock\n"));
		mali_clk_flag = 0;
	       clk_disable(h_mali_clk);
	       clk_disable(h_ahb_mali);
	}
    }
    else if(power_mode == MALI_POWER_MODE_DEEP_SLEEP)
    {
    	//close mali axi/apb clock
	if(mali_clk_flag == 1)
	{
		//MALI_PRINT(("disable mali clock\n"));
		mali_clk_flag = 0;
	       clk_disable(h_mali_clk);
	       clk_disable(h_ahb_mali);
	}
    }
    MALI_SUCCESS;
}
*/

int sun5i_mali_platform_device_register(void)
{
    int err;
    struct __fb_addr_para fb_addr_para={0};

    sunxi_get_fb_addr_para(&fb_addr_para);


    MALI_DEBUG_PRINT(2,("sun5i__mali_platform_device_register() called\n"));

    err = platform_device_add_resources(&mali_gpu_device, mali_gpu_resources, sizeof(mali_gpu_resources) / sizeof(mali_gpu_resources[0]));
    if (0 == err)
    {
        mali_gpu_data.fb_start = fb_addr_para.fb_paddr;
        mali_gpu_data.fb_size = fb_addr_para.fb_size;
        err = platform_device_add_data(&mali_gpu_device, &mali_gpu_data, sizeof(mali_gpu_data));
        if(0 == err)
        {
            err = platform_device_register(&mali_gpu_device);
            if (0 == err)
            {
                mali_platform_init();
#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
#endif
#endif
 
                 MALI_DEBUG_PRINT(2,("sun5i__mali_platform_device_register() sucess!!\n"));

                return 0;
            }
        }

        MALI_DEBUG_PRINT(2,("sun5i__mali_platform_device_register() add data failed!\n"));

        platform_device_unregister(&mali_gpu_device);
    }
    return err;
}

void mali_platform_device_unregister(void)
{
    MALI_DEBUG_PRINT(2, ("mali_platform_device_unregister() called!\n"));
    
    mali_platform_deinit();
    platform_device_unregister(&mali_gpu_device);
}


void mali_gpu_utilization_handler(u32 utilization)
{
}

void set_mali_parent_power_domain(void* dev)
{
}


