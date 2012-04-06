################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../APP_Layer/client_testing/server/server_array/app_shell.cpp \
../APP_Layer/client_testing/server/server_array/dl_layer.cpp \
../APP_Layer/client_testing/server/server_array/funct_tester.cpp \
../APP_Layer/client_testing/server/server_array/phy_q_server_multi.cpp \
../APP_Layer/client_testing/server/server_array/phy_setup.cpp 

OBJS += \
./APP_Layer/client_testing/server/server_array/app_shell.o \
./APP_Layer/client_testing/server/server_array/dl_layer.o \
./APP_Layer/client_testing/server/server_array/funct_tester.o \
./APP_Layer/client_testing/server/server_array/phy_q_server_multi.o \
./APP_Layer/client_testing/server/server_array/phy_setup.o 

CPP_DEPS += \
./APP_Layer/client_testing/server/server_array/app_shell.d \
./APP_Layer/client_testing/server/server_array/dl_layer.d \
./APP_Layer/client_testing/server/server_array/funct_tester.d \
./APP_Layer/client_testing/server/server_array/phy_q_server_multi.d \
./APP_Layer/client_testing/server/server_array/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/client_testing/server/server_array/%.o: ../APP_Layer/client_testing/server/server_array/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


