################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PHY_Layer/testing/client.cpp \
../PHY_Layer/testing/server.cpp \
../PHY_Layer/testing/threads_example.cpp 

OBJS += \
./PHY_Layer/testing/client.o \
./PHY_Layer/testing/server.o \
./PHY_Layer/testing/threads_example.o 

CPP_DEPS += \
./PHY_Layer/testing/client.d \
./PHY_Layer/testing/server.d \
./PHY_Layer/testing/threads_example.d 


# Each subdirectory must supply rules for building sources it contributes
PHY_Layer/testing/%.o: ../PHY_Layer/testing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


