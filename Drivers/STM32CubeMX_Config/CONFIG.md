# STM32CubeMX / Peripheral Configuration

The `Core/` sources in this repository are framework-independent and rely only
on the CMSIS device headers, the startup assembly file and the linker script
that STM32CubeIDE generates for the STM32F411RET6. This note records the exact
peripheral configuration so a fresh CubeMX project can be matched to the
firmware.

## Target

- MCU: STM32F411RET6
- Board: NUCLEO-F411RE
- Core clock: 16 MHz from the internal HSI oscillator (PLL disabled)
- Flash latency: 0 wait states

## Clock Tree

- System clock source: HSI (16 MHz)
- AHB prescaler: 1  (HCLK = 16 MHz)
- APB1 prescaler: 1  (PCLK1 = 16 MHz, TIM2 clock = 16 MHz)
- APB2 prescaler: 1  (PCLK2 = 16 MHz, ADC clock derived from PCLK2)

## Pin Assignments

| Peripheral | Pin | Mode                     | Notes                          |
|------------|-----|--------------------------|--------------------------------|
| USART2_TX  | PA2 | Alternate function 7     | To ST-LINK virtual COM port    |
| USART2_RX  | PA3 | Alternate function 7     | Reserved for host commands     |
| GPIO_OUT   | PA5 | Output push-pull         | On-board LED LD2 heartbeat     |
| ADC1_IN18  | —   | Internal channel         | On-die temperature sensor      |

## Peripheral Settings

### TIM2
- Counter mode: up
- Prescaler: 1599  (16 MHz / 1600 = 10 kHz tick)
- Auto-reload: 9999 (10000 ticks = 1 s overflow)
- Update interrupt: enabled
- NVIC: TIM2 global interrupt enabled, priority 2

### ADC1
- Resolution: 12 bits, right aligned
- Conversion mode: single, software trigger
- Regular sequence length: 1
- Channel 18 sample time: 480 cycles
- Common control: temperature sensor and VREFINT enabled (TSVREFE)

### USART2
- Baud rate: 115200
- Word length: 8 bits
- Parity: none
- Stop bits: 1
- Oversampling: 16
- Transmitter and receiver enabled

## Regenerating in CubeMX

1. Start a new project for STM32F411RET6.
2. Set the clock configuration as above (HSI, all prescalers = 1).
3. Configure USART2, TIM2 and ADC1 with the settings listed here.
4. Assign PA5 as a GPIO output.
5. Generate code, then overwrite the generated `Core/Inc` and `Core/Src` with
   the files in this repository while keeping the generated startup, linker and
   `system_stm32f4xx.c` files.
