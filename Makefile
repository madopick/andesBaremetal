# -----------------------------------------------------------------------------
# Copyright (c) 2018, Andes Technology Corporation
# All rights reserved.
# -----------------------------------------------------------------------------
ifneq ($(MAKECMDGOALS),clean)

PLAT_LIST := AE100 AE210P AE3XX AE250 AE350

$(if $(filter ${PLAT},${PLAT_LIST}),, \
	$(error PLAT ${PLAT} invalid or undefined, should be one of [ ${PLAT_LIST} ]))

ifneq ($(filter $(PLAT), AE350 AE250), $(PLAT))

DEMO ?= UART

# ADDR: 4GB, 16MB
ADDR ?= 4GB

ifneq ($(filter $(ADDR), 4GB 16MB),$(ADDR))
 $(error ADDRing mode "$(ADDR)" is not supported!)
endif

### Compiler definitions

CROSS_COMPILE ?= nds32le-elf-

CC		= $(CROSS_COMPILE)gcc
C++		= $(CROSS_COMPILE)g++
AS		= $(CROSS_COMPILE)gcc
LD		= $(CROSS_COMPILE)gcc
AR		= $(CROSS_COMPILE)ar
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

ifeq ($(DEBUG),1)
OPTIM   := -O0 -g3
else
OPTIM   := -Os -g3
endif

### Define the directories
BUILDDIR 	= build
BSPDIR 		= bsp
DRVDIR 		= driver

ifneq ($(filter $(PLAT), AE210P AE100 AE3XX),$(PLAT))
$(error Unknown PLAT "$(PLAT)" is not supported!)
endif

ifneq ($(filter $(DEMO), UART GPIO RTC I2C SPI PWM WDT),$(DEMO))
$(error Unknown Demo function "$(DEMO)" is not supported!)
endif

ifeq ($(PLAT),AE100)
ifeq ($(filter $(DEMO), I2C RTC),$(DEMO))
$(error Demo Function "$(DEMO)" is not supported by AE100!)
endif
endif

PLATNAME 	= $(shell echo $(PLAT) | tr A-Z a-z)
PLATDIR 	= $(BSPDIR)/v3/$(PLATNAME)

DEMONAME 	= $(shell echo $(DEMO) | tr A-Z a-z)


ifeq ($(filter $(DEMO), UART),$(DEMO))
DEMODIR 	= src
else
DEMODIR 	= src/$(DEMONAME)
endif


### Make variables

DEFINES 	= -D$(PLAT)

ifneq ($(shell echo | $(CC) -E -dM - | grep __NDS32_ISA_V3M__ > /dev/null && echo V3M),V3M)
ifeq ($(USE_CACHEWB), 1)
DEFINES += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITEBACK
endif
ifeq ($(USE_CACHEWT), 1)
DEFINES += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITETHROUGH
endif
endif

ifeq ($(USE_FLASHEXEC), 1)
	DEFINES += -DCFG_FLASHEXEC
endif


### Define the source files we have
# BSP source files
STARTSRCS 	= $(BSPDIR)/v3/start.S
BSPSRCS 	= $(STARTSRCS) $(BSPDIR)/v3/startup-nds32.c $(BSPDIR)/v3/cache.c
BSPSRCS 	+= $(PLATDIR)/$(PLATNAME).c $(PLATDIR)/irq.S $(PLATDIR)/timer.c

# Driver source files
ifeq ($(PLAT),AE100)
DRV_SRCS = usart_$(PLATNAME).c gpio_$(PLATNAME).c spi_$(PLATNAME).c pwm_$(PLATNAME).c wdt_$(PLATNAME).c
else
DRV_SRCS = 	usart_$(PLATNAME).c \
	   		gpio_$(PLATNAME).c \
	   		spi_$(PLATNAME).c \
	   		pwm_$(PLATNAME).c \
	   		rtc_$(PLATNAME).c \
	   		i2c_$(PLATNAME).c \
	   		dma_$(PLATNAME).c \
	   		wdt_$(PLATNAME).c
endif

DRVSRCS = ${addprefix $(DRVDIR)/v3/$(PLATNAME)/, $(DRV_SRCS)}

# Demo source files
DEMO_UART_SRCS = \
		main.c \
	

DEMO_GPIO_SRCS 	= demo_gpio.c
DEMO_RTC_SRCS 	= demo_rtc.c
DEMO_I2C_SRCS 	= demo_i2c.c
DEMO_PWM_SRCS 	= demo_pwm.c
DEMO_SPI_SRCS 	= demo_spi.c
DEMO_WDT_SRCS 	= demo_wdt.c
DEMOSRCS 		= ${addprefix $(DEMODIR)/,$(DEMO_$(DEMO)_SRCS)}
SRCS 			= $(BSPSRCS) $(DEMOSRCS) $(DRVSRCS)

DRVOBJS 	= $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${DRVSRCS})))
OBJS 		= $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS})))

BUILD_DRVLIB_OBJS 	= ${addprefix $(BUILDDIR)/, $(DRVOBJS)}
BUILD_OBJS 			= ${addprefix $(BUILDDIR)/, $(OBJS)}

### Verbosity control. Use 'make V=1' to get verbose builds.

ifeq ($(V),1)
TRACE_CC  =
TRACE_C++ =
TRACE_LD  =
TRACE_AR  =
TRACE_AS  =
Q=
else
TRACE_CC  = @echo "  CC       " $<
TRACE_C++ = @echo "  C++      " $<
TRACE_LD  = @echo "  LD       " $@
TRACE_AR  = @echo "  AR       " $@
TRACE_AS  = @echo "  AS       " $<
Q=@
endif

### Compilation options

LDSCRIPT = $(PLATDIR)/$(PLATNAME).ld

INCLUDE_PATH = -I$(BSPDIR)/v3 -I$(PLATDIR) -I$(DRVDIR)/include -I$(DRVDIR)/v3/$(PLATNAME)/config -I$(DRVDIR)/v3/$(PLATNAME)

CFLAGS = $(INCLUDE_PATH) \
         $(DEFINES) \
         -Wall $(OPTIM) -fno-builtin -ffunction-sections -fdata-sections \
         -fno-strict-aliasing -funroll-loops \
         -fno-delete-null-pointer-checks \
         -DCFG_$(ADDR) \
         $(CMODEL)

LDFLAGS = $(OPTIM) -static -nostartfiles -Wl,--gc-sections $(CMODEL)
ASFLAGS = -D__ASSEMBLY__ $(CFLAGS) -c

### Retarget for execute in FLASH

ifeq ($(PLAT),AE210P)
ifeq ($(USE_FLASHEXEC), 1)
LDFLAGS += -Wl,--section-start=.nds32_init=800000
DEFINES += -DVECTOR_BASE=0x00800000
endif
endif

ifeq ($(PLAT),AE100)
ifeq ($(USE_FLASHEXEC), 1)
LDFLAGS += -Wl,--section-start=.nds32_init=800000
DEFINES += -DVECTOR_BASE=0x00800000
endif
endif

ifeq ($(PLAT),AE3XX)
ifeq ($(USE_FLASHEXEC), 1)
LDFLAGS += -Wl,--section-start=.nds32_init=80000000
DEFINES += -DVECTOR_BASE=0x80000000
endif
endif

### Automatic dependency generation

DEPDIR = $(BUILDDIR)
DEPFLAGS = -MMD -MP -MF $(DEPDIR)/$*.d

### Compilation rules

.SUFFIXES : %.o %.c %.cpp %.S

$(BUILDDIR)/%.o : %.c $(DEPDIR)/%.d
	$(TRACE_CC)
	$(Q)$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : %.cpp $(DEPDIR)/%.d
	$(TRACE_C++)
	$(Q)$(C++) $(DEPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : %.S
	$(TRACE_CC)
	$(Q)$(CC) $(DEPFLAGS) $(ASFLAGS) -c $< -o $@

$(DEPDIR)/%.d:
	@if test ! -d $(dir $@); then \
		mkdir -p $(dir $@); \
	fi

all: $(DEMO).elf $(DEMO).bin

clean:
	@rm -rf build
	
$(DEMO).elf: $(BUILD_OBJS)
	$(TRACE_LD)
	$(Q)$(LD) -T$(LDSCRIPT) $(LDFLAGS) $(CRTBEGIN_OBJ) $(BUILD_OBJS) $(CRTEND_OBJ) -o $(BUILDDIR)/$@

$(DEMO).bin: $(DEMO).elf
	$(Q)$(OBJCOPY) $(BUILDDIR)/$< -O binary $(BUILDDIR)/$@
	@echo Completed


ifneq ($(MAKECMDGOALS),clean)
#include $(patsubst %,$(DEPDIR)/%.d,$(basename $(notdir $(SRCS))))
#include $(patsubst %.c,%.d,$(SRCS))
-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(SRCS)))
endif

endif

else

clean:
	@rm -rf build
	@rm -rf install
	
endif

