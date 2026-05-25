#ifndef USART_H
#define USART_H

#include <stdint.h>

void USART0_init(uint32_t baud);

void USART0_transmit(char data);

void USART0_print(const char *s);

void USART0_print_u32(uint32_t value);

#endif