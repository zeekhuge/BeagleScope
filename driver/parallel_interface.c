/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include "parallel_interface.h"

/**
 * Helper macro to print debug info easily.
 * These are temporary and will be removed/replaced in the final version.
 */
#define log_debug() printk(KERN_DEBUG "[%s] %s\n", __this_module.name, \
			      __FUNCTION__)
#define log_debug_msg(...) printk(KERN_DEBUG __VA_ARGS__ )

static int pi_core_bus_remove (struct device *dev)
{
	log_debug();
	return 0;
}

static int pi_core_bus_probe (struct device *dev)
{
	log_debug();
	return 0;
}

static int pi_core_bus_match (struct device *dev, struct device_driver *drv)
{
	log_debug();
	return 0;
}

struct bus_type pi_bus_type = {
	.name = "parallel_interface",
	.match = pi_core_bus_match,
	.probe = pi_core_bus_probe,
	.remove = pi_core_bus_remove,
};

static int pi_core_unregister_pidev (struct device *dev, void *null)
{
	log_debug();
	of_node_clear_flag(dev->of_node, OF_POPULATED);
	device_unregister(dev);

	return 0;
}

static void pi_core_host_release(struct device *dev)
{
	log_debug();
	put_device(dev);
}

struct pi_bus_host *pi_core_register_host(struct device *dev)
{
	int error;
	struct pi_bus_host *pibushost;

	log_debug();

	pibushost = devm_kzalloc(dev, sizeof(*pibushost), GFP_KERNEL);
	if (IS_ERR(pibushost)){
		dev_err(dev, "Failed to allocate pibushost\n");
		goto return_from_register_host;
	}

	dev_set_drvdata(dev, pibushost);
	pibushost->dev.init_name = "pi-0";
	pibushost->dev.bus = &pi_bus_type;
	pibushost->dev.parent = dev;
	pibushost->dev.of_node = dev->of_node;
	pibushost->dev.release = pi_core_host_release;

	error = device_register(&pibushost->dev);
	if (error) {
		dev_err(dev, "Failed to register the host\n");
		goto free_host;
	}
	return pibushost;

free_host:
put_device(&pibushost->dev);
kfree(&pibushost);
return_from_register_host:
return NULL;
}
EXPORT_SYMBOL_GPL(pi_core_register_host);

int pi_core_unregister_host (struct pi_bus_host *pibushost)
{
	log_debug();
	device_unregister(&pibushost->dev);

	return 0;
}
EXPORT_SYMBOL_GPL(pi_core_unregister_host);

static struct pi_device* pi_core_register_node_pidev(struct device *parent,
						struct device_node *pidev_node)
{
	int ret;
	struct pi_device *pidev;

	log_debug();
	log_debug_msg("Registering node %s as pi-device\n",
		      pidev_node->full_name);

	pidev = devm_kzalloc(parent, sizeof(*pidev), GFP_KERNEL);

	ret = of_modalias_node(pidev_node, pidev->modalias,
			       sizeof(pidev->modalias));
	if(ret){
		dev_err(parent,"Couldn't get modalias\n");
		goto free_alloc_pidev;
	}

	pidev->dev.parent = parent;
	pidev->dev.bus = &pi_bus_type;
	pidev->dev.of_node = of_node_get(pidev_node);
	pidev->dev.init_name = "pidev";

	ret = device_register(&pidev->dev);
	if(ret){
		dev_err(parent,"Couldn't register device\n");
		goto put_device_and_free;
	}

	return pidev;

put_device_and_free:
put_device(&pidev->dev);
free_alloc_pidev:
devm_kfree(parent, pidev);
return NULL;
}

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
	log_debug();
	bus_unregister(&pi_bus_type);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
