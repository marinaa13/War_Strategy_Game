#include "input.h"
#include "adc.h"
#include <avr/io.h>
#include <util/delay.h>

void input_init(void)
{
    DDRC &= ~((1 << BTN_CONFIRM) |
              (1 << BTN_P1_HIT) |
              (1 << BTN_P1_STAND) |
              (1 << BTN_P2_HIT) |
              (1 << BTN_P2_STAND));

    PORTC |= (1 << BTN_CONFIRM) |
             (1 << BTN_P1_HIT) |
             (1 << BTN_P1_STAND) |
             (1 << BTN_P2_HIT) |
             (1 << BTN_P2_STAND);
}

uint8_t button_pressed(uint8_t pin)
{
    if (!(PINC & (1 << pin))) {
        _delay_ms(30);

        if (!(PINC & (1 << pin))) {
            while (!(PINC & (1 << pin)));
            _delay_ms(30);
            return 1;
        }
    }

    return 0;
}

uint8_t button_confirm_pressed(void)
{
    return button_pressed(BTN_CONFIRM);
}

uint8_t button_p1_hit_pressed(void)
{
    return button_pressed(BTN_P1_HIT);
}

uint8_t button_p1_stand_pressed(void)
{
    return button_pressed(BTN_P1_STAND);
}

uint8_t button_p2_hit_pressed(void)
{
    return button_pressed(BTN_P2_HIT);
}

uint8_t button_p2_stand_pressed(void)
{
    return button_pressed(BTN_P2_STAND);
}

uint8_t read_difficulty(void)
{
    uint16_t value = myAnalogRead(0); // același potențiometru pe A0

    if (value < 341)
        return DIFF_EASY;
    else if (value < 682)
        return DIFF_MEDIUM;

    return DIFF_HARD;
}