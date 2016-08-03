/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

//struct pi_device_id {}


struct pi_device {
	struct device		dev;
}

static inline void pi_set_drvdata( struct pi_device *pidev, void *data )
{
	dev_set_drvdata(&spi->dev, data);
}

static inline void *pi_get_drvdata(struct pi_device *pidev)
{
	return dev_get_drvdata(&pidev->dev);
}


struct pi_driver {
	//const struct pi_device_id *id_table;
	int (*probe)(struct pi_device *pidev);
	int (*remove)(struct pi_device *pidev);
	void (*shutdown)(struct pi_device *pidev);
	struct device_driver driver;
};
