################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DL_Layer/testing/server/server_array/app_shell.cpp \
../DL_Layer/testing/server/server_array/dl_layer.cpp \
../DL_Layer/testing/server/server_array/funct_tester.cpp \
../DL_Layer/testing/server/server_array/phy_q_server_multi.cpp \
../DL_Layer/testing/server/server_array/phy_setup.cpp 

OBJS += \
./DL_Layer/testing/server/server_array/app_shell.o \
./DL_Layer/testing/server/server_array/dl_layer.o \
./DL_Layer/testing/server/server_array/funct_tester.o \
./DL_Layer/testing/server/server_array/phy_q_server_multi.o \
./DL_Layer/testing/server/server_array/phy_setup.o 

CPP_DEPS += \
./DL_Layer/testing/server/server_array/app_shell.d \
./DL_Layer/testing/server/server_array/dl_layer.d \
./DL_Layer/testing/server/server_array/funct_tester.d \
./DL_Layer/testing/server/server_array/phy_q_server_multi.d \
./DL_Layer/testing/server/server_array/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
DL_Layer/testing/server/server_array/%.o: ../DL_Layer/testing/server/server_array/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


