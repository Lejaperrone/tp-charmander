################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/functions/config.c \
../src/functions/log.c \
../src/functions/nextItem.c \
../src/functions/positions.c \
../src/functions/processLogic.c \
../src/functions/reset.c \
../src/functions/signals.c 

OBJS += \
./src/functions/config.o \
./src/functions/log.o \
./src/functions/nextItem.o \
./src/functions/positions.o \
./src/functions/processLogic.o \
./src/functions/reset.o \
./src/functions/signals.o 

C_DEPS += \
./src/functions/config.d \
./src/functions/log.d \
./src/functions/nextItem.d \
./src/functions/positions.d \
./src/functions/processLogic.d \
./src/functions/reset.d \
./src/functions/signals.d 


# Each subdirectory must supply rules for building sources it contributes
src/functions/%.o: ../src/functions/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


