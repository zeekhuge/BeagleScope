/*
 * travis_test.c - An simple loadable kernel module to test the working of travis CI.
 *
 * This program has been developed by ZeekHuge.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/of.h>
#include <linux/of_device.h>



static int __init init_travis_test(void)
{
	printk(KERN_DEBUG "travis_test kernel was loaded.\n"); 
	return 0; 
}
module_init(init_travis_test);


static void  __exit exit_travis_test(void)
{
	printk(KERN_DEBUG "travis_test kernel was unloaded.\n");
}
module_exit(exit_travis_test);


MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ZeekHuge");