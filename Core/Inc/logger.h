/**
 ******************************************************************************
 * @file    logger.h
 * @brief   Fixed-size circular buffer that retains the most recent readings.
 *
 * The logger stores a bounded history of samples in RAM. When the buffer is
 * full the oldest record is overwritten, so storage never grows and the most
 * recent LOG_CAPACITY samples are always available for review over UART.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include "sensor.h"

/* One stored log record: a timestamp paired with its reading. */
typedef struct
{
    uint32_t         timestamp_sec;  /* Seconds since boot at capture time   */
    sensor_reading_t reading;        /* The classified temperature sample    */
} log_record_t;

/**
 * @brief  Resets the circular buffer to an empty state.
 * @retval None.
 */
void logger_init(void);

/**
 * @brief  Appends a record, overwriting the oldest entry when full.
 * @param  timestamp_sec  Capture time in seconds since boot.
 * @param  reading        The reading to store.
 * @retval None.
 */
void logger_add(uint32_t timestamp_sec, sensor_reading_t reading);

/**
 * @brief  Reports how many records are currently held.
 * @retval Count of valid records (0..LOG_CAPACITY).
 */
uint16_t logger_count(void);

/**
 * @brief  Streams the stored history over UART, oldest record first.
 * @retval None.
 */
void logger_dump(void);

#endif /* LOGGER_H */
