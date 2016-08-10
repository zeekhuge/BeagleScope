/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/rpmsg.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include "parallel_interface.h"

/*
 * macro to print debug info easily
 */
#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__);


static int pi_bus_probe (struct device *dev)
{
	log_debug();
	return 0;
}

static int pi_bus_match (struct device *dev, struct device_driver *drv)
{
	log_debug();
	return 0;
}

struct  bus_type pi_bus_type = {
	.name = "parallel_interface",
	.match = pi_bus_match,
	.probe = pi_bus_probe,
};

static int __init parallel_interface_driver_init(void)
{
	int ret;

	log_debug();
	ret = bus_register(&pi_bus_type);
	if (ret) {
		pr_err("parallel_interface: couldnt register bus type");
		return ret;
	}

	return 0;
}

static void __exit parallel_interface_driver_exit(void)
{
	log_debug()
	bus_unregister(&pi_bus_type);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
