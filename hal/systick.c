/**
 * @file systick.c
 * @brief SysTick timer implementation for LPC1768
 */

#include "systick.h"

/* ==================== SysTick Register Definitions ==================== */
// SysTick registers (ARM Cortex-M3 core peripheral)
#define SYSTICK_BASE      0xE000E010

typedef struct {
    volatile uint32_t CTRL;   // Control and Status Register
    volatile uint32_t LOAD;   // Reload Value Register
    volatile uint32_t VAL;    // Current Value Register
    volatile uint32_t CALIB;  // Calibration Value Register
} SYSTICK_Type;

#define SYSTICK  ((SYSTICK_Type *) SYSTICK_BASE)

// SysTick Control Register bits
#define SYSTICK_CTRL_ENABLE     (1 << 0)  // Counter enable
#define SYSTICK_CTRL_TICKINT    (1 << 1)  // Enable interrupt
#define SYSTICK_CTRL_CLKSOURCE  (1 << 2)  // Clock source (1=CPU, 0=external)
#define SYSTICK_CTRL_COUNTFLAG  (1 << 16) // Count flag

static volatile uint32_t systick_counter = 0;  // Millisecond counter
static uint32_t us_per_tick = 0;               // Microseconds per SysTick tick

/**
 * @brief SysTick interrupt handler (called every 1ms)
 * @note This function name is defined by CMSIS and will be automatically called
 */
void SysTick_Handler(void) {
    systick_counter++;
}

void systick_init(uint32_t cpu_freq_hz) {
    // Calculate reload value for 1ms tick
    // SysTick counts down from LOAD to 0, then reloads
    uint32_t reload_value = (cpu_freq_hz / 1000) - 1;  // 1ms tick
    
    // Calculate microseconds per tick for micros() function
    us_per_tick = 1000000 / cpu_freq_hz;
    
    // Disable SysTick during configuration
    SYSTICK->CTRL = 0;
    
    // Set reload value
    SYSTICK->LOAD = reload_value & 0x00FFFFFF;  // 24-bit reload value
    
    // Reset current value
    SYSTICK->VAL = 0;
    
    // Configure and enable SysTick
    // - Enable counter
    // - Enable interrupt
    // - Use processor clock
    SYSTICK->CTRL = SYSTICK_CTRL_ENABLE | 
                    SYSTICK_CTRL_TICKINT | 
                    SYSTICK_CTRL_CLKSOURCE;
    
    // Reset counter
    systick_counter = 0;
}

void delay_ms(uint32_t ms) {
    uint32_t start = systick_counter;
    
    // Wait until elapsed time >= ms
    // Handle counter overflow correctly
    while ((systick_counter - start) < ms) {
        // Optionally: __WFI();  // Wait for interrupt (low power)
    }
}

void delay_us(uint32_t us) {
    // For microsecond delays, we use a combination of millis and cycle counting
    
    if (us >= 1000) {
        // If > 1ms, use millisecond delay for most of it
        uint32_t ms = us / 1000;
        delay_ms(ms);
        us = us % 1000;
    }
    
    if (us == 0) return;
    
    // For sub-millisecond delays, use SysTick current value
    uint32_t start = SYSTICK->VAL;
    uint32_t reload = SYSTICK->LOAD;
    uint32_t ticks_needed = us / us_per_tick;
    
    // SysTick counts DOWN, so we need to handle wraparound
    while (1) {
        uint32_t current = SYSTICK->VAL;
        uint32_t elapsed;
        
        if (current <= start) {
            elapsed = start - current;
        } else {
            // Wrapped around
            elapsed = start + (reload - current);
        }
        
        if (elapsed >= ticks_needed) {
            break;
        }
    }
}

uint32_t millis(void) {
    return systick_counter;
}

uint32_t micros(void) {
    uint32_t ms;
    uint32_t tick_val;
    uint32_t reload;
    
    // Read values (disable interrupts briefly for consistency)
    __disable_irq();
    ms = systick_counter;
    tick_val = SYSTICK->VAL;
    reload = SYSTICK->LOAD;
    __enable_irq();
    
    // Calculate microseconds
    // SysTick counts DOWN, so elapsed = (reload - current)
    uint32_t ticks_elapsed = reload - tick_val;
    uint32_t us_in_current_ms = (ticks_elapsed * 1000) / (reload + 1);
    
    return (ms * 1000) + us_in_current_ms;
}