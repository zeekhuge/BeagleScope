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


/* Configuration data, needed to be send to PRUs to get a raw sample */
#define BEAGLESCOPE_CONFIG_RAW_READ_0	0x00000000
#define BEAGLESCOPE_CONFIG_RAW_READ_1	0x00000000
#define BEAGLESCOPE_CONFIG_RAW_READ_2	0x00000000

#define BEAGLESCOPE_CONFIG_RANDOM_BLOCK_READ_0 0x00989681
#define BEAGLESCOPE_CONFIG_RANDOM_BLOCK_READ_1 0xabcd0103
#define BEAGLESCOPE_CONFIG_RANDOM_BLOCK_READ_2 0x80000001

/*
 * macro to print debug info easily
 */
#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__);

struct pi_device pidev;

int pi_register_driver(struct pi_driver *pidrv)
{
	log_debug();
	pidrv_main = pidrv;
	return 0;
}
EXPORT_SYMBOL_GPL(pi_register_driver);

static void pi_write(struct rpmsg_channel *rpdev)
{
	int ret;
	u32 config_raw[] = { 0x00, 0x00, 0x00 };

	log_debug();

	ret = rpmsg_send(rpdev, config_raw, 3*sizeof(u32));
	if (ret)
		pr_err("Failed sending config info to PRUs\n");
}

static void parallel_interface_cb(struct rpmsg_channel *rpdev, void *data,
				  int len, void *priv, u32 src)
{
	log_debug();
}

static int parallel_interface_probe(struct rpmsg_channel *rpdev)
{
	pidev.dev=rpdev->dev;
	log_debug();

	pidrv_main->probe(pidev);
	return 0;
}

static void parallel_interface_remove(struct rpmsg_channel *rpdev)
{
	log_debug();
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
	.callback	= parallel_interface_cb,
	.remove		= parallel_interface_remove,
};

static int __init parallel_interface_driver_init(void)
{
	int ret;

	log_debug();

	ret = register_rpmsg_driver(&parallel_interface_driver);
	if (ret){
		pr_err("Failed to register parallel_interface driver on rpmsg_bus\n");
		return ret;
	}

	return 0;
}

static void __exit parallel_interface_driver_exit(void)
{
	log_debug();
	unregister_rpmsg_driver(&parallel_interface_driver);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
