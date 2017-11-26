/*
 * main.cpp
 *
 *  Created on: Nov 23, 2017
 *      Author: Vinay Divakar
 */

/* Standard Includes */
#include <unistd.h>           /* Symbolic Constants */
#include <sys/types.h>        /* Primitive System Data Types */
#include <errno.h>            /* Errors */
#include <stdio.h>            /* Input/Output */
#include <stdlib.h>           /* General Utilities */
#include <pthread.h>          /* POSIX Threads */
#include <string.h>           /* String handling */
#include <semaphore.h>        /* Semaphore */
#include <cstdio>

/* Custom Includes */
#include "SPI.h"
#include "AD7190.h"

/* Objects */
AD7190 ad7190_obj1;
SPI spi_device1;

/* Global - Shared Variable */
unsigned long adc_data_g = 0x00;

/* Semaphore for Synchronization */
sem_t sem_adc;

void* get_adc_data(void*)
{
    unsigned long temp_dat = 0x00;
    while (1) {
        sem_wait(&sem_adc);
        temp_dat = ad7190_obj1.AD7190_read_data();
        if (temp_dat != 0x00)
            adc_data_g = temp_dat;
        sem_post(&sem_adc);
    }
}

void* print_adc_data(void*)
{
    while (1) {
        sem_wait(&sem_adc);
        printf("%lu\r\n", adc_data_g);
        sem_post(&sem_adc);
    }
}

int main()
{
    /* Initialize SPI */
    if (spi_device1.SPI_DEV1_init(SPIDEV_BYTES_NUM, SPIDEV1_BUS_SPEED_HZ, SPI_SS_LOW,
            SPIDEV_DELAY_US, SPIDEV_DATA_BITS_NUM,
            SPI_MODE3)
        == -1)
        printf("(Main)spidev1.0 initialization failed\r\n");

    else
        printf("(Main)spidev1.0 initialized - READY\r\n");

    ad7190_obj1.AD7190_reset();
    ad7190_obj1.AD7190_configure(CONFIG_REG_CMD_MSB, CONFIG_REG_CMD_MID, CONFIG_REG_CMD_LSB);
    ad7190_obj1.AD7190_mode(MODE_REG_CMD_MSB, MODE_REG_CMD_MID, MODE_REG_CMD_LSB);
    ad7190_obj1.AD7190_dump_regs(DUMP_CONFIG_AND_MODE_REG_CONTENTS);

    /* Lets use threads - Just for fun ;) */
    pthread_t Thread_get_data, Thread_print_data;

    /* Initialize the semaphore - Binary */
    sem_init(&sem_adc, 0, 1);

    /* Create thread-1 for reading ADC data */
    if (pthread_create(&Thread_get_data, NULL, get_adc_data, NULL)) {
        printf("\n ERROR creating thread 1");
        exit(1);
    }

    /* Create thread-2 for printing the read ADC data */
    if (pthread_create(&Thread_print_data, NULL, print_adc_data, NULL)) {
        printf("\n ERROR creating thread 2");
        exit(1);
    }

    /* wait for the thread 1 to finish */
    if (pthread_join(Thread_get_data, NULL)) {
        printf("\n ERROR joining thread");
        exit(1);
    }

    /* wait for the thread 2 to finish */
    if (pthread_join(Thread_print_data, NULL)) {
        printf("\n ERROR joining thread");
        exit(1);
    }

    pthread_exit(0); /* exit thread */
}
