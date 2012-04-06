################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PHY_Layer/old/client/Socket.cpp \
../PHY_Layer/old/client/TCPEchoClient.cpp 

OBJS += \
./PHY_Layer/old/client/Socket.o \
./PHY_Layer/old/client/TCPEchoClient.o 

CPP_DEPS += \
./PHY_Layer/old/client/Socket.d \
./PHY_Layer/old/client/TCPEchoClient.d 


# Each subdirectory must supply rules for building sources it contributes
PHY_Layer/old/client/%.o: ../PHY_Layer/old/client/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


