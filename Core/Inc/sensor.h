/**
 ******************************************************************************
 * @file    sensor.h
 * @brief   Sensor abstraction layer over the ADC temperature channel.
 *
 * Separates the application from the raw converter by returning a structured
 * reading that already carries the temperature, an out-of-range fault flag and
 * a threshold-alert classification.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

/* Threshold classification attached to every reading. */
typedef enum
{
    SENSOR_STATE_NORMAL = 0,  /* Reading sits inside the comfortable band   */
    SENSOR_STATE_HIGH   = 1,  /* Reading is at or above the high limit      */
    SENSOR_STATE_LOW    = 2,  /* Reading is at or below the low limit       */
    SENSOR_STATE_FAULT  = 3   /* Reading is physically implausible          */
} sensor_state_t;

/* One acquisition result. */
typedef struct
{
    int16_t        temperature_dc;  /* Temperature in tenths of a degree C   */
    sensor_state_t state;           /* Threshold / fault classification      */
} sensor_reading_t;

/**
 * @brief  Prepares the sensor layer; assumes adc_init has already run.
 * @retval None.
 */
void sensor_init(void);

/**
 * @brief  Acquires one temperature sample and classifies it.
 * @retval Populated reading structure.
 */
sensor_reading_t sensor_read(void);

/**
 * @brief  Maps a sensor state to a short printable label.
 * @param  state  Classification to translate.
 * @retval Pointer to a constant null-terminated label string.
 */
const char *sensor_state_label(sensor_state_t state);

#endif /* SENSOR_H */
