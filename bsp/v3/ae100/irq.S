! -----------------------------------------------------------------------------
! irq.S
!
! Copyright (c) 2005-2017, Andes Techonology Corporation
! All rights reserved.
! -----------------------------------------------------------------------------

#include "ae100.h"
#include "irq.inc"

!********************************************************************************************************
!	Setup HW Interrupt vector handler
!********************************************************************************************************

	.macro  SET_HWISR num, handler
	.global interrupt_HW\num
	.set    interrupt_HW\num, \handler
	.endm

	! Setup the AE100 HW interrupt vector handler
	SET_HWISR IRQ_RESERVED0_VECTOR,         interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED1_VECTOR,         interrupt_comm_vsr
	SET_HWISR IRQ_PIT_VECTOR,               interrupt_comm_vsr
	SET_HWISR IRQ_SPI1_VECTOR,               interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED4_VECTOR,         interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED5_VECTOR,         interrupt_comm_vsr
	SET_HWISR IRQ_GPIO_VECTOR,              interrupt_hipri_vsr
	SET_HWISR IRQ_UART1_VECTOR,             interrupt_comm_vsr
	SET_HWISR IRQ_UART2_VECTOR,             interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED9_VECTOR,         interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED10_VECTOR,        interrupt_comm_vsr
	!SET_HWISR IRQ_SWI_VECTOR,               interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED12_VECTOR,        interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED13_VECTOR,        interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED14_VECTOR,        interrupt_comm_vsr
	SET_HWISR IRQ_RESERVED15_VECTOR,        interrupt_comm_vsr


!********************************************************************************************************
!       Highest priority HW Interrupt vector service routine (Non-Nested interrupt)
!********************************************************************************************************

interrupt_hipri_vsr:
	! We enter here with the orginal r0~r5 is saved and
	! r0 is HW interrup vector number.

	IRQ_HIPRI_SAVE_ALL

	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la      $r1, ISR_TABLE
	lw      $r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral    $r1

	IRQ_HIPRI_RESTORE_ALL

	iret


!********************************************************************************************************
!       HW Interrupt vector service routine (Nested interrupt)
!********************************************************************************************************

interrupt_comm_vsr:
	! We enter here with the orginal r0~r5 is saved and
	! r0 is HW interrup vector number.

	IRQ_SAVE_ALL

	! allow nested now
	setgie.e

	! The entire CPU state is now stashed on the stack,
	! and the stack is also 8-byte alignment.
	! We can call C program based interrupt handler now.
	la      $r1, ISR_TABLE
	lw      $r1, [$r1+($r0<<2)]             ! ISR function pointer
	jral    $r1                             ! Call ISR

	setgie.d
	dsb

	IRQ_RESTORE_ALL

	iret


!********************************************************************************************************
!       Interrupt default handler
!********************************************************************************************************

	! Macro to define default handlers
	.macro WEAK_DEFAULT weak_sym, default_handler
	.weak \weak_sym
	.set \weak_sym ,\default_handler
	.endm

	! Default irq handler
default_irq_handler:
	b       default_irq_handler

	! Define default handler to be weak symbol.
	! They can be overwritten by other handlers

	WEAK_DEFAULT rsv0_irq_handler,          default_irq_handler
	WEAK_DEFAULT rsv1_irq_handler,          default_irq_handler
	WEAK_DEFAULT pit_irq_handler,           default_irq_handler
	WEAK_DEFAULT spi1_irq_handler,           default_irq_handler
	WEAK_DEFAULT rsv4_irq_handler,          default_irq_handler
	WEAK_DEFAULT rsv5_irq_handler,          default_irq_handler
	WEAK_DEFAULT gpio_irq_handler,          default_irq_handler
	WEAK_DEFAULT uart0_irq_handler,         default_irq_handler
	WEAK_DEFAULT uart1_irq_handler,         default_irq_handler
	WEAK_DEFAULT rsv9_irq_handler,          default_irq_handler
	WEAK_DEFAULT rsv10_irq_handler,         default_irq_handler
	WEAK_DEFAULT swi_irq_handler,           default_irq_handler
	WEAK_DEFAULT rsv12_irq_handler,         default_irq_handler
	WEAK_DEFAULT rsv13_irq_handler,         default_irq_handler
	WEAK_DEFAULT rsv14_irq_handler,         default_irq_handler
	WEAK_DEFAULT rsv15_irq_handler,         default_irq_handler


!********************************************************************************************************
!       Interrupt handler Table
!********************************************************************************************************

	.section .data
	.align  2

	! These tables contain the isr pointers used to deliver interrupts
	.global ISR_TABLE
ISR_TABLE:
	.long   rsv0_irq_handler                !  0: Reserved0
	.long   rsv1_irq_handler                !  1: Reserved1
	.long   pit_irq_handler                 !  2: PIT
	.long   spi1_irq_handler                !  3: SPI
	.long   rsv4_irq_handler                !  4: Reserved4
	.long   rsv5_irq_handler                !  5: Reserved5
	.long   gpio_irq_handler                !  6: GPIO
	.long   uart0_irq_handler               !  7: UART1
	.long   uart1_irq_handler               !  8: UART2
	.long   rsv9_irq_handler                !  9: Reserved9
	.long   rsv10_irq_handler               ! 10: Reserved10
	.long   swi_irq_handler                 ! 11: SWINT
	.long   rsv12_irq_handler               ! 12: Reserved12
	.long   rsv13_irq_handler               ! 13: Reserved13
	.long   rsv14_irq_handler               ! 14: Reserved14
	.long   rsv15_irq_handler               ! 15: Reserved15
	.long   0                               ! 16: EXT_INT_0
	.long   0                               ! 17: EXT_INT_1
	.long   0                               ! 18: EXT_INT_2
	.long   0                               ! 19: EXT_INT_3
	.long   0                               ! 20: EXT_INT_4
	.long   0                               ! 21: EXT_INT_5
	.long   0                               ! 22: EXT_INT_6
	.long   0                               ! 23: EXT_INT_7
	.long   0                               ! 24: EXT_INT_8
	.long   0                               ! 25: EXT_INT_9
	.long   0                               ! 26: EXT_INT_10
	.long   0                               ! 27: EXT_INT_11
	.long   0                               ! 28: EXT_INT_12
	.long   0                               ! 29: EXT_INT_13
	.long   0                               ! 30: EXT_INT_14
	.long   0                               ! 31: EXT_INT_15


