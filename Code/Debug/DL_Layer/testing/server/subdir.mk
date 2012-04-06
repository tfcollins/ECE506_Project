################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DL_Layer/testing/server/app_shell.cpp \
../DL_Layer/testing/server/dl_layer.cpp \
../DL_Layer/testing/server/funct_tester.cpp \
../DL_Layer/testing/server/phy_q_server_func.cpp \
../DL_Layer/testing/server/phy_q_server_multi.cpp \
../DL_Layer/testing/server/phy_setup.cpp 

OBJS += \
./DL_Layer/testing/server/app_shell.o \
./DL_Layer/testing/server/dl_layer.o \
./DL_Layer/testing/server/funct_tester.o \
./DL_Layer/testing/server/phy_q_server_func.o \
./DL_Layer/testing/server/phy_q_server_multi.o \
./DL_Layer/testing/server/phy_setup.o 

CPP_DEPS += \
./DL_Layer/testing/server/app_shell.d \
./DL_Layer/testing/server/dl_layer.d \
./DL_Layer/testing/server/funct_tester.d \
./DL_Layer/testing/server/phy_q_server_func.d \
./DL_Layer/testing/server/phy_q_server_multi.d \
./DL_Layer/testing/server/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
DL_Layer/testing/server/%.o: ../DL_Layer/testing/server/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


