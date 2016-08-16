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
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/string.h>
#include "parallel_interface.h"

/*
 * macro to print debug info easily
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

static int pi_driver_match_device (struct pi_device *pidev,
					      struct pi_driver *pidrv)
{
	struct pi_device_id *id;
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

static int pi_core_bus_match (struct device *dev, struct device_driver *drv)
{
	struct pi_device *pidev;
	struct pi_driver *pidrv;

	log_debug();
	//log_debug_msg("dev = %s \n drv = %s",dev->init_name, drv->name);

	pidrv = to_pi_driver(drv);
	pidev = to_pi_device(dev);

	if(pi_driver_match_device(pidev, pidrv)){
		log_debug_msg("Found match\n");
		return 1;
	}

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

int pi_core_unregister_host (struct pi_bus_host *pibushost)
{
	int ret;

	log_debug();
	ret = device_for_each_child(&pibushost->dev, NULL,
				    pi_core_unregister_pidev);
	if (ret){
		dev_err(&pibushost->dev, "Couldnt unregister all childs\n");
		return ret;
	}
	device_unregister(&pibushost->dev);

	return 0;
}
EXPORT_SYMBOL_GPL(pi_core_unregister_host);

static void pi_core_pidev_release(struct device *dev)
{
	log_debug();
	put_device(dev);
}

static struct pi_device* pi_core_register_node_pidev(struct device *parent,
						struct device_node *pidev_node)
{
	int ret;
	struct pi_device *pidev;

	log_debug();
	log_debug_msg("Registerging node %s as pidevice\n",
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
	pidev->dev.release = pi_core_pidev_release;
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

static void pi_release(struct device *dev)
{
	log_debug();
	put_device(dev);
}


static int pi_rpmsg_probe(struct rpmsg_channel *rpdev)
{
	int ret;
	struct device *pi_bus;

	log_debug();

	pi_bus = devm_kzalloc(&rpdev->dev, sizeof(*pi_bus), GFP_KERNEL);
	pi_bus->release = pi_release;
	pi_bus->init_name = "parallel_interface";

	dev_set_drvdata(&rpdev->dev, pi_bus);

	ret = device_register(pi_bus);
	if (ret){
		pr_err("parallel_interface: couldnt register bus type");
		put_device(pi_bus);
		return ret;
	}

	return 0;
}

static void pi_rpmsg_remove(struct rpmsg_channel *rpdev)
{
	struct device *pi_bus;
	log_debug();
	pi_bus = dev_get_drvdata(&rpdev->dev);
	device_unregister(pi_bus);
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
