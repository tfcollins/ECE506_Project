################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PHY_Layer/server/Socket.cpp \
../PHY_Layer/server/TCPEchoServer-Thread.cpp \
../PHY_Layer/server/TCPEchoServer.cpp 

OBJS += \
./PHY_Layer/server/Socket.o \
./PHY_Layer/server/TCPEchoServer-Thread.o \
./PHY_Layer/server/TCPEchoServer.o 

CPP_DEPS += \
./PHY_Layer/server/Socket.d \
./PHY_Layer/server/TCPEchoServer-Thread.d \
./PHY_Layer/server/TCPEchoServer.d 


# Each subdirectory must supply rules for building sources it contributes
PHY_Layer/server/%.o: ../PHY_Layer/server/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


