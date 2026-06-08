/**
 ******************************************************************************
 * @file    uart_driver_host.c
 * @brief   PC-simulation implementation of the UART telemetry interface.
 *
 * On real hardware uart_send_char writes a byte to the USART2 data register.
 * In the simulation the same byte is written to standard output instead, so
 * the telemetry stream appears in the terminal exactly as it would on a serial
 * monitor. The string, integer and temperature formatting routines are kept
 * identical to the on-target driver so the displayed format is faithful.
 ******************************************************************************
 */

#include "uart_driver.h"
#include <stdio.h>

void uart_init(uint32_t baudrate)
{
    /* No peripheral to configure on a PC; the baud rate is irrelevant here. */
    (void)baudrate;
}

void uart_send_char(char byte)
{
    /* The only hardware-specific line: send the byte to the console rather
     * than to the USART data register. */
    putchar((unsigned char)byte);
}

void uart_send_string(const char *text)
{
    while (*text != '\0')
    {
        uart_send_char(*text);
        text++;
    }
}

void uart_send_int(int32_t value)
{
    char     buffer[12];          /* Enough for -2147483648 plus terminator. */
    uint8_t  index = 0;
    uint32_t magnitude;

    if (value < 0)
    {
        uart_send_char('-');
        magnitude = (uint32_t)(-(value + 1)) + 1U;  /* Safe negate of INT_MIN */
    }
    else
    {
        magnitude = (uint32_t)value;
    }

    do
    {
        buffer[index++] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U);

    while (index > 0U)
    {
        index--;
        uart_send_char(buffer[index]);
    }
}

void uart_send_temperature(int16_t deci_celsius)
{
    int16_t whole = (int16_t)(deci_celsius / 10);
    int16_t frac  = (int16_t)(deci_celsius % 10);

    if (frac < 0)
    {
        frac = (int16_t)(-frac);
    }

    if ((deci_celsius < 0) && (whole == 0))
    {
        uart_send_char('-');
    }

    uart_send_int(whole);
    uart_send_char('.');
    uart_send_int(frac);
}
