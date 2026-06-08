/**
 ******************************************************************************
 * @file    timer_driver_host.c
 * @brief   PC-simulation implementation of the periodic-timer interface.
 *
 * On real hardware TIM2 raises an update interrupt once per second, which sets
 * a tick flag and advances an uptime counter. A PC has no such interrupt, so
 * the simulation provides the same interface backed by plain variables. The
 * extra helper timer_sim_advance stands in for one timer interrupt and is
 * called by the simulation main loop to drive the acquisition cadence.
 ******************************************************************************
 */

#include "timer_driver.h"

static uint32_t s_uptime_sec = 0;   /* Seconds since timer_init.            */
static uint8_t  s_tick       = 0;   /* Pending-tick flag, as on hardware.   */

void timer_init(uint32_t period_sec)
{
    (void)period_sec;
    s_uptime_sec = 0;
    s_tick       = 0;
}

uint8_t timer_tick_pending(void)
{
    return s_tick;
}

void timer_clear_tick(void)
{
    s_tick = 0;
}

uint32_t timer_uptime_seconds(void)
{
    return s_uptime_sec;
}

/* Host-only helper: simulate one timer interrupt firing. Declared where it is
 * used in the simulation main loop. */
void timer_sim_advance(void)
{
    s_uptime_sec++;
    s_tick = 1;
}
