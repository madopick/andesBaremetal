/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */
#ifndef __TIMER_H__
#define __TIMER_H__

/*
 * Exported functions
 */
extern void timer_init(void);
extern void timer_start(unsigned int tmr);
extern void timer_stop(unsigned int tmr);
extern unsigned int timer_read(unsigned int tmr);
extern void timer_set_period(unsigned int tmr, unsigned int period);
extern void timer_irq_enable(unsigned int tmr);
extern void timer_irq_disable(unsigned int tmr);
extern void timer_irq_clear(unsigned int tmr);
extern unsigned int timer_irq_status(unsigned int tmr);

#endif	// __TIMER_H__
