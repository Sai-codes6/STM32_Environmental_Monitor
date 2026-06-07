# STM32 Environmental Monitoring and Data Logging System

A bare-metal firmware project for the STM32F411RE (ARM Cortex-M4) that samples
temperature once per second, classifies each reading against configurable
thresholds, retains a rolling history in a circular buffer and streams telemetry
over UART. Every peripheral is driven at the register level through CMSIS, with
no HAL abstraction layer, so the timer, ADC, UART and GPIO configuration is fully
visible and auditable.

## Project Overview

The firmware demonstrates a complete, self-contained embedded acquisition loop:

- A one-second timer interrupt defines the acquisition cadence.
- The ADC reads the STM32F411 on-die temperature sensor on internal channel 18.
- A sensor layer converts the raw code to degrees Celsius and classifies it as
  normal, high-alert, low-alert or fault.
- A fixed-size circular buffer keeps the most recent samples without any dynamic
  memory allocation.
- USART2 emits a one-line telemetry record per sample and periodically replays
  the stored history. USART2 is wired to the ST-LINK virtual COM port, so output
  appears on a host terminal with no extra hardware.

Using the internal temperature sensor keeps the project runnable on a bare
NUCLEO-F411RE board, while the driver structure maps cleanly onto an external
analog sensor (LM35, NTC divider) by changing only the ADC channel.

## Features

- Register-level GPIO, ADC, USART and timer drivers (CMSIS only).
- Periodic 1 Hz timer interrupt with a short, deterministic ISR.
- 12-bit ADC single-conversion reads with a long sample window for the
  high-impedance temperature channel.
- Fixed-point temperature handling in tenths of a degree (no floating point in
  the acquisition path).
- Threshold alerts (high / low) and out-of-range fault detection.
- Circular data logging with oldest-record overwrite and on-demand replay.
- LED heartbeat that toggles on every sample.

## Hardware Requirements

- NUCLEO-F411RE development board (STM32F411RET6).
- USB cable to the ST-LINK port for power, flashing and serial telemetry.
- No external sensor is required; the internal temperature sensor is used.

| Signal        | Pin  | Function                                  |
|---------------|------|-------------------------------------------|
| USART2 TX     | PA2  | Telemetry out (to ST-LINK VCP)            |
| USART2 RX     | PA3  | Reserved for future host commands         |
| LED (LD2)     | PA5  | Per-sample heartbeat                      |
| Temp sensor   | ADC1_IN18 | Internal, enabled via TSVREFE        |

## Software Requirements

- STM32CubeIDE (or arm-none-eabi-gcc with a matching CMSIS device pack).
- ST-LINK firmware and drivers for flashing.
- Any serial terminal (PuTTY, minicom, screen, Tera Term) set to
  115200 baud, 8 data bits, no parity, 1 stop bit (8-N-1).

## Folder Structure

```
STM32_Environmental_Monitor/
├── README.md
├── LICENSE
├── .gitignore
├── docs/
│   ├── Architecture_Diagram.png
│   └── Flowchart.png
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── adc_driver.h
│   │   ├── uart_driver.h
│   │   ├── timer_driver.h
│   │   ├── logger.h
│   │   └── sensor.h
│   └── Src/
│       ├── main.c
│       ├── adc_driver.c
│       ├── uart_driver.c
│       ├── timer_driver.c
│       ├── logger.c
│       └── sensor.c
├── Drivers/
│   └── STM32CubeMX_Config/
│       └── CONFIG.md
├── tests/
│   └── sample_output_logs.txt
└── release/
    └── STM32_Environmental_Monitor.zip
```

## Build Instructions

The `Core/` sources are framework-independent and drop into a standard
STM32CubeIDE project for the STM32F411RE:

1. Create a new STM32 project in CubeIDE targeting STM32F411RET6 (or open an
   existing NUCLEO-F411RE project).
2. Replace the generated `Core/Inc` and `Core/Src` contents with the files from
   this repository, keeping the CubeIDE-generated `startup_stm32f411xetx.s`,
   linker script and `system_stm32f4xx.c`.
3. Confirm the pin assignments in `Drivers/STM32CubeMX_Config/CONFIG.md` match
   the project's `.ioc` peripheral setup.
4. Build the project (Project → Build All).

A command-line build with the GNU Arm Embedded Toolchain works equally well:
compile every `.c` under `Core/Src`, the CMSIS startup assembly and
`system_stm32f4xx.c`, then link against the F411 linker script.

## Flashing Instructions

1. Connect the NUCLEO-F411RE over USB.
2. In CubeIDE, Run → Run As → STM32 C/C++ Application, or use the command line:
   ```
   st-flash write STM32_Environmental_Monitor.bin 0x08000000
   ```
3. Open a serial terminal on the ST-LINK virtual COM port at 115200 8-N-1.

## Architecture

```
        +----------------+        +------------------+
        |  TIM2 (1 Hz)   |  IRQ   |   Main loop      |
        |  update event  +------->+   (wakes on WFI) |
        +----------------+        +---------+--------+
                                            |
                          read + classify   |
                                            v
        +----------------+        +------------------+
        |  ADC1 (CH18)   +<-------+   sensor layer   |
        |  internal temp |        |  thresholds/fault|
        +----------------+        +---------+--------+
                                            |
                       store + overwrite    |
                                            v
        +----------------+        +------------------+
        |  USART2 (PA2)  +<-------+  circular logger |
        |  telemetry out |        |  LOG_CAPACITY=32 |
        +----------------+        +------------------+
```

## Timer Interrupt Flow

TIM2 is clocked from APB1 at 16 MHz, prescaled to a 10 kHz tick and reloaded at
10000 counts to overflow once per second. The ISR clears the update flag,
advances a seconds counter and sets a pending-tick flag. The main loop, parked
in `__WFI()`, wakes on the interrupt, observes the flag and performs the
acquisition outside interrupt context.

## UART Communication Flow

USART2 runs at 115200 baud derived from the 16 MHz APB1 clock with rounding to
nearest. Transmission is blocking on the TXE flag, which is sufficient for the
low telemetry rate of one short line per second. Integer and fixed-point
helpers format readings without pulling in the standard library `printf`.

## Sensor Processing Flow

Each second the ADC performs one 12-bit conversion on channel 18. The raw code
is scaled to millivolts and mapped to tenths of a degree using the datasheet
linear model (V25 = 760 mV, slope 2.5 mV/°C). The sensor layer first rejects
physically impossible values as faults, then applies the high and low limits.

## Sample Output

```
STM32 Environmental Monitor
Sampling internal temperature once per second.

[1s] temp=26.4C  state=NORMAL
[2s] temp=26.6C  state=NORMAL
[3s] temp=41.2C  state=HIGH-ALERT  *** HIGH TEMPERATURE ***
...
```

A longer capture is included in `tests/sample_output_logs.txt`.

## Future Improvements

- Move the ADC to DMA-driven continuous conversion to remove the busy-wait.
- Replace blocking UART transmission with an interrupt-driven ring buffer.
- Apply factory calibration values for tighter temperature accuracy.
- Add an external sensor option (LM35 / NTC) selectable at build time.
- Persist the log to external I2C EEPROM so history survives a power cycle.
- Wrap the acquisition loop in a FreeRTOS task once multiple sensors are added.

## Troubleshooting

- No serial output: confirm the terminal is on the ST-LINK VCP at 115200 8-N-1
  and that PA2/PA3 are not remapped by another peripheral.
- Temperature reads near 25 °C regardless of conditions: the internal sensor is
  low resolution and uncalibrated; warming the package by touch should move the
  reading, confirming the path works.
- LED does not toggle: verify the TIM2 interrupt is enabled in the NVIC and that
  the ISR name matches the vector table entry `TIM2_IRQHandler`.
