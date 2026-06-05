/**
 ******************************************************************************
 * @file    timer_driver.c
 * @brief   Register-level TIM2 driver implementation.
 *
 * TIM2 is clocked from APB1. With a prescaler that divides the timer clock to
 * 10 kHz, an auto-reload of 10000 counts per second yields a one-second update
 * event. The interrupt only updates two volatile state variables, keeping
 * interrupt latency low and leaving acquisition work to the main loop.
 * Author  : Sai Deepika Alluri
 ******************************************************************************
 */

#include "timer_driver.h"
#include "main.h"
#include "stm32f4xx.h"

/* Counts per second after prescaling the timer clock down to 10 kHz. */
#define TIMER_TICK_HZ        10000UL
#define TIMER_PRESCALER      ((APB1_CLK_HZ / TIMER_TICK_HZ) - 1UL)

/* Shared state touched by both the ISR and the main loop. The volatile
 * qualifier prevents the compiler from caching these across the loop body. */
static volatile uint8_t  s_tick_flag    = 0;
static volatile uint32_t s_uptime_sec   = 0;
static uint32_t          s_period_sec   = 1;

void timer_init(uint32_t period_sec)
{
    if (period_sec == 0U)
    {
        period_sec = 1U;
    }
    s_period_sec = period_sec;

    /* Enable the TIM2 peripheral clock on APB1. */
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    /* Stop the counter while it is reconfigured. */
    TIM2->CR1 &= ~TIM_CR1_CEN;

    /* Prescale the timer clock to a 10 kHz tick and set the auto-reload so the
     * counter overflows once every period_sec seconds. */
    TIM2->PSC = (uint32_t)TIMER_PRESCALER;
    TIM2->ARR = (TIMER_TICK_HZ * period_sec) - 1UL;

    /* Force the prescaler to load and clear any stale update flag. */
    TIM2->EGR  = TIM_EGR_UG;
    TIM2->SR  &= ~TIM_SR_UIF;

    /* Enable the update interrupt at the peripheral and in the NVIC. */
    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_SetPriority(TIM2_IRQn, 2U);
    NVIC_EnableIRQ(TIM2_IRQn);

    /* Start the counter. */
    TIM2->CR1 |= TIM_CR1_CEN;
}

uint8_t timer_tick_pending(void)
{
    return s_tick_flag;
}

void timer_clear_tick(void)
{
    s_tick_flag = 0;
}

uint32_t timer_uptime_seconds(void)
{
    return s_uptime_sec;
}

/**
 * @brief  TIM2 interrupt service routine.
 *
 * Clears the update flag, advances the uptime counter and signals the main
 * loop that a new acquisition window has opened.
 * @retval None.
 */
void TIM2_IRQHandler(void)
{
    if ((TIM2->SR & TIM_SR_UIF) != 0U)
    {
        TIM2->SR &= ~TIM_SR_UIF;     /* Acknowledge the update event.        */
        s_uptime_sec += s_period_sec;
        s_tick_flag = 1U;
    }
}
