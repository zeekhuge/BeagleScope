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
#include <linux/slab.h>
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

static void pi_release (struct device *dev)
{
	log_debug();
}

static struct device pi_bus = {
	.init_name = "parallel_bus",
	.release = pi_release,
};

static int pi_rpmsg_probe (struct rpmsg_channel *rpdev)
{
	int ret;
	log_debug();

	ret = device_register(&pi_bus);
	if (ret){
		pr_err("parallel_interface: couldnt register bus type");
		put_device(&pi_bus);
		return ret;
	}

	return 0;
}

static void pi_rpmsg_remove (struct rpmsg_channel *rpdev)
{
	log_debug();
	device_unregister(&pi_bus);
}

static const struct rpmsg_device_id pi_rpmsg_id[] = {
		{ .name = "parallel_interface" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, pi_rpmsg_id);

static struct rpmsg_driver pi_rpmsg_driver= {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= pi_rpmsg_id,
	.probe		= pi_rpmsg_probe,
	.remove		= pi_rpmsg_remove,
};

static int __init parallel_interface_driver_init(void)
{
	int ret;

	log_debug();

	ret = register_rpmsg_driver(&pi_rpmsg_driver);
	if (ret) {
		pr_err("parallel_interface: couldnt register bus type");
		return ret;
	}

	ret = bus_register(&pi_bus_type);
	if (ret) {
		pr_err("parallel_interface: couldnt register bus type");
		goto remove_rpmsg_registration;
	}

	return 0;

remove_rpmsg_registration:
unregister_rpmsg_driver(&pi_rpmsg_driver);
return ret;
}

static void __exit parallel_interface_driver_exit(void)
{
	log_debug()
	bus_unregister(&pi_bus_type);
	unregister_rpmsg_driver(&pi_rpmsg_driver);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
