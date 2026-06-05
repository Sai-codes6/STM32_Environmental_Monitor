/**
 ******************************************************************************
 * @file    uart_driver.c
 * @brief   Register-level USART2 driver implementation.
 *
 * PA2 and PA3 are switched to alternate function 7 (USART2). The baud-rate
 * divisor is derived from the APB1 clock using integer rounding so the
 * configured rate stays within tolerance of the requested value.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "uart_driver.h"
#include "main.h"
#include "stm32f4xx.h"

#define USART2_AF            7U           /* Alternate function index for PA2/PA3 */

void uart_init(uint32_t baudrate)
{
    /* Clock the GPIOA port and the USART2 peripheral. */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* Put PA2 (TX) and PA3 (RX) into alternate-function mode (MODER = 0b10). */
    GPIOA->MODER &= ~((3U << (2U * 2U)) | (3U << (2U * 3U)));
    GPIOA->MODER |=  ((2U << (2U * 2U)) | (2U << (2U * 3U)));

    /* Select AF7 for both pins in the low alternate-function register. */
    GPIOA->AFR[0] &= ~((0xFU << (4U * 2U)) | (0xFU << (4U * 3U)));
    GPIOA->AFR[0] |=  ((USART2_AF << (4U * 2U)) | (USART2_AF << (4U * 3U)));

    /* Compute the oversampling-by-16 baud divisor with rounding to nearest. */
    uint32_t usartdiv = (APB1_CLK_HZ + (baudrate / 2U)) / baudrate;
    USART2->BRR = usartdiv;

    /* Enable transmitter, receiver and the peripheral itself. */
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

void uart_send_char(char byte)
{
    /* Wait for the transmit data register to drain before loading the next
     * byte, then write it. */
    while ((USART2->SR & USART_SR_TXE) == 0U)
    {
        /* Busy-wait until TXE asserts. */
    }
    USART2->DR = (uint16_t)((uint8_t)byte);
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

    /* Extract decimal digits least-significant first. */
    do
    {
        buffer[index++] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U);

    /* Emit the digits in the correct order. */
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

    /* Preserve a leading minus sign for fractional-only negative values such
     * as -3 tenths, where the integer part rounds to zero. */
    if ((deci_celsius < 0) && (whole == 0))
    {
        uart_send_char('-');
    }

    uart_send_int(whole);
    uart_send_char('.');
    uart_send_int(frac);
}
