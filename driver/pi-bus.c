/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "parallel_interface.h"

/**
 * Helper macro to print debug info easily.
 * These are temporary and will be removed/replaced in the final version.
 */
#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__)
#define log_debug_msg(...) printk(KERN_DEBUG __VA_ARGS__ )

static int __init pi_bus_init(void)
{
	log_debug();

	return 0;
}

static void __exit pi_bus_exit(void)
{
	log_debug();
}

module_init(pi_bus_init);
module_exit(pi_bus_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
