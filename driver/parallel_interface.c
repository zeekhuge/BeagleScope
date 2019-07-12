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

/**
 * pi_core_bus_remove	Function to serve as the 'remove' function for this bus
 *			driver.
 *
 * @dev		The device that is to be removed from the pi-bus.
 *
 * The function retrieves the associated pi_device and pi_driver from the given
 * device, and invokes the remove member function of the pi_driver.
 */
static int pi_core_bus_remove (struct device *dev)
{
	struct pi_driver *pidrv;
	struct pi_device *pidev;
	log_debug();

	pidrv = to_pi_driver(dev->driver);
	pidev = to_pi_device(dev);

	pidrv->remove(pidev);

	return 0;
}

/**
 * pi_core_bus_probe	Function to serve as the 'probe' function for this bus
 *			driver.
 *
 * @dev		The device that is connected to the bus and matches with one of
 *		the driver.
 *
 * The function retrieves the associated pi_device and pi_driver from the given
 * device, and invokes the probe member function of the pi_driver. The function
 * returns 0 if the probe to the pi_driver was successful. It returns the error
 * number otherwise.
 */
static int pi_core_bus_probe (struct device *dev)
{
	int ret;
	struct pi_driver *pidrv;
	struct pi_device *pidev;
	log_debug();

	pidrv = to_pi_driver(dev->driver);
	pidev = to_pi_device(dev);

	ret = pidrv->probe(pidev);
	if (ret) {
		dev_err(dev, "pidrv probe failed\n");
		return ret;
	}

	return 0;
}

/**
 * pi_driver_match_device	Function to match given pi_device with the
 *				given pi_driver and output result if the driver
 *				can handle the device.
 *
 * @pidev	The pi-client-device that needs to be matched against given
 *		pi-client-driver.
 * @pidrv	The pi-device-driver that needs to be matched against given
 *		pi-client-device.
 *
 * The function compares the id of the given pi_driver against the modalias
 * value of the given pi_device. It further compares the name of the pi_driver
 * with the modalias of the given pi_device. If any of the match is found, the
 * function returns 1. It returns 0 otherwise.
 */
static int pi_driver_match_device (struct pi_device *pidev,
				   struct pi_driver *pidrv)
{
	const struct pi_device_id *id;
	log_debug();

	id = pidrv->id_table;
	if(id)
		while(id->name[0]){
			if(!strcmp(pidev->modalias, id->name)){
				return 1;
			}
			id++;
		}

	return (strcmp(pidev->modalias, pidrv->driver.name) == 0);

	return 0;
}

/**
 * pi_core_bus_match	Function that servers as the 'match' function for this
 *			bus driver.
 *
 * @dev		The device that needs to be matched against given driver.
 * @drv		The driver that needs to be matched against given device.
 *
 * The function retrieves the associated pi_device and the pi_driver the given
 * dev and drv, and then using pi_driver_match_device function, checks if the
 * given driver can handle the given device. It returns 1 if the device can be
 * handled, and 0 otherwise.
 */
static int pi_core_bus_match (struct device *dev, struct device_driver *drv)
{
	struct pi_device *pidev;
	struct pi_driver *pidrv;

	log_debug();

	pidrv = to_pi_driver(drv);
	pidev = to_pi_device(dev);

	if(pi_driver_match_device(pidev, pidrv)){
		log_debug_msg("Found match\n");
		dev->driver = drv;
		return 1;
	}

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
 * __pi_register_driver		Function to register a device driver for a
 *				device that would be on pi bus.
 *
 * @name	Char string name of the driver.
 * @owner	The owner module.
 * @pidrv	The pi-bus specific device-driver structure.
 *
 * The function allocates a pi_bus_host object and initializes it with required
 * data. It further registers the device. The function returns a pointer to the
 * pi_bus_host object if successfully registered. It returns NULL otherwise.
 */
int __pi_register_driver (char *name, struct module *owner,
				 struct pi_driver *pidrv)
{
	int ret = 0;

	log_debug();
	pidrv->driver.name = name;
	pidrv->driver.owner = owner;
	pidrv->driver.bus = &pi_bus_type;
	ret = driver_register(&pidrv->driver);
	if (ret)
		pr_err("parallel_interface : couldnt register device driver");

	return ret;
}
EXPORT_SYMBOL_GPL(__pi_register_driver);

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
