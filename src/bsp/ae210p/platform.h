/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "config.h"
#include "nds32_defs.h"
#include "ae210p.h"
#include "uart.h"
#include "timer.h"
#include "gpio.h"

#define CFG_AE210P

/*****************************************************************************
 * Peripheral device HAL declaration
 ****************************************************************************/
#define DEV_DMA              AE210P_DMA
#define DEV_UART1            AE210P_UART1
#define DEV_UART2            AE210P_UART2
#define DEV_PIT              AE210P_PIT
#define DEV_RTC              AE210P_RTC
#define DEV_GPIO             AE210P_GPIO
#define DEV_I2C              AE210P_I2C
#define DEV_SPI1             AE210P_SPI1
#define DEV_SPI2             AE210P_SPI2

/*****************************************************************************
 * Timer HAL declaration
 ****************************************************************************/
#define IRQ_TIMER1_VECTOR    IRQ_PIT_VECTOR
#define timer_irq_handler    pit_irq_handler

/*****************************************************************************
 * Board specified
 ****************************************************************************/
#define GPIO_USED_GCOV       0x02    /* Which GPIO to do GCOV */
#define GPIO_USED_MASK       0x7F    /* Which GPIOs to use */

#ifdef __cplusplus
}
#endif

#endif	/* __PLATFORM_H__ */
