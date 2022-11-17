#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "Driver_SPI.h"
#include "Driver_USART.h"

#define SPI_MASTER_MODE     1
#define SPI_SLAVE_MODE      !(SPI_MASTER_MODE)

// SPI commands
#define SPI_READ    0x0b
#define SPI_WRITE   0x51
#define SPI_DUMMY   0xff

#define TEST_DATA_SIZE            8
#define TOTAL_TRANSFER_SIZE       (TEST_DATA_SIZE + 2) // Total transfer size is cmd(1) + dummy(1) + data(TEST_DATA_SIZE)

#define print(s)          do {                                                \
                                USART_Dri->Send(s, sizeof(s) / sizeof(char)); \
                                wait_usart_complete();                        \
                          } while(0);

static volatile char spi_event_transfer_complete = 0;
static volatile int usart_event_complete = 0;

extern NDS_DRIVER_SPI Driver_SPI1;
extern NDS_DRIVER_USART Driver_USART1;

void wait_usart_complete(void) {
	while (!usart_event_complete);
	usart_event_complete = 0;
}

void usart_callback(uint32_t event) {
	switch (event) {
		case NDS_USART_EVENT_RECEIVE_COMPLETE:
		case NDS_USART_EVENT_TRANSFER_COMPLETE:
		case NDS_USART_EVENT_SEND_COMPLETE:
		case NDS_USART_EVENT_TX_COMPLETE:
			// success: continue task
			usart_event_complete = 1;
			break;
		default:
			while(1);
	}
}

void usart_init(NDS_DRIVER_USART *USART_Dri) {
	if (!USART_Dri)
		return;

	// initialize the USART driver
	USART_Dri->Initialize(usart_callback);

	// power up the USART peripheral
	USART_Dri->PowerControl(NDS_POWER_FULL);

	// configure the USART to 38400 Bits/sec
	USART_Dri->Control(NDS_USART_MODE_ASYNCHRONOUS |
			   NDS_USART_DATA_BITS_8 |
			   NDS_USART_PARITY_NONE |
			   NDS_USART_STOP_BITS_1 |
			   NDS_USART_FLOW_CONTROL_NONE, 38400);

	// enable transmitter lines
	USART_Dri->Control(NDS_USART_CONTROL_TX, 1);
}

#if SPI_MASTER_MODE
static void spi_prepare_data_in(uint8_t *data_in, uint8_t cmd, uint32_t dummy_cnt, uint8_t *w_data, uint32_t wcnt) {
	uint8_t *ptr = data_in;
	uint32_t i;

	// set 1-byte command
	*ptr++ = cmd;

	// set n-byte dummy
	for (i = 0; i < dummy_cnt; i++)
		*ptr++ = SPI_DUMMY;

	// set n-byte w_data
	for (i = 0; i < wcnt; i++)
		*ptr++ = *w_data++;
}
#endif

static void wait_complete(void) {
	while (!spi_event_transfer_complete);
	spi_event_transfer_complete = 0;
}

void spi_callback(uint32_t event) {
	switch (event) {
		case NDS_SPI_EVENT_TRANSFER_COMPLETE:
			spi_event_transfer_complete = 1;
			break;
		case NDS_SPI_EVENT_DATA_LOST:
			while(1);
			break;
	}
}

#if SPI_MASTER_MODE
int main(void) {
	uint8_t data_in[TOTAL_TRANSFER_SIZE] = {0};
	uint8_t data_out[TOTAL_TRANSFER_SIZE] = {0};
	uint8_t w_data[TEST_DATA_SIZE] = {0};
	uint32_t i;

	NDS_DRIVER_SPI *SPI_Dri = &Driver_SPI1;
	NDS_DRIVER_USART *USART_Dri = &Driver_USART1;

	// initialize UART
	usart_init(USART_Dri);

	print("\n\r==== start demo SPI ====\n\r");

	// initialize SPI
	SPI_Dri->Initialize(spi_callback);

	// power up the SPI peripheral
	SPI_Dri->PowerControl(NDS_POWER_FULL);

	// configure the SPI to master, 8-bit data length and bus speed to 1MHz
	SPI_Dri->Control(NDS_SPI_MODE_MASTER |
			 NDS_SPI_CPOL0_CPHA0 |
			 NDS_SPI_MSB_LSB     |
			 NDS_SPI_DATA_BITS(8), 1000000);

	print("\n\rmaster write/read test...");

	// prepare write data
	for (i = 0; i < TEST_DATA_SIZE; i++)
		w_data[i] = i;

	spi_prepare_data_in(data_in,SPI_WRITE , 1, w_data, TEST_DATA_SIZE);

	// write data to slave
	SPI_Dri->Send(data_in, TOTAL_TRANSFER_SIZE);
	wait_complete();

        for (i = 0; i < TEST_DATA_SIZE+2; i++)
                data_in[i] = 0;

	// read data from slave
	spi_prepare_data_in(data_in,SPI_READ, 1, 0, 0);

	SPI_Dri->Transfer(data_out, data_in, TOTAL_TRANSFER_SIZE);
	wait_complete();

	// check data
	for (i = 0; i < TEST_DATA_SIZE; i++) {
		if (data_out[i + 2] != w_data[i]) {
			print("check data FAIL!!\n\r");
			while (1);
		}
	}

	print("PASS!!\n\r");

	return 0;
}
#elif SPI_SLAVE_MODE
int main(void) {
	uint8_t data[TOTAL_TRANSFER_SIZE] = {0};

	NDS_DRIVER_SPI *SPI_Dri = &Driver_SPI1;
        NDS_DRIVER_USART *USART_Dri = &Driver_USART1;

        usart_init(USART_Dri);

	print("\n\r==== start demo SPI ====\n\r");
	
	// initialize SPI
	SPI_Dri->Initialize(spi_callback);

	// power up the SPI peripheral
	SPI_Dri->PowerControl(NDS_POWER_FULL);

	// configure the SPI to slave, 8-bit data length
	SPI_Dri->Control(NDS_SPI_MODE_SLAVE  |
			 NDS_SPI_CPOL0_CPHA0 |
			 NDS_SPI_MSB_LSB     |
			 NDS_SPI_DATA_BITS(8), 0);

	print("\n\rslave write/read test...");
	
	// read data from master
	SPI_Dri->Receive(data, TOTAL_TRANSFER_SIZE);
	wait_complete();

	// write data to master
	SPI_Dri->Send(data, TOTAL_TRANSFER_SIZE);
	wait_complete();

	return 0;
}
#endif
