################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/osada/basura.c \
../src/osada/mapp.c \
../src/osada/osada.c 

OBJS += \
./src/osada/basura.o \
./src/osada/mapp.o \
./src/osada/osada.o 

C_DEPS += \
./src/osada/basura.d \
./src/osada/mapp.d \
./src/osada/osada.d 


# Each subdirectory must supply rules for building sources it contributes
src/osada/%.o: ../src/osada/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


