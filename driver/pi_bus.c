/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

/*
 * macro to print debug info easily
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/rpmsg.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include "parallel_interface.h"

#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__)
#define log_debug_msg(...) printk(KERN_DEBUG __VA_ARGS__ )

static int pibus_platform_probe(struct platform_device *pdev)
{
	int ret;
	struct pi_bus_host *pibushost;

	log_debug();
	pibushost = pi_core_register_host(&pdev->dev);
	if (IS_ERR(pibushost)) {
		dev_err(&pdev->dev, "Couldnt register pi-host\n");
		return -EINVAL;
	}
	platform_set_drvdata(pdev, pibushost);
	ret = pi_core_register_devices(pibushost);
	if (ret){
		dev_err(&pdev->dev, "Couldnt register device\n");
		return ret;
	}
	return 0;
}

static int pibus_platform_remove(struct platform_device *pdev)
{
	struct pi_bus_host * pibushost;
	log_debug();

	pibushost = platform_get_drvdata(pdev);
	pi_core_unregister_host(pibushost);

	return 0;
}

static struct of_device_id pibus_of_id[] = {
		{ .compatible = "ti,pibus0" },
		{ },
};
MODULE_DEVICE_TABLE(of, pibus_of_id);

static int pi_bus_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	int ret;
	struct platform_driver *pibus_pdrv;
	log_debug();

	pibus_pdrv = devm_kzalloc(&rpdev->dev, sizeof(*pibus_pdrv), GFP_KERNEL);
	if (!pibus_pdrv){
		pr_err("pibus : failed to zalloc memmory");
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

static void pi_bus_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	struct platform_driver *pibus_pdrv;

	log_debug();
	pibus_pdrv = dev_get_drvdata(&rpdev->dev);
	platform_driver_unregister(pibus_pdrv);
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
