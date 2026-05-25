#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

#include "twi.h"
#include "input.h"
#include "adc.h"
#include "game_ui.h"
#include "game.h"
#include "leds.h"

int main(void)
{
    twi_init();
    input_init();
    adc_init();
    leds_init();

    _delay_ms(300);

    game_ui_init();

    game_run();

    while (1) {
    }
}