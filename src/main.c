#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h> /* for toupper() */

/* USART Driver */
extern NDS_DRIVER_USART Driver_USART1;
extern NDS_DRIVER_GPIO 	Driver_GPIO;


#define GPIO_SW_USED_MASK     0xff
#define GPIO_7SEG_USED_MASK   0xffff0000

#define GPIO_7SEG1_OFFSET     16
#define GPIO_7SEG2_OFFSET     24


static volatile char usart_event_complete = 0;

NDS_DRIVER_USART *USARTdrv 	= &Driver_USART1;
NDS_DRIVER_GPIO *GPIO_Dri 	= &Driver_GPIO;



void __attribute__((weak)) writesegment(int num);
uint8_t seven_segment_value[10] = {0x3f, 0x06, 0xdb, 0x4f, 0xe6, 0x6d, 0xfc, 0x07, 0x7f, 0x67}; // seven-segment value 0 ~ 9


static void wait_complete(void)
{
	while (!usart_event_complete);
	usart_event_complete = 0;
}

void myUSART_callback(uint32_t event)
{
    switch (event) {
    case NDS_USART_EVENT_RECEIVE_COMPLETE:
    case NDS_USART_EVENT_TRANSFER_COMPLETE:
    case NDS_USART_EVENT_SEND_COMPLETE:
    case NDS_USART_EVENT_TX_COMPLETE:
        /* Success: continue task */
        usart_event_complete = 1;
        break;

    case NDS_USART_EVENT_RX_TIMEOUT:
        while(1);
        break;

    case NDS_USART_EVENT_RX_OVERFLOW:
    case NDS_USART_EVENT_TX_UNDERFLOW:
        while(1);
        break;
    }
}

void __attribute__((weak)) writesegment(int num){
	NDS_DRIVER_GPIO *GPIO_Dri = &Driver_GPIO;
	GPIO_Dri->Write(GPIO_7SEG_USED_MASK, 1);
	GPIO_Dri->Write(seven_segment_value[num] << GPIO_7SEG1_OFFSET, 0);
	GPIO_Dri->Write(seven_segment_value[num] << GPIO_7SEG2_OFFSET, 0);
}


void gpio_callback(uint32_t event) {

	switch (event) {
		case NDS_GPIO_EVENT_PIN0:
			// Set 7-segments value to 1
			writesegment(1);
			break;
		case NDS_GPIO_EVENT_PIN1:
			// Set 7-segments value to 2
			writesegment(2);
			break;
		case NDS_GPIO_EVENT_PIN2:
			// Set 7-segments value to 3
			writesegment(3);
			break;
		case NDS_GPIO_EVENT_PIN3:
			// Set 7-segments value to 4
			writesegment(4);
			break;
		case NDS_GPIO_EVENT_PIN4:
			// Set 7-segments value to 5
			writesegment(5);
			break;
		case NDS_GPIO_EVENT_PIN5:
			// Set 7-segments value to 6
			writesegment(6);
			break;
		case NDS_GPIO_EVENT_PIN6:
			// Set 7-segments value to 7
			writesegment(7);
			break;
		case NDS_GPIO_EVENT_PIN7:
			// Set 7-segments value to 8
			writesegment(8);
			break;
		default:
			break;
	}
}


#define print(s)              do {                                                \
									USARTdrv->Send(s, sizeof(s) / sizeof(char)); \
									wait_complete();                        \
                              } while(0);


/* Application entry function */
int main(void)
{
    char cmd = 0;

#ifdef DEBUG
    NDS_DRIVER_VERSION     version;
    NDS_USART_CAPABILITIES drv_capabilities;

    version = USARTdrv->GetVersion();
    if (version.api < 0x200)   /* requires at minimum API version 2.00 or higher */
    {                          /* error handling */
        return;
    }
    drv_capabilities = USARTdrv->GetCapabilities();
    if (drv_capabilities.event_tx_complete == 0)
    {                          /* error handling */
        return;
    }
#endif

    /*Initialize the USART driver */
    USARTdrv->Initialize(myUSART_callback);
    /*Power up the USART peripheral */
    USARTdrv->PowerControl(NDS_POWER_FULL);
    /*Configure the USART to 38400 Bits/sec */
    USARTdrv->Control(NDS_USART_MODE_ASYNCHRONOUS |
                      NDS_USART_DATA_BITS_8 |
                      NDS_USART_PARITY_NONE |
                      NDS_USART_STOP_BITS_1 |
                      NDS_USART_FLOW_CONTROL_NONE, 38400);

    /* Enable Receiver and Transmitter lines */
    USARTdrv->Control (NDS_USART_CONTROL_TX, 1);
    USARTdrv->Control (NDS_USART_CONTROL_RX, 1);

    //USARTdrv->Send("\n\rPress Enter",14);
    USARTdrv->Send("\n\rPress Enter to receive a message", 34);
    wait_complete();

    // initialize GPIO
	GPIO_Dri->Initialize(gpio_callback);

	// set GPIO direction (7-segments: output, switchs: input)
	GPIO_Dri->SetDir(GPIO_7SEG_USED_MASK, NDS_GPIO_DIR_OUTPUT);
	GPIO_Dri->SetDir(GPIO_SW_USED_MASK, NDS_GPIO_DIR_INPUT);

	// Set switchs interrupt mode to negative edge and enable it
	GPIO_Dri->Control(NDS_GPIO_SET_INTR_NEGATIVE_EDGE | NDS_GPIO_INTR_ENABLE, GPIO_SW_USED_MASK);


	print("\n\r====== ANDES BAREMETAL PROJECT ======\n\r");
	printf("start main loop here\r\n");

    while (1)
    {
        USARTdrv->Receive(&cmd, 1);          /* Get byte from UART */
        wait_complete();
        if (cmd == 13)                       /* CR, send greeting  */
        {
          USARTdrv->Send("\n\rHello World!", 14);
          wait_complete();
        }
    }

    return 0;
}



/*
 * override putchar
 */
#undef putchar
inline int putchar(int c)
{
	USARTdrv->Send(&c, 1);
	wait_complete();

	return c;
}

__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{
	int i;
	for (i = 0; i < size; i++)
		putchar(buf[i]);
}


