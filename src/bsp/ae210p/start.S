##==============================================================================
##
##  start.S
##
##  nds32 startup code
##
##==============================================================================
##
## Copyright (c) 2012-2018 Andes Technology Corporation
## All rights reserved.
##

#include <nds32_init.inc>
#include "config.h"
#include "interrupt.h"

##------------------------------------------------------------------------------
	.macro vector name
	.align 2
	j OS_Trap_\name
	.endm

	.section .nds32_init, "ax"
!========================================================================
! Vector table
!========================================================================
	.align 2
exception_vector:
	/* Exception vector */
	j _start                    !  (0) Trap Reset
	vector TLB_Fill             !  (1) Trap TLB fill
	vector PTE_Not_Present      !  (2) Trap PTE not present
	vector TLB_Misc             !  (3) Trap TLB misc
	vector TLB_VLPT_Miss        !  (4) Trap TLB VLPT miss
	vector Machine_Error        !  (5) Trap Machine error
	vector Debug_Related        !  (6) Trap Debug related
	vector General_Exception    !  (7) Trap General exception
	vector Syscall              !  (8) Syscall

	/* HW interrupt vector */
	vector Interrupt_UNDEF      !  (9) Interrupt HW0
        vector Interrupt_UNDEF      ! (10) Interrupt HW1
        vector Interrupt_HW2        ! (11) Interrupt HW2 (PIT)
        vector Interrupt_UNDEF      ! (12) Interrupt HW3
        vector Interrupt_UNDEF      ! (13) Interrupt HW4
        vector Interrupt_UNDEF      ! (14) Interrupt HW5
        vector Interrupt_HW6        ! (15) Interrupt HW6 (GPIO)
        vector Interrupt_UNDEF      ! (16) Interrupt HW7
        vector Interrupt_UNDEF      ! (17) Interrupt HW8
        vector Interrupt_UNDEF      ! (18) Interrupt HW9
        vector Interrupt_UNDEF      ! (19) Interrupt HW10
        vector Interrupt_HW11       ! (20) Interrupt HW11 (SWI)
        vector Interrupt_UNDEF      ! (21) Interrupt HW12
        vector Interrupt_UNDEF      ! (22) Interrupt HW13
        vector Interrupt_UNDEF      ! (23) Interrupt HW14
        vector Interrupt_UNDEF      ! (24) Interrupt HW15
        vector Interrupt_UNDEF      ! (25) Interrupt HW16
        vector Interrupt_UNDEF      ! (26) Interrupt HW17
        vector Interrupt_UNDEF      ! (27) Interrupt HW18
        vector Interrupt_UNDEF      ! (28) Interrupt HW19
        vector Interrupt_UNDEF      ! (29) Interrupt HW20
        vector Interrupt_UNDEF      ! (30) Interrupt HW21
        vector Interrupt_UNDEF      ! (31) Interrupt HW22
        vector Interrupt_UNDEF      ! (32) Interrupt HW23
        vector Interrupt_UNDEF      ! (33) Interrupt HW24
        vector Interrupt_UNDEF      ! (34) Interrupt HW25
        vector Interrupt_UNDEF      ! (35) Interrupt HW26
        vector Interrupt_UNDEF      ! (36) Interrupt HW27
        vector Interrupt_UNDEF      ! (37) Interrupt HW28
        vector Interrupt_UNDEF      ! (38) Interrupt HW29
        vector Interrupt_UNDEF      ! (39) Interrupt HW30
        vector Interrupt_UNDEF      ! (40) Interrupt HW31

	.align  2
exception_vector_end:

	.section .text

/*
 * exception handlers
 */
OS_Trap_TLB_Fill:
OS_Trap_PTE_Not_Present:
OS_Trap_TLB_Misc:
OS_Trap_TLB_VLPT_Miss:
OS_Trap_Machine_Error:
OS_Trap_Debug_Related:
OS_Trap_General_Exception:
1:	b 1b

OS_Trap_Syscall:
	SYSCALL_SAVE_ALL
	bal syscall_handler
	SYSCALL_RESTORE_ALL
	iret

/* Nested interrupt */
OS_Trap_Interrupt_HW2:
	IRQ_NESTED_SAVE_ALL
	bal pit_irq_handler
	IRQ_NESTED_RESTORE_ALL
	iret

OS_Trap_Interrupt_HW11:
	IRQ_NESTED_SAVE_ALL
	bal swi_irq_handler
	IRQ_NESTED_RESTORE_ALL
	iret

/* Non-Nested interrupt */
OS_Trap_Interrupt_HW6:
	IRQ_SAVE_ALL
	bal gpio_irq_handler
	IRQ_RESTORE_ALL
	iret

/* Undefined interrupt */
OS_Trap_Interrupt_UNDEF:
die:	b die


	! Macro to define default except/irq handlers
	.macro WEAK_DEFAULT weak_sym, default_handler
	.weak \weak_sym
	.set \weak_sym ,\default_handler
	.endm

	! Default except/irq handler
	.global default_handler
default_handler:
	b default_handler

	! Define default except handler to be weak symbol
	WEAK_DEFAULT general_except_handler,   default_handler
	WEAK_DEFAULT syscall_handler,          default_handler

	! Define default irq handler to be weak symbol.
	! They can be overwritten by other handlers
	WEAK_DEFAULT rtc_period_irq_handler,   default_handler
	WEAK_DEFAULT rtc_alarm_irq_handler,    default_handler
	WEAK_DEFAULT pit_irq_handler,          default_handler
	WEAK_DEFAULT spi1_irq_handler,         default_handler
	WEAK_DEFAULT spi2_irq_handler,         default_handler
	WEAK_DEFAULT i2c_irq_handler,          default_handler
	WEAK_DEFAULT gpio_irq_handler,         default_handler
	WEAK_DEFAULT uart1_irq_handler,        default_handler
	WEAK_DEFAULT uart2_irq_handler,        default_handler
	WEAK_DEFAULT dma_irq_handler,          default_handler
	WEAK_DEFAULT bmc_irq_handler,          default_handler
	WEAK_DEFAULT swi_irq_handler,          default_handler
	WEAK_DEFAULT ldma_irq_handler,         default_handler

##------------------------------------------------------------------------------
## Startup code

	.global _start
	.func _start
	.type _start, @function

	.align 2
_start:
	!************************** Begin of do-not-modify **************************
	! Please don't modify this code
	! Initialize the registers used by the compiler

	nds32_init		! NDS32 startup initial macro in <nds32_init.inc>

	!*************************** End of do-not-modify ***************************

	! Do system low level setup. It must be a leaf function.
	bal _nds32_init_mem

	! System initial
	bal init

#ifdef CFG_GCOV
	! Call constructor
	bal _crt_init

	! Call main function
	bal main

	! Call 'exit' to expand GCOV
	jal exit
#else
	! Call main function
	bal main
#endif

	! Infinite loop, if returned accidently
1:	b 1b

	.size _start, .-_start

	.end
