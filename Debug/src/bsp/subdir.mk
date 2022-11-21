################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp/crtbegin.c \
../src/bsp/crtend.c \
../src/bsp/startup-nds32.c 

OBJS += \
./src/bsp/crtbegin.o \
./src/bsp/crtend.o \
./src/bsp/startup-nds32.o 

C_DEPS += \
./src/bsp/crtbegin.d \
./src/bsp/crtend.d \
./src/bsp/startup-nds32.d 


# Each subdirectory must supply rules for building sources it contributes
src/bsp/%.o: ../src/bsp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -mext-dsp -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/vt" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/include" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p/config" -O0 -mcmodel=medium -g3 -Wall -mcpu=d1088 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


