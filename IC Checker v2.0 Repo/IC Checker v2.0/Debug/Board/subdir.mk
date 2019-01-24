################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Board/Button.c \
../Board/Checker.c \
../Board/I2C.c \
../Board/LCD.c 

OBJS += \
./Board/Button.o \
./Board/Checker.o \
./Board/I2C.o \
./Board/LCD.o 

C_DEPS += \
./Board/Button.d \
./Board/Checker.d \
./Board/I2C.d \
./Board/LCD.d 


# Each subdirectory must supply rules for building sources it contributes
Board/%.o: ../Board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m0plus -mthumb -mfloat-abi=soft '-D__weak=__attribute__((weak))' '-D__packed=__attribute__((__packed__))' -DUSE_HAL_DRIVER -DSTM32L053xx -I"C:/Users/aneed/Dropbox/Personal Repository/Projects/IC Checker/STM32 Repos/IC Checker v2.0 Repo/IC Checker v2.0/Board" -I"C:/Users/aneed/Dropbox/Personal Repository/Projects/IC Checker/STM32 Repos/IC Checker v2.0 Repo/IC Checker v2.0/Inc" -I"C:/Users/aneed/Dropbox/Personal Repository/Projects/IC Checker/STM32 Repos/IC Checker v2.0 Repo/IC Checker v2.0/Drivers/CMSIS/Device/ST/STM32L0xx/Include" -I"C:/Users/aneed/Dropbox/Personal Repository/Projects/IC Checker/STM32 Repos/IC Checker v2.0 Repo/IC Checker v2.0/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


