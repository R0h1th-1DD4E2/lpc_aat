/**
 * @file gpio.c
 * @brief Simple GPIO HAL Implementation for LPC1768
 */

#include "gpio.h"
#include <lpc17xx.h>

/* ==================== LPC1768 Register Definitions ==================== */

// GPIO Fast I/O registers (word access)
typedef struct {
    volatile uint32_t FIODIR;   // Direction register (0x00)
    uint32_t RESERVED0[3];
    volatile uint32_t FIOMASK;  // Mask register (0x10)
    volatile uint32_t FIOPIN;   // Pin value register (0x14)
    volatile uint32_t FIOSET;   // Output set register (0x18)
    volatile uint32_t FIOCLR;   // Output clear register (0x1C)
} LPC_GPIO_TypeDef;

// GPIO base addresses
// #define LPC_GPIO0  ((LPC_GPIO_TypeDef *) 0x2009C000)
// #define LPC_GPIO1  ((LPC_GPIO_TypeDef *) 0x2009C020)
// #define LPC_GPIO2  ((LPC_GPIO_TypeDef *) 0x2009C040)
// #define LPC_GPIO3  ((LPC_GPIO_TypeDef *) 0x2009C060)
// #define LPC_GPIO4  ((LPC_GPIO_TypeDef *) 0x2009C080)


// PINSEL registers for pin function selection
// #define LPC_PINCON_BASE   0x4002C000
// #define PINSEL0   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x00))
// #define PINSEL1   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x04))
// #define PINSEL2   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x08))
// #define PINSEL3   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x0C))
// #define PINSEL4   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x10))
// #define PINSEL7   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x1C))
// #define PINSEL9   (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x24))
// #define PINSEL10  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x28))

// PINMODE registers for pull-up/pull-down configuration
// #define PINMODE0  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x40))
// #define PINMODE1  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x44))
// #define PINMODE2  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x48))
// #define PINMODE3  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x4C))
// #define PINMODE4  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x50))
// #define PINMODE7  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x5C))
// #define PINMODE9  (*(volatile uint32_t *)(LPC_PINCON_BASE + 0x64))

// Already defined in lpc17xx.h

/* ==================== Helper Functions ==================== */

// Get GPIO port structure from pin number
static LPC_GPIO_TypeDef* get_gpio_port(uint8_t pin) {
    uint8_t port = pin >> 5;  // Extract port number (bits 7:5)
    
    switch(port) {
        case 0: return LPC_GPIO0;
        case 1: return LPC_GPIO1;
        case 2: return LPC_GPIO2;
        case 3: return LPC_GPIO3;
        case 4: return LPC_GPIO4;
        default: return LPC_GPIO0;
    }
}

// Get pin mask from pin number
static uint32_t get_pin_mask(uint8_t pin) {
    uint8_t pin_num = pin & 0x1F;  // Extract pin number (bits 4:0)
    return (1 << pin_num);
}

// Configure pin function (set to GPIO mode)
static void set_pin_function(uint8_t pin) {
    uint8_t port = pin >> 5;
    uint8_t pin_num = pin & 0x1F;
    uint8_t bit_pos = (pin_num % 16) * 2;  // Each pin uses 2 bits
    
    volatile uint32_t *pinsel_reg;
    
    // Select appropriate PINSEL register
    if (port == 0) {
        pinsel_reg = (pin_num < 16) ? &PINSEL0 : &PINSEL1;
    } else if (port == 1) {
        pinsel_reg = (pin_num < 16) ? &PINSEL2 : &PINSEL3;
    } else if (port == 2) {
        pinsel_reg = &PINSEL4;
    } else if (port == 3) {
        pinsel_reg = &PINSEL7;
    } else if (port == 4) {
        pinsel_reg = &PINSEL9;
    } else {
        return;
    }
    
    // Clear and set function bits (00 = GPIO)
    *pinsel_reg &= ~(0x3 << bit_pos);
}

// Configure pin pull mode
static void set_pin_pull(uint8_t pin, gpio_pull_t pull) {
    uint8_t port = pin >> 5;
    uint8_t pin_num = pin & 0x1F;
    uint8_t bit_pos = (pin_num % 16) * 2;  // Each pin uses 2 bits
    
    volatile uint32_t *pinmode_reg;
    
    // Select appropriate PINMODE register
    if (port == 0) {
        pinmode_reg = (pin_num < 16) ? &PINMODE0 : &PINMODE1;
    } else if (port == 1) {
        pinmode_reg = (pin_num < 16) ? &PINMODE2 : &PINMODE3;
    } else if (port == 2) {
        pinmode_reg = &PINMODE4;
    } else if (port == 3) {
        pinmode_reg = &PINMODE7;
    } else if (port == 4) {
        pinmode_reg = &PINMODE9;
    } else {
        return;
    }
    
    // Clear and set pull mode bits
    *pinmode_reg &= ~(0x3 << bit_pos);
    *pinmode_reg |= (pull << bit_pos);
}

/* ==================== Public Functions ==================== */

void gpio_init(void) {
    // GPIO is always powered on LPC1768, no action needed
    // This function exists for consistency and future expansion
}

void gpio_config(uint8_t pin, gpio_dir_t dir, gpio_pull_t pull) {
    LPC_GPIO_TypeDef *gpio = get_gpio_port(pin);
    uint32_t mask = get_pin_mask(pin);
    
    // 1. Set pin function to GPIO (00)
    set_pin_function(pin);
    
    // 2. Configure pull mode
    set_pin_pull(pin, pull);
    
    // 3. Set direction
    if (dir == GPIO_OUTPUT) {
        gpio->FIODIR |= mask;   // Set bit = output
    } else {
        gpio->FIODIR &= ~mask;  // Clear bit = input
    }
}

void gpio_write(uint8_t pin, gpio_state_t value) {
    LPC_GPIO_TypeDef *gpio = get_gpio_port(pin);
    uint32_t mask = get_pin_mask(pin);
    
    if (value == GPIO_HIGH) {
        gpio->FIOSET = mask;  // Set pin high
    } else {
        gpio->FIOCLR = mask;  // Set pin low
    }
}

gpio_state_t gpio_read(uint8_t pin) {
    LPC_GPIO_TypeDef *gpio = get_gpio_port(pin);
    uint32_t mask = get_pin_mask(pin);
    
    // Read pin state
    return (gpio->FIOPIN & mask) ? GPIO_HIGH : GPIO_LOW;
}

void gpio_toggle(uint8_t pin) {
    LPC_GPIO_TypeDef *gpio = get_gpio_port(pin);
    uint32_t mask = get_pin_mask(pin);
    
    // XOR to toggle
    gpio->FIOPIN ^= mask;
}