################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/FunctionTests.c \
../src/Helper.c \
../src/NSGAII.c \
../src/Ridesharing.c 

OBJS += \
./src/FunctionTests.o \
./src/Helper.o \
./src/NSGAII.o \
./src/Ridesharing.o 

C_DEPS += \
./src/FunctionTests.d \
./src/Helper.d \
./src/NSGAII.d \
./src/Ridesharing.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -std=c99 -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


