/**
 ******************************************************************************
 * @file    adc_driver_host.c
 * @brief   PC-simulation implementation of the ADC temperature interface.
 *
 * Real hardware reads the on-die temperature sensor through ADC1. With no
 * sensor present on a PC, adc_read_channel returns raw 12-bit codes drawn from
 * a built-in temperature profile that walks through normal, high, low and
 * fault conditions so every classification path is exercised. The raw-to-
 * Celsius conversion in adc_read_temperature_dc is identical to the on-target
 * driver, so the simulation also demonstrates the real fixed-point maths and
 * the quantisation error a 12-bit ADC introduces.
 ******************************************************************************
 */

#include "adc_driver.h"

/* Datasheet constants for the STM32F411 on-die temperature sensor, matching
 * the on-target driver. */
#define TEMP_V25_MV          760          /* Sensor output at 25 C, mV       */
#define TEMP_OFFSET_DC       250          /* 25.0 C expressed in tenths      */

/* Sentinel inside the profile that forces an out-of-range reading so the
 * FAULT branch in the sensor layer is demonstrated. */
#define PROFILE_FAULT        (-30000)

/* Synthetic temperature walk in tenths of a degree Celsius. The thresholds in
 * main.h are HIGH >= 40.0 C (400) and LOW <= 5.0 C (50). */
static const int profile_dc[] = {
    220, 235, 248, 260, 255, 270, 285, 300, 312, 298,   /* normal band      */
    330, 360, 405, 430, 415, 360,                        /* climb past HIGH  */
    300, 250, 180,  90,  45,  20,                        /* fall below LOW   */
    PROFILE_FAULT,                                       /* sensor fault     */
    160, 210, 245, 258, 263                              /* recover to normal*/
};
static const int profile_len = (int)(sizeof(profile_dc) / sizeof(profile_dc[0]));
static int       profile_pos = 0;

/* Inverse of the firmware conversion: turn a target temperature in tenths of a
 * degree into the raw 12-bit code that would produce roughly that reading. */
static uint16_t raw_from_target_dc(int target_dc)
{
    int  vsense_mv = ((target_dc - TEMP_OFFSET_DC) / 4) + TEMP_V25_MV;
    long raw       = (long)vsense_mv * (long)ADC_FULL_SCALE / (long)ADC_VREF_MV;

    if (raw < 0)
    {
        raw = 0;
    }
    if (raw > (long)(ADC_FULL_SCALE - 1U))
    {
        raw = (long)(ADC_FULL_SCALE - 1U);
    }
    return (uint16_t)raw;
}

void adc_init(void)
{
    /* No converter to power up in the simulation. */
    profile_pos = 0;
}

uint16_t adc_read_channel(uint8_t channel)
{
    (void)channel;

    int target = profile_dc[profile_pos];
    profile_pos = (profile_pos + 1) % profile_len;

    if (target == PROFILE_FAULT)
    {
        return 0U;   /* Extreme low code converts to a clearly faulty value. */
    }
    return raw_from_target_dc(target);
}

int16_t adc_read_temperature_dc(void)
{
    uint16_t raw = adc_read_channel((uint8_t)ADC_TEMP_CHANNEL);

    /* Identical fixed-point conversion to the on-target driver. */
    int32_t vsense_mv = ((int32_t)raw * (int32_t)ADC_VREF_MV) / (int32_t)ADC_FULL_SCALE;
    int32_t temp_dc   = ((vsense_mv - TEMP_V25_MV) * 4) + TEMP_OFFSET_DC;

    return (int16_t)temp_dc;
}
