################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/workspace_22_03_2016/RidesharingNSGA-II/src/Calculations.c \
E:/workspace_22_03_2016/RidesharingNSGA-II/src/FunctionTests.c \
E:/workspace_22_03_2016/RidesharingNSGA-II/src/Helper.c \
E:/workspace_22_03_2016/RidesharingNSGA-II/src/NSGAII.c \
E:/workspace_22_03_2016/RidesharingNSGA-II/src/RidesharingNSGAII.c 

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
src/Calculations.o: E:/workspace_22_03_2016/RidesharingNSGA-II/src/Calculations.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/FunctionTests.o: E:/workspace_22_03_2016/RidesharingNSGA-II/src/FunctionTests.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/Helper.o: E:/workspace_22_03_2016/RidesharingNSGA-II/src/Helper.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/NSGAII.o: E:/workspace_22_03_2016/RidesharingNSGA-II/src/NSGAII.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/RidesharingNSGAII.o: E:/workspace_22_03_2016/RidesharingNSGA-II/src/RidesharingNSGAII.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


