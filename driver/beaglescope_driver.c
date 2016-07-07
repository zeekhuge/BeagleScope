/*
 * Copyright (c) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/rpmsg.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kfifo.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/iio/iio.h>

struct beaglescope_state {
	struct rpmsg_channel *rpdev;
};


static const struct iio_info beaglescope_info = {
};


static int beaglescope_driver_probe (struct rpmsg_channel *rpmsg_dev)
{
	int ret;
	struct iio_dev *indio_dev;
	struct beaglescope_state *st;
	struct rpmsg_device_id *id;

	dev_dbg(&rpmsg_dev->dev, "Driver probed\n");

	indio_dev = devm_iio_device_alloc(&rpmsg_dev->dev, sizeof(*st));
	if (!indio_dev) {
		ret = -ENOMEM;
		goto error_ret;
	}

	id = &rpmsg_dev->id;
	st = iio_priv(indio_dev);

	dev_set_drvdata(&rpmsg_dev->dev, indio_dev);

	indio_dev->dev.parent = &rpmsg_dev->dev;
	indio_dev->name = id->name;
	indio_dev->info = &beaglescope_info;

	ret = devm_iio_device_register(&rpmsg_dev->dev, indio_dev);
	if (ret < 0) {
		pr_err("Failed to register with iio\n");
		goto error_device_register;
	}

	return 0;

error_device_register:
error_ret:
	return ret;
}


static void beaglescope_driver_remove(struct rpmsg_channel *rpmsg_dev)
{
	struct iio_dev *indio_dev;

	indio_dev = dev_get_drvdata(&rpmsg_dev->dev);
	devm_iio_device_unregister(&rpmsg_dev->dev, indio_dev);
}


static const struct rpmsg_device_id beaglescope_id[] = {
		{ .name = "beaglescope" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, beaglescope_id);


static struct rpmsg_driver beaglescope_driver= {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= beaglescope_id,
	.probe		= beaglescope_driver_probe,
	.remove		= beaglescope_driver_remove,
};


static int __init beaglescope_driver_init(void)
{

	int ret;

	ret = register_rpmsg_driver(&beaglescope_driver);
	if (ret){
		pr_err("Failed to register beaglescope driver on rpmsg_bus\n");
		return ret;
	}

	pr_debug("Successfully registered to rpmsg_bus\n");
	return 0;

}


static void __exit beaglescope_driver_exit(void)
{
	unregister_rpmsg_driver (&beaglescope_driver);
}


module_init(beaglescope_driver_init);
module_exit(beaglescope_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_DESCRIPTION("BeagleScope Driver");
MODULE_LICENSE("GPL v2");
