/**
 ******************************************************************************
 * @file    sensor.c
 * @brief   Sensor abstraction layer implementation.
 *
 * Wraps the ADC temperature read with range checking. Implausible values are
 * flagged as faults before any threshold comparison so that a disconnected or
 * saturated sensor never masquerades as a valid alert.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "sensor.h"
#include "adc_driver.h"
#include "main.h"

void sensor_init(void)
{
    /* The ADC layer owns the converter configuration. This hook exists so the
     * application initialises the sensor explicitly and so future calibration
     * state has a clear home. */
}

sensor_reading_t sensor_read(void)
{
    sensor_reading_t reading;

    reading.temperature_dc = adc_read_temperature_dc();

    if ((reading.temperature_dc < (int16_t)TEMP_FAULT_MIN_DC) ||
        (reading.temperature_dc > (int16_t)TEMP_FAULT_MAX_DC))
    {
        reading.state = SENSOR_STATE_FAULT;
    }
    else if (reading.temperature_dc >= (int16_t)TEMP_HIGH_LIMIT_DC)
    {
        reading.state = SENSOR_STATE_HIGH;
    }
    else if (reading.temperature_dc <= (int16_t)TEMP_LOW_LIMIT_DC)
    {
        reading.state = SENSOR_STATE_LOW;
    }
    else
    {
        reading.state = SENSOR_STATE_NORMAL;
    }

    return reading;
}

const char *sensor_state_label(sensor_state_t state)
{
    switch (state)
    {
        case SENSOR_STATE_NORMAL: return "NORMAL";
        case SENSOR_STATE_HIGH:   return "HIGH-ALERT";
        case SENSOR_STATE_LOW:    return "LOW-ALERT";
        case SENSOR_STATE_FAULT:  return "FAULT";
        default:                  return "UNKNOWN";
    }
}
