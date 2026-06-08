# Running the Environmental Monitor on a PC (no hardware)

The main project under `Core/` is real STM32F411RE firmware: it talks directly
to hardware registers and is meant to be cross-compiled with the ARM toolchain
and flashed to a board. That build cannot run on a normal PC.

This `simulation/` folder provides a second build that compiles with an
ordinary C compiler and runs straight in VSCode, so the project can be seen
working without any board. The hardware-independent logic is reused unchanged;
only the three peripheral drivers are swapped for PC versions.

## What runs in the simulation

Reused from the real firmware, compiled unchanged:

- `Core/Src/sensor.c` - temperature classification (normal / high / low / fault)
- `Core/Src/logger.c` - the fixed-size circular log buffer

PC stand-ins (in this folder), replacing the register-level drivers:

- `adc_driver_host.c` - feeds a synthetic temperature profile; the raw-to-Celsius
  maths is identical to the firmware
- `uart_driver_host.c` - prints telemetry to the terminal instead of USART2; the
  number and temperature formatting is identical to the firmware
- `timer_driver_host.c` - a software seconds counter standing in for the 1 Hz
  timer interrupt
- `main_host.c` - the same acquisition loop as `Core/Src/main.c`, without the
  wait-for-interrupt sleep and hardware bring-up
- `stm32f4xx.h` - an empty placeholder so the shared headers include cleanly on
  a PC

## Required files (the complete compilable set)

```
Core/Inc/   main.h sensor.h logger.h adc_driver.h uart_driver.h timer_driver.h
Core/Src/   sensor.c logger.c
simulation/ stm32f4xx.h
            adc_driver_host.c uart_driver_host.c timer_driver_host.c main_host.c
```

The on-target files `Core/Src/main.c`, `adc_driver.c`, `uart_driver.c` and
`timer_driver.c` are NOT part of the simulation build; their PC equivalents in
this folder are used instead.

## Step 1 - Install a C compiler

The simulation needs `gcc` (or `clang`) on the system PATH.

- Windows: install MSYS2 and then `pacman -S mingw-w64-ucrt-x86_64-gcc`, or
  install MinGW-w64. Confirm with `gcc --version` in a new terminal.
- macOS: install the Command Line Tools with `xcode-select --install`
  (this provides `clang`; set `CC=clang` if `gcc` is not found).
- Linux: `sudo apt install build-essential gdb` (or the distro equivalent).

In VSCode, install the Microsoft C/C++ extension for build, run and debug.

## Step 2 - Build and run

Three equivalent options:

1. VSCode build task: press Ctrl+Shift+B (Cmd+Shift+B on macOS) and choose
   "Build Environmental Monitor (simulation)". The executable `env_monitor_sim`
   appears in the project root.

2. VSCode run/debug: open the Run and Debug view, select
   "Run Environmental Monitor (simulation)" and press F5. It builds first, then
   runs with the debugger so breakpoints can be set in `sensor.c`, `logger.c`,
   or any simulation source.

3. Terminal: from the `simulation/` folder run `make run`, or from the project
   root run the compiler directly:

   ```
   gcc -std=c11 -Wall -Wextra -ICore/Inc -Isimulation \
       Core/Src/sensor.c Core/Src/logger.c \
       simulation/adc_driver_host.c simulation/uart_driver_host.c \
       simulation/timer_driver_host.c simulation/main_host.c \
       -o env_monitor_sim
   ./env_monitor_sim
   ```

On Windows the executable may be named `env_monitor_sim.exe`; run it as
`.\env_monitor_sim.exe` and, if needed, point the `program` field in
`.vscode/launch.json` at that name.

## Expected output

The run walks through every classification path and replays the history every
ten samples:

```
STM32 Environmental Monitor (PC simulation)
Sampling a synthetic temperature once per cycle.

[1s] temp=21.8C  state=NORMAL
...
[14s] temp=42.6C  state=HIGH-ALERT  *** HIGH TEMPERATURE ***
...
[21s] temp=4.2C  state=LOW-ALERT  *** LOW TEMPERATURE ***
[23s] temp=-279.0C  state=FAULT  *** SENSOR FAULT ***
...
---- LOG HISTORY ----
t=1s  temp=21.8C  state=NORMAL
...
---------------------
```

The displayed temperatures differ slightly from the round numbers in the
profile because the reading is pushed through the same 12-bit ADC conversion
the firmware uses; that quantisation is realistic, not a bug.

To change behaviour: edit the `profile_dc` array in `adc_driver_host.c` to feed
different temperatures, or raise `SIM_SAMPLE_COUNT` in `main_host.c` (or wrap
its loop in `for(;;)`) for a longer or continuous run.

## Building for real hardware (for reference)

To build the actual firmware under `Core/` for an STM32F411RE board, a
different toolchain is needed and the result is flashed, not run on the PC:

- the ARM bare-metal compiler `arm-none-eabi-gcc`
- the CMSIS device header and `system_stm32f4xx.c` for the STM32F411
- a startup file (`startup_stm32f411xetx.s`) and a linker script
  (`STM32F411RETx_FLASH.ld`)
- a flashing tool such as ST-LINK or the STM32CubeIDE / STM32CubeProgrammer

The simplest route for real hardware is to import the `Core/` sources into a new
STM32CubeIDE project, which supplies the startup, linker and CMSIS files
automatically. Even then, the firmware only runs once flashed to a board (or
inside an emulator such as QEMU); it does not run as a PC program.
