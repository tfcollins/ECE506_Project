################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../APP_Layer/client_testing/app_shell.cpp \
../APP_Layer/client_testing/client.cpp \
../APP_Layer/client_testing/crc.cpp \
../APP_Layer/client_testing/dl_layer.cpp \
../APP_Layer/client_testing/phy_q.cpp \
../APP_Layer/client_testing/phy_setup.cpp 

OBJS += \
./APP_Layer/client_testing/app_shell.o \
./APP_Layer/client_testing/client.o \
./APP_Layer/client_testing/crc.o \
./APP_Layer/client_testing/dl_layer.o \
./APP_Layer/client_testing/phy_q.o \
./APP_Layer/client_testing/phy_setup.o 

CPP_DEPS += \
./APP_Layer/client_testing/app_shell.d \
./APP_Layer/client_testing/client.d \
./APP_Layer/client_testing/crc.d \
./APP_Layer/client_testing/dl_layer.d \
./APP_Layer/client_testing/phy_q.d \
./APP_Layer/client_testing/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
APP_Layer/client_testing/%.o: ../APP_Layer/client_testing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


