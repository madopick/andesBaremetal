#
# Copyright (c) 2012-2018 Andes Technology Corporation
# All rights reserved.
#

PROG	?= Demo

# PLAT: AE210P, AE100, AE3XX
PLAT	= AE210P

# DEMO: INTERRUPT,
DEMO	= INT

# Mode: LOAD, BURN, XIP
MODE    ?= XIP

# ADDR: 4GB, 16MB
ADDR	?= 4GB

# HWZOL / DEBUG / SIMU / GCOV: 1/0
HWZOL	?= 0
DEBUG	?= 0
SIMU	?= 0
GCOV	?= 0

$(info platform: $(PLAT), address:$(ADDR), mode:$(MODE), HWZOL:$(HWZOL), SIMU:$(SIMU), GCOV:$(GCOV), DEBUG:$(DEBUG))

ifneq ($(filter $(ADDR), 4GB 16MB),$(ADDR))
 $(error ADDRing mode "$(ADDR)" is not supported!)
endif

ifneq ($(filter $(MODE), LOAD BURN XIP),$(MODE))
 $(error MODE "$(MODE)" is not supported!)
endif

PLATNAME = $(shell echo $(PLAT) | tr A-Z a-z)
DEMONAME = $(shell echo $(DEMO) | tr A-Z a-z)

ifeq ($(MODE),XIP)
 ifeq ($(filter $(DEMO), ECC),$(DEMO))
  $(error DEMO "$(DEMO)" + MODE "$(MODE)" is not supported!)
 endif
endif

# Define the directories
ROOT			= ..
SRC_PATH		= ../src
BSP_SOURCE_DIR		= $(SRC_PATH)/bsp
DEMO_SOURCE_DIR		= $(SRC_PATH)/demo

# Define the source files we have

# BSP source files

STARTUP_SRCS = $(BSP_SOURCE_DIR)/$(PLATNAME)/start.S

BSP_SRCS = \
	$(BSP_SOURCE_DIR)/startup-nds32.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/$(PLATNAME).c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/uart.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/timer.c \
	$(BSP_SOURCE_DIR)/$(PLATNAME)/gpio.c

ifeq ($(filter $(PLAT), AE100 AE3XX),$(PLAT))
	ifeq ($(MODE), BURN)
		BSP_SRCS += $(BSP_SOURCE_DIR)/$(PLATNAME)/loader.c
	endif
endif

LIBC_SRCS =

# Demo source files
DEMO_SRCS = \
	$(DEMO_SOURCE_DIR)/main.c


# compilation options

LDSCRIPT = $(ROOT)/nds32-$(PLATNAME).ld

ifeq ($(MODE),XIP)
	ifneq ($(PLAT),AE210P)
		LDSCRIPT := $(LDSCRIPT:.ld=)-xip.ld
	endif
endif

INCLUDES = \
	-I$(BSP_SOURCE_DIR) \
	-I$(BSP_SOURCE_DIR)/$(PLATNAME) \
	-I$(BSP_SOURCE_DIR)/driver/include \
	-I$(DEMO_SOURCE_DIR)

# Define all object files.
SRCS = $(STARTUP_SRCS) $(BSP_SRCS) $(LIBC_SRCS) $(DEMO_SRCS)
OBJS = $(patsubst %.S,%.o,$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,${SRCS})))

### Use crtbebin1.o and crtend1.o for global constructors

CRTBEGIN_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ:=$(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)

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

### Toolchain

CROSS_COMPILE ?= nds32le-elf-

CC	:= $(CROSS_COMPILE)gcc
OBJDUMP	:= $(CROSS_COMPILE)objdump
OBJCOPY	:= $(CROSS_COMPILE)objcopy
AR	:= $(CROSS_COMPILE)ar
AS	:= $(CROSS_COMPILE)as

ifeq ($(DEBUG),1)
	OPTIM   := -O0 -g3
else
	OPTIM   := -Os -g3
endif

# Common compilation options

CFLAGS = \
	-Wall $(OPTIM) $(INCLUDES) $(DEFINES) \
	-fomit-frame-pointer -fno-strict-aliasing -fno-builtin -funroll-loops \
	-ffunction-sections -fdata-sections \
	-DCFG_MAKEFILE -DCFG_$(MODE)

ASFLAGS = -D__ASSEMBLY__ $(CFLAGS) -c

LDFLAGS = -T$(LDSCRIPT) $(OPTIM) -static -nostartfiles

CEXTLDFLAGS =

ifeq ($(DEMO),ISPS)
	LDFLAGS += -misps
endif


### Make variables

ifeq ($(ADDR), 16MB)
	CFLAGS += -DCFG_16MB
endif

ifeq ($(SIMU), 1)
	CFLAGS += -DCFG_SIMU
endif

ifeq ($(HWZOL), 1)
	ifeq ($(shell echo | $(CC) -E -dM -mext-zol - | grep '\<__NDS32_EXT_ZOL__\>' > /dev/null && echo "ZOL"), ZOL)
		CFLAGS += -DCFG_HWZOL
		LDFLAGS += -mext-zol
	else
		$(error "ZOL" is not supported by toolchain!)
	endif
endif

ifeq ($(GCOV), 1)
	SRCS += $(BSP_SOURCE_DIR)/crtbegin.c $(BSP_SOURCE_DIR)/crtend.c
	CFLAGS += -DCFG_GCOV --coverage
	LDFLAGS += -mvh
	CEXTLDFLAGS += -lgcov
endif


# Compilation rules
.SUFFIXES : %.o %.c %.cpp %.S

%.o: %.c
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(CFLAGS) -o $@ $<

%.o: %.S
	$(TRACE_CC)
	$(Q)$(CC) -c -MMD $(ASFLAGS) -o $@ $<

all: $(PROG).elf

$(PROG).elf  : $(OBJS) Makefile 
	$(TRACE_LD)
	$(Q)$(CC) -o $@ $(LDFLAGS) $(OBJS) $(CEXTLDFLAGS)
	$(OBJCOPY) -O binary -S $(PROG).elf $(PROG).bin
	$(OBJDUMP) -d $(PROG).elf > $(PROG).asm
	@echo Completed

clean :
	@rm -f $(OBJS) $(OBJS:.o=.d)
	@rm -f $(PROG).elf 
	@rm -f $(PROG).bin
	@rm -f $(PROG).asm

# Automatic dependency generation
ifneq ($(MAKECMDGOALS),clean)
-include $(OBJS:.o=.d)
endif
