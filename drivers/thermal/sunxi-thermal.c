/*
 * drivers/thermal/sunxi-thermal.c
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
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <linux/thermal.h>
#include <asm/io.h>
#include "sunxi-thermal.h"

static int sunxi_get_frequency_level(unsigned int cpu, unsigned int freq)
{
	int i = 0, ret = -EINVAL;
	struct cpufreq_frequency_table *table = NULL;
#ifdef CONFIG_CPU_FREQ
	table = cpufreq_frequency_get_table(cpu);
#endif
	if (!table)
		return ret;

	while (table[i].frequency != CPUFREQ_TABLE_END) {
		if (table[i].frequency == CPUFREQ_ENTRY_INVALID)
			continue;
		if (table[i].frequency == freq)
			return i;
		i++;
	}
	return ret;
}


/* Bind callback functions for thermal zone */
static int sunxi_ths_bind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int ret = 0, i;
	unsigned long lower, upper;
	unsigned long max_state = 0;
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	if (NULL == ths_zone->sunxi_ths_sensor_conf->name)
		return -EINVAL;

	printk(KERN_DEBUG "%s : %s", __func__, cdev->type);

	/* No matching cooling device */
	if (strcmp(ths_zone->sunxi_ths_sensor_conf->name, cdev->type) != 0)
		return 0;

	lower = THERMAL_NO_LIMIT;
	upper = THERMAL_NO_LIMIT;

	/* Bind the thermal zone to the cpufreq cooling device */
	for (i = 0; i < ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count; i++) {
		if (ths_zone->sunxi_ths_sensor_conf->trip_data->trip_val[i] !=
			ths_zone->sunxi_ths_sensor_conf->cooling_data->freq_data[i].temp_level) {
			continue;
		}

		cdev->ops->get_max_state(cdev, &max_state);

		lower = sunxi_get_frequency_level(0,
			ths_zone->sunxi_ths_sensor_conf->cooling_data->freq_data[i].freq_clip_max);

		lower = max_state - lower;

		upper = sunxi_get_frequency_level(0,
			ths_zone->sunxi_ths_sensor_conf->cooling_data->freq_data[i].freq_clip_min);

		upper = max_state - upper;
		printk(KERN_DEBUG "ths_zone trip = %d, lower = %ld, upper = %ld\n",
			ths_zone->sunxi_ths_sensor_conf->trip_data->trip_val[i],
			lower, upper);

		if (thermal_zone_bind_cooling_device(thermal, i, cdev,
							upper, lower)) {
			pr_err("error binding cdev inst %d\n", i);
			ret = -EINVAL;
		}
		ths_zone->bind = true;
	}
	return ret;
}

/* Unbind callback functions for thermal zone */
static int sunxi_ths_unbind(struct thermal_zone_device *thermal,
			struct thermal_cooling_device *cdev)
{
	int ret = 0, i;
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;
	
	if (ths_zone->bind == false)
		return 0;

	/* Unbind the thermal zone to the cpufreq cooling device */
	for (i = 0; i < ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count; i++) {
		if (thermal_zone_unbind_cooling_device(thermal, i,
								cdev)) {
			pr_err("error binding cdev inst %d\n", i);
			ret = -EINVAL;
		}
		ths_zone->bind = false;
	}
	return ret;
}

/* Get trip type callback functions for thermal zone */
static int sunxi_ths_get_trip_type(struct thermal_zone_device *thermal, int trip,
				 enum thermal_trip_type *type)
{
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	if ((trip < 0) || (trip > ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count-1))
		return -EINVAL;

	if (trip == (ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count-1))
		*type = THERMAL_TRIP_CRITICAL;
	else
		*type = THERMAL_TRIP_ACTIVE;
	return 0;
}

/* Get trip temperature callback functions for thermal zone */
static int sunxi_ths_get_trip_temp(struct thermal_zone_device *thermal, int trip,
				unsigned long *temp)
{
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	if ((trip < 0) || (trip > ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count-1))
		return -EINVAL;

	*temp = ths_zone->sunxi_ths_sensor_conf->trip_data->trip_val[trip];

	return 0;
}

/* Get critical temperature callback functions for thermal zone */
static int sunxi_ths_get_crit_temp(struct thermal_zone_device *thermal,
				unsigned long *temp)
{
	int ret;
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	ret = sunxi_ths_get_trip_temp(thermal,
			(ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count-1), temp);
	return ret;
}

/* Get mode callback functions for thermal zone */
static int sunxi_ths_get_mode(struct thermal_zone_device *thermal,
			enum thermal_device_mode *mode)
{
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	if (ths_zone)
		*mode = ths_zone->mode;
	return 0;
}

/* Set mode callback functions for thermal zone */
static int sunxi_ths_set_mode(struct thermal_zone_device *thermal,
			enum thermal_device_mode mode)
{
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	if (!ths_zone->therm_dev) {
		pr_notice("thermal zone not registered\n");
		return 0;
	}

	mutex_lock(&ths_zone->therm_dev->lock);

	if (mode == THERMAL_DEVICE_ENABLED)
		ths_zone->therm_dev->polling_delay = IDLE_INTERVAL;
	else
		ths_zone->therm_dev->polling_delay = 0;

	mutex_unlock(&ths_zone->therm_dev->lock);

	ths_zone->mode = mode;
	thermal_zone_device_update(ths_zone->therm_dev);
	pr_info("thermal polling set for duration=%d msec\n",
				ths_zone->therm_dev->polling_delay);
	return 0;
}


/* Get temperature callback functions for thermal zone */
static int sunxi_ths_get_temp(struct thermal_zone_device *thermal,
			unsigned long *temp)
{
	struct sunxi_thermal_zone *ths_zone = thermal->devdata;

	*temp = (unsigned long)(ths_zone->sunxi_ths_sensor_conf->read_temperature(ths_zone->id));
	return 0;
}

/* Get the temperature trend */
static int sunxi_ths_get_trend(struct thermal_zone_device *thermal,
			int trip, enum thermal_trend *trend)
{
	int ret = 0;
	unsigned long trip_temp;

	ret = sunxi_ths_get_trip_temp(thermal, trip, &trip_temp);
	if (ret < 0)
		return ret;
	if (thermal->temperature >= trip_temp)
		*trend = THERMAL_TREND_RAISING;
	else
		*trend = THERMAL_TREND_DROPPING;

	return 0;
}

/* Operation callback functions for thermal zone */
static struct thermal_zone_device_ops const sunxi_ths_dev_ops = {
	.bind = sunxi_ths_bind,
	.unbind = sunxi_ths_unbind,
	.get_temp = sunxi_ths_get_temp,
	.get_trend = sunxi_ths_get_trend,
	.get_mode = sunxi_ths_get_mode,
	.set_mode = sunxi_ths_set_mode,
	.get_trip_type = sunxi_ths_get_trip_type,
	.get_trip_temp = sunxi_ths_get_trip_temp,
	.get_crit_temp = sunxi_ths_get_crit_temp,
};

/* Un-Register with the in-kernel thermal management */
void sunxi_ths_unregister_thermal(struct sunxi_thermal_zone *ths_zone)
{
	if (!ths_zone)
		return;

	if (ths_zone->therm_dev)
		thermal_zone_device_unregister(ths_zone->therm_dev);

	pr_info("suxi_ths: Kernel Thermal management unregistered\n");
}
EXPORT_SYMBOL(sunxi_ths_unregister_thermal);

int sunxi_ths_register_thermal(struct sunxi_thermal_zone *ths_zone)
{
	int ret = 0;

	ths_zone->therm_dev = thermal_zone_device_register(ths_zone->name,
		ths_zone->sunxi_ths_sensor_conf->trip_data->trip_count, 0, ths_zone, &sunxi_ths_dev_ops, NULL, 0,
		IDLE_INTERVAL);

	if (IS_ERR(ths_zone->therm_dev)) {
		pr_err("Failed to register thermal zone device\n");
		ret = -EINVAL;
		goto err_unregister;
	}
	ths_zone->mode = THERMAL_DEVICE_ENABLED;

	pr_info("suxi_ths: Kernel Thermal management registered\n");

	return 0;

err_unregister:
	sunxi_ths_unregister_thermal(ths_zone);
	return ret;
}
EXPORT_SYMBOL(sunxi_ths_register_thermal);
