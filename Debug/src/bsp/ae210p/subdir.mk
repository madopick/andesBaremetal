################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp/ae210p/ae210p.c \
../src/bsp/ae210p/gpio.c \
../src/bsp/ae210p/timer.c \
../src/bsp/ae210p/uart.c 

S_UPPER_SRCS += \
../src/bsp/ae210p/start.S 

OBJS += \
./src/bsp/ae210p/ae210p.o \
./src/bsp/ae210p/gpio.o \
./src/bsp/ae210p/start.o \
./src/bsp/ae210p/timer.o \
./src/bsp/ae210p/uart.o 

S_UPPER_DEPS += \
./src/bsp/ae210p/start.d 

C_DEPS += \
./src/bsp/ae210p/ae210p.d \
./src/bsp/ae210p/gpio.d \
./src/bsp/ae210p/timer.d \
./src/bsp/ae210p/uart.d 


# Each subdirectory must supply rules for building sources it contributes
src/bsp/ae210p/%.o: ../src/bsp/ae210p/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -mext-dsp -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/vt" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/include" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p/config" -O0 -mcmodel=medium -g3 -Wall -mcpu=d1088 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/bsp/ae210p/%.o: ../src/bsp/ae210p/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -mext-dsp -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/vt" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/include" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p/config" -O0 -mcmodel=medium -g3 -Wall -mcpu=d1088 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


