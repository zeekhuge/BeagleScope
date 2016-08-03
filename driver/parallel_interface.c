/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/rpmsg.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include "parallel_interface.h"

/*
 * macro to print debug info easily
 */
#define log_debug(msg) printk(KERN_DEBUG "%s: %s\n", __FILE__, msg);

static int parallel_interface_probe(struct rpmsg_channel *rpdev)
{
	int ret;
	struct pi_device *pidev;

	pidev = devm_kzalloc(&rpdev->dev, sizeof(*pidev), GFP_KERNEL);
	if (!pidev)
		return -ENOMEM;
}

static const struct rpmsg_device_id parallel_interface_id[] = {
		{ .name = "parallel-interface" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, parallel_interface_id);


static struct rpmsg_driver parallel_interface_driver= {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= parallel_interface_id,
	.probe		= parallel_interface_probe,
//	.callback	= parallel_interface_cb,
	.remove		= parallel_interface_remove,
};


static int __init parallel_interface_driver_init(void)
{
	int ret;

	ret = register_rpmsg_driver(&parallel_interface_driver);
	if (ret){
		pr_err("Failed to register parallel_interface driver on rpmsg_bus\n");
		return ret;
	}

	return 0;
}


static void __exit parallel_interface_driver_exit(void)
{
	unregister_rpmsg_driver(&parallel_interface_driver);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
