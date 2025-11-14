/**
 * @file countdown_timer.c
 * @brief Countdown Timer with 4-digit 7-segment display
 */

#include "gpio.h"
#include "systick.h"

// Button pins
#define BTN_COUNTDOWN   P1_20  // Switch to countdown mode
#define BTN_SET         P1_21  // Increment time in set mode
#define BTN_START       P1_22  // Start/Pause toggle
#define BTN_RESET       P1_23  // Reset timer

// 7-segment pins (segments a-h)
#define SEG_A   P0_0
#define SEG_B   P0_1
#define SEG_C   P0_2
#define SEG_D   P0_3
#define SEG_E   P0_4
#define SEG_F   P0_5
#define SEG_G   P0_6
#define SEG_DP  P0_7  // Decimal point

// Digit enable pins (common cathode/anode)
#define DIGIT_1   P2_0  // Leftmost digit
#define DIGIT_2   P2_1
#define DIGIT_3   P2_2
#define DIGIT_4   P2_3  // Rightmost digit

// 7-segment patterns (common cathode: 1=on, 0=off)
const uint8_t seg_patterns[10] = {
    0x3F, // 0: abcdef
    0x06, // 1: bc
    0x5B, // 2: abdeg
    0x4F, // 3: abcdg
    0x66, // 4: bcfg
    0x6D, // 5: acdfg
    0x7D, // 6: acdefg
    0x07, // 7: abc
    0x7F, // 8: abcdefg
    0x6F  // 9: abcdfg
};

const uint8_t seg_pins[8] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};
const uint8_t digit_pins[4] = {DIGIT_1, DIGIT_2, DIGIT_3, DIGIT_4};

// Timer state
typedef enum {
    STATE_SET,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_DONE
} timer_state_t;

volatile timer_state_t state = STATE_SET;
volatile uint32_t timer_value = 0;  // In seconds
volatile uint32_t set_value = 60;   // Default 60 seconds
volatile uint32_t last_tick = 0;
volatile uint8_t btn_start_prev = 0;
volatile uint8_t btn_countdown_prev = 0;
volatile uint8_t btn_set_prev = 0;
volatile uint8_t btn_reset_prev = 0;

void display_init(void) {
    // Configure segment pins
    for(uint8_t i = 0; i < 8; i++) {
        gpio_config(seg_pins[i], GPIO_OUTPUT, GPIO_PULL_NONE);
        gpio_write(seg_pins[i], GPIO_LOW);
    }
    
    // Configure digit enable pins
    for(uint8_t i = 0; i < 4; i++) {
        gpio_config(digit_pins[i], GPIO_OUTPUT, GPIO_PULL_NONE);
        gpio_write(digit_pins[i], GPIO_LOW);
    }
}

void buttons_init(void) {
    gpio_config(BTN_COUNTDOWN, GPIO_INPUT, GPIO_PULL_UP);
    gpio_config(BTN_SET, GPIO_INPUT, GPIO_PULL_UP);
    gpio_config(BTN_START, GPIO_INPUT, GPIO_PULL_UP);
    gpio_config(BTN_RESET, GPIO_INPUT, GPIO_PULL_UP);
}

void write_segment(uint8_t pattern) {
    for(uint8_t i = 0; i < 7; i++) {
        gpio_write(seg_pins[i], (pattern >> i) & 1);
    }
}

void display_digit(uint8_t digit_pos, uint8_t value, uint8_t dp) {
    // Turn off all digits
    for(uint8_t i = 0; i < 4; i++) {
        gpio_write(digit_pins[i], GPIO_LOW);
    }
    
    // Write segment pattern
    if(value <= 9) {
        write_segment(seg_patterns[value]);
    } else {
        write_segment(0x00);  // Blank
    }
    
    // Decimal point
    gpio_write(SEG_DP, dp);
    
    // Enable selected digit
    gpio_write(digit_pins[digit_pos], GPIO_HIGH);
}

void display_number(uint16_t number) {
    static uint8_t current_digit = 0;
    
    uint8_t d1 = (number / 1000) % 10;
    uint8_t d2 = (number / 100) % 10;
    uint8_t d3 = (number / 10) % 10;
    uint8_t d4 = number % 10;
    
    // Multiplex display
    switch(current_digit) {
        case 0: display_digit(0, d1, 0); break;
        case 1: display_digit(1, d2, 1); break;  // DP after second digit (MM:SS)
        case 2: display_digit(2, d3, 0); break;
        case 3: display_digit(3, d4, 0); break;
    }
    
    current_digit = (current_digit + 1) % 4;
}

uint8_t read_button(uint8_t pin, uint8_t *prev_state) {
    uint8_t current = !gpio_read(pin);  // Active low
    uint8_t pressed = 0;
    
    if(current && !(*prev_state)) {
        pressed = 1;
        delay_ms(50);  // Debounce
    }
    
    *prev_state = current;
    return pressed;
}

void timer_update(void) {
    uint32_t current_time = millis();
    
    if(state == STATE_RUNNING && (current_time - last_tick >= 1000)) {
        last_tick = current_time;
        
        if(timer_value > 0) {
            timer_value--;
        } else {
            state = STATE_DONE;
        }
    }
}

void process_buttons(void) {
    // Countdown mode button
    if(read_button(BTN_COUNTDOWN, &btn_countdown_prev)) {
        if(state == STATE_SET) {
            timer_value = set_value;
        }
    }
    
    // Set button (increment time in set mode)
    if(read_button(BTN_SET, &btn_set_prev)) {
        if(state == STATE_SET) {
            set_value += 10;
            if(set_value > 5999) set_value = 10;  // Max 99:59
            timer_value = set_value;
        }
    }
    
    // Start/Pause button
    if(read_button(BTN_START, &btn_start_prev)) {
        if(state == STATE_SET) {
            state = STATE_RUNNING;
            timer_value = set_value;
            last_tick = millis();
        } else if(state == STATE_RUNNING) {
            state = STATE_PAUSED;
        } else if(state == STATE_PAUSED) {
            state = STATE_RUNNING;
            last_tick = millis();
        } else if(state == STATE_DONE) {
            state = STATE_SET;
            timer_value = set_value;
        }
    }
    
    // Reset button
    if(read_button(BTN_RESET, &btn_reset_prev)) {
        state = STATE_SET;
        timer_value = set_value;
    }
}

uint16_t format_time_mmss(uint32_t seconds) {
    uint32_t minutes = seconds / 60;
    uint32_t secs = seconds % 60;
    return (minutes * 100) + secs;
}

int main(void) {
    systick_init(12000000);  // 12MHz
    gpio_init();
    
    display_init();
    buttons_init();
    
    timer_value = set_value;
    
    while(1) {
        process_buttons();
        timer_update();
        
        uint16_t display_val = format_time_mmss(timer_value);
        display_number(display_val);
        
        delay_us(2000);  // Multiplex delay
    }
    
    return 0;
}