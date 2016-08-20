/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rpmsg.h>
#include <linux/platform_device.h>
#include "parallel_interface.h"

/**
 * Helper macro to print debug info easily.
 * These are temporary and will be removed/replaced in the final version.
 */
#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__)
#define log_debug_msg(...) printk(KERN_DEBUG __VA_ARGS__ )

static int pi_bus_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	log_debug();

	return 0;
}

static void pi_bus_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	log_debug();
}

static const struct rpmsg_device_id pi_bus_rpmsg_id[] = {
		{ .name = "pibus0" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, pi_bus_rpmsg_id);

static struct rpmsg_driver pi_bus_rpmsg_driver = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= pi_bus_rpmsg_id,
	.probe		= pi_bus_rpmsg_probe,
	.remove		= pi_bus_rpmsg_remove,
};

static int __init pi_bus_init(void)
{
	int ret;
	log_debug();

	ret = register_rpmsg_driver(&pi_bus_rpmsg_driver);
	if (ret) {
		pr_err("pi_bus: couldnt register bus type");
		return ret;
	}

	return 0;
}

static void __exit pi_bus_exit(void)
{
	log_debug();

	unregister_rpmsg_driver(&pi_bus_rpmsg_driver);
}

module_init(pi_bus_init);
module_exit(pi_bus_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
