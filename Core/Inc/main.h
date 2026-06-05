/**
 ******************************************************************************
 * @file    main.h
 * @brief   Top-level configuration and entry-point declarations for the
 *          STM32 Environmental Monitoring and Data Logging System.
 *
 * Target  : STM32F411RE (ARM Cortex-M4), NUCLEO-F411RE board.
 * Toolset : CMSIS register-level access (no HAL abstraction layer).
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx.h"
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* System clock                                                                */
/* -------------------------------------------------------------------------- */
/* The firmware runs from the internal 16 MHz HSI oscillator without the PLL.  */
/* Keeping the clock at HSI removes PLL lock dependencies and makes the timer  */
/* and UART baud-rate arithmetic straightforward to verify by hand.            */
#define SYSCLK_HZ            16000000UL   /* SYSCLK = HSI = 16 MHz             */
#define APB1_CLK_HZ          16000000UL   /* APB1 prescaler = 1               */
#define APB2_CLK_HZ          16000000UL   /* APB2 prescaler = 1               */

/* -------------------------------------------------------------------------- */
/* Telemetry / logging configuration                                           */
/* -------------------------------------------------------------------------- */
#define UART_BAUDRATE        115200UL     /* USART2 baud rate                 */
#define SAMPLE_PERIOD_SEC    1U           /* One acquisition per second       */
#define LOG_CAPACITY         32U          /* Circular buffer depth (records)  */

/* -------------------------------------------------------------------------- */
/* Threshold and fault limits (degrees Celsius, fixed point x10)               */
/* -------------------------------------------------------------------------- */
/* Temperatures are carried as integer tenths of a degree to avoid floating    */
/* point in the acquisition path. 255 represents 25.5 degrees Celsius.         */
#define TEMP_HIGH_LIMIT_DC   400          /* 40.0 C : high-temperature alert  */
#define TEMP_LOW_LIMIT_DC    50           /*  5.0 C : low-temperature alert   */
#define TEMP_FAULT_MIN_DC   (-400)        /* Reading below this flags a fault */
#define TEMP_FAULT_MAX_DC    1250         /* Reading above this flags a fault */

/* -------------------------------------------------------------------------- */
/* On-board indicator                                                          */
/* -------------------------------------------------------------------------- */
#define LED_PORT             GPIOA
#define LED_PIN              5U           /* NUCLEO-F411RE green LED LD2 (PA5) */

/* -------------------------------------------------------------------------- */
/* Public entry points                                                         */
/* -------------------------------------------------------------------------- */

/**
 * @brief  Configures flash latency and selects HSI as the system clock.
 * @retval None.
 */
void system_clock_init(void);

#endif /* MAIN_H */
