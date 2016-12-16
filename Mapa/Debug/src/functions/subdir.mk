################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/functions/collections_list_extension.c \
../src/functions/config.c \
../src/functions/log.c \
../src/functions/recursos.c \
../src/functions/signals.c 

OBJS += \
./src/functions/collections_list_extension.o \
./src/functions/config.o \
./src/functions/log.o \
./src/functions/recursos.o \
./src/functions/signals.o 

C_DEPS += \
./src/functions/collections_list_extension.d \
./src/functions/config.d \
./src/functions/log.d \
./src/functions/recursos.d \
./src/functions/signals.d 


# Each subdirectory must supply rules for building sources it contributes
src/functions/%.o: ../src/functions/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -Icurses -Inivel-gui -Incurses -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


