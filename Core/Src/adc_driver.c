/**
 ******************************************************************************
 * @file    adc_driver.c
 * @brief   Register-level ADC1 driver implementation.
 *
 * The driver runs ADC1 in single-conversion, software-triggered mode at
 * 12-bit resolution. The internal temperature sensor on channel 18 requires a
 * long sample time and the TSVREFE bit in the shared common control register.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "adc_driver.h"
#include "stm32f4xx.h"

/* Datasheet constants for the STM32F411 on-die temperature sensor. */
#define TEMP_V25_MV          760          /* Sensor output at 25 C, mV       */
/* Average slope is 2.5 mV/C. Multiplying the millivolt delta by 4 and the     */
/* offset by 10 expresses the result directly in tenths of a degree without    */
/* any floating point: (dV / 2.5) * 10 == dV * 4.                              */
#define TEMP_OFFSET_DC       250          /* 25.0 C expressed in tenths       */

void adc_init(void)
{
    /* Gate the ADC1 peripheral clock on the APB2 bus. */
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* Enable the temperature sensor and internal reference path. The TSVREFE
     * bit lives in the ADC common control register shared by all converters. */
    ADC->CCR |= ADC_CCR_TSVREFE;

    /* Twelve-bit resolution is the reset state of CR1[25:24]; clear the field
     * explicitly so the setting does not depend on prior configuration. */
    ADC1->CR1 &= ~ADC_CR1_RES;

    /* Right-aligned data, single conversion (no continuous, no scan). */
    ADC1->CR2 &= ~(ADC_CR2_ALIGN | ADC_CR2_CONT);
    ADC1->SQR1 &= ~ADC_SQR1_L;          /* Sequence length = 1 conversion    */

    /* Longest sample time (480 cycles) for channel 18. The temperature sensor
     * has a high source impedance and needs a generous sampling window. */
    ADC1->SMPR1 |= (0x7U << (3U * (ADC_TEMP_CHANNEL - 10U)));

    /* Power the converter. A short settling delay follows ADON per the
     * reference manual before the first conversion is trustworthy. */
    ADC1->CR2 |= ADC_CR2_ADON;
    for (volatile uint32_t d = 0; d < 10000U; d++)
    {
        __NOP();
    }
}

uint16_t adc_read_channel(uint8_t channel)
{
    /* Place the requested channel as the only entry in the regular sequence. */
    ADC1->SQR3 = (uint32_t)(channel & 0x1FU);

    /* Software-trigger one conversion. */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Block until the end-of-conversion flag asserts. A bounded guard count
     * prevents a stuck converter from hanging the whole acquisition path. */
    uint32_t guard = 0;
    while (((ADC1->SR & ADC_SR_EOC) == 0U) && (guard < 1000000U))
    {
        guard++;
    }

    /* Reading the data register also clears the EOC flag. */
    return (uint16_t)(ADC1->DR & 0x0FFFU);
}

int16_t adc_read_temperature_dc(void)
{
    uint16_t raw = adc_read_channel((uint8_t)ADC_TEMP_CHANNEL);

    /* Convert the raw code to a sensor voltage in millivolts. */
    int32_t vsense_mv = ((int32_t)raw * (int32_t)ADC_VREF_MV) / (int32_t)ADC_FULL_SCALE;

    /* Apply the datasheet linear model, producing tenths of a degree. */
    int32_t temp_dc = ((vsense_mv - TEMP_V25_MV) * 4) + TEMP_OFFSET_DC;

    return (int16_t)temp_dc;
}
