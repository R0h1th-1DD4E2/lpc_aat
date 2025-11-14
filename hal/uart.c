/**
 * @file uart.c
 * @brief Simple UART HAL Implementation for LPC1768
 */

#include "uart.h"
#include <lpc17xx.h>
#include <stdarg.h>
#include <stdio.h>

/* ==================== UART Register Structure ==================== */
typedef struct {
    union {
        volatile uint32_t RBR;  // Receiver Buffer (read)
        volatile uint32_t THR;  // Transmit Holding (write)
        volatile uint32_t DLL;  // Divisor Latch LSB
    };
    union {
        volatile uint32_t DLM;  // Divisor Latch MSB
        volatile uint32_t IER;  // Interrupt Enable
    };
    union {
        volatile uint32_t IIR;  // Interrupt ID (read)
        volatile uint32_t FCR;  // FIFO Control (write)
    };
    volatile uint32_t LCR;      // Line Control
    volatile uint32_t MCR;      // Modem Control (UART1 only)
    volatile uint32_t LSR;      // Line Status
    volatile uint32_t MSR;      // Modem Status (UART1 only)
    volatile uint32_t SCR;      // Scratch Pad
    volatile uint32_t ACR;      // Auto-baud Control
    volatile uint32_t ICR;      // IrDA Control
    volatile uint32_t FDR;      // Fractional Divider
    volatile uint32_t reserved;
    volatile uint32_t TER;      // Transmit Enable
} LPC_UART_TypeDef_Custom;

/* ==================== Helper Functions ==================== */

static LPC_UART_TypeDef_Custom* get_uart_base(uart_num_t uart) {
    switch(uart) {
        case UART_0: return (LPC_UART_TypeDef_Custom*)LPC_UART0;
        case UART_1: return (LPC_UART_TypeDef_Custom*)LPC_UART1;
        case UART_2: return (LPC_UART_TypeDef_Custom*)LPC_UART2;
        case UART_3: return (LPC_UART_TypeDef_Custom*)LPC_UART3;
        default: return (LPC_UART_TypeDef_Custom*)LPC_UART0;
    }
}

static void configure_uart_pins(uart_num_t uart) {
    switch(uart) {
        case UART_0:  // P0.2 (TX), P0.3 (RX)
            PINSEL0 &= ~((3<<4) | (3<<6));
            PINSEL0 |= ((1<<4) | (1<<6));  // Function 01
            break;
        case UART_1:  // P2.0 (TX), P2.1 (RX)
            PINSEL4 &= ~((3<<0) | (3<<2));
            PINSEL4 |= ((2<<0) | (2<<2));  // Function 10
            break;
        case UART_2:  // P0.10 (TX), P0.11 (RX)
            PINSEL0 &= ~((3<<20) | (3<<22));
            PINSEL0 |= ((1<<20) | (1<<22));  // Function 01
            break;
        case UART_3:  // P0.0 (TX), P0.1 (RX)
            PINSEL0 &= ~((3<<0) | (3<<2));
            PINSEL0 |= ((2<<0) | (2<<2));  // Function 10
            break;
    }
}

static void power_on_uart(uart_num_t uart) {
    switch(uart) {
        case UART_0: LPC_SC->PCONP |= (1<<3); break;
        case UART_1: LPC_SC->PCONP |= (1<<4); break;
        case UART_2: LPC_SC->PCONP |= (1<<24); break;
        case UART_3: LPC_SC->PCONP |= (1<<25); break;
    }
}

/* ==================== Public Functions ==================== */

void uart_init(uart_num_t uart, uint32_t baud) {
    LPC_UART_TypeDef_Custom *UARTx = get_uart_base(uart);
    
    // 1. Power on UART
    power_on_uart(uart);
    
    // 2. Configure pins
    configure_uart_pins(uart);
    
    // 3. Set PCLK to CCLK/4 = 25MHz (assuming 100MHz CPU)
    uint32_t pclk = 25000000;
    
    // 4. Calculate divisor (assuming FDR = 1.0)
    uint32_t divisor = pclk / (16 * baud);
    
    // 5. Enable DLAB (Divisor Latch Access Bit)
    UARTx->LCR = (1<<7);
    
    // 6. Set divisor
    UARTx->DLL = divisor & 0xFF;
    UARTx->DLM = (divisor >> 8) & 0xFF;
    
    // 7. Configure: 8-N-1 (8 data bits, no parity, 1 stop bit)
    UARTx->LCR = 0x03;  // DLAB=0, 8-bit data
    
    // 8. Enable FIFO, reset TX/RX FIFOs
    UARTx->FCR = 0x07;
    
    // 9. Enable transmission
    UARTx->TER = 0x80;
}

void uart_putc(uart_num_t uart, char data) {
    LPC_UART_TypeDef_Custom *UARTx = get_uart_base(uart);
    
    // Wait until THR is empty (bit 5 of LSR)
    while((UARTx->LSR & (1<<5)) == 0);
    
    UARTx->THR = data;
}

void uart_puts(uart_num_t uart, const char *str) {
    while(*str) {
        uart_putc(uart, *str++);
    }
}

char uart_getc(uart_num_t uart) {
    LPC_UART_TypeDef_Custom *UARTx = get_uart_base(uart);
    
    // Wait until data ready (bit 0 of LSR)
    while((UARTx->LSR & (1<<0)) == 0);
    
    return UARTx->RBR & 0xFF;
}

bool uart_available(uart_num_t uart) {
    LPC_UART_TypeDef_Custom *UARTx = get_uart_base(uart);
    return (UARTx->LSR & (1<<0)) != 0;
}

void uart_printf(uart_num_t uart, const char *format, ...) {
    char buffer[256];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    uart_puts(uart, buffer);
}