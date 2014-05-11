/*
 * arch/arm/mach-sunxi/dram-freq/dram-freq.c
 *
 * Copyright (C) 2012 - 2016 Reuuimlla Limited
 * Pan Nan <pannan@reuuimllatech.com>
 *
 * SUNXI dram frequency dynamic scaling driver
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/suspend.h>
#include <linux/devfreq.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>
#include <linux/clk/sunxi_name.h>
#include <linux/wakelock.h>
#include <mach/sys_config.h>
#include "sunxi-dramfreq.h"

#undef DRAMFREQ_DBG
#undef DRAMFREQ_ERR
#if (1)
    #define DRAMFREQ_DBG(format,args...)   printk("[dramfreq] "format,##args)
#else
    #define DRAMFREQ_DBG(format,args...)   do{}while(0)
#endif
#define DRAMFREQ_ERR(format,args...)   printk(KERN_ERR "[dramfreq] ERR:"format,##args)

#define SRAM_MDFS_START (0xf0000000)
#define CCM_AHB_REG     (SUNXI_CCM_VBASE + 0x54)

#if defined(CONFIG_HAS_EARLYSUSPEND) && defined(CONFIG_DEVFREQ_GOV_VANS)
extern int dramfreq_lock_cpu0(void);
extern int dramfreq_unlock_cpu0(void);
#endif
extern char *mdfs_bin_start;
extern char *mdfs_bin_end;

static struct clk *clk_pll5; /* pll5 clock handler */
static struct clk *ahb1;     /* ahb1 clock handler */
__dram_para_t softfreq_info;
static spinlock_t mdfs_spin_lock;
static struct devfreq *this_df = NULL;
static unsigned long long getfreq_time_usecs = 0;
static DEFINE_MUTEX(ahb_update_lock);
static struct wake_lock dramfreq_wake_lock;

#ifdef CONFIG_DRAM_FREQ_DVFS
#define TABLE_LENGTH (8)
static unsigned int table_length_syscfg = 0;
static unsigned int last_vdd = 1240; /* backup last target voltage, default is 1.24v */
static struct regulator *dcdc2;
struct dramfreq_dvfs {
    unsigned int    freq;   /* cpu frequency, based on KHz */
    unsigned int    volt;   /* voltage for the frequency, based on mv */
};
static struct dramfreq_dvfs dvfs_table_syscfg[TABLE_LENGTH];
#endif

#ifdef CONFIG_DRAM_FREQ_DVFS
static inline unsigned int dramfreq_vdd_value(unsigned int freq)
{
    struct dramfreq_dvfs *dvfs_inf = NULL;
    dvfs_inf = &dvfs_table_syscfg[0];

    while((dvfs_inf+1)->freq >= freq)
        dvfs_inf++;

    return dvfs_inf->volt;
}

static int __init_vftable_syscfg(void)
{
    int i, ret = 0;
    char name[16] = {0};
    script_item_u val;
    script_item_value_type_e type;

    type = script_get_item("dram_dvfs_table", "LV_count", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch LV_count from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("LV_count value is %x\n", val.val);
    table_length_syscfg = val.val;

    /* table_length_syscfg must be < TABLE_LENGTH */
    if(table_length_syscfg > TABLE_LENGTH){
        DRAMFREQ_ERR("LV_count from sysconfig is out of bounder\n");
        ret = -1;
        goto fail;
    }

    for (i = 1; i <= table_length_syscfg; i++){
        sprintf(name, "LV%d_freq", i);
        type = script_get_item("dram_dvfs_table", name, &val);
        if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
            DRAMFREQ_ERR("get LV%d_freq from sysconfig failed\n", i);
            return -ENODEV;
        }
        dvfs_table_syscfg[i-1].freq = val.val / 1000;

        sprintf(name, "LV%d_volt", i);
        type = script_get_item("dram_dvfs_table", name, &val);
        if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
            DRAMFREQ_ERR("get LV%d_volt from sysconfig failed\n", i);
            return -ENODEV;
        }
        dvfs_table_syscfg[i-1].volt = val.val;
    }

fail:
    return ret;
}

static void __vftable_show(void)
{
    int i;

    DRAMFREQ_DBG("-------------------Dram V-F Table--------------------\n");
    for(i = 0; i < table_length_syscfg; i++){
        DRAMFREQ_DBG("\tvoltage = %4dmv \tfrequency = %4dKHz\n", 
            dvfs_table_syscfg[i].volt, dvfs_table_syscfg[i].freq);
    }
    DRAMFREQ_DBG("-----------------------------------------------------\n");
}
#endif

static unsigned long __dramfreq_get(struct clk *pll5)
{
    unsigned long pll5_rate, dram_freq;

    pll5_rate = clk_get_rate(pll5) / 1000;

	if(readl(SUNXI_CCM_VBASE + 0x020) & 0x3)	//pll normal mode
		dram_freq = pll5_rate * 2;
	else							//pll bypass mode
		dram_freq = pll5_rate / 4;

    return dram_freq;
}

/**
 * dramfreq_get - get the current DRAM frequency (in KHz)
 *
 */
unsigned long dramfreq_get(void)
{
    return __dramfreq_get(clk_pll5);
}
EXPORT_SYMBOL_GPL(dramfreq_get);

int __ahb_set_rate(unsigned long ahb_freq)
{
    mutex_lock(&ahb_update_lock);

    if (clk_prepare_enable(ahb1)) {
        DRAMFREQ_ERR("try to enable ahb1 output failed!\n");
        goto err;
    }

    if (clk_get_rate(ahb1) == ahb_freq) {
        mutex_unlock(&ahb_update_lock);
        return 0;
    }

    if (clk_set_rate(ahb1, ahb_freq)) {
        DRAMFREQ_ERR("try to set ahb1 rate to %lu failed!\n", ahb_freq);
        goto err;
    }

    mutex_unlock(&ahb_update_lock);
    return 0;

err:
    mutex_unlock(&ahb_update_lock);
    return -1;
}

/**
 *  freq_or_div:
 *  mdfs: freq div
 *  soft_switch: target freq
 */
static int __dramfreq_set(struct devfreq *df, unsigned int freq_or_div)
{
    int (*mdfs_main)(__dram_para_t *softfreq_switch);
    unsigned long flags;
    ktime_t calltime = ktime_set(0, 0), delta, rettime;

    mdfs_main = (int (*)(__dram_para_t *softfreq_switch))SRAM_MDFS_START;
    /* move code to sram */
    memcpy((void *)SRAM_MDFS_START, (void *)&mdfs_bin_start, (int)&mdfs_bin_end - (int)&mdfs_bin_start);
    softfreq_info.dram_clk = freq_or_div / 1000;
    softfreq_info.high_freq =  df->max_freq / 1000;

    spin_lock_irqsave(&mdfs_spin_lock, flags);
    calltime = ktime_get();

    mdfs_main(&softfreq_info);

    rettime = ktime_get();
    delta = ktime_sub(rettime, calltime);
    getfreq_time_usecs = ktime_to_ns(delta) >> 10;
    spin_unlock_irqrestore(&mdfs_spin_lock, flags);

    DRAMFREQ_DBG("[switch time]: %Ld usecs\n", getfreq_time_usecs);
    DRAMFREQ_DBG("dram: %luKHz->%luKHz ok!\n", df->previous_freq, dramfreq_get());

    return 0;
}

static int dramfreq_target(struct device *dev, unsigned long *freq, u32 flags)
{
    struct platform_device *pdev = container_of(dev, struct platform_device, dev);
    struct devfreq *df = platform_get_drvdata(pdev);
    int ret = 0;
#ifdef CONFIG_DRAM_FREQ_DVFS
    unsigned int new_vdd;
#endif

    if (*freq == df->previous_freq) {
        DRAMFREQ_DBG("freq_calc == df->previous_freq\n");
        return 0;
    }

    DRAMFREQ_DBG("want to set dram frequency from %luKHz to %luKHz\n", df->previous_freq, *freq);
    if (*freq > df->previous_freq) {
        if (!__ahb_set_rate(200000000))
            DRAMFREQ_DBG("set ahb to 200MHz ok!\n");
    }

#ifdef CONFIG_DRAM_FREQ_DVFS
    new_vdd = dramfreq_vdd_value(*freq);
    if (dcdc2 && (new_vdd > last_vdd)) {
        ret = regulator_set_voltage(dcdc2, new_vdd*1000, new_vdd*1000);
        if(ret < 0) {
            DRAMFREQ_DBG("fail to set regulator voltage!\n");
            regulator_put(dcdc2);
            return ret;
        }
        DRAMFREQ_DBG("dcdc2: %dmv->%dmv ok!\n", last_vdd, new_vdd);
    }
#endif

    wake_lock(&dramfreq_wake_lock);
    __dramfreq_set(df, *freq);
    wake_unlock(&dramfreq_wake_lock);

#ifdef CONFIG_DRAM_FREQ_DVFS
    if (dcdc2 && (new_vdd < last_vdd)) {
        ret = regulator_set_voltage(dcdc2, new_vdd*1000, new_vdd*1000);
        if (ret < 0) {
            DRAMFREQ_DBG("fail to set regulator voltage!\n");
            new_vdd = last_vdd;
            regulator_put(dcdc2);
            return ret;
        }
        DRAMFREQ_DBG("dcdc2: %dmv->%dmv ok!\n", last_vdd, new_vdd);
    }
    last_vdd = new_vdd;
#endif

    if (*freq < df->previous_freq) {
        if (!__ahb_set_rate(500000))
            DRAMFREQ_DBG("set ahb to 50MHz ok!\n");
    }

    *freq = dramfreq_get();
    return 0;
}

static int dramfreq_get_dev_status(struct device *dev,
                        struct devfreq_dev_status *stat)
{
    return 0;
}

static struct devfreq_dev_profile dram_devfreq_profile = {
    .target         = dramfreq_target,
    .get_dev_status = dramfreq_get_dev_status,
};

static ssize_t mdfs_table_show(struct device *dev, struct device_attribute *attr,
            char *buf)
{
    int m, n;

    for (m = 0; m < DRAM_MDFS_TABLE_PARA0; m++) {
        for (n = 0; n < DRAM_MDFS_TABLE_PARA1; n++) {
            DRAMFREQ_DBG("softfreq_table[%d][%d]=0x%x\n", m, n, softfreq_info.table[m][n]);
        }
    }

    return 0;
}

static DEVICE_ATTR(mdfs_table, S_IRUGO, mdfs_table_show, NULL);

static const struct attribute *dramfreq_attrib[] = {
    &dev_attr_mdfs_table.attr,
    NULL
};

static __devinit int sunxi_dramfreq_probe(struct platform_device *pdev)
{
    void *tmp_tbl_soft = NULL;
    int err = 0;
    script_item_u val;
    script_item_value_type_e type;
    unsigned int sun6i_dramfreq_min = 0, sun6i_dramfreq_max = 0;

    type = script_get_item("dram_para", "dram_clk", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_clk from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_clk value is %x\n", val.val);
    softfreq_info.dram_clk = val.val;
    sun6i_dramfreq_max = softfreq_info.dram_clk * 1000;
    DRAMFREQ_DBG("sun6i_dramfreq_max=%u\n", sun6i_dramfreq_max);

    type = script_get_item("dram_para", "dram_type", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_type from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_type value is %x\n", val.val);
    softfreq_info.dram_type = val.val;

    type = script_get_item("dram_para", "dram_zq", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_zq from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_zq value is %x\n", val.val);
    softfreq_info.dram_zq = val.val;

    type = script_get_item("dram_para", "dram_odt_en", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_odt_en from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_odt_en value is %x\n", val.val);
    softfreq_info.dram_odt_en = val.val;

    type = script_get_item("dram_para", "dram_para1", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_para1 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_para1 value is %x\n", val.val);
    softfreq_info.dram_para1 = val.val;

    type = script_get_item("dram_para", "dram_para2", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_para2 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_para2 value is %x\n", val.val);
    softfreq_info.dram_para2 = val.val;

    type = script_get_item("dram_para", "dram_mr0", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_mr0 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_mr0 value is %x\n", val.val);
    softfreq_info.dram_mr0 = val.val;

    type = script_get_item("dram_para", "dram_mr1", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_mr1 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_mr1 value is %x\n", val.val);
    softfreq_info.dram_mr1 = val.val;

    type = script_get_item("dram_para", "dram_mr2", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_mr2 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_mr2 value is %x\n", val.val);
    softfreq_info.dram_mr2 = val.val;

    type = script_get_item("dram_para", "dram_mr3", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_mr3 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_mr3 value is %x\n", val.val);
    softfreq_info.dram_mr3 = val.val;

    type = script_get_item("dram_para", "dram_tpr0", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr0 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr0 value is %x\n", val.val);
    softfreq_info.dram_tpr0 = val.val;

    type = script_get_item("dram_para", "dram_tpr1", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr1 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr1 value is %x\n", val.val);
    softfreq_info.dram_tpr1 = val.val;

    type = script_get_item("dram_para", "dram_tpr2", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr2 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr2 value is %x\n", val.val);
    softfreq_info.dram_tpr2 = val.val;

    type = script_get_item("dram_para", "dram_tpr3", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr3 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr3 value is %x\n", val.val);
    softfreq_info.dram_tpr3 = val.val;

    type = script_get_item("dram_para", "dram_tpr4", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr4 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr4 value is %x\n", val.val);
    softfreq_info.dram_tpr4 = val.val;

    type = script_get_item("dram_para", "dram_tpr5", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr5 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr5 value is %x\n", val.val);
    softfreq_info.dram_tpr5 = val.val;

    type = script_get_item("dram_para", "dram_tpr6", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr6 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr6 value is %x\n", val.val);
    softfreq_info.dram_tpr6 = val.val;

    type = script_get_item("dram_para", "dram_tpr7", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr7 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr7 value is %x\n", val.val);
    softfreq_info.dram_tpr7 = val.val;

    type = script_get_item("dram_para", "dram_tpr8", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr8 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr8 value is %x\n", val.val);
    softfreq_info.dram_tpr8 = val.val;

    type = script_get_item("dram_para", "dram_tpr9", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr9 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr9 value is %x\n", val.val);
    softfreq_info.dram_tpr9 = val.val;

    type = script_get_item("dram_para", "dram_tpr10", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr10 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr10 value is %x\n", val.val);
    softfreq_info.dram_tpr10 = val.val;

    type = script_get_item("dram_para", "dram_tpr11", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr11 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr11 value is %x\n", val.val);
    softfreq_info.dram_tpr11 = val.val;

    type = script_get_item("dram_para", "dram_tpr12", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr12 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr12 value is %x\n", val.val);	
    softfreq_info.dram_tpr12 = val.val;
    sun6i_dramfreq_min = softfreq_info.dram_tpr12 * 1000;
    DRAMFREQ_DBG("sun6i_dramfreq_min=%u\n", sun6i_dramfreq_min);

    type = script_get_item("dram_para", "dram_tpr13", &val);
    if (SCIRPT_ITEM_VALUE_TYPE_INT != type) {
        DRAMFREQ_ERR("fetch dram_tpr13 from sysconfig failed\n");
        return -ENODEV;
    }
    DRAMFREQ_DBG("dram_tpr13 value is %x\n", val.val);
    softfreq_info.dram_tpr13 = val.val;

    tmp_tbl_soft = __va(SYS_CONFIG_MEMBASE + SYS_CONFIG_MEMSIZE - 1024);
    memcpy(softfreq_info.table, tmp_tbl_soft, sizeof(softfreq_info.table));

    clk_pll5 = clk_get(NULL, PLL5_CLK);
    if (!clk_pll5 || IS_ERR(clk_pll5)) {
        DRAMFREQ_ERR("try to get PLL5 failed!\n");
        err = -ENOENT;
        goto err_pll5;
    }

    ahb1 = clk_get(NULL, AHB1_CLK);
    if (!ahb1 || IS_ERR(ahb1)) {
        DRAMFREQ_ERR("try to get AHB1 failed!\n");
        err = -ENOENT;
        goto err_ahb1;
    }

    dram_devfreq_profile.initial_freq = __dramfreq_get(clk_pll5);
    //this_df = devfreq_add_device(&pdev->dev, &dram_devfreq_profile, &devfreq_userspace, NULL);
    this_df = devfreq_add_device(&pdev->dev, &dram_devfreq_profile, &devfreq_vans, NULL);
    if (IS_ERR(this_df)) {
        DRAMFREQ_ERR("add devfreq device failed!\n");
        err = PTR_ERR(this_df);
        goto err_devfreq;
    }

    sun6i_dramfreq_min = sun6i_dramfreq_max /4;
    this_df->min_freq = this_df->scaling_min_freq = sun6i_dramfreq_min;
    this_df->max_freq = this_df->scaling_max_freq = sun6i_dramfreq_max;
    platform_set_drvdata(pdev, this_df);

    err = sysfs_create_files(&pdev->dev.kobj, dramfreq_attrib);
    if (err) {
        DRAMFREQ_ERR("create sysfs file failed\n");
        goto err_create_files;
    }

#ifdef CONFIG_DRAM_FREQ_DVFS
    err = __init_vftable_syscfg();
    if(err) {
        DRAMFREQ_ERR("init V-F Table failed\n");
        goto err_create_files;
    }

    __vftable_show();

    dcdc2 = regulator_get(NULL, "axp22_dcdc2");
    if (IS_ERR(dcdc2)) {
        DRAMFREQ_ERR("some error happen, fail to get regulator!");
        goto err_create_files;
    } else {
        last_vdd = regulator_get_voltage(dcdc2) / 1000;
        DRAMFREQ_DBG("last_vdd=%d\n", last_vdd);
    }
#endif

    wake_lock_init(&dramfreq_wake_lock, WAKE_LOCK_SUSPEND, "dramfreq_wakelock");

    DRAMFREQ_DBG("sunxi dramfreq probe ok!\n");

    return 0;

err_create_files:
    devfreq_remove_device(this_df);
err_devfreq:
    clk_put(ahb1);
    ahb1 = NULL;
err_ahb1:
    clk_put(clk_pll5);
    clk_pll5 = NULL;
err_pll5:
    return err;
}

static __devexit int sunxi_dramfreq_remove(struct platform_device *pdev)
{
    struct devfreq *df = platform_get_drvdata(pdev);

    devfreq_remove_device(df);

    if (!ahb1 || IS_ERR(ahb1)) {
        DRAMFREQ_ERR("ahb1 handle is invalid, just return!\n");
        return -EINVAL;
    } else {
        clk_put(ahb1);
        ahb1 = NULL;
    }

    if (!clk_pll5 || IS_ERR(clk_pll5)) {
        DRAMFREQ_ERR("clk_pll5 handle is invalid, just return!\n");
        return -EINVAL;
    } else {
        clk_put(clk_pll5);
        clk_pll5 = NULL;
    }

    wake_lock_destroy(&dramfreq_wake_lock);

    return 0;
}

static struct platform_driver sunxi_dramfreq_driver = {
    .probe  = sunxi_dramfreq_probe,
    .remove = sunxi_dramfreq_remove,
    .driver = {
        .name   = "sunxi-dramfreq",
        .owner  = THIS_MODULE,
    },
};

struct platform_device sunxi_dramfreq_device = {
    .name   = "sunxi-dramfreq",
    .id     = -1,
};

static int __init sunxi_dramfreq_init(void)
{
    int ret = 0;

    ret = platform_device_register(&sunxi_dramfreq_device);
    if (ret) {
        DRAMFREQ_ERR("dramfreq device init failed!\n");
        goto out;
    }

    ret = platform_driver_register(&sunxi_dramfreq_driver);
    if (ret) {
        DRAMFREQ_ERR("dramfreq driver init failed!\n");
        goto out;
    }

out:
    return ret;
}
late_initcall(sunxi_dramfreq_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SUN6I dramfreq driver with devfreq framework");
MODULE_AUTHOR("pannan");
