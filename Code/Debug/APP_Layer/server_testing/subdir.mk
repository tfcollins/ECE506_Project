################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../APP_Layer/server_testing/phy_setup.cpp \
../APP_Layer/server_testing/server.cpp 

OBJS += \
./APP_Layer/server_testing/phy_setup.o \
./APP_Layer/server_testing/server.o 

CPP_DEPS += \
./APP_Layer/server_testing/phy_setup.d \
./APP_Layer/server_testing/server.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/server_testing/%.o: ../APP_Layer/server_testing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


