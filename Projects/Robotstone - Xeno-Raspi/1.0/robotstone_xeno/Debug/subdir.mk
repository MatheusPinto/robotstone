################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../whetstone.c 

CPP_SRCS += \
../RobotMaster.cpp \
../RobotSlave.cpp \
../RobotTask.cpp \
../Robotstone.cpp \
../communic.cpp \
../main.cpp \
../realtime.cpp \
../rtheap.cpp \
../rtsignal.cpp \
../stdout.cpp \
../task.cpp \
../test_led.cpp \
../timer.cpp 

OBJS += \
./RobotMaster.o \
./RobotSlave.o \
./RobotTask.o \
./Robotstone.o \
./communic.o \
./main.o \
./realtime.o \
./rtheap.o \
./rtsignal.o \
./stdout.o \
./task.o \
./test_led.o \
./timer.o \
./whetstone.o 

C_DEPS += \
./whetstone.d 

CPP_DEPS += \
./RobotMaster.d \
./RobotSlave.d \
./RobotTask.d \
./Robotstone.d \
./communic.d \
./main.d \
./realtime.d \
./rtheap.d \
./rtsignal.d \
./stdout.d \
./task.d \
./test_led.d \
./timer.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	arm-linux-gnueabihf-g++ -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/xenomai/include/cobalt -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/include -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/xenomai/include -O0 -g3 -Wall -c --sysroot=/home/matheus/Apps/eclipse/raspberrypi/rootfs  -fmessage-length=0 -mfpu=vfpv3-d16 -march=armv7-a -mthumb -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__COBALT__ -v -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	arm-linux-gnueabihf-g++ -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/xenomai/include/cobalt -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/include -I/home/matheus/Apps/eclipse/raspberrypi/rootfs/usr/xenomai/include -O0 -g3 -Wall -c --sysroot=/home/matheus/Apps/eclipse/raspberrypi/rootfs  -fmessage-length=0 -mfpu=vfpv3-d16 -march=armv7-a -mthumb -D_GNU_SOURCE -D_REENTRANT -fasynchronous-unwind-tables -D__COBALT__ -v -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


