/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */
#ifndef __GPIO_H__
#define __GPIO_H__

/*
 * Exported functions
 */
extern void gpio_init(unsigned int pins);
extern unsigned int gpio_read(void);
extern void gpio_irq_clear(unsigned int pins);

#endif	// __GPIO_H__
