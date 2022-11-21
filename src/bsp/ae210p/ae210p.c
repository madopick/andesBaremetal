/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include "platform.h"

/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((naked));
void _nds32_init_mem(void)
{
	/* Enable DLM */
	__nds32__mtsr_dsb(EDLM_BASE | 0x1, NDS32_SR_DLMB);
}

/* Overwrite c_startup weak function */
void c_startup(void)
{
#define MEMCPY(des, src, n)     __builtin_memcpy ((des), (src), (n))
#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))
	/* Data section initialization */
	extern char __data_lmastart, __data_start, _edata;
	extern char __bss_start, _end;
	unsigned int size = &_edata - &__data_start;

	/* Copy data section from LMA to VMA */
	MEMCPY(&__data_start, &__data_lmastart, size);

	/* Clear bss section */
	size = &_end - &__bss_start;
	MEMSET(&__bss_start, 0, size);
}

/*
 * All AE210P system initialization
 */
void system_init(void)
{
	uart_init(38400);
}
