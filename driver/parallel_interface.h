/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef __PARALLEL_INTERFACE__
#define __PARALLEL_INTERFACE__

struct pi_device {
	struct device dev;
};

struct pi_driver {
	struct device_driver driver;
	int (*probe)(struct pi_device *dev);
	void (*remove)(struct pi_device *dev);
	void (*callback)(struct pi_device *, void *, int, void *, u32);
};

#define to_pi_driver(drv)\
	container_of(drv, struct pi_driver, driver);

static inline void pi_unregister_driver (struct pi_driver *pidrv){
	driver_unregister(&pidrv->driver);
}

extern int __pi_register_driver (char *name, struct module *owner,
				 struct pi_driver *pidrv);

#define pi_register_driver(drv) \
	__pi_register_driver (KBUILD_MODNAME ,THIS_MODULE, drv)

#define module_pi_driver(__pi_driver) \
	module_driver(__pi_driver, pi_register_driver, pi_unregister_driver)

#endif /*__PARALLEL_INTERFACE__*/
