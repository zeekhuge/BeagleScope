/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef __PARALLEL_INTERFACE__
#define __PARALLEL_INTERFACE__


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

#define PI_MODALIAS_LENGTH	32

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

extern int pi_core_register_devices(struct pi_bus_host *);
extern struct pi_bus_host *pi_core_register_host(struct device *dev);
extern int pi_core_unregister_host (struct pi_bus_host *pibushost);

#endif /*__PARALLEL_INTERFACE__*/
