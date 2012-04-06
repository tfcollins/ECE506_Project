################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PHY_Layer/testing/client.cpp \
../PHY_Layer/testing/dl_layer.cpp \
../PHY_Layer/testing/non_blocking.cpp \
../PHY_Layer/testing/overall_sudo.cpp \
../PHY_Layer/testing/phy_q.cpp \
../PHY_Layer/testing/phy_q_server.cpp \
../PHY_Layer/testing/phy_q_server_func.cpp \
../PHY_Layer/testing/phy_setup.cpp \
../PHY_Layer/testing/queue_example.cpp \
../PHY_Layer/testing/server.cpp \
../PHY_Layer/testing/threads_example.cpp 

OBJS += \
./PHY_Layer/testing/client.o \
./PHY_Layer/testing/dl_layer.o \
./PHY_Layer/testing/non_blocking.o \
./PHY_Layer/testing/overall_sudo.o \
./PHY_Layer/testing/phy_q.o \
./PHY_Layer/testing/phy_q_server.o \
./PHY_Layer/testing/phy_q_server_func.o \
./PHY_Layer/testing/phy_setup.o \
./PHY_Layer/testing/queue_example.o \
./PHY_Layer/testing/server.o \
./PHY_Layer/testing/threads_example.o 

CPP_DEPS += \
./PHY_Layer/testing/client.d \
./PHY_Layer/testing/dl_layer.d \
./PHY_Layer/testing/non_blocking.d \
./PHY_Layer/testing/overall_sudo.d \
./PHY_Layer/testing/phy_q.d \
./PHY_Layer/testing/phy_q_server.d \
./PHY_Layer/testing/phy_q_server_func.d \
./PHY_Layer/testing/phy_setup.d \
./PHY_Layer/testing/queue_example.d \
./PHY_Layer/testing/server.d \
./PHY_Layer/testing/threads_example.d 


# Each subdirectory must supply rules for building sources it contributes
PHY_Layer/testing/%.o: ../PHY_Layer/testing/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


