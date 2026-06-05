/**
 ******************************************************************************
 * @file    timer_driver.h
 * @brief   Register-level TIM2 driver producing a periodic update interrupt.
 *
 * TIM2 is configured to overflow once per second. Its interrupt service
 * routine advances a seconds counter and raises a flag that the application
 * polls in the main loop, keeping the ISR short and deterministic.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include <stdint.h>

/**
 * @brief  Configures TIM2 to generate an update interrupt every period_sec.
 * @param  period_sec  Interrupt period in whole seconds.
 * @retval None.
 */
void timer_init(uint32_t period_sec);

/**
 * @brief  Reports whether a timer tick has elapsed since the last clear.
 * @retval Non-zero when a new tick is pending, zero otherwise.
 */
uint8_t timer_tick_pending(void);

/**
 * @brief  Clears the pending-tick flag after the application has serviced it.
 * @retval None.
 */
void timer_clear_tick(void);

/**
 * @brief  Returns the number of seconds elapsed since timer_init ran.
 * @retval Monotonic seconds counter.
 */
uint32_t timer_uptime_seconds(void);

#endif /* TIMER_DRIVER_H */
