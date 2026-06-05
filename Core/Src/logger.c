/**
 ******************************************************************************
 * @file    logger.c
 * @brief   Fixed-size circular log buffer implementation.
 *
 * Records are written into a static array indexed by a head pointer that wraps
 * at LOG_CAPACITY. A separate count saturates at the capacity so the structure
 * distinguishes the partially filled state from the wrapped state without any
 * dynamic allocation.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "logger.h"
#include "main.h"
#include "uart_driver.h"

/* Static storage for the history. No heap is used anywhere in the firmware. */
static log_record_t s_records[LOG_CAPACITY];
static uint16_t     s_head  = 0;   /* Index of the next slot to write.       */
static uint16_t     s_count = 0;   /* Number of valid records held.          */

void logger_init(void)
{
    s_head  = 0;
    s_count = 0;
}

void logger_add(uint32_t timestamp_sec, sensor_reading_t reading)
{
    s_records[s_head].timestamp_sec = timestamp_sec;
    s_records[s_head].reading       = reading;

    /* Advance the head and wrap. The oldest record is overwritten once the
     * buffer is full, which bounds RAM use regardless of run time. */
    s_head = (uint16_t)((s_head + 1U) % LOG_CAPACITY);

    if (s_count < LOG_CAPACITY)
    {
        s_count++;
    }
}

uint16_t logger_count(void)
{
    return s_count;
}

void logger_dump(void)
{
    /* Compute the index of the oldest stored record. When the buffer has
     * wrapped, the oldest entry sits at the current head; otherwise it sits at
     * index zero. */
    uint16_t start = (s_count == LOG_CAPACITY) ? s_head : 0U;

    uart_send_string("---- LOG HISTORY ----\r\n");

    for (uint16_t i = 0; i < s_count; i++)
    {
        uint16_t idx = (uint16_t)((start + i) % LOG_CAPACITY);

        uart_send_string("t=");
        uart_send_int((int32_t)s_records[idx].timestamp_sec);
        uart_send_string("s  temp=");
        uart_send_temperature(s_records[idx].reading.temperature_dc);
        uart_send_string("C  state=");
        uart_send_string(sensor_state_label(s_records[idx].reading.state));
        uart_send_string("\r\n");
    }

    uart_send_string("---------------------\r\n");
}
