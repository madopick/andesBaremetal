/*
 * Copyright (c) 2012-2018 Andes Technology Corporation
 * All rights reserved.
 *
 */

#ifndef __AE3XX_H__
#define __AE3XX_H__

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
#define CPUFREQ                 (60 * MHz)
#define HCLKFREQ                (CPUFREQ)
#define PCLKFREQ                (CPUFREQ / 2)
#define UCLKFREQ                (15 * MHz)

/*****************************************************************************
 * IRQ Vector
 ****************************************************************************/
#define IRQ_RTCPERIOD_VECTOR    0
#define IRQ_RTCALARM_VECTOR     1
#define IRQ_PIT_VECTOR          2
#define IRQ_SPI1_VECTOR         3
#define IRQ_SPI2_VECTOR         4
#define IRQ_I2C_VECTOR          5
#define IRQ_GPIO_VECTOR         6
#define IRQ_UART1_VECTOR        7
#define IRQ_UART2_VECTOR        8
#define IRQ_DMA_VECTOR          9
#define IRQ_RESERVED10_VECTOR   10
#define IRQ_SWI_VECTOR          11
#define IRQ_LDMA_VECTOR         12
#define IRQ_PFM_VECTOR	        13
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

#define __I                     volatile const  /* 'read only' permissions      */
#define __O                     volatile        /* 'write only' permissions     */
#define __IO                    volatile        /* 'read / write' permissions   */

/*****************************************************************************
 * SMU - AE3XX
 ****************************************************************************/
typedef struct {
	__I  unsigned int SYSVER;               /* 0x00 System ID and Revision */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__IO unsigned int BID;                  /* 0x10 Board ID */
	__IO unsigned int RSR;                  /* 0x14 Reset Status */
	     unsigned int CRR;                  /* 0x18 Reserved */
	__IO unsigned int CMDR;                 /* 0x1C System Command */
	__IO unsigned int ITT;                  /* 0x20 Interrupt Trigger Type */
	     unsigned int RESERVED1[2];         /* 0x24 ~ 0x28 Reserved */
	__IO unsigned int PCR;                  /* 0x2C Platform Configuration */
	     unsigned int RESERVED2[8];         /* 0x30 ~ 0x4C Reeserved */
	__IO unsigned int IVB;                  /* 0x50 Interrupt Vector Base */
} AE3XX_SMU_RegDef;

/*****************************************************************************
 * DMAC - AE3XX
 ****************************************************************************/
typedef struct {
	__IO uint32_t  CTRL;                    // DMA Channel Control Register
	__IO uint32_t  TRANSIZE;                // DMA Channel Total Transfer Size Register
	__IO uint32_t  SRCADDRL;                // DMA Channel Source Address(low part) Register
        __IO uint32_t  SRCADDRH;                // DMA Channel Source Address(high part) Register
	__IO uint32_t  DSTADDRL;                // DMA Channel Destination Address Register(low part)
        __IO uint32_t  DSTADDRH;                // DMA Channel Destination Address Register(high part)
	__IO uint32_t  LLPL;                    // DMA Channel Linked List Pointer Register(low part)
	__IO uint32_t  LLPH;                    // DMA Channel Linked List Pointer Register(high part)
} DMA_CHANNEL_REG;

typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0C Reserved */
	__I  unsigned int DMACFG;               /* 0x10 DMA Configure Register */
	     unsigned int RESERVED1[3];         /* 0x14 ~ 0x1C Reserved */
	__IO unsigned int DMACTRL;              /* 0x20 DMA Control Register */
	__O  unsigned int CHABORT;              /* 0x24 DMA Channel Abort Register */
	     unsigned int RESERVED2[2];         /* 0x28 ~ 0x2C Reserved */
	__IO unsigned int INTSTATUS;            /* 0x30 Interrupt Status Register */
	__I  unsigned int CHEN;			/* 0x34 Channel Enable Register*/
	     unsigned int RESERVED3[2];         /* 0x38 ~ 0x3C Reserved */
	DMA_CHANNEL_REG   CHANNEL[9];           /* 0x40 ~ 0x54 Channel #n Registers */
} AE3XX_DMA_RegDef;

/*****************************************************************************
 * UARTx - AE3XX
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
} AE3XX_UART_RegDef;

/*****************************************************************************
 * PIT - AE3XX
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
} AE3XX_PIT_RegDef;

/*****************************************************************************
 * WDT - AE3XX
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
	__IO unsigned int CTRL;                 /* 0x10 Control Register */
	__O  unsigned int RESTART;              /* 0x14 Restart Register */
	__O  unsigned int WREN;                 /* 0x18 Write Enable Register */
	__IO unsigned int ST;                   /* 0x1C Status Register */
} AE3XX_WDT_RegDef;

/*****************************************************************************
 * RTC - AE3XX
 ****************************************************************************/
typedef struct _AE3XX_RTC_RegDef
{
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
	__IO unsigned int CNTR;                 /* 0x10 Counter Register */
	__IO unsigned int ALARM;                /* 0x14 Alarm Register */
	__IO unsigned int CTRL;                 /* 0x18 Control Register */
	__IO unsigned int STATUS;               /* 0x1C Status Register */
} AE3XX_RTC_RegDef;

/*****************************************************************************
 * GPIO - AE3XX
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__I  unsigned int CFG;                  /* 0x10 Configuration Register */
	     unsigned int RESERVED1[3];         /* 0x14 ~ 0x1c Reserved */
	__I  unsigned int DATAIN;               /* 0x20 Channel Data-in Register */
	__IO unsigned int DATAOUT;              /* 0x24 Channel Data-out Register */
	__IO unsigned int CHANNELDIR;           /* 0x28 Channel Direction Register */
	__O  unsigned int DOUTCLEAR;            /* 0x2c Channel Data-out Clear Register */
	__O  unsigned int DOUTSET;              /* 0x30 Channel Data-out Set Register */
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
} AE3XX_GPIO_RegDef;

/*****************************************************************************
 * I2C - AE3XX
 ****************************************************************************/
typedef struct _AE3XX_I2C_RegDef
{
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED[3];          /* 0x04 ~ 0x0C Reserved */
	__I  unsigned int CFG;                  /* 0x10 Configuration Register */
	__IO unsigned int INTEN;                /* 0x14 Interrupt Enable Register */
	__IO unsigned int STATUS;               /* 0x18 Status Register */
	__IO unsigned int ADDR;                 /* 0x1C Address Register */
	__IO unsigned int DATA;                 /* 0x20 Data Register */
	__IO unsigned int CTRL;                 /* 0x24 Control Register */
	__IO unsigned int CMD;                  /* 0x28 Command Register */
	__IO unsigned int SETUP;                /* 0x2C Setup Register */
} AE3XX_I2C_RegDef;

/*****************************************************************************
 * SPIx - AE3XX
 ****************************************************************************/
typedef struct {
	__I  unsigned int IDREV;                /* 0x00 ID and Revision Register */
	     unsigned int RESERVED0[3];         /* 0x04 ~ 0x0c Reserved */
	__IO unsigned int TRANSFMT;             /* 0x10 SPI Transfer Format Register */
	__IO unsigned int DIRECTIO;             /* 0x14 SPI Direct IO Control Register */
	     unsigned int RESERVED1[2];         /* 0x18 ~ 0x1c Reserved */
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
	     unsigned int RESERVED3[3];         /* 0x54 ~ 0x5c Reserved */
	__IO unsigned int SLVST;                /* 0x60 SPI Slave Status Register */
	__I  unsigned int SLVDATACNT;           /* 0x64 SPI Slave Data Count Register */
	     unsigned int RESERVED4[5];         /* 0x68 ~ 0x78 Reserved */
	__I  unsigned int CONFIG;               /* 0x7c Configuration Register */
} AE3XX_SPI_RegDef;

/*****************************************************************************
 * Memory Map
 ****************************************************************************/

#define _IO_(addr)              (addr)

#define DRAM_BASE               0x00000000
#define SPIMEM_BASE             0x80000000
#define SRAM_BASE               0x90000000

#define BMC_BASE                _IO_(0xC0000000)
#define AHBDEC                  _IO_(0xE0000000)
#define APBBRG_BASE             _IO_(0xF0000000)
#define SMU_BASE                _IO_(0xF0100000)
#define UART1_BASE              _IO_(0xF0200000)
#define UART2_BASE              _IO_(0xF0300000)
#define PIT_BASE                _IO_(0xF0400000)
#define WDT_BASE                _IO_(0xF0500000)
#define RTC_BASE                _IO_(0xF0600000)
#define GPIO_BASE               _IO_(0xF0700000)
#define I2C_BASE                _IO_(0xF0A00000)
#define SPI1_BASE               _IO_(0xF0B00000)
#define DMAC_BASE               _IO_(0xF0C00000)
#define SPI2_BASE               _IO_(0xF0F00000)

/*****************************************************************************
 * Peripheral declaration
 ****************************************************************************/
#define AE3XX_SMU               ((AE3XX_SMU_RegDef *)  SMU_BASE)
#define AE3XX_UART1             ((AE3XX_UART_RegDef *) UART1_BASE)
#define AE3XX_UART2             ((AE3XX_UART_RegDef *) UART2_BASE)
#define AE3XX_PIT               ((AE3XX_PIT_RegDef *)  PIT_BASE)
#define AE3XX_WDT               ((AE3XX_WDT_RegDef *)  WDT_BASE)
#define AE3XX_RTC               ((AE3XX_RTC_RegDef *)  RTC_BASE)
#define AE3XX_GPIO              ((AE3XX_GPIO_RegDef *) GPIO_BASE)
#define AE3XX_I2C               ((AE3XX_I2C_RegDef *)  I2C_BASE)
#define AE3XX_SPI1              ((AE3XX_SPI_RegDef *)  SPI1_BASE)
#define AE3XX_SPI2              ((AE3XX_SPI_RegDef *)  SPI2_BASE)
#define AE3XX_DMA               ((AE3XX_DMA_RegDef *)  DMAC_BASE)

#endif	/* __ASSEMBLER__ */

#endif	/* __AE3XX_H__ */
