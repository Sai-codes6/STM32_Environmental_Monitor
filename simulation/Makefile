# Makefile for the PC simulation of the STM32 Environmental Monitor.
# Builds a normal host executable - no STM32 hardware or ARM toolchain needed.
#
# Usage:
#   make        # compile ./env_monitor_sim
#   make run    # compile and run
#   make clean  # remove the built executable

CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra
INCLUDES = -ICore/Inc -Isimulation

SOURCES  = Core/Src/sensor.c \
           Core/Src/logger.c \
           simulation/adc_driver_host.c \
           simulation/uart_driver_host.c \
           simulation/timer_driver_host.c \
           simulation/main_host.c

TARGET   = env_monitor_sim

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(TARGET).exe

.PHONY: all run clean
