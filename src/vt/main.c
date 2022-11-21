/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */
#include <nds32_intrinsic.h>
#include "config.h"
#include "platform.h"
#include <stdio.h>

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


/**************************************************************
 * @brief	Timer interrupt
 * @retval 	None
 **************************************************************/
void timer_irq_handler(void)
{
	uart_puts("* Enter Timer ISR, It comes in every 4 secs. *\r\n");

	/* Clear HW/Timer1 interrupt status */
	timer_irq_clear(TIMER1);

	GIE_ENABLE();
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
	unsigned int period = (4 * (PCLKFREQ / TICK_HZ)) >> SIMU_FACTOR;
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

volatile int global = 5;
volatile int global_bss;
volatile int a, b, c, d, e, f, g, h;

int main(void)
{
	uart_puts("\nISR in C example\n");

	/* Enable global interrupt */
	GIE_ENABLE();

	if (global !=5) {
		uart_puts("data section copy failed.\n");
		while (1) ;
	}
	uart_puts("data section copy successfully.\n");
	if (global_bss != 0) {
		uart_puts("bss section clear failed.\n");
		while (1) ;
	}
	uart_puts("bss section clean successfully.\n");

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
	}
	return 0;
}
