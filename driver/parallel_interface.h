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

extern struct pi_bus_host *pi_core_register_host(struct device *dev);
extern int pi_core_unregister_host (struct pi_bus_host *pibushost);

#endif /*__PARALLEL_INTERFACE__*/
