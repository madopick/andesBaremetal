/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#include <nds32_intrinsic.h>
#include "nds32_defs.h"
#include "platform.h"

#ifdef CFG_GCOV
#include <stdio.h>
#include <stdlib.h>
#endif

#ifndef VECTOR_BASE
#define VECTOR_BASE	0x00000000
#endif

#define PSW_MSK                                         \
        (PSW_mskGIE | PSW_mskINTL | PSW_mskPOM | PSW_mskAEN | PSW_mskIFCON | PSW_mskCPL)
#define PSW_INIT                                        \
        (0x0UL << PSW_offGIE                            \
         | 0x0UL << PSW_offINTL                         \
         | 0x1UL << PSW_offPOM                          \
         | 0x0UL << PSW_offAEN                          \
         | 0x0UL << PSW_offIFCON                        \
         | 0x7UL << PSW_offCPL)

#define IVB_MSK                                         \
        (IVB_mskEVIC | IVB_mskESZ | IVB_mskIVBASE)
#define IVB_INIT                                        \
        ((VECTOR_BASE >> IVB_offIVBASE) << IVB_offIVBASE\
         | 0x0UL << IVB_offESZ                          \
         | 0x0UL << IVB_offEVIC)


#pragma weak c_startup = c_startup_null

void c_startup(void);

/*
 * Default c_startup() function which used for those relocation from LMA to VMA.
 */
static void c_startup_null(void)
{
	/* We do nothing for those LMA equal to VMA */
}

void cpu_init(void)
{
	unsigned int reg;

	/* Enable BTB & RTP since the default setting is disabled. */
	reg = __nds32__mfsr(NDS32_SR_MISC_CTL) & ~(MISC_CTL_makBTB | MISC_CTL_makRTP);
	__nds32__mtsr(reg, NDS32_SR_MISC_CTL);

	/* Set PSW GIE/INTL to 0, superuser & CPL to 7 */
	reg = (__nds32__mfsr(NDS32_SR_PSW) & ~PSW_MSK) | PSW_INIT;
#ifdef CFG_HWZOL
	/* Enable PSW AEN */
	reg |= ( 0x1 << PSW_offAEN );
#endif
	__nds32__mtsr(reg, NDS32_SR_PSW);

	/* Set PPL2FIX_EN to 0 to enable Programmable Priority Level */
	__nds32__mtsr(0x0, NDS32_SR_INT_CTRL);

	/* Set vector size: 4 byte, base: VECTOR_BASE, mode: IVIC */
	reg = (__nds32__mfsr(NDS32_SR_IVB) & ~IVB_MSK) | IVB_INIT;
	__nds32__mtsr(reg, NDS32_SR_IVB);

	/* Mask and clear hardware interrupts */
	if (reg & IVB_mskIVIC_VER) {
		/* IVB.IVIC_VER >= 1*/
		__nds32__mtsr(0x0, NDS32_SR_INT_MASK2);
		__nds32__mtsr(-1, NDS32_SR_INT_PEND2);
	} else {
		__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK) & ~0xFFFF, NDS32_SR_INT_MASK);
	}
}

/*
 * NDS32 reset handler to reset all devices sequentially and call application
 * entry function.
 */
void init(void)
{
	extern void system_init(void);

	/*
	 * Initialize LMA/VMA sections.
	 * Relocation for any sections that need to be copied from LMA to VMA.
	 */
	c_startup();

	/*
	 * Initialize CPU to a post-reset state, ensuring the ground doesn't
	 * shift under us while we try to set things up.
	 */
	cpu_init();

	/* Call platform specific hardware initialization */
	system_init();

	/* Double check ZOL supporting */
	/*
	 * Check whether the CPU configured with ZOL supported.
	 * The MSC_CFG.MSC_EXT($cr4) indicates MSC_CFG2 register exist
	 * and MSC_CFG2($cr7) bit 5 indicates ZOL supporting.
	 */
#ifdef CFG_HWZOL
	if (!((__nds32__mfsr(NDS32_SR_MSC_CFG) & (3 << 30))
	     && (__nds32__mfsr(NDS32_SR_MSC_CFG2) & (1 << 5)))) {
		/* CPU doesn't support ZOL, but build with ZOL supporting. */
		uart_puts("CPU doesn't support ZOL, but build with ZOL supporting !!\n");
		while(1);
	}
#else
	if ((__nds32__mfsr(NDS32_SR_MSC_CFG) & (3 << 30))
	    && (__nds32__mfsr(NDS32_SR_MSC_CFG2) & (1 << 5))) {
		/* CPU supports ZOL, but build without ZOL supporting. */
		uart_puts("CPU supports ZOL, but build without ZOL supporting !!\n");
		while(1);
	}
#endif
}

#ifdef CFG_GCOV
void abort(void)
{
	fflush(stdout);
	exit(1);
}
#endif
