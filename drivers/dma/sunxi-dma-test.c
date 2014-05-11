/*
 * drivers/dma/sunxi-dma-test.c
 *
 * Copyright (C) 2013-2015 Allwinnertech Co., Ltd
 *
 * Author: Sugar <shuge@allwinnertech.com>
 *
 * Sunxi DMA test module
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/freezer.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>

/*
 * Debug dir tree
 * sunxi_dma/
 * |-result
 * |-state
 * |-mode
 * |-nr_chan
 * |-buf_size
 *
 */

struct dentry *root;

static int sunxi_dma_debugfs_show(struct seq_file *s, void *data)
{
	return 0;
}

static int sunxi_dma_dbgfs_ops(struct inode *inode, struct file *file)
{
	return single_open(file, sunxi_dma_debugfs_show, inode->i_private);
}

static const struct file_operations sunxi_dma_dbgfs_ops = {
	.open		= sunxi_dma_dbgfs_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int __init sunxi_dma_test_init(void)
{
	printk(KERN_INFO "[sunxi_dma_test]: Init this module!\n");
	root = debugfs_create_dir("sunxi_dma", NULL);
	if (!root) {
		printk(KERN_ERR "[sunxi_dma_test]: Can't create the test_root dir!\n");
		return -EINVAL;
	}

	return 0;
}

static void __exit sunxi_dma_test_exit(void)
{
	printk(KERN_INFO "[sunxi_dma_test]: Exit this module!\n");
}

module_init(sunxi_dma_test_init);
module_exit(sunxi_dma_test_exit);
MODULE_AUTHOR("Sugar");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Sunxi DMA Test Module");

