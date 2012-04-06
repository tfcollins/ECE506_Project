################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../PHY_Layer/old/server/Socket.cpp \
../PHY_Layer/old/server/TCPEchoServer-Thread.cpp \
../PHY_Layer/old/server/TCPEchoServer.cpp 

OBJS += \
./PHY_Layer/old/server/Socket.o \
./PHY_Layer/old/server/TCPEchoServer-Thread.o \
./PHY_Layer/old/server/TCPEchoServer.o 

CPP_DEPS += \
./PHY_Layer/old/server/Socket.d \
./PHY_Layer/old/server/TCPEchoServer-Thread.d \
./PHY_Layer/old/server/TCPEchoServer.d 


# Each subdirectory must supply rules for building sources it contributes
PHY_Layer/old/server/%.o: ../PHY_Layer/old/server/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


