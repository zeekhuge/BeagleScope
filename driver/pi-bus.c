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

static int pibus_platform_probe(struct platform_device *pdev)
{
	log_debug();
	return 0;
}

static int pibus_platform_remove(struct platform_device *pdev)
{
	log_debug();
	return 0;
}

/**
 * pibus_of_id	of_device_id type structure to register this platform device.
 */
static struct of_device_id pibus_of_id[] = {
		{ .compatible = "ti,pibus0" },
		{ },
};
MODULE_DEVICE_TABLE(of, pibus_of_id);

/**
 * pi_bus_rpmsg_probe	The function that will serve as the probe function for
 *			the registered rpmsg_channel.
 *
 * @rpdev	The rpmsg channel that was created and who's name matched name
 *		value in the rpmsg_device_id structure.
 *
 * This probe function gets called when a rpmsg channel, having matching id
 * according to the rpmsg_device_id structure, is present. The probe function
 * allocates a platform_driver object, initializes it, and registers it as a
 * platform device. Thus this driver can serve as the platform driver for
 * pi-bus core only after the PRUs are booted up with associated firmware, and
 * thus this probe function is invoked.
 */
static int pi_bus_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	int ret;
	struct platform_driver *pibus_pdrv;
	log_debug();

	pibus_pdrv = devm_kzalloc(&rpdev->dev, sizeof(*pibus_pdrv), GFP_KERNEL);
	if (!pibus_pdrv){
		pr_err("pibus : failed to zalloc memory");
		return -ENOMEM;
	}

	dev_set_drvdata(&rpdev->dev, pibus_pdrv);
	pibus_pdrv->driver.name = "pibus";
	pibus_pdrv->driver.owner = THIS_MODULE;
	pibus_pdrv->driver.mod_name = KBUILD_MODNAME;
	pibus_pdrv->driver.of_match_table = pibus_of_id;
	pibus_pdrv->probe = pibus_platform_probe;
	pibus_pdrv->remove = pibus_platform_remove;

	ret = platform_driver_register(pibus_pdrv);
	if (ret){
		pr_err("pibus : unable to register platform driver");
		return ret;
	}

	return 0;
}

/**
 * pi_bus_rpmsg_remove	The function to serve as the remove function for the
 *			rpmsg channel.
 *
 * @rpdev	The rpmsg channel that was created and who's name matched name
 *		value in the rpmsg_device_id structure.
 *
 * In this function, the driver unregisters itself as a platform driver. Thus
 * the driver cannon serve as a pi-bus platform driver once the associated
 * rpmsg-channel is destroyed and thus pi_bus_rpmsg_remove is called.
 */
static void pi_bus_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	struct platform_driver *pibus_pdrv;

	log_debug();
	pibus_pdrv = dev_get_drvdata(&rpdev->dev);
	platform_driver_unregister(pibus_pdrv);
}

/**
 * pi_bus_rpmsg_id	The rpmsg_device_id type structure that is used to
 *			register the rpmsg_client driver.
 */
static const struct rpmsg_device_id pi_bus_rpmsg_id[] = {
		{ .name = "pibus0" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, pi_bus_rpmsg_id);

/**
 * pi_bus_rpmsg_driver	The rpmsg specific driver structure that used to
 *			register the driver to the rpmsg bus.
 */
static struct rpmsg_driver pi_bus_rpmsg_driver = {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= pi_bus_rpmsg_id,
	.probe		= pi_bus_rpmsg_probe,
	.remove		= pi_bus_rpmsg_remove,
};

/**
 * pi_bus_init	the __init function that will be called at the time of loading
 *		the driver.
 *
 * In this function, the driver registers itself as a client rpmsg driver.
 * Thus the rpmsg associated probe method will be invoked on creation of the
 * rpmsg channel associated with the value of id_table field of rpmsg_driver
 * object.
 */
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

/**
 * pi_bus_exit	the __exit function that will be called at the time of loading
 *		the driver.
 *
 * The driver unregisters itself as a rpmsg-client driver.
 */
static void __exit pi_bus_exit(void)
{
	log_debug();

	unregister_rpmsg_driver(&pi_bus_rpmsg_driver);
}

module_init(pi_bus_init);
module_exit(pi_bus_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
