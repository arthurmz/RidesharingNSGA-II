################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/Calculations.c \
C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/FunctionTests.c \
C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/Helper.c \
C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/NSGAII.c \
C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/RidesharingNSGAII.c 

OBJS += \
./src/Calculations.o \
./src/FunctionTests.o \
./src/Helper.o \
./src/NSGAII.o \
./src/RidesharingNSGAII.o 

C_DEPS += \
./src/Calculations.d \
./src/FunctionTests.d \
./src/Helper.d \
./src/NSGAII.d \
./src/RidesharingNSGAII.d 


# Each subdirectory must supply rules for building sources it contributes
src/Calculations.o: C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/Calculations.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I. -std=c99 -lm -g -O0 -Ilm -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/FunctionTests.o: C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/FunctionTests.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I. -std=c99 -lm -g -O0 -Ilm -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Helper.o: C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/Helper.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I. -std=c99 -lm -g -O0 -Ilm -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/NSGAII.o: C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/NSGAII.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I. -std=c99 -lm -g -O0 -Ilm -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/RidesharingNSGAII.o: C:/Users/Arthur/UFRN/workspace/RidesharingNSGA-II/src/RidesharingNSGAII.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I. -std=c99 -lm -g -O0 -Ilm -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


