/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include "platform.h"

#define BAUD_RATE(n)            (UCLKFREQ / (n) / 16)

void uart_init(unsigned int baudrate)
{
	/* Set DLAB to 1 */
	DEV_UART2->LCR |= 0x80;

	/* Set DLL for baudrate */
	DEV_UART2->DLL = (BAUD_RATE(baudrate) >> 0) & 0xff;
	DEV_UART2->DLM = (BAUD_RATE(baudrate) >> 8) & 0xff;

	/* LCR: Length 8, no parity, 1 stop bit. */
	DEV_UART2->LCR = 0x03;

	/* FCR: Enable FIFO, reset TX and RX. */
	DEV_UART2->FCR = 0x07;
}

void uart_putc(int c)
{
#define SERIAL_LSR_THRE         0x20
	while ((DEV_UART2->LSR & SERIAL_LSR_THRE) == 0) ;

	DEV_UART2->THR = c;
}

int uart_puts(const char *s)
{
	int len = 0;

	while (*s) {
		uart_putc(*s);

		if (*s == '\n')
			uart_putc('\r');
		s++;
		len++;
	}
	return len;
}

int outbyte(int c)
{
	uart_putc(c);
	if (c =='\n')
		uart_putc('\r');
	return c;
}
