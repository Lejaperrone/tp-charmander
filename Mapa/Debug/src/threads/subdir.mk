################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/threads/deadlock.c \
../src/threads/planificador.c 

OBJS += \
./src/threads/deadlock.o \
./src/threads/planificador.o 

C_DEPS += \
./src/threads/deadlock.d \
./src/threads/planificador.d 


# Each subdirectory must supply rules for building sources it contributes
src/threads/%.o: ../src/threads/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -Icurses -Inivel-gui -Incurses -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


