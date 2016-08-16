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

struct pi_device_id {
	char name[PI_NAME_SIZE];
	kernel_ulong_t driver_data;	/* Data private to the driver */
};

struct pi_bus_host {
	struct device dev;
};
#define to_pi_bus_host(__dev)\
	container_of(__dev, struct pi_bus_ctrlr, dev);

#define PI_MODALIAS_LENGTH	PI_NAME_SIZE
struct pi_device {
	struct pi_bus_host *pibushost;
	char modalias[PI_MODALIAS_LENGTH];
	struct device dev;
};
#define to_pi_device(__dev)\
	container_of(__dev, struct pi_device, dev);

struct pi_driver {
	const struct pi_device_id *id_table;
	struct device_driver driver;
	int (*probe)(struct pi_device *dev);
	void (*remove)(struct pi_device *dev);
	void (*callback)(struct pi_device *, void *, int, void *, u32);
};
#define to_pi_driver(__drv)\
	container_of(__drv, struct pi_driver, driver);

static inline void pi_unregister_driver (struct pi_driver *pidrv){
	driver_unregister(&pidrv->driver);
}

extern int __pi_register_driver (char *name, struct module *owner,
				 struct pi_driver *pidrv);

#define pi_register_driver(__drv) \
	__pi_register_driver (KBUILD_MODNAME ,THIS_MODULE, __drv)

#define module_pi_driver(__pi_driver) \
	module_driver(__pi_driver, pi_register_driver, pi_unregister_driver)

extern int pi_core_register_devices(struct pi_bus_host *);
extern struct pi_bus_host *pi_core_register_host(struct device *dev);
extern int pi_core_unregister_host (struct pi_bus_host *pibushost);


#endif /*__PARALLEL_INTERFACE__*/
