
#include "Driver_GPIO.h"
#include "Driver_USART.h"

// GPIO driver
extern NDS_DRIVER_GPIO Driver_GPIO;
extern NDS_DRIVER_USART Driver_USART1;

#define GPIO_SW_USED_MASK     0xff
#define GPIO_7SEG_USED_MASK   0xffff0000

#define GPIO_7SEG1_OFFSET     16
#define GPIO_7SEG2_OFFSET     24

#define print(s)              do {                                                \
                                    USART_Dri->Send(s, sizeof(s) / sizeof(char)); \
                                    wait_usart_complete();                        \
                              } while(0);

static volatile int usart_event_complete = 0;
void __attribute__((weak)) writesegment(int num);
uint8_t seven_segment_value[10] = {0x3f, 0x06, 0xdb, 0x4f, 0xe6, 0x6d, 0xfc, 0x07, 0x7f, 0x67}; // seven-segment value 0 ~ 9

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

int main(void) {
	NDS_DRIVER_GPIO *GPIO_Dri = &Driver_GPIO;
	NDS_DRIVER_USART *USART_Dri = &Driver_USART1;

	// initialize UART
	usart_init(USART_Dri);

	print("\n\r==== start demo GPIO ====\n\r");

	// initialize GPIO
	GPIO_Dri->Initialize(gpio_callback);

	// set GPIO direction (7-segments: output, switchs: input)
	GPIO_Dri->SetDir(GPIO_7SEG_USED_MASK, NDS_GPIO_DIR_OUTPUT);
	GPIO_Dri->SetDir(GPIO_SW_USED_MASK, NDS_GPIO_DIR_INPUT);

	// Set switchs interrupt mode to negative edge and enable it
	GPIO_Dri->Control(NDS_GPIO_SET_INTR_NEGATIVE_EDGE | NDS_GPIO_INTR_ENABLE, GPIO_SW_USED_MASK);

	print("\n\rPlease press SW1-SW7 and seven-segments will show the corresponding value...\n\r");

	return 0;
}
