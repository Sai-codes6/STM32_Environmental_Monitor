/**
 ******************************************************************************
 * @file    main_host.c
 * @brief   PC-simulation entry point for the Environmental Monitor.
 *
 * This mirrors the on-target main loop but replaces the wait-for-interrupt
 * sleep and the hardware bring-up with a plain bounded loop. Each pass calls
 * timer_sim_advance to stand in for the one-second timer interrupt, then runs
 * exactly the same acquisition body as the firmware: read a sample, classify
 * it, log it into the circular buffer, print a telemetry line, and replay the
 * stored history every DUMP_INTERVAL samples.
 *
 * The sensor classification (sensor.c) and the circular log (logger.c) are the
 * real project sources, compiled unchanged, so the simulation exercises the
 * genuine application logic on a PC with no hardware attached.
 ******************************************************************************
 */

#include "main.h"
#include "adc_driver.h"
#include "uart_driver.h"
#include "timer_driver.h"
#include "sensor.h"
#include "logger.h"

/* Replay the stored history once every DUMP_INTERVAL samples (as in main.c). */
#define DUMP_INTERVAL        10U

/* Number of one-second samples to run before the simulation exits cleanly.
 * Raise this for a longer run, or wrap the loop in for(;;) for a continuous
 * stream. */
#define SIM_SAMPLE_COUNT     30U

/* Provided by timer_driver_host.c: simulate one timer interrupt firing. */
extern void timer_sim_advance(void);

int main(void)
{
    uart_init(UART_BAUDRATE);
    adc_init();
    sensor_init();
    logger_init();
    timer_init(SAMPLE_PERIOD_SEC);

    uart_send_string("\r\nSTM32 Environmental Monitor (PC simulation)\r\n");
    uart_send_string("Sampling a synthetic temperature once per cycle.\r\n\r\n");

    uint32_t samples_since_dump = 0;

    for (uint32_t i = 0; i < SIM_SAMPLE_COUNT; i++)
    {
        /* Stand-in for the 1 Hz timer update interrupt. */
        timer_sim_advance();

        if (timer_tick_pending() != 0U)
        {
            timer_clear_tick();

            sensor_reading_t reading = sensor_read();
            uint32_t         now     = timer_uptime_seconds();

            logger_add(now, reading);
            /* The on-board LED heartbeat has no equivalent on a PC. */

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

            samples_since_dump++;
            if (samples_since_dump >= DUMP_INTERVAL)
            {
                samples_since_dump = 0;
                logger_dump();
            }
        }
    }

    return 0;
}
