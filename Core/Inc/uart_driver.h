/**
 ******************************************************************************
 * @file    uart_driver.h
 * @brief   Register-level USART2 driver used for telemetry output.
 *
 * USART2 is routed to PA2 (TX) and PA3 (RX), which connect to the ST-LINK
 * virtual COM port on the NUCLEO-F411RE, so output appears on a host serial
 * terminal without extra wiring.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <stdint.h>

/**
 * @brief  Configures PA2/PA3 alternate functions and brings up USART2.
 * @param  baudrate  Desired baud rate in bits per second.
 * @retval None.
 */
void uart_init(uint32_t baudrate);

/**
 * @brief  Transmits a single byte, blocking until the data register is free.
 * @param  byte  Character to send.
 * @retval None.
 */
void uart_send_char(char byte);

/**
 * @brief  Transmits a null-terminated string.
 * @param  text  Pointer to the string to send.
 * @retval None.
 */
void uart_send_string(const char *text);

/**
 * @brief  Transmits a signed integer in decimal form.
 * @param  value  Value to print.
 * @retval None.
 */
void uart_send_int(int32_t value);

/**
 * @brief  Transmits a temperature expressed in tenths of a degree as a
 *         human-readable decimal, for example 253 prints as "25.3".
 * @param  deci_celsius  Temperature in tenths of a degree Celsius.
 * @retval None.
 */
void uart_send_temperature(int16_t deci_celsius);

#endif /* UART_DRIVER_H */
