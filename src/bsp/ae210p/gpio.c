/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include "platform.h"

void gpio_init(unsigned int pins)
{
	/* Set direction to input */
	DEV_GPIO->CHANNELDIR &= ~pins;

	/* Clear output */
	DEV_GPIO->DOUTCLEAR = -1;

	/* Set input mode to Negative-edge */
	DEV_GPIO->INTRMODE0 = 0x55555555;
	DEV_GPIO->INTRMODE1 = 0x55555555;
	DEV_GPIO->INTRMODE2 = 0x55555555;
	DEV_GPIO->INTRMODE3 = 0x55555555;

	/* Set De-bounce */
	DEV_GPIO->DEBOUNCECTRL = 0x800000FF;
	DEV_GPIO->DEBOUNCEEN = pins;

	/* Clear and enable interrupt */
	DEV_GPIO->INTRSTATUS = -1;
	DEV_GPIO->INTREN = pins;
}



//unsigned int gpio_read(void)
//{
//	return	(DEV_GPIO->DATAIN);
//}



void gpio_irq_clear(unsigned int pins)
{
	DEV_GPIO->INTRSTATUS = pins;
}
