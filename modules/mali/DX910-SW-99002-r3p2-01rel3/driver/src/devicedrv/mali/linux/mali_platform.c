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
#include <linux/regulator/consumer.h>
#include <linux/err.h>
#include <linux/module.h>  
#include <linux/clk.h>
#include <mach/irqs.h>
//#include <mach/clock.h>
#include <mach/sys_config.h>
#include <mach/includes.h>
#include <mach/powernow.h>
#include <linux/pm_runtime.h>
#include <linux/clk/sunxi_name.h>
#include <linux/delay.h>

#define REG_RD32(reg)    (*((volatile u32 *)(reg)))         
#define REG_WR32(reg, value) (*((volatile u32 *)(reg))  = (value))  

#define DEF_PLL8_CLK   381000000
#define  __USE_SYSCLK_DEF__

static int mali_clk_div    = 1;
static int mali_clk_flag   = 0;
struct clk *h_mali_clk  = NULL;
struct clk *h_gpu_pll   = NULL;
module_param(mali_clk_div, int, S_IRUSR | S_IWUSR | S_IWGRP | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(mali_clk_div, "Clock divisor for mali");


struct __fb_addr_para {
unsigned int fb_paddr;
unsigned int fb_size;
};

extern void sunxi_get_fb_addr_para(struct __fb_addr_para *fb_addr_para);



struct mali_dvfstab
{
    unsigned int vol_max;
    unsigned int freq_max;
    unsigned int freq_min;
    unsigned int mbus_freq;
};
struct mali_dvfstab mali_dvfs_table[] = {
    //extremity
    {1.2*1000*1000, 480*1000*1000, 480*1000*1000, 400*1000*1000},
    //perf
    {1.1*1000*1000, 360*1000*1000, 360*1000*1000, 300*1000*1000},
    //normal
    {1.1*1000*1000, 360*1000*1000, 360*1000*1000, 300*1000*1000},
    {0,0,0,0},
    {0,0,0,0},
};

unsigned int cur_mode   = SW_POWERNOW_PERFORMANCE;
unsigned int user_event = 0;
struct regulator *mali_regulator = NULL;


static void mali_platform_device_release(struct device *device);
void mali_gpu_utilization_handler(struct mali_gpu_utilization_data *data);
static void mali_mode_set(unsigned long setmode);
static int mali_powernow_mod_change(unsigned long code, void *cmd);

////////////////////////////////////////////////////////////////////////

typedef enum mali_power_mode_tag
{
	MALI_POWER_MODE_ON,
	MALI_POWER_MODE_LIGHT_SLEEP,
	MALI_POWER_MODE_DEEP_SLEEP,
} mali_power_mode;

static struct resource mali_gpu_resources[]=
{
                                     
    MALI_GPU_RESOURCES_MALI400_MP2_PMU(SUNXI_GPU_PBASE, SUNXI_IRQ_GPUGP, SUNXI_IRQ_GPUGPMMU, \
                                        SUNXI_IRQ_GPUPP0, SUNXI_IRQ_GPUPPMMU0, SUNXI_IRQ_GPUPP1, SUNXI_IRQ_GPUPPMMU1)

};

static struct platform_device mali_gpu_device =
{
    .name = MALI_GPU_NAME_UTGARD,
    .id = 0,
    .dev.release = mali_platform_device_release,
};

static struct mali_gpu_device_data mali_gpu_data = 
{
    .shared_mem_size = 512*1024*1024,
    .utilization_interval = 2000,
    .utilization_callback = mali_gpu_utilization_handler,
};

static void mali_platform_device_release(struct device *device)
{
    MALI_DEBUG_PRINT(2,("mali_platform_device_release() called\n"));
}

_mali_osk_errcode_t mali_platform_init(void)
{
	
#ifndef __USE_SYSCLK_DEF__
	//pll8 enable ,it is gpu clk source
    int pll8clk = 0;	
	int reg_val;

	reg_val = REG_RD32(0xf1c20038);
	printk("===pll8 init reg 0x%x",reg_val);
	reg_val |= 0x80000000;
	REG_WR32(0xf1c20038, reg_val);
	
	if(reg_val & (1 << 24))
	{
		if(reg_val & (1 << 25))
		{
			printk("pll8 frac out 297M\n");
			pll8clk = 297000000;
		}
		else
		{

			pll8clk = 270000000;
			printk("pll8 frac out 270M\n");
		}
	}
	else
	{
		int n = (reg_val >> 8) & 0x7f;
		int m = (reg_val & 0xf) + 1;
		pll8clk = 24000000*n/m;
		printk("pll8 clk out %d M",pll8clk/1000000);
	}
	//gpu clk enable
	REG_WR32(0xf1c201a0, 0x80000003);
	printk("gpu clk is set:%dM",pll8clk/(3+1));
	//gpu aph gate  enable
	reg_val = REG_RD32(0xf1c20064);
	reg_val |= 1<<20;
	REG_WR32(0xf1c20064, reg_val);
#else
	unsigned long rate = 0;
	unsigned int vol = 0;
	script_item_u   mali_use, clk_drv;
	mali_regulator = regulator_get(NULL, "axp22_dcdc2");
	if (IS_ERR(mali_regulator)) {
	    printk(KERN_ERR "get mali regulator failed\n");
        mali_regulator = NULL;
	}
	
   	//get mali clk
	h_mali_clk = clk_get(NULL, GPU_CLK);
	if(!h_mali_clk || IS_ERR(h_mali_clk)){
		MALI_PRINT(("try to get mali clock failed!\n"));
		return _MALI_OSK_ERR_FAULT;
	} else
		pr_info("%s(%d): get %s handle success!\n", __func__, __LINE__, GPU_CLK);

	h_gpu_pll = clk_get(NULL, PLL8_CLK);
	if(!h_gpu_pll || IS_ERR(h_gpu_pll)){
		MALI_PRINT(("try to get ve pll clock failed!\n"));
        return _MALI_OSK_ERR_FAULT;
	} else
		pr_info("%s(%d): get %s handle success!\n", __func__, __LINE__, "pll8");


	if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("clock", "pll8", &clk_drv)) {
		pr_info("%s(%d): get clock->pll8 success! pll=%d\n", __func__,
			__LINE__, clk_drv.val);
		if(clk_drv.val > 0)
			rate = clk_drv.val*1000*1000;
	} else
	{
		rate = DEF_PLL8_CLK;
		pr_info("%s(%d): get mali_para->mali_clkdiv failed!\n", __func__, __LINE__);
		printk("========set gpu_pll8 = %d\n",DEF_PLL8_CLK);
	}


	if(clk_set_rate(h_gpu_pll, rate)){
		MALI_PRINT(("try to set gpu pll clock failed!\n"));
		return _MALI_OSK_ERR_FAULT;
	} else
		printk("%s(%d): set pll8 clock rate success!\n", __func__, __LINE__);

	//set mali clock
	rate = clk_get_rate(h_gpu_pll);
	printk("%s(%d): get gpu pll rate %d!\n", __func__, __LINE__, (int)rate);
    
	mali_dvfs_table[SW_POWERNOW_NORMAL].freq_max = rate;
    mali_dvfs_table[SW_POWERNOW_NORMAL].freq_min = rate;
	mali_dvfs_table[SW_POWERNOW_PERFORMANCE].freq_max =rate;
    mali_dvfs_table[SW_POWERNOW_PERFORMANCE].freq_min = rate;
	if(mali_regulator)
	{
		vol = regulator_get_voltage(mali_regulator);
		mali_dvfs_table[SW_POWERNOW_NORMAL].vol_max = vol;
		mali_dvfs_table[SW_POWERNOW_PERFORMANCE].vol_max = vol;
		printk(KERN_ERR "%s(%d): get gpu pll volt is %d mV!\n", __func__, __LINE__, (int)vol*1000);
	}
	
	
	if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("mali_para", "mali_used", &mali_use)) {
		pr_info("%s(%d): get mali_para->mali_used success! mali_use %d\n", __func__, __LINE__, mali_use.val);
		if(mali_use.val == 1) {
			if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("mali_para", "mali_clkdiv", &clk_drv)) {
				pr_info("%s(%d): get mali_para->mali_clkdiv success! clk_drv %d\n", __func__,
					__LINE__, clk_drv.val);
				if(clk_drv.val > 0)
					mali_clk_div = clk_drv.val;
				else
					mali_clk_div = 1;

				pr_info("%s(%d): get mali_para->mali_clkdiv success! mali_clk_div %d\n", __func__,
					__LINE__, mali_clk_div);


			} else
			{
				pr_info("%s(%d): get mali_para->mali_clkdiv failed,default div 1!\n", __func__, __LINE__);
				mali_clk_div = 1;
			}
		}
	} else
	{
		pr_info("%s(%d): get mali_para->mali_used failed!default div 1\n", __func__, __LINE__);
		mali_clk_div = 1;
	}
	rate = rate / mali_clk_div; 

	pr_info("%s(%d): mali_clk_div %d\n", __func__, __LINE__, mali_clk_div);
	if(clk_set_rate(h_mali_clk, rate)){
		MALI_PRINT(("try to set mali clock failed!\n"));
		return _MALI_OSK_ERR_FAULT;
	} else
		printk("%s(%d): set mali clock rate success!\n", __func__, __LINE__);
		
	if(mali_clk_flag == 0)//jshwang add 2012-8-23 16:05:50
	{
		//printk(KERN_WARNING "enable mali clock\n");
		printk("enable mali clock\n");
		mali_clk_flag = 1;
		
		if(clk_prepare_enable(h_gpu_pll)){
		    MALI_PRINT(("try to enable mali clock failed!\n"));
		}
		mdelay(5);
		if(clk_prepare_enable(h_mali_clk)){
		    MALI_PRINT(("try to enable mali clock failed!\n"));
		}
	} 
#endif
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_deinit(void)
{
    /*close mali axi/apb clock*/
#ifndef __USE_SYSCLK_DEF__
	printk("===mali_paltform_deinit\n");	
#else
    if(mali_clk_flag == 1){
        //MALI_PRINT(("disable mali clock\n"));
        mali_clk_flag = 0;
        clk_disable_unprepare(h_mali_clk);
        clk_disable_unprepare(h_gpu_pll);
    }
#endif
    MALI_SUCCESS;
}

_mali_osk_errcode_t mali_platform_power_mode_change(mali_power_mode power_mode)
{

    if(power_mode == MALI_POWER_MODE_ON){
        if(mali_clk_flag == 0){
            mali_clk_flag = 1;

            //printk("enable mali clock\n");
	    if(clk_prepare_enable(h_gpu_pll)){
                printk("try to enable mali pll clock failed!\n");
                return _MALI_OSK_ERR_FAULT;
            }
	    mdelay(5);
            if(clk_prepare_enable(h_mali_clk)){
                printk("try to enable mali clock failed!\n");
                return _MALI_OSK_ERR_FAULT;
            }
        }
    } else if(power_mode == MALI_POWER_MODE_LIGHT_SLEEP){
        //close mali axi/apb clock/
        if(mali_clk_flag == 1){
            //printk("disable mali clock\n");
            mali_clk_flag = 0;

            clk_disable_unprepare(h_mali_clk);
	    clk_disable_unprepare(h_gpu_pll);
        }
    } else if(power_mode == MALI_POWER_MODE_DEEP_SLEEP){
    	//close mali axi/apb clock
        if(mali_clk_flag == 1){
            //MALI_PRINT(("disable mali clock\n"));
            mali_clk_flag = 0;
	    clk_disable_unprepare(h_mali_clk);
	    clk_disable_unprepare(h_gpu_pll);
        }
    }

    MALI_SUCCESS;
}

extern unsigned long totalram_pages;
static int powernow_notifier_call(struct notifier_block *this, unsigned long code, void *cmd)
{
    if (cur_mode == code){
        return 0;
    }
     MALI_DEBUG_PRINT(2, ("mali mode change\n\n\n\n\n\n\n"));
    return mali_powernow_mod_change(code, cmd);
}

static struct notifier_block powernow_notifier = {
	.notifier_call = powernow_notifier_call,
};


static int mali_freq_init(void)
{
    script_item_u   mali_use, mali_max_freq, mali_vol;



	if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("mali_para", "mali_used", &mali_use)) {
		pr_info("%s(%d): get mali_para->mali_used success! mali_use %d\n", __func__, __LINE__, mali_use.val);
		if(mali_use.val == 1) {
			if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("mali_para", "mali_extreme_freq", &mali_max_freq)) {
                    if (mali_max_freq.val > 0 ){
                        mali_dvfs_table[SW_POWERNOW_EXTREMITY].freq_max = mali_max_freq.val*1000*1000;
                        mali_dvfs_table[SW_POWERNOW_EXTREMITY].freq_min = mali_max_freq.val*1000*1000;
                        MALI_DEBUG_PRINT(2, ("mali_extreme_freq:%d Mhz\n", mali_max_freq.val));
                    }    
					else
					{
						printk(KERN_ERR "mali_extreme used default %d Mhz\n",mali_dvfs_table[SW_POWERNOW_EXTREMITY].freq_max);
					}
            }
			else
			{
				printk(KERN_ERR "mali_extreme used default %d Mhz\n",mali_dvfs_table[SW_POWERNOW_EXTREMITY].freq_max);
			}
			if(SCIRPT_ITEM_VALUE_TYPE_INT == script_get_item("mali_para", "mali_extreme_vol", &mali_vol)) {
                    if (mali_vol.val > 0 ){
                        mali_dvfs_table[SW_POWERNOW_EXTREMITY].vol_max = mali_vol.val*1000;
                        MALI_DEBUG_PRINT(2, ("mali_extreme_vol:%d Mv\n", mali_vol.val));
                    }    
					else
					{
						printk(KERN_ERR "mali_extreme used default %d uV\n",mali_dvfs_table[SW_POWERNOW_EXTREMITY].vol_max);
					}
            }
			else
			{
				printk(KERN_ERR "mali_extreme used default %d uV\n",mali_dvfs_table[SW_POWERNOW_EXTREMITY].vol_max);
			}
			
		}
	} else
		printk(KERN_ERR "%s(%d): get mali_para->mali_used failed,use default!\n", __func__, __LINE__);
  
    register_sw_powernow_notifier(&powernow_notifier);

	return 0;
}

int sun8i_mali_platform_device_register(void)
{
    int err;

    unsigned long mem_sz = 0;
    struct __fb_addr_para fb_addr_para={0};

    sunxi_get_fb_addr_para(&fb_addr_para);
    MALI_DEBUG_PRINT(2,("sun8i__mali_platform_device_register() called\n"));
    printk("=====gpu=====:fb_start = 0x%x,sz = %x\n",fb_addr_para.fb_paddr,fb_addr_para.fb_size);

    err = platform_device_add_resources(&mali_gpu_device, mali_gpu_resources, sizeof(mali_gpu_resources) / sizeof(mali_gpu_resources[0]));
    if (0 == err){
        //mali_gpu_data.dedicated_mem_start = gpu_addr.paddr - PLAT_PHYS_OFFSET;
        //mali_gpu_data.dedicated_mem_size = gpu_addr.size;
        mali_gpu_data.fb_start = fb_addr_para.fb_paddr;
        mali_gpu_data.fb_size = fb_addr_para.fb_size;
	
	mem_sz = (totalram_pages  * PAGE_SIZE )/1024; 
	if(mem_sz > 512*1024)
	{
		mali_gpu_data.shared_mem_size = 1024*1024*1024;
		printk(KERN_ERR "mem %ld kB,mali shared mem is 1G\n",mem_sz);
	}
	else
	{
		mali_gpu_data.shared_mem_size = 512*1024*1024;
		printk(KERN_ERR "mem %ld KB,mali shared mem is 512M\n",mem_sz);

	}

        err = platform_device_add_data(&mali_gpu_device, &mali_gpu_data, sizeof(mali_gpu_data));
        if(0 == err){
            err = platform_device_register(&mali_gpu_device);
            if (0 == err){
                if(_MALI_OSK_ERR_OK != mali_platform_init())return _MALI_OSK_ERR_FAULT;
#ifdef CONFIG_PM_RUNTIME
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
				pm_runtime_set_autosuspend_delay(&(mali_gpu_device.dev), 1000);
				pm_runtime_use_autosuspend(&(mali_gpu_device.dev));
#endif
				pm_runtime_enable(&(mali_gpu_device.dev));
#endif
                MALI_PRINT(("sun8i_mali_platform_device_register() sucess!!\n"));
				mali_freq_init();
                return 0;
            }
        }

        MALI_DEBUG_PRINT(0,("sun8i_mali_platform_device_register() add data failed!\n"));

        platform_device_unregister(&mali_gpu_device);
    }
    return err;
}
static int mali_powernow_mod_change(unsigned long code, void *cmd)
{

    switch (code) {
        case SW_POWERNOW_EXTREMITY:
            MALI_DEBUG_PRINT(2,(">>> %s\n\n\n\n\n\n\n\n", (char *)cmd));
            mali_mode_set(code);
            break;

        case SW_POWERNOW_PERFORMANCE:
        case SW_POWERNOW_NORMAL:
            MALI_DEBUG_PRINT(2,(">>> %s\n\n\n\n\n\n\n\n\n", (char *)cmd));
            mali_mode_set(code);
            break;

        case SW_POWERNOW_USEREVENT:
            MALI_DEBUG_PRINT(2,(">>> %s\n\n\n\n\n\n\n\n\n", (char *)cmd));
            break;
        case SW_POWERNOW_USB:
            MALI_DEBUG_PRINT(2,(">>> %s\n\n\n\n\n\n", (char *)cmd));
           // mali_mode_set(code);
            break;
        default:
            MALI_DEBUG_PRINT(2, ("powernow no such mode:%d, plz check!\n",code));
            break;
    }
    return 0;
}




void mali_platform_device_unregister(void)
{
    MALI_DEBUG_PRINT(2, ("mali_platform_device_unregister() called!\n"));
    
    mali_platform_deinit();

	register_sw_powernow_notifier(&powernow_notifier);
    platform_device_unregister(&mali_gpu_device);
	if (mali_regulator) {
		   regulator_put(mali_regulator);
		   mali_regulator = NULL;
	}
}


void mali_gpu_utilization_handler(struct mali_gpu_utilization_data *data)
{
}

static int mali_dvfs_change_status(struct regulator *mreg, unsigned int vol,
                                    struct clk *mali_clk, unsigned int mali_freq, 
                                    struct clk *mbus_clk, unsigned int mbus_freq)
{
	int rate = 0;
	unsigned int mvol   = 0;
	
	if (mreg && vol != 0){
		mvol = regulator_get_voltage(mreg);
		if(mvol != vol)
        	regulator_set_voltage(mreg, vol, vol);
		mvol = regulator_get_voltage(mreg);
		MALI_DEBUG_PRINT(2, ("set gpu volt:%d uV!\n",mvol));
	}
	if(mali_freq != 0 && mali_clk)
	{
	    if(clk_set_rate(mali_clk, mali_freq)){
			printk(KERN_ERR "try to set gpu pll clock failed!\n");
			return _MALI_OSK_ERR_FAULT;
		} else
			 MALI_DEBUG_PRINT(2, ( "%s(%d): set pll8 clock rate success!\n", __func__, __LINE__));
	}
	//set mali clock
	if(mali_clk)
	{
		rate = clk_get_rate(mali_clk);
		MALI_DEBUG_PRINT(2, ( "%s(%d): get gpu pll rate %d!\n", __func__, __LINE__, (int)rate));
	}
    return 0;
}

static void mali_mode_set(unsigned long setmode)
{
	unsigned mode = setmode;//defence complictly
	
    cur_mode = setmode;
     
    //check the mode valid
    if (mode > SW_POWERNOW_USB){
        mode = SW_POWERNOW_PERFORMANCE;
        cur_mode = SW_POWERNOW_PERFORMANCE;
    }
    //extremmity mode, if user event comes, we do nothing
    if (mode == SW_POWERNOW_EXTREMITY){
        mali_dvfs_change_status(mali_regulator, 
                                mali_dvfs_table[SW_POWERNOW_EXTREMITY].vol_max,
                                h_gpu_pll,
                                mali_dvfs_table[SW_POWERNOW_EXTREMITY].freq_max,
                                0,
                               0);
        user_event =0;
        MALI_DEBUG_PRINT(2,("mali_dvfs_work_handler set freq success, cur mode:%d, cur_freq:%d\n", 
                mode, (int)clk_get_rate(h_mali_clk),  regulator_get_voltage(mali_regulator)));
    }
	
	if (mode == SW_POWERNOW_NORMAL || mode == SW_POWERNOW_PERFORMANCE){
		  mali_dvfs_change_status(mali_regulator, 
								  mali_dvfs_table[SW_POWERNOW_PERFORMANCE].vol_max,
								  h_gpu_pll,
								  mali_dvfs_table[SW_POWERNOW_PERFORMANCE].freq_max,
								  0,
								  0);
		  user_event =0;
		  MALI_DEBUG_PRINT(2,("mali_dvfs_work_handler set freq success, cur mode:%d, cur_freq:%d\n", 
				  mode, (int)clk_get_rate(h_mali_clk),  regulator_get_voltage(mali_regulator)));
	  }
  	
	
    return;
}


void set_mali_parent_power_domain(void* dev)
{
}

