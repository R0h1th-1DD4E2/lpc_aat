/**
 * @file gpio.h
 * @brief Simple GPIO HAL for LPC1768
 */

#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>

/* ==================== Pin Naming ==================== */
// Use PORT and PIN like Arduino: GPIO_PIN(0, 22) = P0.22
#define GPIO_PIN(port, pin)  ((port << 5) | (pin & 0x1F))

// Quick pin definitions (add more as needed)
#define P0_0    GPIO_PIN(0, 0)
#define P0_22   GPIO_PIN(0, 22)   
#define P1_18   GPIO_PIN(1, 18)   
#define P1_20   GPIO_PIN(1, 20)   
#define P1_21   GPIO_PIN(1, 21)   
#define P1_23   GPIO_PIN(1, 23)
#define P2_0    GPIO_PIN(2, 0)

/* ==================== Direction ==================== */
typedef enum {
    GPIO_INPUT  = 0,
    GPIO_OUTPUT = 1
} gpio_dir_t;

/* ==================== Pull Mode ==================== */
typedef enum {
    GPIO_PULL_NONE = 0,  // No pull-up/pull-down (floating)
    GPIO_PULL_DOWN = 1,  // Pull-down enabled
    GPIO_PULL_UP   = 2,  // Pull-up enabled
    GPIO_REPEATER  = 3   // Repeater mode (keeps last value)
} gpio_pull_t;

/* ==================== Pin State ==================== */
typedef enum {
    GPIO_LOW  = 0,
    GPIO_HIGH = 1
} gpio_state_t;

/* ==================== Functions ==================== */

/**
 * @brief Initialize GPIO system (call once at startup)
 */
void gpio_init(void);

/**
 * @brief Configure a GPIO pin
 * @param pin Pin number (use GPIO_PIN macro or Px_y defines)
 * @param dir Direction: GPIO_INPUT or GPIO_OUTPUT
 * @param pull Pull mode: GPIO_PULL_NONE, GPIO_PULL_UP, GPIO_PULL_DOWN
 * @example gpio_config(P0_22, GPIO_OUTPUT, GPIO_PULL_NONE);
 */
void gpio_config(uint8_t pin, gpio_dir_t dir, gpio_pull_t pull);

/**
 * @brief Write value to GPIO pin (must be configured as OUTPUT)
 * @param pin Pin number
 * @param value GPIO_HIGH or GPIO_LOW
 */
void gpio_write(uint8_t pin, gpio_state_t value);

/**
 * @brief Read value from GPIO pin
 * @param pin Pin number
 * @return GPIO_HIGH or GPIO_LOW
 */
gpio_state_t gpio_read(uint8_t pin);

/**
 * @brief Toggle GPIO pin output
 * @param pin Pin number (must be configured as OUTPUT)
 */
void gpio_toggle(uint8_t pin);

/* ==================== Arduino-Style Aliases ==================== */
#define pinMode(pin, mode)      gpio_config(pin, mode, GPIO_PULL_NONE)
#define digitalWrite(pin, val)  gpio_write(pin, val)
#define digitalRead(pin)        gpio_read(pin)
#define HIGH                    GPIO_HIGH
#define LOW                     GPIO_LOW
#define INPUT                   GPIO_INPUT
#define OUTPUT                  GPIO_OUTPUT

#endif // GPIO_H