################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../APP_Layer/server_testing/APP_server.cpp \
../APP_Layer/server_testing/dl_layer.cpp \
../APP_Layer/server_testing/phy_q_server_multi.cpp \
../APP_Layer/server_testing/phy_setup.cpp 

OBJS += \
./APP_Layer/server_testing/APP_server.o \
./APP_Layer/server_testing/dl_layer.o \
./APP_Layer/server_testing/phy_q_server_multi.o \
./APP_Layer/server_testing/phy_setup.o 

CPP_DEPS += \
./APP_Layer/server_testing/APP_server.d \
./APP_Layer/server_testing/dl_layer.d \
./APP_Layer/server_testing/phy_q_server_multi.d \
./APP_Layer/server_testing/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/server_testing/%.o: ../APP_Layer/server_testing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


