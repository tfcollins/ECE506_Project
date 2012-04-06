################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../DL_Layer/testing/client/app_shell.cpp \
../DL_Layer/testing/client/crc.cpp \
../DL_Layer/testing/client/dl_layer.cpp \
../DL_Layer/testing/client/phy_q.cpp \
../DL_Layer/testing/client/phy_setup.cpp 

OBJS += \
./DL_Layer/testing/client/app_shell.o \
./DL_Layer/testing/client/crc.o \
./DL_Layer/testing/client/dl_layer.o \
./DL_Layer/testing/client/phy_q.o \
./DL_Layer/testing/client/phy_setup.o 

CPP_DEPS += \
./DL_Layer/testing/client/app_shell.d \
./DL_Layer/testing/client/crc.d \
./DL_Layer/testing/client/dl_layer.d \
./DL_Layer/testing/client/phy_q.d \
./DL_Layer/testing/client/phy_setup.d 


# Each subdirectory must supply rules for building sources it contributes
DL_Layer/testing/client/%.o: ../DL_Layer/testing/client/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


