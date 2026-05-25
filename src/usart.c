#include "usart.h"
#include <avr/io.h>
#include <stdint.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

void USART0_init(uint32_t baud)
{
    uint16_t ubrr = (uint16_t)(F_CPU / (16UL * baud) - 1UL);

    UBRR0H = (uint8_t)(ubrr >> 8);
    UBRR0L = (uint8_t)(ubrr & 0xFF);

    UCSR0A = 0;
    UCSR0B = (1 << TXEN0);                        // enable TX
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);       // 8N1
}

void USART0_transmit(char data)
{
    while (!(UCSR0A & (1 << UDRE0))) {;}
    UDR0 = data;
}

void USART0_print(const char *s)
{
    while (*s) {
        USART0_transmit(*s++);
    }
}

void USART0_print_u32(uint32_t value)
{
    char buf[11];
    uint8_t i = 10;

    buf[i] = '\0';

    if (value == 0) {
        USART0_transmit('0');
        return;
    }

    while (value > 0 && i > 0) {
        buf[--i] = '0' + (value % 10);
        value /= 10;
    }

    USART0_print(&buf[i]);
}