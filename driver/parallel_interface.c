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

/**
 * pi_bus_type	The bus_type structure that will be used to register this
 *		driver as a bus.
 */
struct bus_type pi_bus_type = {
	.name = "parallel_interface",
	.match = pi_core_bus_match,
	.probe = pi_core_bus_probe,
	.remove = pi_core_bus_remove,
};

/**
 * pi_core_unregister_pidev	Function to unregister the given device.
 *
 * @dev		The device that needs to be unregistered.
 * @null	Any other data that needs to be given.
 *
 * The function clears the OF_POPULATED flag in the given device and then
 * unregisters it.
 */
static int pi_core_unregister_pidev (struct device *dev, void *null)
{
	log_debug();
	of_node_clear_flag(dev->of_node, OF_POPULATED);
	device_unregister(dev);

	return 0;
}

/**
 * pi_core_host_release		Function that will serve as the 'release'
 *				member function for all the pi_bus_host devices.
 *
 * @dev		The device that needs to be released.
 *
 * The function puts back the given device object, as it is always required
 * before the device is unregistered.
 */
static void pi_core_host_release(struct device *dev)
{
	log_debug();
	put_device(dev);
}

/**
 * pi_core_register_host	Function that will serve as the 'release'
 *				member function for all the pi_bus_host
 *				devices.
 *
 * @dev		The device element of the platform device that was detected by
 *		by the platform-bus driver.
 *
 * The function allocates a pi_bus_host object and initializes it with required
 * data. It further registers the device. The function returns a pointer to the
 * pi_bus_host object if successfully registered. It returns NULL otherwise.
 *
 * TODO: Add support to add more than one host device if more than one
 * compatible device tree nodes are present.
 */
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

/**
 * pi_core_unregister_host	Function to unregister the pi-host device.
 *
 * @pibushost	The pi_bus_host device that needs to be unregistered.
 *
 * The function iterates over all the child device of the given pi_bus_host
 * device, unregisters each of them. It further unregisters the pibushost
 * pibushost device.
 * The function returns 0 if all of the child device and the host device are
 * successfully unregistered, otherwise returns the error code.
 */
int pi_core_unregister_host (struct pi_bus_host *pibushost)
{
	int ret;

	log_debug();
	ret = device_for_each_child(&pibushost->dev, NULL,
				    pi_core_unregister_pidev);
	if (ret){
		dev_err(&pibushost->dev, "Couldnt unregister all child\n");
		return ret;
	}
	device_unregister(&pibushost->dev);

	return 0;
}
EXPORT_SYMBOL_GPL(pi_core_unregister_host);

/**
 * pi_core_pidev_release	Function that will serve as the 'release'
 *				member function for all the pi_device devices.
 *
 * @dev		The device that needs to be released.
 *
 * The function puts back the given device object, as it is always required
 * before the device is unregistered.
 */
static void pi_core_pidev_release(struct device *dev)
{
	log_debug();
	put_device(dev);
}

/**
 * pi_core_register_node_pidev	The function to register child device from the
 * 				given child device_node.
 *
 * @parent	The device object that needs to be set as parent to the device
 *		that will be associated with the given device_node.
 * @pidev_node	The device_node for which the device is to be created.
 *
 * The function allocates a pi_device object, associates it with the given
 * device_node and sets the given parent as its parent device. The function
 * then returns the created device if its successfully created. It returns a
 * NULL pointer otherwise.
 */
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
	pidev->dev.release = pi_core_pidev_release;
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

/**
 * pi_core_register_devices	The function to register all the child devices.
 *
 * @pibushost	The pi_bus_host device whose child device needs to be
 *		registered.
 *
 * The function allocates a pi_device object, associates it with the given
 * device_node and sets the given parent as its parent device. The function
 * then returns the 0 if all the child devices are registered successfully, and
 * 0 otherwise.
 */
int pi_core_register_devices(struct pi_bus_host *pibushost)
{
	struct pi_device *pidev;
	struct device_node *node;

	log_debug();

	if (!pibushost->dev.of_node)
		return -EINVAL;

	for_each_available_child_of_node(pibushost->dev.of_node, node) {
		if (of_node_test_and_set_flag(node, OF_POPULATED))
			continue;

		pidev = pi_core_register_node_pidev(&pibushost->dev, node);
		if (IS_ERR(pidev)){
			log_debug_msg("Couldnt register pidev\n");
			return -EINVAL;
		}
	}

	return 0;
}
EXPORT_SYMBOL_GPL(pi_core_register_devices);

/**
 * parallel_interface_driver_init	The __init function for this driver
 *
 * The function registers this driver as a bus driver.
 */
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

/**
 * parallel_interface_driver_exit	The __exit function for this driver
 *
 * The function undoes whatever is done by the corresponding __init function.
 * For this driver, it unregisters the driver as a bus driver.
 */
static void __exit parallel_interface_driver_exit(void)
{
	log_debug();
	bus_unregister(&pi_bus_type);
}

module_init(parallel_interface_driver_init);
module_exit(parallel_interface_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_LICENSE("GPL v2");
