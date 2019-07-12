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

/* The value written by kernel into the status field of rpmsg.vdev
 * structure, indicating that the rpmsg device is accepted by the
 * virtio bus
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4

/* RPMsg channel details
 * Settings for the  RPMsg channel that is represented by PRU0
 *
 * RPMSG_CHAN_NAME : The name of the channel. Each channel has s
 * specific name, that decides the controlling driver on the
 * kernel side.
 *
 * RPMSG_CHAN_PORT : Port number of the rpmsg channel.
 *
 * RPMSG_CHAN_DESC : description of the rpmsg channel.
 */
#define RPMSG_CHAN_NAME			"beaglescope"
#define RPMSG_CHAN_PORT			30
#define RPMSG_CHAN_DESC			"Channel 30"

/*
 * The Constant return value used to indicate that a 44 bytes
 * block of data has has been written to the 440 byte buffer,
 * but no kick has yet been generated.
 */
#define PRU_RPMSG_MSG_ADDED	123

volatile register uint32_t __R30;
volatile register uint32_t __R31;


/* Buffer to save data send by kernel to PRU0 using RPMsg*/
uint32_t msg_from_kernel[RPMSG_BUF_SIZE/4];

/* Buffer array to save data that is send by pru1 to pru0 */
uint8_t sampled_data[DATA_SIZE];

/*
 * The variables that are used by pru_rpmsg_send_large_buffer()
 * but are declared globally so as to retain their value between
 * various repeated calls to the pru_rpmsg_send_large_buffer()
 */
uint16_t counter=0;
struct pru_rpmsg_hdr   *msg;
uint32_t               msg_len;
int16_t                head;
struct pru_virtqueue   *virtqueue;

/*
 * The pru_rpmsg_hdr structure to store information about the
 * message related to its source and destination.
 * The definition of the structure has been taken from the
 * implementation of the pru_rpmsg_send()
 */
struct pru_rpmsg_hdr {
       uint32_t        src;
       uint32_t        dst;
       uint32_t        reserved;
       uint16_t        len;
       uint16_t        flags;
       uint8_t data[0];
};

/*
 * The pru_rpmsg_send_large_buffer()
 * The function writes '*data' to the buffer which ultimately gets kicked
 * to the main processing unit. Data to this buffer is written in blocks of
 * 'len' bytes and the buffer gets kicked as soon as the written data size
 * increases or equals 440 bytes.
 * The function is actually derived from the implementation of
 * pru_rpmsg_send() function.
 */
int16_t pru_rpmsg_send_large_buffer(
    struct pru_rpmsg_transport *transport,
    uint32_t                                   src,
    uint32_t                                   dst,
    void                                       *data,
    uint16_t                                   len
)
{

       /*
        * The length of our payload is larger than the maximum RPMsg buffer si
e
        * allowed
        */

       if (counter == 0) {
               virtqueue = &transport->virtqueue0;
               head = pru_virtqueue_get_avail_buf(virtqueue, (void **)&msg, &msg_len);

               if (head < 0)
                       return PRU_RPMSG_NO_BUF_AVAILABLE;
       }

       /* Copy local data buffer to the descriptor buffer address */
       memcpy(msg->data + counter, data, len);
       counter += len;

       if (counter < 440) {
               return PRU_RPMSG_MSG_ADDED;
       }

       msg->len = counter;
       msg->dst = dst;
       msg->src = src;
       msg->flags = 0;
       msg->reserved = 0;

       /* Add the used buffer */
       if (pru_virtqueue_add_used_buf(virtqueue, head, msg_len) < 0)
               return PRU_RPMSG_INVALID_HEAD;

       /* Kick the ARM host */
       pru_virtqueue_kick(virtqueue);
       counter = 0;
       return PRU_RPMSG_SUCCESS;
}


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

	 * pru1_switch : the pointer used as a switch to turn off and on the
	 * sampling process. The pointer points to the starting of the
	 * msg_from_kernel buffer.
	 *
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
	 *
	 * read_mode : pointer to the msg_from_kernel buffer, to extract the
	 * read mode
	 *
	 * raw_data : the variable to save single sample data in RAW_READ mode
	 *
	 * bank_to_use : variable to choose one out of the 3 banks to read data
	 * from.
	 *
	 * ptr_to_shared_mem : pointer to shared memory with 0 offset inside
	 * PRU-ICSS.
	 */
	struct pru_rpmsg_transport transport;
	uint8_t *pru1_switch = (uint8_t *)msg_from_kernel;
	uint16_t src, dst, len;
	volatile uint8_t *status;
	uint8_t *read_mode = (uint8_t *)&msg_from_kernel[2];
	uint32_t raw_data;
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

	/* Clear the status of the registers that will be used in this programs
	   As they have been un-serviced in the last software tun */
	CT_INTC.SICR_bit.STS_CLR_IDX = INT_ARM_to_P0;
	CT_INTC.SICR_bit.STS_CLR_IDX = INT_P1_to_P0;

	/*
	 * Wait for the kernel to confirm that the device can be handled by the
	 * kernel virtio bus.
	 */
	status = &resourceTable.rpmsg_vdev.status;
	while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

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
				 RPMSG_CHAN_NAME, RPMSG_CHAN_DESC, RPMSG_CHAN_PORT)
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

				if (len < 12){
					if (*(pru1_switch) == 0){
						msg_from_kernel[2] &= (uint32_t)
							(1 << 0);
					} else {
						msg_from_kernel[2] |= (uint32_t)
							(1 << 31);
					}

					*(ptr_to_shared_mem + 2) = msg_from_kernel[2];
				}else{
					*(ptr_to_shared_mem) = msg_from_kernel[0];
					*(ptr_to_shared_mem + 1) = msg_from_kernel[1];
					*(ptr_to_shared_mem + 2) = msg_from_kernel[2];
				}


				__R31 = R31_P0_to_P1;
				bank_to_use = SP_BANK_0;
			}
		}
		/* Part of the code that gets executed when INT_P1_to_P0
		 * interrupt has been generated. This is when PRU1 has
		 * transfered the sampled data onto one of the banks and want
		 * PRU0 to read it.
		 * Following steps are executed in this part of code:
		 * - Clear the status of INT_P1_to_P0 interrupt.
		 * - Checks the read mode
		 * - In case the read mode is RAW_READ, it reads single value
		 * from SP_BANK_0 bank, and sends that to the kernel
		 * - In case read mode is BLOCK_READ :
		 * - Acquire data from one of the 3 banks, depending upon the value
		 * of bank_to_use.
		 * - Updates value of bank_to_use variable.
		 * - Send the acquired data to the kernel and further to the user
		 * using rpmsg bus.
		 */

		if (CT_INTC.SECR0_bit.ENA_STS_31_0 & (1<<INT_P1_to_P0)){

			CT_INTC.SICR_bit.STS_CLR_IDX = INT_P1_to_P0;

			if (*read_mode == RAW_READ) {
				__xin(SP_BANK_0,
				      DATA_START_REGISTER_NUMBER,
				      0,
				      raw_data);

				pru_rpmsg_send(&transport,
						dst, src,
						&raw_data,
						sizeof(raw_data));
			}else{

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

				pru_rpmsg_send_large_buffer(&transport,
					       dst, src,
					       sampled_data,
					       DATA_SIZE);
			}
		}
	}

}
