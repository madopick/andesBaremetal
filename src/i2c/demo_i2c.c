#include <stdio.h>
#include "Driver_I2C.h"
#include "Driver_USART.h"
#include <string.h>

#define I2C_MASTER_MODE_TEST         1
#define I2C_SLAVE_MODE_TEST          !(I2C_MASTER_MODE_TEST)

// I2C slave tx/rx flag
typedef enum _SLAVE_TRX_FLAG_ID
{
	I2C_SLAVE_TX_NONE = 0,
	I2C_SLAVE_RX_NONE = 0,
	I2C_SLAVE_TX_START,
	I2C_SLAVE_TX_COMPLETE,
	I2C_SLAVE_RX_START,
	I2C_SLAVE_RX_COMPLETE,
}SLAVE_TRX_FLAG_ID;

// Orca EVB EEPROM I2C slave address
#define EEPROM_I2C_ADDR              0x50

// simulate I2C slave rom flash device
// sim 10 bit I2C slave address
#define SIM_ROM_I2C_ADDR             (0x3FA)
// #define SIM_ROM_I2C_ADDR          (0x60)

#define I2C_SLAVE_ADDR               SIM_ROM_I2C_ADDR

// sim I2C flash memory address 0x0 ~ 0x400
// Max memory locations available
#define EEPROM_MAX_ADDR              1024
// Max bytes to write in one step
#define EEPROM_MAX_WRITE             16

// sim delay
#define MAX_WAIT_CNT	             0
#define MAX_XFER_SZ                  256

#define MEMSET(s, c, n)              __builtin_memset ((s), (c), (n))
#define MEMCPY(des, src, n)          __builtin_memcpy((des), (src), (n))

#define MASTER_START_STR		("\n\r=====Start demo I2C master: please hardwire 2 test EVB=====\n\r")
#define SLAVE_START_STR		("\n\r=====Start demo I2C slave: please hardwire 2 test EVB=====\n\r")
#define I2C_INIT_STR                 ("\n\rI2C init .....\n\r")
#define I2C_FIFO_MASTER_TX_STR       ("\n\rI2C FIFO master tx .....\n\r")
#define I2C_FIFO_MASTER_RX_STR       ("\n\rI2C FIFO master rx .....\n\r")
#define I2C_INIT_ERR_STR             ("\n\rI2C init error.....\n\r")
#define I2C_FIFO_MASTER_TX_ERR_STR   ("\n\rI2C FIFO master tx error .....\n\r")
#define I2C_FIFO_MASTER_RX_ERRSTR    ("\n\rI2C FIFO master rx error.....\n\r")
#define I2C_FIFO_MASTER_TEST_OK_STR  ("\n\rI2C FIFO master test pass .....\n\r")
#define I2C_FIFO_MASTER_TEST_ERR_STR ("\n\rI2C FIFO master test fail.....\n\r")

#define I2C_SLAVE_INIT_STR           ("\n\rI2C slave init .....\n\r")
#define I2C_SLAVE_INIT_ERR_STR       ("\n\rI2C slave init error.....\n\r")
#define I2C_FIFO_SLAVE_TX_STR        ("\n\rI2C FIFO master tx .....\n\r")
#define I2C_FIFO_SLAVE_RX_STR        ("\n\rI2C FIFO master rx .....\n\r")

extern NDS_DRIVER_I2C Driver_I2C0;
extern NDS_DRIVER_USART Driver_USART1;
static NDS_DRIVER_I2C* pDrv_I2C = &Driver_I2C0;

#if I2C_MASTER_MODE_TEST
static volatile int uart_event_complete = 0;
static uint16_t Device_Addr = I2C_SLAVE_ADDR;
static uint8_t wr_buf[EEPROM_MAX_WRITE + 2] = {0};
static volatile uint32_t cnt = 0;
// 2 byte address since EEPROM_MAX_ADDR is 1024
static volatile uint16_t ROM_Addr = 0x0216;
// 10 byte data
static volatile uint8_t ROM_Data[10] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0, 0x12, 0x34};
static volatile uint8_t Tmp[10] = {0};
static NDS_DRIVER_USART* pDrv_USART = &Driver_USART1;

// UART Print() not recommend to be used in I2C ISR since I2C interrupt priority level is not the highest
#define Print(s)           do                                               \
                           {                                                \
                                pDrv_USART->Send(s, sizeof(s)/sizeof(char));\
                                wait_uart_complete();                       \
                           } while(0);
void uart_callback(uint32_t event)
{
	switch (event)
	{
	case NDS_USART_EVENT_RECEIVE_COMPLETE:
	case NDS_USART_EVENT_TRANSFER_COMPLETE:
	case NDS_USART_EVENT_SEND_COMPLETE:
	case NDS_USART_EVENT_TX_COMPLETE:
		// Success: continue task
		uart_event_complete = 1;
		break;
	default:
		while(1);
	}
}

void wait_uart_complete(void)
{
	while (!uart_event_complete);
	uart_event_complete = 0;
}

void usart_init(NDS_DRIVER_USART* USART)
{
	if(!USART)
	{
		return;
	}

	// Initialize the USART driver
	USART->Initialize(uart_callback);

	// Power up the USART peripheral
	USART->PowerControl(NDS_POWER_FULL);

	// Configure the USART to 38400 Bits/sec
	USART->Control(NDS_USART_MODE_ASYNCHRONOUS |
					NDS_USART_DATA_BITS_8 |
					NDS_USART_PARITY_NONE |
					NDS_USART_STOP_BITS_1 |
					NDS_USART_FLOW_CONTROL_NONE, 38400);

	// Enable Transmitter lines
	USART->Control (NDS_USART_CONTROL_TX, 1);
}

int32_t i2c_master_tx(uint16_t addr, const uint8_t* buf, uint32_t len)
{
	// 2 bytes for sim I2C flash memory address 0x0 ~ 0x400
	wr_buf[0] = (uint8_t)(addr >> 8);
	wr_buf[1] = (uint8_t)(addr & 0xFF);

	MEMCPY(&wr_buf[2], &buf[0], len);

	pDrv_I2C->MasterTransmit(Device_Addr, wr_buf, len + 2, false);
	while(pDrv_I2C->GetStatus().busy);

	if(pDrv_I2C->GetDataCount() != (len + 2))
	{
		return -1;
	}

	return 0;
}

int32_t i2c_master_rx(uint16_t addr, uint8_t* buf, uint32_t len)
{
	uint8_t a[2] = {0};

	a[0] = (uint8_t)(addr >> 8);
	a[1] = (uint8_t)(addr & 0xFF);
	// xfer_pending is true => no stop condition, still busy
	// since xfer_pending is true, no stop condition and busy bit will always be pull high
	// 10-bit slave address must set STOP bit
	// tell EEPROM which ROM address will be read
	pDrv_I2C->MasterTransmit(Device_Addr, a, 2, false);
	while(pDrv_I2C->GetStatus().busy);

	if(pDrv_I2C->GetDataCount() != 2)
	{
		return -1;
	}

	// read-request to read the data from the ROM address
	pDrv_I2C->MasterReceive(Device_Addr, buf, len, false);
	while(pDrv_I2C->GetStatus().busy);

	if(pDrv_I2C->GetDataCount() != len)
	{
		return -1;
	}

	return 0;
}

int32_t i2c_master_init()
{
	int32_t status = NDS_DRIVER_OK;

	// default slave mode
	status = pDrv_I2C->Initialize(NULL);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->Control(NDS_I2C_BUS_CLEAR, 0);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->PowerControl(NDS_POWER_FULL);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->Control(NDS_I2C_BUS_SPEED, NDS_I2C_BUS_SPEED_STANDARD);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	Device_Addr = I2C_SLAVE_ADDR;

	// simulate I2C slave rom flash device
	// sim I2C slave address
	status = pDrv_I2C->Control(NDS_I2C_OWN_ADDRESS, (Device_Addr | NDS_I2C_ADDRESS_10BIT));
	// status = pDrv_I2C->Control(NDS_I2C_OWN_ADDRESS, (Device_Addr));
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	return status;
}

int main(void)
{
	int32_t status = 0, i = 0;
	char uart_buf[0x100];

	MEMSET(&uart_buf[0], 0, sizeof(uart_buf));

	usart_init(pDrv_USART);

	Print(MASTER_START_STR);

	if(pDrv_I2C)
	{
		Print(I2C_INIT_STR);

		status = i2c_master_init();
		if(status != NDS_DRIVER_OK)
		{
			Print(I2C_INIT_ERR_STR);
			while(1);
		}

		Print(I2C_FIFO_MASTER_TX_STR);

		status = i2c_master_tx(ROM_Addr, (uint8_t*)&ROM_Data, sizeof(ROM_Data));

		Print(I2C_FIFO_MASTER_RX_STR);

		status = i2c_master_rx(ROM_Addr, (uint8_t*)&Tmp, sizeof(Tmp));

		for(i = 0; i < 10; i++)
		{
			// error hit
			if(ROM_Data[i] != Tmp[i])
			{
				Print(I2C_FIFO_MASTER_TEST_ERR_STR);
				while(1);
			}
		}

		Print(I2C_FIFO_MASTER_TEST_OK_STR);

		// exit demo
		pDrv_I2C->Uninitialize();
	}
	else
	{
		// pDrv_I2C is NULL
	}

	return 0;
}

#elif I2C_SLAVE_MODE_TEST
static volatile int uart_event_complete = 0;
// simulate I2C slave rom flash device
static volatile uint16_t flash_addr = 0;
static volatile uint8_t slave_rom_flash[EEPROM_MAX_ADDR] = {0};

static volatile int slave_tx = I2C_SLAVE_TX_NONE;
static volatile int slave_rx = I2C_SLAVE_RX_NONE;
static NDS_DRIVER_USART* pDrv_USART = &Driver_USART1;

// UART Print() not recommend to be used in I2C ISR since I2C interrupt priority level is not the highest
#define Print(s)           do                                               \
                           {                                                \
                                pDrv_USART->Send(s, sizeof(s)/sizeof(char));\
                                wait_uart_complete();                       \
                           } while(0);

void uart_callback(uint32_t event)
{
	switch (event)
	{
	case NDS_USART_EVENT_RECEIVE_COMPLETE:
	case NDS_USART_EVENT_TRANSFER_COMPLETE:
	case NDS_USART_EVENT_SEND_COMPLETE:
	case NDS_USART_EVENT_TX_COMPLETE:
		// Success: continue task
		uart_event_complete = 1;
		break;
	default:
		while(1);
	}
}

void wait_uart_complete(void)
{
	while (!uart_event_complete);
	uart_event_complete = 0;
}

void usart_init(NDS_DRIVER_USART* USART)
{
	if(!USART)
	{
		return;
	}

	// Initialize the USART driver
	USART->Initialize(uart_callback);

	// Power up the USART peripheral
	USART->PowerControl(NDS_POWER_FULL);

	// Configure the USART to 38400 Bits/sec
	USART->Control(NDS_USART_MODE_ASYNCHRONOUS |
					NDS_USART_DATA_BITS_8 |
					NDS_USART_PARITY_NONE |
					NDS_USART_STOP_BITS_1 |
					NDS_USART_FLOW_CONTROL_NONE, 38400);

	// Enable Transmitter lines
	USART->Control (NDS_USART_CONTROL_TX, 1);
}

void i2c_callback(uint32_t event)
{
	// callback function body
	if(event & NDS_I2C_EVENT_TRANSFER_INCOMPLETE)
	{
		if(event & NDS_I2C_EVENT_SLAVE_RECEIVE)
		{
			// Slave address hit is the entry for slave mode driver
			// to detect slave RX/TX action depend on master TX/RX action
			slave_rx = I2C_SLAVE_RX_START;
		}
		else if(event & NDS_I2C_EVENT_SLAVE_TRANSMIT)
		{
			// Slave address hit is the entry for slave mode driver
			// to detect slave RX/TX action depend on master TX/RX action
			slave_tx = I2C_SLAVE_TX_START;
		}
	}
	else if(event & NDS_I2C_EVENT_TRANSFER_DONE)
	{
		if(slave_rx == I2C_SLAVE_RX_START)
		{
			slave_rx = I2C_SLAVE_RX_COMPLETE;
		}

		if(slave_tx == I2C_SLAVE_TX_START)
		{
			slave_tx = I2C_SLAVE_TX_COMPLETE;
		}
	}
	else if(event & NDS_I2C_EVENT_ADDRESS_NACK)
	{
		if(slave_tx == I2C_SLAVE_TX_START)
		{
			slave_tx = I2C_SLAVE_TX_COMPLETE;
		}
	}
}

int32_t i2c_slave_tx()
{
	uint16_t r_addr = 0;

	r_addr = flash_addr;

	do
	{
		pDrv_I2C->SlaveTransmit((uint8_t*)&slave_rom_flash[r_addr], 1);
		r_addr++;

		while(pDrv_I2C->GetStatus().busy);

		// error hit
		if((r_addr - flash_addr) > MAX_XFER_SZ)
		{
			while(1);
		}
	}
	while(slave_tx != I2C_SLAVE_TX_COMPLETE);

	return 0;
}

int32_t i2c_slave_rx()
{
	uint32_t data_count = 0;
	uint8_t tmp[2] = {0};
	uint16_t w_addr = 0;

	// master device issue write-request w/ flash address,
	// then issue read-request to read flash data from the address
	flash_addr = 0;

	// A new I2C data transaction(start-addr-data-stop)
	data_count = pDrv_I2C->GetDataCount();

	// error hit
	if(data_count > MAX_XFER_SZ)
	{
		while(1);
	}

	if(data_count > 0)
	{
		if(data_count >= 2)
		{
			// the first 2 bytes are falsh address
			// high byte
			pDrv_I2C->SlaveReceive(&tmp[0], 1);
			// low byte
			pDrv_I2C->SlaveReceive(&tmp[1], 1);

			w_addr = flash_addr = ((tmp[0] << 8) | tmp[1]);
			pDrv_I2C->SlaveReceive((uint8_t*)&slave_rom_flash[w_addr], data_count - 2);
			w_addr += (data_count - 2);
		}
		else
		{
			pDrv_I2C->SlaveReceive((uint8_t*)&slave_rom_flash[0], data_count);
		}
	}
	// I2C data transaction w/o payload data
	else
	{
	}
	return 0;
}

int32_t i2c_slave_init()
{
	int32_t status = NDS_DRIVER_OK;

	// default slave mode
	status = pDrv_I2C->Initialize(i2c_callback);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->Control(NDS_I2C_BUS_CLEAR, 0);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->PowerControl(NDS_POWER_FULL);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	status = pDrv_I2C->Control(NDS_I2C_BUS_SPEED, NDS_I2C_BUS_SPEED_STANDARD);
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	// simulate I2C slave rom flash device
	// sim I2C slave address
	status = pDrv_I2C->Control(NDS_I2C_OWN_ADDRESS, (I2C_SLAVE_ADDR | NDS_I2C_ADDRESS_10BIT));
	// status = pDrv_I2C->Control(NDS_I2C_OWN_ADDRESS, (I2C_SLAVE_ADDR));
	if(status != NDS_DRIVER_OK)
	{
		return status;
	}

	return status;
}

int main(void)
{
	int32_t status = 0;
	char uart_buf[0x100];

	MEMSET(&uart_buf[0], 0, sizeof(uart_buf));

	usart_init(pDrv_USART);
	
	Print(SLAVE_START_STR);
	
	if(pDrv_I2C)
	{
		status = i2c_slave_init();
		if(status != NDS_DRIVER_OK)
		{
			while(1);
		}

		do
		{
			if(slave_rx == I2C_SLAVE_RX_COMPLETE)
			{
				// A new I2C data transaction(start-addr-data-stop)
				i2c_slave_rx();
				slave_rx = I2C_SLAVE_RX_NONE;
			}

			if(slave_tx == I2C_SLAVE_TX_START)
			{
				// A new I2C data transaction(start-addr-data-stop)
				i2c_slave_tx();
				slave_tx = I2C_SLAVE_TX_NONE;
			}
		}
		while(1);
	}
	else
	{
		// pDrv_I2C is NULL
	}

	return 0;
}
#endif
