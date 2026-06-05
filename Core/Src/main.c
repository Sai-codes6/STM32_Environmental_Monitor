/**
 ******************************************************************************
 * @file    main.c
 * @brief   Application entry point for the STM32 Environmental Monitoring and
 *          Data Logging System.
 *
 * Boot sequence: configure the system clock, bring up the indicator LED, the
 * UART telemetry channel, the ADC and the one-second timer. The main loop then
 * sleeps until the timer interrupt signals a new acquisition window, at which
 * point one temperature sample is read, classified, logged into the circular
 * buffer and streamed over UART. The history is replayed periodically.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "main.h"
#include "adc_driver.h"
#include "uart_driver.h"
#include "timer_driver.h"
#include "sensor.h"
#include "logger.h"
#include "stm32f4xx.h"

/* Replay the stored history once every DUMP_INTERVAL samples. */
#define DUMP_INTERVAL        10U

void system_clock_init(void)
{
    /* One flash wait state is comfortably safe for a 16 MHz core at 3.3 V and
     * removes any dependence on the reset-default latency value. */
    FLASH->ACR &= ~FLASH_ACR_LATENCY;
    FLASH->ACR |= FLASH_ACR_LATENCY_0WS;

    /* Ensure the internal 16 MHz oscillator is running and stable. */
    RCC->CR |= RCC_CR_HSION;
    while ((RCC->CR & RCC_CR_HSIRDY) == 0U)
    {
        /* Wait for HSI to stabilise. */
    }

    /* Select HSI as the system clock source. */
    RCC->CFGR &= ~RCC_CFGR_SW;
    RCC->CFGR |= RCC_CFGR_SW_HSI;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI)
    {
        /* Wait until the switch completes. */
    }

    /* AHB and both APB buses run undivided at 16 MHz. */
    RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
}

/**
 * @brief  Configures the on-board LED pin as a push-pull output.
 * @retval None.
 */
static void led_init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* General-purpose output mode (MODER = 0b01) on the LED pin. */
    LED_PORT->MODER &= ~(3U << (2U * LED_PIN));
    LED_PORT->MODER |=  (1U << (2U * LED_PIN));

    /* Push-pull, no pull resistors, low speed are the reset defaults and are
     * left in place. Drive the LED off to start. */
    LED_PORT->ODR &= ~(1U << LED_PIN);
}

/**
 * @brief  Inverts the LED output, giving a visible per-sample heartbeat.
 * @retval None.
 */
static void led_toggle(void)
{
    LED_PORT->ODR ^= (1U << LED_PIN);
}

int main(void)
{
    system_clock_init();
    led_init();
    uart_init(UART_BAUDRATE);
    adc_init();
    sensor_init();
    logger_init();
    timer_init(SAMPLE_PERIOD_SEC);

    uart_send_string("\r\nSTM32 Environmental Monitor\r\n");
    uart_send_string("Sampling internal temperature once per second.\r\n\r\n");

    uint32_t samples_since_dump = 0;

    for (;;)
    {
        /* Sleep until an interrupt wakes the core. The timer update is the
         * only periodic source, so the loop body runs once per second. */
        __WFI();

        if (timer_tick_pending() != 0U)
        {
            timer_clear_tick();

            sensor_reading_t reading = sensor_read();
            uint32_t         now     = timer_uptime_seconds();

            logger_add(now, reading);
            led_toggle();

            /* Emit a one-line telemetry record for this sample. */
            uart_send_string("[");
            uart_send_int((int32_t)now);
            uart_send_string("s] temp=");
            uart_send_temperature(reading.temperature_dc);
            uart_send_string("C  state=");
            uart_send_string(sensor_state_label(reading.state));

            if (reading.state == SENSOR_STATE_HIGH)
            {
                uart_send_string("  *** HIGH TEMPERATURE ***");
            }
            else if (reading.state == SENSOR_STATE_LOW)
            {
                uart_send_string("  *** LOW TEMPERATURE ***");
            }
            else if (reading.state == SENSOR_STATE_FAULT)
            {
                uart_send_string("  *** SENSOR FAULT ***");
            }

            uart_send_string("\r\n");

            /* Periodically replay the retained history. */
            samples_since_dump++;
            if (samples_since_dump >= DUMP_INTERVAL)
            {
                samples_since_dump = 0;
                logger_dump();
            }
        }
    }
}
