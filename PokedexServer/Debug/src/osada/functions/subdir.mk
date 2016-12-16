################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/osada/functions/bitmap.c \
../src/osada/functions/data.c \
../src/osada/functions/dump.c \
../src/osada/functions/mutex.c \
../src/osada/functions/tabla_archivos.c \
../src/osada/functions/tabla_asignaciones.c 

OBJS += \
./src/osada/functions/bitmap.o \
./src/osada/functions/data.o \
./src/osada/functions/dump.o \
./src/osada/functions/mutex.o \
./src/osada/functions/tabla_archivos.o \
./src/osada/functions/tabla_asignaciones.o 

C_DEPS += \
./src/osada/functions/bitmap.d \
./src/osada/functions/data.d \
./src/osada/functions/dump.d \
./src/osada/functions/mutex.d \
./src/osada/functions/tabla_archivos.d \
./src/osada/functions/tabla_asignaciones.d 


# Each subdirectory must supply rules for building sources it contributes
src/osada/functions/%.o: ../src/osada/functions/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


