/**
 ******************************************************************************
 * @file    stm32f4xx.h  (PC-simulation placeholder)
 * @brief   Minimal stand-in for the STM32 device header.
 *
 * The shared project headers (for example main.h) include "stm32f4xx.h"
 * because the on-target build needs the device register definitions. The PC
 * simulation never touches hardware registers, so this placeholder simply lets
 * those headers include cleanly when compiling with a normal host compiler.
 *
 * When building for real hardware, the genuine device header shipped with the
 * STM32 toolchain (CMSIS) is used instead of this file.
 ******************************************************************************
 */

#ifndef STM32F4XX_H
#define STM32F4XX_H

/* Intentionally empty: no register definitions are needed for the host build. */

#endif /* STM32F4XX_H */
