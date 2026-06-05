/**
 ******************************************************************************
 * @file    adc_driver.h
 * @brief   Register-level ADC1 driver for the internal temperature sensor.
 *
 * Provides single-conversion reads on ADC1 and a conversion routine that maps
 * the on-die temperature sensor voltage to degrees Celsius (tenths).
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef ADC_DRIVER_H
#define ADC_DRIVER_H

#include <stdint.h>

/* ADC1 internal channel that carries the on-die temperature sensor on the
 * STM32F411. The common control register must also enable TSVREFE. */
#define ADC_TEMP_CHANNEL     18U

/* Reference voltage on the NUCLEO-F411RE analog supply, in millivolts. */
#define ADC_VREF_MV          3300U

/* 12-bit successive-approximation resolution. */
#define ADC_FULL_SCALE       4096U

/**
 * @brief  Enables the ADC1 clock, powers the converter and turns on the
 *         internal temperature/reference channel.
 * @retval None.
 */
void adc_init(void);

/**
 * @brief  Runs one software-triggered conversion on the requested channel.
 * @param  channel  ADC channel number (0..18).
 * @retval Raw 12-bit conversion result (0..4095).
 */
uint16_t adc_read_channel(uint8_t channel);

/**
 * @brief  Reads the internal temperature sensor and converts to Celsius.
 * @retval Temperature in tenths of a degree Celsius (e.g. 253 = 25.3 C).
 */
int16_t adc_read_temperature_dc(void);

#endif /* ADC_DRIVER_H */
