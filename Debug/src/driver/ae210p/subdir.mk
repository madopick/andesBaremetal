################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/driver/ae210p/cache.c \
../src/driver/ae210p/dma_ae210p.c \
../src/driver/ae210p/gpio_ae210p.c \
../src/driver/ae210p/i2c_ae210p.c \
../src/driver/ae210p/pwm_ae210p.c \
../src/driver/ae210p/rtc_ae210p.c \
../src/driver/ae210p/spi_ae210p.c \
../src/driver/ae210p/usart_ae210p.c \
../src/driver/ae210p/wdt_ae210p.c 

OBJS += \
./src/driver/ae210p/cache.o \
./src/driver/ae210p/dma_ae210p.o \
./src/driver/ae210p/gpio_ae210p.o \
./src/driver/ae210p/i2c_ae210p.o \
./src/driver/ae210p/pwm_ae210p.o \
./src/driver/ae210p/rtc_ae210p.o \
./src/driver/ae210p/spi_ae210p.o \
./src/driver/ae210p/usart_ae210p.o \
./src/driver/ae210p/wdt_ae210p.o 

C_DEPS += \
./src/driver/ae210p/cache.d \
./src/driver/ae210p/dma_ae210p.d \
./src/driver/ae210p/gpio_ae210p.d \
./src/driver/ae210p/i2c_ae210p.d \
./src/driver/ae210p/pwm_ae210p.d \
./src/driver/ae210p/rtc_ae210p.d \
./src/driver/ae210p/spi_ae210p.d \
./src/driver/ae210p/usart_ae210p.d \
./src/driver/ae210p/wdt_ae210p.d 


# Each subdirectory must supply rules for building sources it contributes
src/driver/ae210p/%.o: ../src/driver/ae210p/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -mext-dsp -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/bsp/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/vt" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/include" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p" -I"/cygdrive/C/Andestech/sourcetree/baremetal/src/driver/ae210p/config" -O0 -mcmodel=medium -g3 -Wall -mcpu=d1088 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


