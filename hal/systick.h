/**
 * @file hal_systick.h
 * @brief Simple SysTick-based timing HAL for LPC1768
 * @note Provides Arduino-style delay functions and millisecond counter
 */

#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

/* ==================== Configuration ==================== */
// Default CPU frequency (can be overridden before calling systick_init)
#ifndef SYSTEM_CLOCK_HZ
#define SYSTEM_CLOCK_HZ  100000000UL  // 100 MHz
#endif

/* ==================== Functions ==================== */

/**
 * @brief Initialize SysTick timer for 1ms tick
 * @param cpu_freq_hz CPU frequency in Hz (e.g., 100000000 for 100MHz)
 * @note Call this once at startup before using delay functions
 * @example systick_init(100000000);  // 100 MHz
 */
void systick_init(uint32_t cpu_freq_hz);

/**
 * @brief Delay for specified milliseconds (blocking)
 * @param ms Milliseconds to delay
 * @example delay_ms(1000);  // Wait 1 second
 */
void delay_ms(uint32_t ms);

/**
 * @brief Delay for specified microseconds (blocking)
 * @param us Microseconds to delay
 * @note Accurate for delays > 10us. For shorter delays, use NOP loops
 * @example delay_us(500);  // Wait 500 microseconds
 */
void delay_us(uint32_t us);

/**
 * @brief Get millisecond counter since systick_init() was called
 * @return Milliseconds elapsed (wraps around after ~49 days)
 * @example uint32_t start = millis(); ... uint32_t elapsed = millis() - start;
 */
uint32_t millis(void);

/**
 * @brief Get microsecond counter (high resolution)
 * @return Microseconds elapsed (wraps around after ~71 minutes)
 * @note Uses SysTick current value for sub-millisecond precision
 */
uint32_t micros(void);

/* ==================== Arduino-Style Aliases ==================== */
#define delay(ms)  delay_ms(ms)
#define delayMicroseconds(us)  delay_us(us)

#endif // HAL_SYSTICK_H