/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */
#include <nds32_intrinsic.h>
#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <ctype.h> /* for toupper() */

#include "Driver_GPIO.h"
#include "Driver_RTC.h"
#include "Driver_USART.h"

#ifdef CFG_GCOV
#include <stdlib.h>
#endif

#define NDS32_HWINT_ID(hw)      NDS32_INT_H##hw
#define NDS32_HWINT(hw)         NDS32_HWINT_ID(hw)

#define TICK_HZ         		1

#ifdef CFG_SIMU
 #define SIMU_FACTOR    		(4)
#else
 #define SIMU_FACTOR    		(0)
#endif

#define TIMER1          		0
#define TIMER2          		1
#define TIMER3          		2


#define GPIO_SW_USED_MASK     	0xff
#define GPIO_7SEG_USED_MASK   	0xffff0000

#define GPIO_7SEG1_OFFSET     	16
#define GPIO_7SEG2_OFFSET     	24

///External HW driver
extern NDS_DRIVER_RTC 	Driver_RTC;
extern NDS_DRIVER_GPIO 	Driver_GPIO;
extern NDS_DRIVER_USART Driver_USART1;

NDS_DRIVER_GPIO*	GPIO_Dri;


static uint8_t waitDelay = 0;
static uint8_t numA = 0;
static uint8_t numB = 0;


static inline void GIE_ENABLE()
{
	__nds32__setgie_en();
}

static inline void GIE_DISABLE()
{
	__nds32__setgie_dis();
	__nds32__dsb();
}


/**************************************************************
 * @brief 	Andes Syscall handler
 * @retval 	None
 **************************************************************/
void syscall_handler(void)
{
	static int cnt = 0;
	char buf[0x100];

	sprintf(buf, "Syscall handler #%d\n", cnt);
	uart_puts(buf);

	if (++cnt < 5)
		__nds32__syscall(0x5000);
}


/**************************************************************
 * @brief 	Andes Software Int
 * @retval 	None
 **************************************************************/
void swi_irq_handler(void)
{
	uart_puts("A software interrupt occurs ...\n");

	/*
	 * Since INT_PEND is the RO register except bit:16 (SWI),
	 * set zero value to it directly to clear bit:16.
	 */
	__nds32__clr_pending_swint();
}


/**************************************************************
 * @brief Andes GPIO Int Callback
 * @retval None
 **************************************************************/
void gpio_callback(uint32_t event)
{
	switch (event)
	{
		case NDS_GPIO_EVENT_PIN0:
			uart_puts("GPIO_PIN1\r\n");
			 break;

		case NDS_GPIO_EVENT_PIN1:
			uart_puts("GPIO_PIN2\r\n");
			break;

		case NDS_GPIO_EVENT_PIN2:
		case NDS_GPIO_EVENT_PIN3:
		case NDS_GPIO_EVENT_PIN4:
		case NDS_GPIO_EVENT_PIN5:
		case NDS_GPIO_EVENT_PIN6:
		case NDS_GPIO_EVENT_PIN7:
		case NDS_GPIO_EVENT_PIN8:
		default:
			break;
	}
}




//void gpio_irq_handler(void)
//{
//	GIE_ENABLE();
//
//	uart_puts("* Enter GPIO ISR *\n");
//
//	/* Clear GPIO interrupt status */
//	gpio_irq_clear(0xFFFFFFFF);
//
//	/* This service will take 6 secs */
//	unsigned int period = (6 * (PCLKFREQ / TICK_HZ)) >> SIMU_FACTOR;
//	timer_set_period(TIMER2, period << 1);
//	timer_start(TIMER2);
//
//	while(1) {
//		unsigned int tm2_cntr = timer_read(TIMER2);
//#ifdef CFG_GCOV
//		if (!(gpio_read() & GPIO_USED_GCOV)) {
//			uart_puts("Program terminated for GCOV.\n");
//			exit(0);
//		}
//#endif
//		if (tm2_cntr > period) break;
//	}
//
//	timer_stop(TIMER2);
//
//	uart_puts("* End of GPIO ISR it takes 6 secs *\n");
//}


/// 7 segment value libs.
uint8_t seven_segment_value[14] = {
		0x3f, 	// 0.
		0x06, 	// 1.
		0xdb, 	// 2.
		0x4f, 	// 3.
		0xe6, 	// 4.
		0x6d, 	// 5.
		0xfc, 	// 6.
		0x07, 	// 7.
		0x7f, 	// 8.
		0x67,	// 9.
		0x00,	// Disable.

		0x80,	// Dot.
		0x37,	// n.
		0x71,	// f.
};


/***************************************************
 * @brief  write to 7 segment value (0 - 9).
 * @retval none
 ***************************************************/
void __attribute__((weak)) writesegment(uint8_t numA, uint8_t numB)
{
	NDS_DRIVER_GPIO *GPIO_Dri = &Driver_GPIO;
	GPIO_Dri->Write(GPIO_7SEG_USED_MASK, 1);
	GPIO_Dri->Write(seven_segment_value[numA] << GPIO_7SEG1_OFFSET, 0);
	GPIO_Dri->Write(seven_segment_value[numB] << GPIO_7SEG2_OFFSET, 0);
}



/**************************************************************
 * @brief	Timer interrupt
 * @retval 	None
 **************************************************************/
void timer_irq_handler(void)
{
	//uart_puts("* Enter Timer ISR, It comes in every 4 secs. *\r\n");

	/* Clear HW/Timer1 interrupt status */
	timer_irq_clear(TIMER1);

	GIE_ENABLE();

	waitDelay = 1;
}



/**************************************************************
 * @brief	Interrupt setup
 * @retval 	None
 **************************************************************/
static void init_interrupt(void)
{
	/* Init GPIO */
#if 0
	gpio_init(GPIO_USED_MASK);
#else
	GPIO_Dri 	= &Driver_GPIO;
	GPIO_Dri->Initialize(gpio_callback);

	/* set GPIO direction (7-segments: output, switchs: input) */
	GPIO_Dri->SetDir(GPIO_7SEG_USED_MASK, NDS_GPIO_DIR_OUTPUT);
	GPIO_Dri->SetDir(GPIO_SW_USED_MASK, NDS_GPIO_DIR_INPUT);

	/* Set GPIO interrupt mode to negative edge and enable it */
	GPIO_Dri->Control(NDS_GPIO_SET_INTR_NEGATIVE_EDGE | NDS_GPIO_INTR_ENABLE, GPIO_SW_USED_MASK);
#endif


	/* Init TIMER */
	timer_init();

	/* set GPIO priority to highest, TIMER1 priority to lowest */
	__nds32__set_int_priority(NDS32_HWINT(IRQ_GPIO_VECTOR), 0);
	__nds32__set_int_priority(NDS32_HWINT(IRQ_TIMER1_VECTOR), 3);

	/* enable HW# (timer1, GPIO & SWI) */
	__nds32__enable_int(NDS32_HWINT(IRQ_TIMER1_VECTOR));
	__nds32__enable_int(NDS32_HWINT(IRQ_GPIO_VECTOR));
	__nds32__enable_int(NDS32_HWINT(IRQ_SWI_VECTOR));

	/* Start Timer1 with interrupt enabled */
	unsigned int period = (1 * (PCLKFREQ / TICK_HZ)) >> SIMU_FACTOR;
	timer_set_period(TIMER1, period);
	timer_irq_enable(TIMER1);
	timer_start(TIMER1);
}

#define NOINLINE __attribute__((noinline))
double f3 = 0;

NOINLINE int geti()
{
	int i;
	i = 100;
	i = i + i;
	return i;   /* i is 200 */
}

NOINLINE int calc2(int a, int b, int c, int d, int e, int f, int g, int h)
{
	int i = geti();
	a = a + b + c + d + e + f + g + h + i;
	return a;   /* a = 1800 */
}

NOINLINE int calc(int a, int b, int c, int d, int e, int f, int g, int h)
{
	int i = geti();
	a = a * 2;
	b = b * 2;
	c = c * 2;
	d = d * 2;
	e = e * 2;
	f = f * 2;
	g = g * 2;
	h = h * 2;
	a = calc2(a, b, c, d, e, f, g, h) + i;
	return a;   /* a = 2000 */
}







volatile int a, b, c, d, e, f, g, h;

int main(void)
{
	printf("\n\r");
	printf("=== ISR Test ===\n\r");

	/* Enable global interrupt */
	GIE_ENABLE();

	/* This is syscall test.
	 * You can comment it if it is not necessary. */
	/* Generate system call */
	//__nds32__syscall(0x5000);

	/* Initialize interrupt */
	init_interrupt();

	/* These are software interrupt test.
	 * You can comment it if it is not necessary. */
	/* Generate software interrupt */
	__nds32__set_pending_swint();

	while (1)
	{
		/* To test if stack works normally.
		 * You can comment it if it is not
		 * necessary. */
		a= b= c= d= e= f= g= h= 100;
		a = calc(a, b, c, d, e, f, g, h);
		if ((a != 2000))
		{
			uart_puts("\nSome thing wrong!\n");
			GIE_DISABLE();
			return 1;
		}

		while(!waitDelay){

		}

		waitDelay = 0;
		uart_puts("1 secs delay.\r\n");

		if (numA < 9)
		{
			numA++;
		}
		else
		{
			numA = 0;
			if (numB < 9)
				numB++;
			else
				numB = 0;
		}

		writesegment(numA, numB);

	}
	return 0;
}



/*
 * override putchar
 */
#undef putchar
inline int putchar(int c)
{
	uart_putc(toupper(c));
	return c;
}

__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{
	int i;
	for (i = 0; i < size; i++)
		putchar(buf[i]);
}



