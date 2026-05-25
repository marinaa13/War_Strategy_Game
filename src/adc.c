#include <avr/io.h>
#include <stdint.h>

#include "adc.h"

/* ADC init */
void adc_init(void)
{
    // ADC prescaler = 128
    // La 16 MHz: frecvența ADC = 16MHz / 128 = 125kHz, ok pentru ADC
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

    // Referință ADC = AVCC
    ADMUX |= (1 << REFS0);

    // Pentru Mega 2560: ne asigurăm că MUX5 este 0,
    // adică folosim canalele ADC0-ADC7, nu ADC8-ADC15
#ifdef MUX5
    ADCSRB &= ~(1 << MUX5);
#endif

    // Enable ADC
    ADCSRA |= (1 << ADEN);
}

/* ADC read */
uint16_t myAnalogRead(uint8_t channel)
{
    // Pe Mega 2560:
    // A0 = PF0 = ADC0
    // A1 = PF1 = ADC1
    // ...
    // A7 = PF7 = ADC7
    channel &= 0b00000111;

#ifdef MUX5
    ADCSRB &= ~(1 << MUX5); // folosim ADC0-ADC7
#endif

    // Păstrăm REFS0/REFS1 și ADLAR, ștergem canalul vechi
    ADMUX &= 0b11100000;

    // Selectăm canalul ADC nou
    ADMUX |= channel;

    // Start single conversion
    ADCSRA |= (1 << ADSC);

    // Wait for conversion to complete
    while (ADCSRA & (1 << ADSC));

    return ADC;
}