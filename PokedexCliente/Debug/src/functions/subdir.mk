################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/functions/fuse.c \
../src/functions/log.c 

OBJS += \
./src/functions/fuse.o \
./src/functions/log.o 

C_DEPS += \
./src/functions/fuse.d \
./src/functions/log.d 


# Each subdirectory must supply rules for building sources it contributes
src/functions/%.o: ../src/functions/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


