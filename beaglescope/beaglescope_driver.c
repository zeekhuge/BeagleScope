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
#include <linux/iio/buffer.h>
#include <linux/iio/kfifo_buf.h>
#include <linux/iio/triggered_buffer.h>
#include <linux/iio/trigger_consumer.h>

/*
 * macro to print debug info easily
 */
#define log_debug(msg) printk(KERN_DEBUG "%s: %s\n", __FILE__, msg);

/* Configuration data, needed to be send to PRUs to get a raw sample */

#define BEAGLESCOPE_OFFSET_REF_VDD 2

#define BEAGLESCOPE_RAW_READ 0
#define BEAGLESCOPE_BLOCK_READ 1

struct beaglescope_state {
	struct rpmsg_channel *rpdev;
	struct device *dev;
	u32 pru_config[3];
	u32 raw_data;
	bool got_raw;
	int sampling_frequency ;
	wait_queue_head_t wait_list;
};

/* beaglescope_adc_channels - structure that holds information about the
   channels that are present on the adc */
static const struct iio_chan_spec beaglescope_adc_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.indexed = 1,
		.channel = 0,
		.info_mask_separate = BIT(IIO_CHAN_INFO_RAW) |
					BIT(IIO_CHAN_INFO_SCALE)|
					BIT(IIO_CHAN_INFO_OFFSET)|
					BIT(IIO_CHAN_INFO_SAMP_FREQ),
		.scan_index =0,
		.scan_type = {
			.sign = 'u',
			.realbits = 10,
			.storagebits = 16,
			.shift = 0,
			.endianness = IIO_LE,
		},
	},
};

/*
 * set_beaglescope_sampling_frequency - To set the sampling_frequency and save
 * for the current instance of the device into the beaglescope_state structure
 *
 * @st			current beaglescope state instance
 * @*val		pointer to the required sampling frequency
 *
 * Description - The function maps 3 pointers to the different regions of the
 * pru_config buffer. It then store the configuration data into appropriately
 * pointed regions. The frequency value is converted into:
 * the time period then to
 * number of pru cycles
 * it is then divided into 3 parts, 2 of which are use for the clock on time
 * (cycle_after/before_sample) and the other (cycle_between_sample) is used for
 * clock off time.
 * These 3 values need to be odd (this is PRU's requirement). Finally the
 * resultant configuration value is used to get the resultant frequency, as the
 * pru cant attain all the frequencies but a large number of frequencies. This
 * resultant frequency is what saved in the current state of the beaglescope
 */
static void set_beaglescope_sampling_frequency(struct beaglescope_state *st,
					      int *val)
{

	u32 *cycle_between_sample = st->pru_config;
	u16 *cycle_before_sample = (u16 *)&st->pru_config[1];
	u16 *cycle_after_sample = ((u16*)&st->pru_config[1])+1;
	u32 time_period_ns;
	u32 pru_cycles;
	u32 remainder;

	log_debug("set_pru_sampling_frequency");

	time_period_ns = 1000000000/(*val);
	pru_cycles = time_period_ns/5;
	pru_cycles -= 5;
	remainder = pru_cycles%4;
	pru_cycles = pru_cycles/4;
	*cycle_between_sample = 2*pru_cycles + (remainder == 3 ? 3 : 1 );
	if (pru_cycles%2){
		*cycle_after_sample = pru_cycles;
		*cycle_before_sample = pru_cycles;
	}else{
		*cycle_after_sample = pru_cycles+1;
		*cycle_before_sample = pru_cycles-1;
	}

	st->sampling_frequency = 1000000000/((*cycle_between_sample +
			   *cycle_before_sample +
			   *cycle_after_sample + 5) * 5);
	pr_err("%u, %u, %u\n", *cycle_between_sample, *cycle_before_sample,
	       *cycle_after_sample);
	 pr_err("Requested sampling freqeuency %u\n",*val);
	 pr_err("Available sampling freqeuency %u\n",st->sampling_frequency);

}


/*
 * set_beaglescope_read_mode - To set read mode into the configuration data
 *
 * @st			current beaglescope state instance
 * @read_mode		which has to be set
 *
 * Description - The function maps the appropriate pointers to the configuration
 * buffer and uses them to place set the read mode in which the pru need to be
 * activated. The function uses and configures last 32bits of the buffer. It
 * sets the read mode in the first bit of the those 32 bits and then sets the
 * enable bit, ie, the last bit in those 32bits.
 */
static void set_beaglescope_read_mode(struct beaglescope_state *st,
				      bool read_mode)
{
	bool *config_pru_read_mode = (bool *)&st->pru_config[2];
	bool *config_pru_enable_bit = ((bool *)&st->pru_config[2]) + 31;

	log_debug("set_pru_read_mode");
	st->pru_config[2]=0;
	*config_pru_read_mode = read_mode;
	*config_pru_enable_bit = 1;
	st->pru_config[2] |= 1<<31;
	printk(KERN_DEBUG "misc_config_data = %u\n",st->pru_config[2]);
}

/*
 * get_beaglescope_read_mode - To get the read mode of the current beaglescope
 * device instance
 *
 * @st		The current instance of the beaglescope state structure
 *
 * Description - The function returns the read mode to which the beaglescop is
 * set. This is basically done by reading the associated bit of the pru_config
 * buffer. It returns boolean value.
 */
static bool get_beaglescope_read_mode(struct beaglescope_state *st)
{
	return *((bool *)&st->pru_config[2]);
}

/*
 * beaglescope_read_from_pru - To start the PRUs in the required reading mode
 *
 * @indio_dev	pointer to the instance of iio device
 *
 * Description - The function starts the PRUs by sending the configuration data
 * that has been prepared by call to set_beaglescope_sampling_frequency() and
 * set_pru_read_mode() function calls. The message is then dispatched by the
 * rpmsg callback method. This function also checks if the required rpmsg device
 * has been released or not. If it has been released, the driver would return
 * with an error.
 *
 * In case the read_mode is BEAGLESCOPE_RAW_READ, this function waits for the
 * callback to interrupt, by using the wait_list and returns only after the
 * data from the callback has been saved into the raw_data field of the current
 * beaglescope_state structure.
 * If the read_mode is BEAGLESCOPE_BLOCK_READ, the function just configures the
 * PRUs and returns. The rpmsg callback method then pushes the data onto the
 * iio_buffer.
 */
static int beaglescope_read_from_pru(struct iio_dev *indio_dev)
{
	int ret;
	struct beaglescope_state *st;

	log_debug("beaglescope_read_from_pru");

	st = iio_priv(indio_dev);

	if (!st->rpdev){
		dev_err(st->dev, "Required rpmsg device has been released\n");
		return -EINVAL;
	}

	ret = rpmsg_send(st->rpdev, (void *)st->pru_config,
			    3*sizeof(u32));
	if (ret)
		dev_err(st->dev, "Failed sending config info to PRUs\n");

	if (get_beaglescope_read_mode(st) == BEAGLESCOPE_RAW_READ) {
		ret = wait_event_interruptible(st->wait_list, st->got_raw);
		if (ret)
			return -EINTR;
	}

	return 0;
}

/*
 * beaglescope_stop_sampling - to stop sampling process of the PRUS immediately
 */
static int beaglescope_stop_sampling_pru(struct iio_dev *indio_dev )
{
	int ret;
	char stop_val = 0;
	struct beaglescope_state *st;

	st = iio_priv(indio_dev);

	if (!st->rpdev){
		dev_err(st->dev, "Required rpmsg device already released\n");
		return -EINVAL;
	}

	ret = rpmsg_send(st->rpdev, (void *)&stop_val, sizeof(stop_val));
	if (ret)
		dev_err(st->dev, "failed to stop beaglescope sampling\n");

	return ret;
}

/*
 * beaglescope_buffer_postenable - function to do necessay work
 * just after the buffer gets enabled
 */
static int beaglescope_buffer_postenable(struct iio_dev *indio_dev)
{
	int ret;
	struct beaglescope_state *st;

	st = iio_priv(indio_dev);
	set_beaglescope_read_mode(st, BEAGLESCOPE_BLOCK_READ);
	ret = beaglescope_read_from_pru(indio_dev);
	log_debug("postenable");
	return ret;
}

/*
 * beaglescope_buffer_postenable - function to do necessay work
 * just before the buffer gets disabled
 */
static int beaglescope_buffer_predisable(struct iio_dev *indio_dev)
{
	log_debug("predisable");
	return beaglescope_stop_sampling_pru(indio_dev);
}

static const struct iio_buffer_setup_ops beaglescope_buffer_setup_ops = {
	.postenable = &beaglescope_buffer_postenable,
	.predisable = &beaglescope_buffer_predisable,
};

static int beaglescope_read_raw(struct iio_dev *indio_dev,
               struct iio_chan_spec const *chan,
               int *val,
               int *val2,
               long mask)
{
       int ret;
       struct beaglescope_state *st;
       log_debug("read_raw");

       st = iio_priv(indio_dev);

       switch (mask) {
       case IIO_CHAN_INFO_RAW:
		set_beaglescope_read_mode(st, BEAGLESCOPE_RAW_READ);
		ret = beaglescope_read_from_pru(indio_dev);
		if (ret){
			dev_err(st->dev, "Couldnt read raw data\n");
			return -EINVAL;
		}
	       *val = st->raw_data;
	       return IIO_VAL_INT;
	case IIO_CHAN_INFO_SCALE:
	       *val = 1;
	       return IIO_VAL_INT;
	case IIO_CHAN_INFO_OFFSET:
	       *val = - BEAGLESCOPE_OFFSET_REF_VDD;
	       return IIO_VAL_INT;
	case IIO_CHAN_INFO_SAMP_FREQ:
	       *val = st->sampling_frequency;
	       return IIO_VAL_INT;
        default:
                return -EINVAL;
       }

}

static int beaglescope_write_raw(struct iio_dev *indio_dev,
			       struct iio_chan_spec const *chan,
			       int val,
			       int val2,
			       long mask)
{
	struct beaglescope_state *st;
	st = iio_priv(indio_dev);

	log_debug("write_raw");

	switch (mask){
	case IIO_CHAN_INFO_SAMP_FREQ:
		set_beaglescope_sampling_frequency(st, &val);
		return 0;
	default:
		return -EINVAL;
	}
}

/* beaglescope_info - Structure contains constant data about the driver */
static const struct iio_info beaglescope_info = {
	.read_raw = beaglescope_read_raw,
	.write_raw = beaglescope_write_raw,
	.driver_module = THIS_MODULE,
};

/**
 * beaglescope_driver_cb() - function gets invoked each time the pru sends some
 * data.
 *
 */
static void beaglescope_driver_cb(struct rpmsg_channel *rpdev, void *data,
				  int len, void *priv, u32 src)
{
	struct beaglescope_state *st;
	struct iio_dev *indio_dev;
	u16 *dataw = data;
	int count;

	indio_dev = dev_get_drvdata(&rpdev->dev);
	st = iio_priv(indio_dev);

	if (get_beaglescope_read_mode(st) == BEAGLESCOPE_RAW_READ){
		log_debug("callback - raw mode");
		st->raw_data=*((u32 *)data);
		st->got_raw = 1;
		wake_up_interruptible(&st->wait_list);
	}else{
		for (count =0; count < len/2; count++) {
			iio_push_to_buffers(indio_dev, dataw + count);
		}
	}
}

/**
 * beaglescope_driver_probe() - function gets invoked when the rpmsg channel
 * as mentioned in the beaglescope_id table
 *
 * The function
 * - allocates space for the IIO device
 * - registers the device to the IIO subsystem
 * - exposes the sys entries according to the channels info
 */
static int beaglescope_driver_probe (struct rpmsg_channel *rpdev)
{
	int ret;
	struct iio_dev *indio_dev;
	struct beaglescope_state *st;
	struct rpmsg_device_id *id;
	struct iio_buffer *buffer;


	log_debug("probe");

	indio_dev = devm_iio_device_alloc(&rpdev->dev, sizeof(*st));
	if (!indio_dev) {
		return -ENOMEM;
	}

	id = &rpdev->id;
	st = iio_priv(indio_dev);

	st->rpdev = rpdev;
	st->dev = &rpdev->dev;

	dev_set_drvdata(&rpdev->dev, indio_dev);

	indio_dev->dev.parent = &rpdev->dev;
	indio_dev->name = id->name;
	indio_dev->info = &beaglescope_info;
	indio_dev->setup_ops = &beaglescope_buffer_setup_ops;
	indio_dev->modes = INDIO_DIRECT_MODE | INDIO_BUFFER_SOFTWARE;
	indio_dev->channels = beaglescope_adc_channels;
	indio_dev->num_channels = ARRAY_SIZE(beaglescope_adc_channels);

	buffer = devm_iio_kfifo_allocate(&indio_dev->dev);
	if (!buffer) {
		return -ENOMEM;
	}

	iio_device_attach_buffer(indio_dev, buffer);

	init_waitqueue_head(&st->wait_list);

	ret = iio_device_register(indio_dev);
	if (ret < 0) {
		pr_err("Failed to register with iio\n");
		return ret;
	}

	return 0;

}

/**
 * beaglescope_driver_remove() - function gets invoked when the rpmsg device is
 * removed
 */
static void beaglescope_driver_remove(struct rpmsg_channel *rpdev)
{
	struct iio_dev *indio_dev;

	indio_dev = dev_get_drvdata(&rpdev->dev);

	iio_device_free(indio_dev);
}

/* beaglescope_id - Structure that holds the channel name for which this driver
   should be probed */
static const struct rpmsg_device_id beaglescope_id[] = {
		{ .name = "beaglescope" },
		{ },
};
MODULE_DEVICE_TABLE(rpmsg, beaglescope_id);

/* beaglescope_driver - The structure containing the pointers to read/write
   functions to send data to the pru */
static struct rpmsg_driver beaglescope_driver= {
	.drv.name	= KBUILD_MODNAME,
	.drv.owner	= THIS_MODULE,
	.id_table	= beaglescope_id,
	.probe		= beaglescope_driver_probe,
	.callback	= beaglescope_driver_cb,
	.remove		= beaglescope_driver_remove,
};

/**
 * beaglescope_driver_init() : driver driver registration
 *
 * The initialization function gets invoked when the driver is loaded. The
 * function registers itself on the virtio_rpmsg_bus and it gets invoked when
 * the pru creates a channel named as in the beaglescope_id structure.
 */
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

/**
 * beaglescope_driver_exit() - function invoked when the driver is unloaded
 */
static void __exit beaglescope_driver_exit(void)
{
	unregister_rpmsg_driver (&beaglescope_driver);
}

module_init(beaglescope_driver_init);
module_exit(beaglescope_driver_exit);

MODULE_AUTHOR("Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>");
MODULE_DESCRIPTION("BeagleScope Driver");
MODULE_LICENSE("GPL v2");
