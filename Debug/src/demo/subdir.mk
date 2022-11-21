################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/demo/main.c 

OBJS += \
./src/demo/main.o 

C_DEPS += \
./src/demo/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/demo/%.o: ../src/demo/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Andes C Compiler'
	$(CROSS_COMPILE)gcc -I"/cygdrive/C/Users/madop/AndeSight3/workspace/demo-int/src/bsp" -I"/cygdrive/C/Users/madop/AndeSight3/workspace/demo-int/src/bsp/ae210p" -I"/cygdrive/C/Users/madop/AndeSight3/workspace/demo-int/src/demo" -O0 -mcmodel=medium -g3 -Wall -mcpu=n968a -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d) $(@:%.o=%.o)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


