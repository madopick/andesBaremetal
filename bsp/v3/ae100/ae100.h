/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __AE100_H__
#define __AE100_H__

#ifndef __ASSEMBLER__
#include <inttypes.h>
#include <nds32_intrinsic.h>
#endif

/*****************************************************************************
 * System clock
 ****************************************************************************/
#define KHz                     1000
#define MHz                     1000000

#define OSCFREQ                 (20 * MHz)
#define CPUFREQ                 (20 * MHz)
#define HCLKFREQ                (CPUFREQ)
#define PCLKFREQ                (CPUFREQ)
#define UCLKFREQ                (OSCFREQ)

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#define IRQ_RESERVED0_VECTOR    0
#define IRQ_RESERVED1_VECTOR    1
#define IRQ_PIT_VECTOR          2
#define IRQ_SPI_VECTOR          3
#define IRQ_RESERVED4_VECTOR    4
#define IRQ_RESERVED5_VECTOR    5
#define IRQ_GPIO_VECTOR         6
#define IRQ_UART1_VECTOR        7
#define IRQ_UART2_VECTOR        8
#define IRQ_RESERVED9_VECTOR    9
#define IRQ_RESERVED10_VECTOR   10
#define IRQ_SWI_VECTOR          11
#define IRQ_RESERVED12_VECTOR   12
#define IRQ_RESERVED13_VECTOR   13
#define IRQ_RESERVED14_VECTOR   14
#define IRQ_RESERVED15_VECTOR   15
#define IRQ_RESERVED16_VECTOR   16
#define IRQ_RESERVED17_VECTOR   17
#define IRQ_RESERVED18_VECTOR   18
#define IRQ_RESERVED19_VECTOR   19
#define IRQ_RESERVED20_VECTOR   20
#define IRQ_RESERVED21_VECTOR   21
#define IRQ_RESERVED22_VECTOR   22
#define IRQ_RESERVED23_VECTOR   23
#define IRQ_RESERVED24_VECTOR   24
#define IRQ_RESERVED25_VECTOR   25
#define IRQ_RESERVED26_VECTOR   26
#define IRQ_RESERVED27_VECTOR   27
#define IRQ_RESERVED28_VECTOR   28
#define IRQ_RESERVED29_VECTOR   29
#define IRQ_RESERVED30_VECTOR   30
#define IRQ_RESERVED31_VECTOR   31

#ifndef __ASSEMBLER__

/*****************************************************************************
 * Device Specific Peripheral Registers structures
 ****************************************************************************/

#define __I                     volatile const	/* 'read only' permissions      */
#define __O                     volatile        /* 'write only' permissions     */
#define __IO                    volatile        /* 'read / write' permissions   */

/*****************************************************************************
 * SMU - AE100
 ****************************************************************************/
typedef struct {
	__I  unsigned int SYSVER;               /* 0x00 System ID and Revision */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__IO unsigned int BID;                  /* 0x10 Board ID */
	__IO unsigned int RSR;                  /* 0x14 Reset Status */
	     unsigned int RESERVED1;            /* 0x18 Reserved */
	__IO unsigned int CMDR;                 /* 0x1C System Command */
	__IO unsigned int ITT;                  /* 0x20 Interrupt Trigger Type */
	__IO unsigned int IVB;                  /* 0x24 Interrupt Vector Base */
} AE100_SMU_RegDef;

/*****************************************************************************
 * UARTx - AE100
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0C Reserved */
	__I  unsigned int CFG;                  /* 0x10 Hardware Configure Register */
	__IO unsigned int OSCR;                 /* 0x14 Over Sample Control Register */
	     unsigned int RESERVED1[2];         /* 0x18 ~ 0x1C Reserved */
	union {
		__IO unsigned int RBR;          /* 0x20 Receiver Buffer Register */
		__O  unsigned int THR;          /* 0x20 Transmitter Holding Register */
		__IO unsigned int DLL;          /* 0x20 Divisor Latch LSB */
	};
	union {
		__IO unsigned int IER;          /* 0x24 Interrupt Enable Register */
		__IO unsigned int DLM;          /* 0x24 Divisor Latch MSB */
	};
	union {
		__IO unsigned int IIR;          /* 0x28 Interrupt Identification Register */
		__O  unsigned int FCR;          /* 0x28 FIFO Control Register */
	};
	__IO unsigned int LCR;                  /* 0x2C Line Control Register */
	__IO unsigned int MCR;                  /* 0x30 Modem Control Register */
	__IO unsigned int LSR;                  /* 0x34 Line Status Register */
	__IO unsigned int MSR;                  /* 0x38 Modem Status Register */
	__IO unsigned int SCR;                  /* 0x3C Scratch Register */
} AE100_UART_RegDef;

/*****************************************************************************
 * PIT - AE100
 ****************************************************************************/
typedef struct {
	__IO uint32_t  CTRL;                    // PIT Channel Control Register
	__IO uint32_t  RELOAD;                  // PIT Channel Reload Register
	__IO uint32_t  COUNTER;                 // PIT Channel Counter Register
	__IO uint32_t  RESERVED[1];
} PIT_CHANNEL_REG;

typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
	__I  unsigned int CFG;                  /* 0x10 Configuration Register */
	__IO unsigned int INTEN;                /* 0x14 Interrupt Enable Register */
	__IO unsigned int INTST;                /* 0x18 Interrupt Status Register */
	__IO unsigned int CHNEN;                /* 0x1C Channel Enable Register */
	PIT_CHANNEL_REG   CHANNEL[4];           /* 0x20 ~ 0x50 Channel #n Registers */
} AE100_PIT_RegDef;

/*****************************************************************************
 * WDT - AE100
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
	__IO unsigned int CTRL;                 /* 0x10 Control Register */
	__O  unsigned int RESTART;              /* 0x14 Restart Register */
	__O  unsigned int WREN;                 /* 0x18 Write Enable Register */
	__IO unsigned int ST;                   /* 0x1C Status Register */
} AE100_WDT_RegDef;

/*****************************************************************************
 * GPIO - AE100
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__I  unsigned int CFG;                  /* 0x10 Configuration Register */
	     unsigned int RESERVED1[3];         /* 0x14 ~ 0x1c Reserved */
	__I  unsigned int DATAIN;               /* 0x20 Channel Data-in Register */
	__IO unsigned int DATAOUT;              /* 0x24 Channel Data-out Register */
	__IO unsigned int CHANNELDIR;           /* 0x28 Channel Direction Register */
	__O  unsigned int DOUTCLEAR;            /* 0x2c Channel Data-out clear Register */
	__O  unsigned int DOUTSET;              /* 0x30 Channel Data-out set Register */
	     unsigned int RESERVED2[3];         /* 0x34 ~ 0x3c Reserved */
	__IO unsigned int PULLEN;               /* 0x40 Pull Enable Register */
	__IO unsigned int PULLTYPE;             /* 0x44 Pull Type Register */
	     unsigned int RESERVED3[2];         /* 0x48 ~ 0x4c Reserved */
	__IO unsigned int INTREN;               /* 0x50 Interrupt Enable Register */
	__IO unsigned int INTRMODE0;            /* 0x54 Interrupt Mode Register (0~7) */
	__IO unsigned int INTRMODE1;            /* 0x58 Interrupt Mode Register (8~15) */
	__IO unsigned int INTRMODE2;            /* 0x5c Interrupt Mode Register (16~23) */
	__IO unsigned int INTRMODE3;            /* 0x60 Interrupt Mode Register (24~31) */
	__IO unsigned int INTRSTATUS;           /* 0x64 Interrupt Status Register */
             unsigned int RESERVED4[2];         /* 0x68 ~ 0x6c Reserved */
	__IO unsigned int DEBOUNCEEN;           /* 0x70 De-bounce Enable Register */
	__IO unsigned int DEBOUNCECTRL;         /* 0x74 De-bounce Control Register */
} AE100_GPIO_RegDef;

/*****************************************************************************
 * SPI - AE100
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__IO unsigned int TRANSFMT;             /* 0x10 SPI Transfer Format Register */
	     unsigned int RESERVED1[3];         /* 0x14 ~ 0x1c Reserved */
	__IO unsigned int TRANSCTRL;            /* 0x20 SPI Transfer Control Register */
	__IO unsigned int CMD;                  /* 0x24 SPI Command Register */
	__IO unsigned int ADDR;                 /* 0x28 SPI Address Register */
	__IO unsigned int DATA;                 /* 0x2c SPI Data Register */
	__IO unsigned int CTRL;                 /* 0x30 SPI Conrtol Register */
	__I  unsigned int STATUS;               /* 0x34 SPI Status Register */
	__IO unsigned int INTREN;               /* 0x38 SPI Interrupt Enable Register */
	__O  unsigned int INTRST;               /* 0x3c SPI Interrupt Status Register */
	__IO unsigned int TIMING;               /* 0x40 SPI Interface Timing Register */
	     unsigned int RESERVED2[3];         /* 0x44 ~ 0x4c Reserved */
	__IO unsigned int MEMCTRL;              /* 0x50 SPI Memory Access Control Register */
	     unsigned int RESERVED4[10];        /* 0x54 ~ 0x78 Reserved */
	__I  unsigned int CONFIG;               /* 0x7c Configuration Register */
} AE100_SPI_RegDef;

/*****************************************************************************
 * Memory Map
 ****************************************************************************/

#define _IO_(addr)              (addr)

#define MEM_BASE                0x00000000
#define SPIMEM_BASE             0x00800000

#define APBBRG_BASE             _IO_(0x00F00000)
#define SMU_BASE                _IO_(0x00F01000)
#define UART1_BASE              _IO_(0x00F02000)
#define UART2_BASE              _IO_(0x00F03000)
#define PIT_BASE                _IO_(0x00F04000)
#define WDT_BASE                _IO_(0x00F05000)
#define GPIO_BASE               _IO_(0x00F07000)
#define SPI_BASE                _IO_(0x00F0B000)

/*****************************************************************************
 * Peripheral declaration
 ****************************************************************************/
#define AE100_SMU               ((AE100_SMU_RegDef *)  SMU_BASE)
#define AE100_UART1             ((AE100_UART_RegDef *) UART1_BASE)
#define AE100_UART2             ((AE100_UART_RegDef *) UART2_BASE)
#define AE100_PIT               ((AE100_PIT_RegDef *)  PIT_BASE)
#define AE100_WDT               ((AE100_WDT_RegDef *)  WDT_BASE)
#define AE100_GPIO              ((AE100_GPIO_RegDef *) GPIO_BASE)
#define AE100_SPI               ((AE100_SPI_RegDef *)  SPI_BASE)

#endif	/* __ASSEMBLER__ */

#endif	/* __AE100_H__ */
