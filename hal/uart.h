/**
 * @file uart.h
 * @brief Simple UART HAL for LPC1768
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

/* ==================== UART Selection ==================== */
typedef enum {
    UART_0 = 0,
    UART_1 = 1,
    UART_2 = 2,
    UART_3 = 3
} uart_num_t;

/* ==================== Functions ==================== */

/**
 * @brief Initialize UART
 * @param uart UART number (0-3)
 * @param baud Baud rate (e.g., 9600, 115200)
 */
void uart_init(uart_num_t uart, uint32_t baud);

/**
 * @brief Send single byte
 * @param uart UART number
 * @param data Byte to send
 */
void uart_putc(uart_num_t uart, char data);

/**
 * @brief Send string
 * @param uart UART number
 * @param str Null-terminated string
 */
void uart_puts(uart_num_t uart, const char *str);

/**
 * @brief Receive single byte (blocking)
 * @param uart UART number
 * @return Received byte
 */
char uart_getc(uart_num_t uart);

/**
 * @brief Check if data available
 * @param uart UART number
 * @return true if data available
 */
bool uart_available(uart_num_t uart);

/**
 * @brief Send formatted string (printf-style)
 * @param uart UART number
 * @param format Format string
 */
void uart_printf(uart_num_t uart, const char *format, ...);

/* ==================== Arduino-Style Aliases ==================== */
#define Serial uart_0_instance
#define Serial_begin(baud) uart_init(UART_0, baud)
#define Serial_print(str) uart_puts(UART_0, str)
#define Serial_println(str) do { uart_puts(UART_0, str); uart_putc(UART_0, '\n'); } while(0)
#define Serial_available() uart_available(UART_0)
#define Serial_read() uart_getc(UART_0)

#endif // UART_H