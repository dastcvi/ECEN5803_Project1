#-----------------------------------------------------------------------------
# Copyright (C) 2018
# Alex St. Clair and Matthew Haney
# University of Colorado, Boulder
#
# Redistribution, modification, or use of this software in source or binary
# forms is permitted as long as the files maintain this copyright. Users are
# permitted to modify this and use it to learn about the field of embedded
# software. Alex St. Clair, Matthew Haney, and the University of Colorado
# are not liable for any misuse of this material.
#
#-----------------------------------------------------------------------------

CXX:=arm-none-eabi-g++
PLATFORM_FLAGS:=-mthumb -mcpu=cortex-m0plus -mfpu=fpv4-sp-d16 --specs=nosys.specs
INCLUDE_FLAGS:=-Imbed -Imbed/TARGET_KL25Z -Imbed/TARGET_KL25Z/TOOLCHAIN_ARM_STD -Imbed/TARGET_KL25Z/TARGET_Freescale -Imbed/TARGET_KL25Z/TARGET_Freescale/TARGET_KLXX -Imbed/TARGET_KL25Z/TARGET_Freescale/TARGET_KLXX/TARGET_KL25Z

LDFLAGS:=-g -O0 -T MKL25Z128xxx4_flash.ld
CFLAGS:=-g -O0 -std=c++11 -DTARGET_KL25Z
TARGET:=module2.bin

SOURCE:=module2.cpp MMA8451Q.cpp tsi_sensor.cpp
OBJ:=$(SOURCE:.cpp=.o)

.PHONY: clean redo

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(PLATFORM_FLAGS) $(CFLAGS) $(INCLUDE_FLAGS) -c $^ -o $@

clean:
	rm -f $(OBJ)
	rm -f $(TARGET)

redo: clean $(TARGET)
