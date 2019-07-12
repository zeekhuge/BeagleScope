/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef __PARALLEL_INTERFACE__
#define __PARALLEL_INTERFACE__

#include <linux/types.h>
#include <linux/uuid.h>

#define PI_NAME_SIZE	32
#define PI_MODULE_PREFIX "pi:"

/**
 * pi_device_id		Structure to be used in a device-driver to supply the
 *			device id used to match a device.
 *
 * @name		The name of the device that needs to be matched with
 *			the driver.
 * @driver_data		The device specific data that will be needed by the
 *			device driver once its matched.
 */
struct pi_device_id {
	char name[PI_NAME_SIZE];
	kernel_ulong_t driver_data;
};

/**
 * pi_bus_host		The structure to be used for bus host device.
 *
 * @dev		The device member of the structure.
 */
struct pi_bus_host {
	struct device dev;
};

/**
 * to_pi_bus_host	Helper macro to get the pi_bus_host device from its
 *			member device object.
 *
 * @__dev	Pointer to the device associated with a pi_bus_host device.
 */
#define to_pi_bus_host(__dev)\
	container_of(__dev, struct pi_bus_ctrlr, dev);

#define PI_MODALIAS_LENGTH	PI_NAME_SIZE

/**
 * pi_device	The structure to be used for a client device on the pi bus.
 *
 * @pibushost		The bus host device to which the client device is
 *			connected.
 * @modalias		The modalias value associated with the device node.
 * @dev			The member device of the pi_device structure.
 */
struct pi_device {
	struct pi_bus_host *pibushost;
	char modalias[PI_MODALIAS_LENGTH];
	struct device dev;
};

/**
 * to_pi_device		Helper macro to get the pi_device object from its
 *			member device object.
 *
 * @__dev	Pointer to the device associated with pi_device object.
 */
#define to_pi_device(__dev)\
	container_of(__dev, struct pi_device, dev);

/**
 * pi_driver	The structure to be used by the device-driver to register
 *		itself as a device driver onto the pi-bus.
 *
 * @id_table	The table containing ids of the device supported by the device
 *		driver.
 * @driver	The device_driver member of the pi_driver structure.
 * @probe	The device driver probe method.
 * @remove	The device driver remove method.
 * @callback	The device driver callback method that will be called when
 *		there is some data from the device for the driver.
 */
struct pi_driver {
	const struct pi_device_id *id_table;
	struct device_driver driver;
	int (*probe)(struct pi_device *dev);
	void (*remove)(struct pi_device *dev);
	void (*callback)(struct pi_device *, void *, int, void *, u32);
};

/**
 * to_pi_driver		Helper macro to get the pi_driver object from its
 *			member device_driver object.
 *
 * @__drv	Pointer to the device_driver associated with a pi_driver
 *		object.
 */
#define to_pi_driver(__drv)\
	container_of(__drv, struct pi_driver, driver);

/**
 * pi_unregister_driver		Function that can be used to unregister an
 *				already registered device driver.
 *
 * @pidrv	The pi_driver structure associated with the device driver.
 */
static inline void pi_unregister_driver (struct pi_driver *pidrv){
	driver_unregister(&pidrv->driver);
}

extern int __pi_register_driver (char *name, struct module *owner,
				 struct pi_driver *pidrv);

/**
 * pi_register_driver	Helper macro to register the device driver onto the
 *			pi-bus. The macro should only be called when there is
 *			something else to be done in the driver __init
 *			function. In simpler cases, the macro module_pi_driver
 *			can be used.
 *
 * @__drv	The pi_driver associated with the device driver.
 */
#define pi_register_driver(__drv) \
	__pi_register_driver (KBUILD_MODNAME ,THIS_MODULE, __drv)

/**
 * module_pi_driver	Helper macro for drivers that don't do anything special
 *			in module init/exit. This eliminates a lot of
 *			boilerplate. Each module may only use this macro once,
 *			and calling it replaces module_init() and
 *			module_exit().
 *
 * @__pi_driver		the pi-bus specific device driver structure of
 *			pi_driver type.
 */
#define module_pi_driver(__pi_driver) \
	module_driver(__pi_driver, pi_register_driver, pi_unregister_driver)

extern int pi_core_register_devices(struct pi_bus_host *);
extern struct pi_bus_host *pi_core_register_host(struct device *dev);
extern int pi_core_unregister_host (struct pi_bus_host *pibushost);

#endif /*__PARALLEL_INTERFACE__*/
