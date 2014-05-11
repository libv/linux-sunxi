/*
 * drivers/input/misc/sunxi-temperature.c
 *
 * Copyright (C) 2013-2014 allwinner.
 *	Li Ming<liming@allwinnertech.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/thermal.h>
#include <linux/cpufreq.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpu.h>
#include <linux/sunxi-cpu-cooling.h>

/**
 * struct cpufreq_cooling_device - data for cooling device with cpufreq
 * @id: unique integer value corresponding to each cpufreq_cooling_device
 *	registered.
 * @cool_dev: thermal_cooling_device pointer to keep track of the
 *	registered cooling device.
 * @cpufreq_state: integer value representing the current state of cpufreq
 *	cooling	devices.
 * @cpufreq_val: integer value representing the absolute value of the clipped
 *	frequency.
 * @allowed_cpus: all the cpus involved for this cpufreq_cooling_device.
 *
 * This structure is required for keeping information of each
 * cpufreq_cooling_device registered. In order to prevent corruption of this a
 * mutex lock cooling_cpufreq_lock is used.
 */
struct cpufreq_cooling_device {
	int id;
	struct thermal_cooling_device *cool_dev;
	unsigned int cpufreq_state;
	unsigned int cpufreq_val;
	struct cpumask allowed_cpus;
};
static DEFINE_IDR(cpufreq_idr);
static DEFINE_MUTEX(cooling_cpufreq_lock);

static unsigned int cpufreq_dev_count;

/* notify_table passes value to the CPUFREQ_ADJUST callback function. */
#define NOTIFY_INVALID NULL

static int get_idr(struct idr *idr, struct mutex *lock, int *id)
{
	int err;

again:
	if (unlikely(idr_pre_get(idr, GFP_KERNEL) == 0))
		return -ENOMEM;

	if (lock)
		mutex_lock(lock);
	err = idr_get_new(idr, NULL, id);
	if (lock)
		mutex_unlock(lock);
	if (unlikely(err == -EAGAIN))
		goto again;
	else if (unlikely(err))
		return err;

	*id = *id & MAX_ID_MASK;
	return 0;
}

/**
 * release_idr - function to free the unique id.
 * @idr: struct idr * handle used for creating the id.
 * @id: int value representing the unique id.
 */
static void release_idr(struct idr *idr, int id)
{
	mutex_lock(&cooling_cpufreq_lock);
	idr_remove(idr, id);
	mutex_unlock(&cooling_cpufreq_lock);
}

/* cpufreq cooling device callback functions are defined below */

/**
 * cpufreq_get_max_state - callback function to get the max cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the max cooling state.
 *
 * Callback for the thermal cooling device to return the cpufreq
 * max cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int cpufreq_get_max_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	unsigned int count = 7;
	
	if (count > 0)
		*state = count;

	printk("%s: %ld", __func__, *state);

	return 0;
}

/**
 * cpufreq_get_cur_state - callback function to get the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: fill this variable with the current cooling state.
 *
 * Callback for the thermal cooling device to return the cpufreq
 * current cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int cpufreq_get_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long *state)
{
	struct cpufreq_cooling_device *cpufreq_device = cdev->devdata;
	
	*state = cpufreq_device->cpufreq_state;
	printk("%s: %ld\n", __func__, *state);

	return 0;
}

/**
 * cpufreq_set_cur_state - callback function to set the current cooling state.
 * @cdev: thermal cooling device pointer.
 * @state: set this variable to the current cooling state.
 *
 * Callback for the thermal cooling device to change the cpufreq
 * current cooling state.
 *
 * Return: 0 on success, an error code otherwise.
 */
static int cpufreq_set_cur_state(struct thermal_cooling_device *cdev,
				 unsigned long state)
{
	struct cpufreq_cooling_device *cpufreq_device = cdev->devdata;

	cpufreq_device->cpufreq_state = state;
	printk("%s: %ld\n", __func__, state);
	return 0;
}

/* Bind cpufreq callbacks to thermal cooling device ops */
static struct thermal_cooling_device_ops const cpufreq_cooling_ops = {
	.get_max_state = cpufreq_get_max_state,
	.get_cur_state = cpufreq_get_cur_state,
	.set_cur_state = cpufreq_set_cur_state,
};

/**
 * cpufreq_cooling_register - function to create cpufreq cooling device.
 * @clip_cpus: cpumask of cpus where the frequency constraints will happen.
 *
 * This interface function registers the cpufreq cooling device with the name
 * "thermal-cpufreq-%x". This api can support multiple instances of cpufreq
 * cooling devices.
 *
 * Return: a valid struct thermal_cooling_device pointer on success,
 * on failure, it returns a corresponding ERR_PTR().
 */
struct thermal_cooling_device *
cpufreq_cooling_register(const struct cpumask *clip_cpus)
{
	struct thermal_cooling_device *cool_dev;
	struct cpufreq_cooling_device *cpufreq_dev = NULL;

	char dev_name[THERMAL_NAME_LENGTH];
	int ret = 0;

	printk("%s: enter", __func__);

	cpufreq_dev = kzalloc(sizeof(struct cpufreq_cooling_device),
			      GFP_KERNEL);
	if (!cpufreq_dev)
		return ERR_PTR(-ENOMEM);

	cpumask_copy(&cpufreq_dev->allowed_cpus, clip_cpus);

	ret = get_idr(&cpufreq_idr, &cooling_cpufreq_lock, &cpufreq_dev->id);
	if (ret) {
		kfree(cpufreq_dev);
		return ERR_PTR(-EINVAL);
	}

	snprintf(dev_name, sizeof(dev_name), "thermal-cpufreq-%d",
		 cpufreq_dev->id);

	cool_dev = thermal_cooling_device_register(dev_name, cpufreq_dev,
						   &cpufreq_cooling_ops);
	if (!cool_dev) {
		printk("%s: cool dev register failed", __func__);
		release_idr(&cpufreq_idr, cpufreq_dev->id);
		kfree(cpufreq_dev);
		return ERR_PTR(-EINVAL);
	}
	cpufreq_dev->cool_dev = cool_dev;
	cpufreq_dev->cpufreq_state = 0;
	mutex_lock(&cooling_cpufreq_lock);

	cpufreq_dev_count++;

	mutex_unlock(&cooling_cpufreq_lock);

	printk("%s: exit", __func__);

	return cool_dev;
}
EXPORT_SYMBOL_GPL(cpufreq_cooling_register);

/**
 * cpufreq_cooling_unregister - function to remove cpufreq cooling device.
 * @cdev: thermal cooling device pointer.
 *
 * This interface function unregisters the "thermal-cpufreq-%x" cooling device.
 */
void cpufreq_cooling_unregister(struct thermal_cooling_device *cdev)
{
	struct cpufreq_cooling_device *cpufreq_dev = cdev->devdata;

	mutex_lock(&cooling_cpufreq_lock);
	cpufreq_dev_count--;

	mutex_unlock(&cooling_cpufreq_lock);

	thermal_cooling_device_unregister(cpufreq_dev->cool_dev);
	release_idr(&cpufreq_idr, cpufreq_dev->id);
	kfree(cpufreq_dev);
}
EXPORT_SYMBOL_GPL(cpufreq_cooling_unregister);

