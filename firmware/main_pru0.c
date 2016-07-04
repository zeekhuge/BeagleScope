/*
 * Copyright (C) 2016 Zubeen Tolani <ZeekHuge - zeekhuge@gmail.com>
 *
 * The code is developed as a part of BeagleScope project
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <pru_virtqueue.h>
#include <pru_rpmsg.h>
#include <rsc_types.h>
#include "resource_table_pru0.h"
#include "common_pru_defs.h"

volatile register uint32_t __R30;
volatile register uint32_t __R31;


/* Buffer to save data send by kernel to PRU0 using RPMsg*/
uint32_t msg_from_kernel[RPMSG_BUF_SIZE/4];

/* Data structure to save data that is send by pru1 to pru0 */
struct data_from_pru1{
	uint8_t input_data[DATA_SIZE];
}sampled_data;


/* main */
void main(void)
{

	/*
	 * Variable definitions
	 *
	 * transport : transport as an object of pru_rpmsg_transport structure,
	 * wraps the virtual rings that are used to communicate. It is
	 * a required argument for many RPMsg APIS
	 *
	 * src : variable to save address of the rpmsg channel that sends data to
	 * the pru. This is the rpmsg channel on kernel side.
	 *
	 * dst : variable to save the address of rpmsg channel to which data was
	 * send by the kernel. This is the rpmsg channel on PRU side.
	 *
	 * len : variable to save length of the data send over rpmsg transport
	 * layer.
	 *
	 * status : used as a pointer to the memory location in PRU-ICSS where
	 * status of the kernel's rpmsg channel is saved. This memory location is
	 * initialized with VIRTIO_CONFIG_S_DRIVER_OK by kernel once kernel is
	 * ready to receive messages over the rpmsg channel.
	 *
	 * message_number : variable to monitor the number of messages sent by
	 * user over the char device file /dev/rpmsg30
	 *
	 * bank_to_use : variable to choose one out of the 3 banks to read data
	 * from.
	 *
	 * ptr_to_shared_mem : pointer to shared memory with 0 offset inside
	 * PRU-ICSS.
	 */
	struct pru_rpmsg_transport transport;
	uint16_t src, dst, len;
	volatile uint8_t *status;
	uint8_t message_number=0;
	uint8_t bank_to_use = SP_BANK_0;
	int32_t *ptr_to_shared_mem = (int32_t *) SHARED_MEM_ADDR;

	/*
	 * Register initialization
	 * 	Writing to an INTC register or CGF register can be done with
	 * 	the help of predefined structures. These structures are :
	 * 	CT_INTC and CT_CFG. Writing to these registers basically
	 * 	accesse the peripheral through constant-table and loads the
	 * 	indicated register with appropriate value.
	 *
	 * CT_INTC.SYSCFG_bit.STANDBY_INIT : the object is used to write data to
	 * the SYSCFG register. Writing 0 to the STANDBY_INIT section of the
	 * register opens up the OCP master port that is used for PRU to ARM
	 * communication.
	 *
	 * CT_INTC.EISR_bit.EN_SET_IDX : the object is used to write data to the
	 * EISR register. Writing an index number to the EN_SET_IDX section of the
	 * register results in enabling of that interrupt.
	 */
	CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;
	CT_INTC.EISR_bit.EN_SET_IDX = INT_P1_to_P0;

	/*
	 * Wait for the kernel to confirm that the device can be handled by the
	 * kernel virtio bus.
	 */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & 4));

	/*
	 * Initialize pru_virtqueue
	 * The rpmsg channel on the pru side is a virtio rpmsg implementation
	 * based on virtio framework. This virtual rpmsg device uses virtual
	 * queues.
	 */
	pru_virtqueue_init(&transport.virtqueue0,
			   &resourceTable.rpmsg_vring0,
			   INT_P0_to_ARM, INT_ARM_to_P0);
	pru_virtqueue_init(&transport.virtqueue1,
			   &resourceTable.rpmsg_vring1,
			   INT_P0_to_ARM, INT_ARM_to_P0);

	/*
	 * Announcement
	 * Virtio rpmsg uses name service announcements
	 * PRU can send an CREATE or DESTROY message to the kernel that destroys
	 * or creates the channel and probes the required driver.
	 */
	while (pru_rpmsg_channel(RPMSG_NS_CREATE,
				 &transport,
				 "rpmsg-pru", "Channel 30", 30)
	       != PRU_RPMSG_SUCCESS);


	/*
	 * The infinite loop
	 */
	while (1) {
		/*
		 * Part of the code that get executed on receiving INT_ARM_to_P0
		 * interrupt from ARM. This is usually when an rpmsg message is
		 * sent by the kernel to the PRU.
		 * Whenever an INT_ARM_to_P0 interrupt is encountered following
		 * steps are executed:
		 * - status of the interrupt is cleared
		 * - message from the rpmsg channel is received
		 * - message is echoed back to the kernel
		 * - message is saved in the shared memory as depending upon the
		 * message_number
		 * - value of message number is changed depending upon its
		 * current value
		 * - INT_P0_to_P1 is generated depending upon the value of
		 * message_number.
		 */
		if(__R31 & (1<<HOST_ARM_TO_PRU0_CB)) {

			CT_INTC.SICR_bit.STS_CLR_IDX = INT_ARM_to_P0;

			if (pru_rpmsg_receive(&transport,
					      &src, &dst,
					      msg_from_kernel,
					      &len
					      ) == PRU_RPMSG_SUCCESS) {

				pru_rpmsg_send(&transport,
					       dst, src,
					       &msg_from_kernel[0],
					       sizeof(int32_t));

				*(ptr_to_shared_mem + message_number) = msg_from_kernel[0];


				message_number++;
				if (message_number > 1){
					__R31 = R31_P0_to_P1;
					message_number = 0;
					bank_to_use = SP_BANK_0;
				}
			}
		}
		/* Part of the code that gets executed when INT_P1_to_P0
		 * interrupt has been generated. This is when PRU1 has
		 * transfered the sampled data onto one of the banks and want
		 * PRU0 to read it.
		 * Following steps are executed in this part of code:
		 * - Clear the status of INT_P1_to_P0 interrupt.
		 * - Acquire data from one of the 3 banks, depending upon the value
		 * of bank_to_use.
		 * - Updates value of bank_to_use variable.
		 * - Send the acquired data to the kernel and further to the user
		 * using rpmsg bus.
		 */

		if (CT_INTC.SECR0_bit.ENA_STS_31_0 & (1<<INT_P1_to_P0)){

			CT_INTC.SICR_bit.STS_CLR_IDX = INT_P1_to_P0;

			switch(bank_to_use){
				case SP_BANK_0:
					__xin(SP_BANK_0,
					      DATA_START_REGISTER_NUMBER,
					      0,
					      sampled_data) ;
				break;
				case SP_BANK_1:
					__xin(SP_BANK_1,
					      DATA_START_REGISTER_NUMBER,
					      0,
					      sampled_data) ;
				break;
				case SP_BANK_2:
					__xin(SP_BANK_2,
					      DATA_START_REGISTER_NUMBER,
					      0,
					      sampled_data) ;
			}

			bank_to_use = (bank_to_use == SP_BANK_2) ? SP_BANK_0 : bank_to_use + 1  ;

			pru_rpmsg_send(&transport,
				       dst, src,
				       sampled_data.input_data,
				       sizeof(sampled_data));
		}
	}

}
