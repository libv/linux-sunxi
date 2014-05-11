/*
 * drivers/thermal/sunxi-temperature.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *	Li Ming<liming@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <asm/io.h>
#include <linux/init-input.h>
#include <linux/thermal.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include "sunxi-thermal.h"
#include "sunxi-temperature.h"

static u32 debug_mask = 0;
#define dprintk(level_mask, fmt, arg...)	if (unlikely(debug_mask & level_mask)) \
	printk(KERN_DEBUG fmt , ## arg)

struct sunxi_ths_data {
	void __iomem *base_addr;
	int circle_num;
	int temp_data1[10];
	int temp_data2[10];
	int temp_data3[10];
	int temp_data4[10];
	int thermal_data[6];
	atomic_t delay;
	struct delayed_work work;

	int irq_used;

	struct input_dev *ths_input_dev;
	atomic_t input_delay;
	atomic_t input_enable;
	struct delayed_work input_work;
	struct mutex input_enable_mutex;

#ifdef CONFIG_PM
struct dev_pm_domain ths_pm_domain;
#endif
};
struct sunxi_ths_data *thermal_data;
static int temperature[6] = {5, 5, 5, 5, 5, 5};
static DEFINE_SPINLOCK(data_lock);
static struct ths_config_info ths_info = {
	.input_type = THS_TYPE,
};

static void sunxi_ths_reg_clear(void);
static void sunxi_ths_reg_init(void);

/********************************** input *****************************************/

static ssize_t sunxi_ths_input_delay_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	dprintk(DEBUG_CONTROL_INFO, "%d, %s\n", atomic_read(&thermal_data->input_delay), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&thermal_data->input_delay));

}

static ssize_t sunxi_ths_input_delay_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if (data > THERMAL_DATA_DELAY)
		data = THERMAL_DATA_DELAY;
	atomic_set(&thermal_data->input_delay, (unsigned int) data);

	return count;
}


static ssize_t sunxi_ths_input_enable_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	dprintk(DEBUG_CONTROL_INFO, "%d, %s\n", atomic_read(&thermal_data->input_enable), __FUNCTION__);
	return sprintf(buf, "%d\n", atomic_read(&thermal_data->input_enable));
}

static void sunxi_ths_input_set_enable(struct device *dev, int enable)
{
	int pre_enable = atomic_read(&thermal_data->input_enable);

	mutex_lock(&thermal_data->input_enable_mutex);
	if (enable) {
		if (pre_enable == 0) {
			schedule_delayed_work(&thermal_data->input_work,
				msecs_to_jiffies(atomic_read(&thermal_data->input_delay)));
			atomic_set(&thermal_data->input_enable, 1);
		}

	} else {
		if (pre_enable == 1) {
			cancel_delayed_work_sync(&thermal_data->input_work);
			atomic_set(&thermal_data->input_enable, 0);
		}
	}
	mutex_unlock(&thermal_data->input_enable_mutex);
}

static ssize_t sunxi_ths_input_enable_store(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned long data;
	int error;

	error = strict_strtoul(buf, 10, &data);
	if (error)
		return error;
	if ((data == 0)||(data==1)) {
		sunxi_ths_input_set_enable(dev,data);
	}

	return count;
}

static ssize_t sunxi_ths_show_temp(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	ssize_t cnt = 0;
	int i = 0;
	printk("%s: enter \n", __func__);

	for (i=0; i<4; i++) {
		cnt += sprintf(buf + cnt,"temperature[%d]:%d\n" , i, ths_read_data(i));
	}

	return cnt;
}

static ssize_t sunxi_ths_set_temp(struct device *dev,
		struct device_attribute *attr,
		const char *buf, size_t count)
{
	unsigned int data1, data2;
	unsigned long irqflags;
	char *endp;

	cancel_delayed_work_sync(&thermal_data->work);

	data1 = simple_strtoul(buf, &endp, 10);
	if (*endp != ' ') {
		printk("%s: %d\n", __func__, __LINE__);
		return -EINVAL;
	}
	buf = endp + 1;

	data2 = simple_strtoul(buf, &endp, 10);

	spin_lock_irqsave(&data_lock, irqflags);
	temperature[data1] = data2;
	spin_unlock_irqrestore(&data_lock, irqflags);

	printk("temperature[%d] = %d\n", data1, data2);

	return count;
}


static DEVICE_ATTR(delay, S_IRUGO|S_IWUSR|S_IWGRP,
		sunxi_ths_input_delay_show, sunxi_ths_input_delay_store);
static DEVICE_ATTR(enable, S_IRUGO|S_IWUSR|S_IWGRP|S_IWOTH,
		sunxi_ths_input_enable_show, sunxi_ths_input_enable_store);
static DEVICE_ATTR(temperature,S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH,
		sunxi_ths_show_temp, sunxi_ths_set_temp);

static struct attribute *sunxi_ths_input_attributes[] = {
	&dev_attr_delay.attr,
	&dev_attr_enable.attr,
	&dev_attr_temperature.attr,
	NULL
};

static struct attribute_group sunxi_ths_input_attribute_group = {
	.attrs = sunxi_ths_input_attributes
};

static void sunxi_ths_input_work_func(struct work_struct *work)
{
	static int tempetature = 5;
	struct sunxi_ths_data *data = container_of((struct delayed_work *)work,
			struct sunxi_ths_data, input_work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&data->input_delay));

	tempetature = ths_read_data(4);
	input_report_abs(data->ths_input_dev, ABS_MISC, tempetature);
	input_sync(data->ths_input_dev);
	dprintk(DEBUG_DATA_INFO, "%s: temperature %d,\n", __func__, tempetature);

	schedule_delayed_work(&data->input_work, delay);
}


int sunxi_ths_input_init(struct sunxi_ths_data *data)
{
	int err = 0;

	data->ths_input_dev = input_allocate_device();
	if (IS_ERR_OR_NULL(data->ths_input_dev)) {
		printk(KERN_ERR "temp_dev: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	data->ths_input_dev->name = "sunxi-ths";
	data->ths_input_dev->phys = "sunxiths/input0";
	data->ths_input_dev->id.bustype = BUS_HOST;
	data->ths_input_dev->id.vendor = 0x0001;
	data->ths_input_dev->id.product = 0x0001;
	data->ths_input_dev->id.version = 0x0100;

	input_set_capability(data->ths_input_dev, EV_ABS, ABS_MISC);
	input_set_abs_params(data->ths_input_dev, ABS_MISC, -50, 180, 0, 0);

	err = input_register_device(data->ths_input_dev);
	if (0 < err) {
		printk("%s: could not register input device\n", __func__);
		input_free_device(data->ths_input_dev);
		goto fail2;
	}

	INIT_DELAYED_WORK(&data->input_work, sunxi_ths_input_work_func);

	mutex_init(&data->input_enable_mutex);
	atomic_set(&data->input_enable, 0);
	atomic_set(&data->input_delay, THERMAL_DATA_DELAY);

	err = sysfs_create_group(&data->ths_input_dev->dev.kobj,
						 &sunxi_ths_input_attribute_group);
	if (err < 0)
	{
		printk("%s: sysfs_create_group err\n", __func__);
		goto fail3;
	}

	return err;
fail3:
	input_unregister_device(data->ths_input_dev);
fail2:
	kfree(data->ths_input_dev);
fail1:
	return err;

}

void sunxi_ths_input_exit(struct sunxi_ths_data *data)
{
	sysfs_remove_group(&data->ths_input_dev->dev.kobj, &sunxi_ths_input_attribute_group);
	input_unregister_device(data->ths_input_dev);
}

/********************************** input end *****************************************/


static void ths_write_data(int index, struct sunxi_ths_data *data)
{
	unsigned long irqflags;

	spin_lock_irqsave(&data_lock, irqflags);
	temperature[index] = data->thermal_data[index];
	spin_unlock_irqrestore(&data_lock, irqflags);

	return;
}

int ths_read_data(int index)
{
	unsigned long irqflags;
	int data;

	spin_lock_irqsave(&data_lock, irqflags);
	data = temperature[index];
	spin_unlock_irqrestore(&data_lock, irqflags);

	return data;
}

static void calc_temperature(int value, int divisor, int minus, struct sunxi_ths_data *data)
{
	unsigned int i = 0;
	uint64_t avg_temp[4] = {0, 0, 0, 0};

	if (data->circle_num % 5)
		return;
	if (value > 0) {
		for (i = 0; i < data->circle_num; i++) {
			avg_temp[0] += data->temp_data1[i];
		}
	}
	if (value > 1) {
		for (i = 0; i < data->circle_num; i++) {
			avg_temp[1] += data->temp_data2[i];
		}
	}
	if (value > 2) {
		for (i = 0; i < data->circle_num; i++) {
			avg_temp[2] += data->temp_data3[i];
		}
	}
	if (value > 3) {
		for (i = 0; i < data->circle_num; i++) {
			avg_temp[3] += data->temp_data4[i];
		}
	}

	for (i = 0; i < value; i++) {
		do_div(avg_temp[i], data->circle_num);
		dprintk(DEBUG_DATA_INFO, "avg_temp_reg=%lld\n", avg_temp[i]);

		avg_temp[i] *= 100;
		if (0 != avg_temp[i])
			do_div(avg_temp[i], divisor);

		avg_temp[i] -= minus;
		dprintk(DEBUG_DATA_INFO, "avg_temp=%lld C\n", avg_temp[i]);

		data->thermal_data[i] = avg_temp[i];
	}

	data->circle_num = 0;
	return;
}

#ifdef CONFIG_PM
static int sunxi_ths_suspend(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "%s: suspend\n", __func__);
	if (NORMAL_STANDBY == standby_type) {
		mutex_lock(&thermal_data->input_enable_mutex);
		if (atomic_read(&thermal_data->input_enable)==1) {
			cancel_delayed_work_sync(&thermal_data->input_work);
		}
		mutex_unlock(&thermal_data->input_enable_mutex);

		if (thermal_data->irq_used)
			disable_irq_nosync(THS_IRQNO);
		cancel_delayed_work_sync(&thermal_data->work);
		sunxi_ths_reg_clear();
	} else if (SUPER_STANDBY == standby_type) {
		mutex_lock(&thermal_data->input_enable_mutex);
		if (atomic_read(&thermal_data->input_enable)==1) {
			cancel_delayed_work_sync(&thermal_data->input_work);
		}
		mutex_unlock(&thermal_data->input_enable_mutex);

		if (thermal_data->irq_used)
			disable_irq_nosync(THS_IRQNO);
		cancel_delayed_work_sync(&thermal_data->work);
		sunxi_ths_reg_clear();
	}
	return 0;
}

static int sunxi_ths_resume(struct device *dev)
{
	dprintk(DEBUG_SUSPEND, "%s: resume\n", __func__);
	if (NORMAL_STANDBY == standby_type) {
		sunxi_ths_reg_init();
		if (thermal_data->irq_used)
			enable_irq(THS_IRQNO);
		schedule_delayed_work(&thermal_data->work,
				msecs_to_jiffies(atomic_read(&thermal_data->delay)));

		mutex_lock(&thermal_data->input_enable_mutex);
		if (atomic_read(&thermal_data->input_enable)==1) {
			schedule_delayed_work(&thermal_data->input_work,
				msecs_to_jiffies(atomic_read(&thermal_data->input_delay)));
		}
		mutex_unlock(&thermal_data->input_enable_mutex);
	} else if (SUPER_STANDBY == standby_type) {
		sunxi_ths_reg_init();
		if (thermal_data->irq_used)
			enable_irq(THS_IRQNO);
		schedule_delayed_work(&thermal_data->work,
				msecs_to_jiffies(atomic_read(&thermal_data->delay)));

		mutex_lock(&thermal_data->input_enable_mutex);
		if (atomic_read(&thermal_data->input_enable)==1) {
			schedule_delayed_work(&thermal_data->input_work,
				msecs_to_jiffies(atomic_read(&thermal_data->input_delay)));
		}
		mutex_unlock(&thermal_data->input_enable_mutex);
	}
	return 0;
}
#endif

#ifdef CONFIG_ARCH_SUN8IW3P1
static void sunxi_ths_work_func(struct work_struct *work)
{
	struct sunxi_ths_data *data = container_of((struct delayed_work *)work,
			struct sunxi_ths_data, work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&data->delay));

	data->temp_data1[data->circle_num] = readl(data->base_addr + THS_DATA_REG);
	dprintk(DEBUG_DATA_INFO, "THS data = %d\n", data->temp_data1[data->circle_num]);

	data->circle_num++;
	calc_temperature(1, 625, 267, data);

	data->thermal_data[4] = data->thermal_data[0];
	ths_write_data(0, data);
	ths_write_data(4, data);

	schedule_delayed_work(&data->work, delay);
}
static void sunxi_ths_reg_clear(void)
{
	writel(0, thermal_data->base_addr + THS_CTRL_REG1);
	writel(0, thermal_data->base_addr + THS_PRO_CTRL_REG);
}

static void sunxi_ths_reg_init(void)
{
	writel(THS_CTRL_REG0_VALUE, thermal_data->base_addr + THS_CTRL_REG0);
	writel(THS_CTRL_REG1_VALUE, thermal_data->base_addr + THS_CTRL_REG1);
	writel(THS_PRO_CTRL_REG_VALUE, thermal_data->base_addr + THS_PRO_CTRL_REG);

	dprintk(DEBUG_INIT, "THS_CTRL_REG0 = 0x%x\n", readl(thermal_data->base_addr + THS_CTRL_REG0));
	dprintk(DEBUG_INIT, "THS_CTRL_REG1 = 0x%x\n", readl(thermal_data->base_addr + THS_CTRL_REG1));
	dprintk(DEBUG_INIT, "THS_PRO_CTRL_REG = 0x%x\n", readl(thermal_data->base_addr + THS_PRO_CTRL_REG));
}

struct	thermal_trip_point_conf sunxi_trip_data = {
	.trip_count		= 3,
	.trip_val[0]		= 80,
	.trip_val[1]		= 100,
	.trip_val[2]		= 110,
};

struct thermal_cooling_conf sunxi_cooling_data = {
	.freq_data[0] = {
		.freq_clip_min = 408000,
		.freq_clip_max = 2016000,
		.temp_level = 80,
	},
	.freq_data[1] = {
		.freq_clip_min = 60000,
		.freq_clip_max = 408000,
		.temp_level = 100,
	},
	.freq_clip_count = 2,
};

static struct thermal_sensor_conf sunxi_sensor_conf = {
	.name			= "thermal-cpufreq-0",
	.read_temperature	= ths_read_data,
	.trip_data		= &sunxi_trip_data,
	.cooling_data		= &sunxi_cooling_data,
};

struct sunxi_thermal_zone ths_zone = {
	.id			= 0,
	.name			= "sunxi-therm",
	.sunxi_ths_sensor_conf	= &sunxi_sensor_conf,
};

static int __init sunxi_ths_init(void)
{
	int err = 0;

	dprintk(DEBUG_INIT, "%s: enter!\n", __func__);

	if (input_fetch_sysconfig_para(&(ths_info.input_type))) {
		printk("%s: err.\n", __func__);
		return -EPERM;
	}

	thermal_data = kzalloc(sizeof(*thermal_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(thermal_data)) {
		printk(KERN_ERR "thermal_data: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	sunxi_ths_input_init(thermal_data);
	err = sunxi_ths_register_thermal(&ths_zone);
	if(err < 0) {
		printk(KERN_ERR "therma: register thermal core failed\n");
		goto fail2;
	}

	thermal_data->base_addr = (void __iomem *)THERMAL_BASSADDRESS;
	sunxi_ths_reg_init();
	thermal_data->irq_used = 0;

#ifdef CONFIG_PM
	thermal_data->ths_pm_domain.ops.suspend = sunxi_ths_suspend;
	thermal_data->ths_pm_domain.ops.resume = sunxi_ths_resume;
	thermal_data->ths_input_dev->dev.pm_domain = &thermal_data->ths_pm_domain;
#endif
	INIT_DELAYED_WORK(&thermal_data->work, sunxi_ths_work_func);
	atomic_set(&thermal_data->delay, (unsigned int) THERMAL_READ_DELAY);
	schedule_delayed_work(&thermal_data->work,
				msecs_to_jiffies(atomic_read(&thermal_data->delay)));

	dprintk(DEBUG_INIT, "%s: OK!\n", __func__);

	return 0;
fail2:
	sunxi_ths_input_exit(thermal_data);
	kfree(thermal_data);
fail1:
	return err;
}

static void __exit sunxi_ths_exit(void)
{
	cancel_delayed_work_sync(&thermal_data->input_work);
	cancel_delayed_work_sync(&thermal_data->work);
	sunxi_ths_reg_clear();
	sunxi_ths_unregister_thermal(&ths_zone);
	sunxi_ths_input_exit(thermal_data);
	kfree(thermal_data);
}

#elif defined(CONFIG_ARCH_SUN9IW1P1)

static inline unsigned long ths_get_intsta(struct sunxi_ths_data *data)
{
	return (readl(data->base_addr + THS_INT_STA_REG));
}

static inline void ths_clr_intsta(struct sunxi_ths_data *data)
{
	writel(THS_CLEAR_INT_STA, data->base_addr + THS_INT_STA_REG);
}

static irqreturn_t ths_irq_service(int irqno, void *dev_id)
{
	struct sunxi_ths_data *data = container_of(dev_id,
					struct sunxi_ths_data, ths_input_dev);
	unsigned long intsta;
	dprintk(DEBUG_DATA_INFO, "THS IRQ Serve\n");

	intsta = ths_get_intsta(data);

	ths_clr_intsta(data);

	if (intsta & THS_INTS_SHT0) {

	}
	if (intsta & THS_INTS_SHT1) {

	}
	if (intsta & THS_INTS_SHT2) {

	}
	if (intsta & THS_INTS_SHT3) {

	}

	if (intsta & THS_INTS_ALARM0) {

	}
	if (intsta & THS_INTS_ALARM1) {

	}
	if (intsta & THS_INTS_ALARM2) {

	}
	if (intsta & THS_INTS_ALARM3) {

	}

	return IRQ_HANDLED;
}

static void sunxi_ths_work_func(struct work_struct *work)
{
	int i = 0;
	int temp;
	uint64_t avg_temp = 0;
	struct sunxi_ths_data *data = container_of((struct delayed_work *)work,
			struct sunxi_ths_data, work);
	unsigned long delay = msecs_to_jiffies(atomic_read(&data->delay));

	data->temp_data1[data->circle_num] = readl(data->base_addr + THS_DATA_REG0);
	dprintk(DEBUG_DATA_INFO, "THS data0 = %d\n", data->temp_data1[data->circle_num]);
	data->temp_data2[data->circle_num] = readl(data->base_addr + THS_DATA_REG1);
	dprintk(DEBUG_DATA_INFO, "THS data1 = %d\n", data->temp_data2[data->circle_num]);
	data->temp_data3[data->circle_num] = readl(data->base_addr + THS_DATA_REG2);
	dprintk(DEBUG_DATA_INFO, "THS data2 = %d\n", data->temp_data3[data->circle_num]);
	data->temp_data4[data->circle_num] = readl(data->base_addr + THS_DATA_REG3);
	dprintk(DEBUG_DATA_INFO, "THS data3 = %d\n", data->temp_data4[data->circle_num]);

	data->circle_num++;
	calc_temperature(4, 625, 267, data);

	if (0 == data->circle_num) {
		temp = data->thermal_data[0];
		for(i=0; i < 4; i++) {
			if (temp < data->thermal_data[i])
				temp = data->thermal_data[i];
			ths_write_data(i, data);
			avg_temp += data->thermal_data[i];
		}
		data->thermal_data[4] = temp;
		ths_write_data(4, data);
		do_div(avg_temp, 4);
		data->thermal_data[5] = (int)(avg_temp);
		ths_write_data(5, data);
	}

	schedule_delayed_work(&data->work, delay);
}

static void sunxi_ths_reg_clear(void)
{
	writel(0, thermal_data->base_addr + THS_CTRL_REG);
}

static void sunxi_ths_reg_init(void)
{
	writel(THS_CTRL_VALUE, thermal_data->base_addr + THS_CTRL_REG);
	writel(THS_INT_CTRL_VALUE, thermal_data->base_addr + THS_INT_CTRL_REG);
	writel(THS_CLEAR_INT_STA, thermal_data->base_addr + THS_INT_STA_REG);
	writel(THS_FILT_CTRL_VALUE, thermal_data->base_addr + THS_FILT_CTRL_REG);

	writel(0, thermal_data->base_addr + THS_INT_ALM_TH_REG0);
	writel(0, thermal_data->base_addr + THS_INT_ALM_TH_REG1);
	writel(0, thermal_data->base_addr + THS_INT_ALM_TH_REG2);
	writel(0, thermal_data->base_addr + THS_INT_ALM_TH_REG3);

	writel(0, thermal_data->base_addr + THS_INT_SHUT_TH_REG0);
	writel(0, thermal_data->base_addr + THS_INT_SHUT_TH_REG1);
	writel(0, thermal_data->base_addr + THS_INT_SHUT_TH_REG2);
	writel(0, thermal_data->base_addr + THS_INT_SHUT_TH_REG3);

	dprintk(DEBUG_INIT, "THS_CTRL_REG = 0x%x\n", readl(thermal_data->base_addr + THS_CTRL_REG));
	dprintk(DEBUG_INIT, "THS_INT_CTRL_REG = 0x%x\n", readl(thermal_data->base_addr + THS_INT_CTRL_REG));
	dprintk(DEBUG_INIT, "THS_INT_STA_REG = 0x%x\n", readl(thermal_data->base_addr + THS_INT_STA_REG));
	dprintk(DEBUG_INIT, "THS_FILT_CTRL_REG = 0x%x\n", readl(thermal_data->base_addr + THS_FILT_CTRL_REG));
}

/****************************** thermal zone 1 *************************************/

struct	thermal_trip_point_conf sunxi_trip_data_1 = {
	.trip_count		= 3,
	.trip_val[0]		= 80,
	.trip_val[1]		= 100,
	.trip_val[2]		= 110,
};

struct thermal_cooling_conf sunxi_cooling_data_1 = {
	.freq_data[0] = {
		.freq_clip_min = 408000,
		.freq_clip_max = 2016000,
		.temp_level = 80,
	},
	.freq_data[1] = {
		.freq_clip_min = 60000,
		.freq_clip_max = 408000,
		.temp_level = 100,
	},
	.freq_clip_count = 2,
};

static struct thermal_sensor_conf sunxi_sensor_conf_1 = {
	.name			= "thermal-cpufreq-0",
	.read_temperature	= ths_read_data,
	.trip_data		= &sunxi_trip_data_1,
	.cooling_data		= &sunxi_cooling_data_1,
};

struct sunxi_thermal_zone ths_zone_1 = {
	.id			= 4,
	.name			= "sunxi-therm-1",
	.sunxi_ths_sensor_conf	= &sunxi_sensor_conf_1,
};

/**************************** thermal zone 1 end ***********************************/

/****************************** thermal zone 2 *************************************/

struct	thermal_trip_point_conf sunxi_trip_data_2 = {
	.trip_count		= 1,
	.trip_val[0]		= 110,
};

struct thermal_cooling_conf sunxi_cooling_data_2 = {
	.freq_data[0] = {
		.freq_clip_min = 60000,
		.freq_clip_max = 2016000,
		.temp_level = 110,
	},
	.freq_clip_count = 1,
};

static struct thermal_sensor_conf sunxi_sensor_conf_2 = {
	.name			= "thermal-cpufreq-0",
	.read_temperature	= ths_read_data,
	.trip_data		= &sunxi_trip_data_2,
	.cooling_data		= &sunxi_cooling_data_2,
};

struct sunxi_thermal_zone ths_zone_2 = {
	.id			= 5,
	.name			= "sunxi-therm-2",
	.sunxi_ths_sensor_conf	= &sunxi_sensor_conf_2,
};

/**************************** thermal zone 2 end ***********************************/

static int __init sunxi_ths_init(void)
{
	int err = 0;

	dprintk(DEBUG_INIT, "%s: enter!\n", __func__);

	if (input_fetch_sysconfig_para(&(ths_info.input_type))) {
		printk("%s: err.\n", __func__);
		return -EPERM;
	}

	thermal_data = kzalloc(sizeof(*thermal_data), GFP_KERNEL);
	if (IS_ERR_OR_NULL(thermal_data)) {
		printk(KERN_ERR "thermal_data: not enough memory for input device\n");
		err = -ENOMEM;
		goto fail1;
	}

	sunxi_ths_input_init(thermal_data);
	err = sunxi_ths_register_thermal(&ths_zone_1);
	if(err < 0) {
		printk(KERN_ERR "therma: register thermal core failed\n");
		goto fail2;
	}
	err = sunxi_ths_register_thermal(&ths_zone_2);
	if(err < 0) {
		printk(KERN_ERR "therma: register thermal core failed\n");
		goto fail2;
	}

	thermal_data->base_addr = (void __iomem *)THERMAL_BASSADDRESS;
	sunxi_ths_reg_init();
	thermal_data->irq_used = 1;
	if (request_irq(THS_IRQNO, ths_irq_service, 0, "Thermal",
			thermal_data->ths_input_dev)) {
		err = -EBUSY;
		goto fail3;
	}

#ifdef CONFIG_PM
	thermal_data->ths_pm_domain.ops.suspend = sunxi_ths_suspend;
	thermal_data->ths_pm_domain.ops.resume = sunxi_ths_resume;
	thermal_data->ths_input_dev->dev.pm_domain = &thermal_data->ths_pm_domain;
#endif
	INIT_DELAYED_WORK(&thermal_data->work, sunxi_ths_work_func);
	atomic_set(&thermal_data->delay, (unsigned int) THERMAL_DATA_DELAY);
	schedule_delayed_work(&thermal_data->work,
				msecs_to_jiffies(atomic_read(&thermal_data->delay)));

	dprintk(DEBUG_INIT, "%s: OK!\n", __func__);

	return 0;
fail3:
	sunxi_ths_unregister_thermal(&ths_zone_2);
	sunxi_ths_unregister_thermal(&ths_zone_1);
fail2:
	sunxi_ths_input_exit(thermal_data);
	kfree(thermal_data);
fail1:
	return err;
}

static void __exit sunxi_ths_exit(void)
{
	cancel_delayed_work_sync(&thermal_data->input_work);
	cancel_delayed_work_sync(&thermal_data->work);
	free_irq(THS_IRQNO, thermal_data->ths_input_dev);
	sunxi_ths_reg_clear();
	sunxi_ths_unregister_thermal(&ths_zone_2);
	sunxi_ths_unregister_thermal(&ths_zone_1);
	sunxi_ths_input_exit(thermal_data);
	kfree(thermal_data);
}
#endif

module_init(sunxi_ths_init);
module_exit(sunxi_ths_exit);
module_param_named(debug_mask, debug_mask, int, 0644);
MODULE_DESCRIPTION("thermal seneor driver");
MODULE_AUTHOR("Li Ming <liming@allwinnertech.com>");
MODULE_LICENSE("GPL");

