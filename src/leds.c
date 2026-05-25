#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "leds.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

#include "input.h"   // pentru MODE_AI / MODE_PVP

#define LED_COUNT 5

// Arduino Mega 2560:
// D52 = PB1 -> banda de sus
// D50 = PB3 -> banda de jos
#define STRIP_TOP_PIN     PB3
#define STRIP_BOTTOM_PIN  PB1

static inline void ws2812_send_bit(uint8_t pin, uint8_t bit)
{
    if (bit) {
        PORTB |= (1 << pin);

        asm volatile(
            "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
            "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
        );

        PORTB &= ~(1 << pin);

        asm volatile(
            "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
            "nop\n\t"
        );
    } else {
        PORTB |= (1 << pin);

        asm volatile(
            "nop\n\t""nop\n\t"
        );

        PORTB &= ~(1 << pin);

        asm volatile(
            "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
            "nop\n\t""nop\n\t""nop\n\t""nop\n\t"
            "nop\n\t""nop\n\t"
        );
    }
}

static void ws2812_send_byte(uint8_t pin, uint8_t byte)
{
    for (uint8_t mask = 0x80; mask != 0; mask >>= 1) {
        ws2812_send_bit(pin, byte & mask);
    }
}

// WS2812B foloseste de obicei ordinea GRB
static void ws2812_send_color(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
    ws2812_send_byte(pin, g);
    ws2812_send_byte(pin, r);
    ws2812_send_byte(pin, b);
}

static void strip_fill(uint8_t pin, uint8_t r, uint8_t g, uint8_t b)
{
    uint8_t sreg = SREG;
    cli();

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        ws2812_send_color(pin, r, g, b);
    }

    SREG = sreg;

    _delay_us(80);
}

static void strip_one_pixel(uint8_t pin,
                            uint8_t index,
                            uint8_t r,
                            uint8_t g,
                            uint8_t b)
{
    uint8_t sreg = SREG;
    cli();

    for (uint8_t i = 0; i < LED_COUNT; i++) {
        if (i == index) {
            ws2812_send_color(pin, r, g, b);
        } else {
            ws2812_send_color(pin, 0, 0, 0);
        }
    }

    SREG = sreg;

    _delay_us(80);
}

static void both_fill(uint8_t top_r,
                      uint8_t top_g,
                      uint8_t top_b,
                      uint8_t bottom_r,
                      uint8_t bottom_g,
                      uint8_t bottom_b)
{
    strip_fill(STRIP_TOP_PIN, top_r, top_g, top_b);
    strip_fill(STRIP_BOTTOM_PIN, bottom_r, bottom_g, bottom_b);
}

void leds_init(void)
{
    DDRB |= (1 << STRIP_TOP_PIN) | (1 << STRIP_BOTTOM_PIN);

    PORTB &= ~((1 << STRIP_TOP_PIN) | (1 << STRIP_BOTTOM_PIN));

    leds_clear();
}

void leds_clear(void)
{
    strip_fill(STRIP_TOP_PIN, 0, 0, 0);
    strip_fill(STRIP_BOTTOM_PIN, 0, 0, 0);
}

void leds_start_animation(void)
{
    leds_clear();
    _delay_ms(200);

    /*
     * 1. Loading din exterior spre centru
     */
    for (uint8_t step = 0; step < LED_COUNT; step++) {
        strip_one_pixel(STRIP_TOP_PIN, step, 0, 25, 45);
        strip_one_pixel(STRIP_BOTTOM_PIN, LED_COUNT - 1 - step, 0, 25, 45);
        _delay_ms(180);
    }

    leds_clear();
    _delay_ms(150);

    /*
     * 2. Loading invers, din centru spre exterior
     */
    for (int8_t step = LED_COUNT - 1; step >= 0; step--) {
        strip_one_pixel(STRIP_TOP_PIN, step, 35, 0, 45);
        strip_one_pixel(STRIP_BOTTOM_PIN, LED_COUNT - 1 - step, 35, 0, 45);
        _delay_ms(180);

        if (step == 0) {
            break;
        }
    }

    leds_clear();
    _delay_ms(200);

    /*
     * 3. Puls mov/albastru, ca efect de pornire
     */
    for (uint8_t brightness = 5; brightness <= 50; brightness += 5) {
        both_fill(brightness, 0, brightness,
                  0, brightness / 2, brightness);
        _delay_ms(70);
    }

    for (uint8_t brightness = 50; brightness >= 5; brightness -= 5) {
        both_fill(brightness, 0, brightness,
                  0, brightness / 2, brightness);
        _delay_ms(70);

        if (brightness == 5) {
            break;
        }
    }

    leds_clear();
    _delay_ms(150);

    /*
     * 4. Alternare sus/jos, ca un semnal de start
     */
    for (uint8_t i = 0; i < 3; i++) {
        strip_fill(STRIP_TOP_PIN, 0, 45, 0);
        strip_fill(STRIP_BOTTOM_PIN, 0, 0, 0);
        _delay_ms(180);

        strip_fill(STRIP_TOP_PIN, 0, 0, 0);
        strip_fill(STRIP_BOTTOM_PIN, 0, 45, 0);
        _delay_ms(180);
    }

    leds_clear();
    _delay_ms(150);


    /*
     * 5. Flash final de start
     */
    for (uint8_t i = 0; i < 3; i++) {
        both_fill(0, 50, 0,
                  0, 50, 0);
        _delay_ms(140);

        leds_clear();
        _delay_ms(120);
    }

    leds_clear();
}

void leds_mode_selected_animation(uint8_t mode)
{
    if (mode == MODE_AI) {
        // Player vs AI: mov/albastru
        for (uint8_t i = 0; i < 2; i++) {
            both_fill(25, 0, 40,
                      0, 0, 45);
            _delay_ms(220);

            leds_clear();
            _delay_ms(120);
        }
    } else {
        // Player vs Player: verde/cyan
        for (uint8_t i = 0; i < 2; i++) {
            both_fill(0, 45, 0,
                      0, 35, 35);
            _delay_ms(220);

            leds_clear();
            _delay_ms(120);
        }
    }
}

void leds_round_winner_animation(uint8_t winner)
{
    uint8_t winner_pin;

    if (winner == LEDS_WINNER_P1) {
        winner_pin = STRIP_TOP_PIN;
    } else if (winner == LEDS_WINNER_P2) {
        winner_pin = STRIP_BOTTOM_PIN;
    } else {
        // Tie: efect mov pe ambele benzi
        for (uint8_t i = 0; i < LED_COUNT; i++) {
            strip_one_pixel(STRIP_TOP_PIN, i, 35, 0, 45);
            strip_one_pixel(STRIP_BOTTOM_PIN, i, 35, 0, 45);
            _delay_ms(90);
        }

        for (int8_t i = LED_COUNT - 1; i >= 0; i--) {
            strip_one_pixel(STRIP_TOP_PIN, i, 35, 0, 45);
            strip_one_pixel(STRIP_BOTTOM_PIN, i, 35, 0, 45);
            _delay_ms(90);

            if (i == 0) {
                break;
            }
        }

        leds_clear();
        return;
    }

    // Mov mai intens, stanga -> dreapta -> stanga
    for (uint8_t i = 0; i < LED_COUNT; i++) {
        strip_one_pixel(winner_pin, i, 40, 0, 55);
        _delay_ms(100);
    }

    for (int8_t i = LED_COUNT - 1; i >= 0; i--) {
        strip_one_pixel(winner_pin, i, 40, 0, 55);
        _delay_ms(100);

        if (i == 0) {
            break;
        }
    }

    leds_clear();
}

void leds_game_over_animation(uint8_t winner)
{
    uint8_t winner_pin;
    uint8_t loser_pin;

    uint8_t win_r = 0;
    uint8_t win_g = 0;
    uint8_t win_b = 0;

    if (winner == LEDS_WINNER_P1) {
        winner_pin = STRIP_TOP_PIN;
        loser_pin = STRIP_BOTTOM_PIN;

        // Player 1 winner -> verde
        win_r = 0;
        win_g = 50;
        win_b = 0;
    } else if (winner == LEDS_WINNER_P2) {
        winner_pin = STRIP_BOTTOM_PIN;
        loser_pin = STRIP_TOP_PIN;

        // Player 2 / AI winner -> albastru
        win_r = 0;
        win_g = 0;
        win_b = 50;
    } else {
        /*
         * TIE animation: ambele benzi pulseaza alb/galben.
         */
        leds_clear();
        _delay_ms(200);

        // Pas 1: fade in galben
        for (uint8_t brightness = 5; brightness <= 50; brightness += 5) {
            both_fill(brightness, brightness / 2, 0,
                      brightness, brightness / 2, 0);
            _delay_ms(90);
        }

        // Pas 2: fade out
        for (uint8_t brightness = 50; brightness >= 5; brightness -= 5) {
            both_fill(brightness, brightness / 2, 0,
                      brightness, brightness / 2, 0);
            _delay_ms(90);

            if (brightness == 5) {
                break;
            }
        }

        // Pas 3: chase sincron
        for (uint8_t i = 0; i < LED_COUNT; i++) {
            strip_one_pixel(STRIP_TOP_PIN, i, 45, 30, 0);
            strip_one_pixel(STRIP_BOTTOM_PIN, i, 45, 30, 0);
            _delay_ms(140);
        }

        // Pas 4: flash ambele
        for (uint8_t i = 0; i < 4; i++) {
            both_fill(45, 35, 0,
                      45, 35, 0);
            _delay_ms(160);

            leds_clear();
            _delay_ms(120);
        }

        // Pas 5: final fade alb
        for (uint8_t brightness = 5; brightness <= 45; brightness += 5) {
            both_fill(brightness, brightness, brightness,
                      brightness, brightness, brightness);
            _delay_ms(70);
        }

        _delay_ms(400);
        leds_clear();
        return;
    }

    leds_clear();
    _delay_ms(200);

    /*
     * PAS 1: winner strip chase
     */
    for (uint8_t loop = 0; loop < 2; loop++) {
        for (uint8_t i = 0; i < LED_COUNT; i++) {
            strip_one_pixel(winner_pin, i, win_r, win_g, win_b);
            strip_fill(loser_pin, 0, 0, 0);
            _delay_ms(140);
        }
    }

    leds_clear();
    _delay_ms(180);

    /*
     * PAS 2: crestere treptata luminozitate pe banda castigatorului
     */
    for (uint8_t brightness = 5; brightness <= 55; brightness += 5) {
        uint8_t r = (win_r > 0) ? brightness : 0;
        uint8_t g = (win_g > 0) ? brightness : 0;
        uint8_t b = (win_b > 0) ? brightness : 0;

        strip_fill(winner_pin, r, g, b);
        strip_fill(loser_pin, 0, 0, 0);

        _delay_ms(90);
    }

    _delay_ms(250);

    /*
     * PAS 3: scadere treptata luminozitate, apoi revenire
     */
    for (uint8_t brightness = 55; brightness >= 10; brightness -= 5) {
        uint8_t r = (win_r > 0) ? brightness : 0;
        uint8_t g = (win_g > 0) ? brightness : 0;
        uint8_t b = (win_b > 0) ? brightness : 0;

        strip_fill(winner_pin, r, g, b);
        strip_fill(loser_pin, 0, 0, 0);

        _delay_ms(80);

        if (brightness == 10) {
            break;
        }
    }

    for (uint8_t brightness = 10; brightness <= 55; brightness += 5) {
        uint8_t r = (win_r > 0) ? brightness : 0;
        uint8_t g = (win_g > 0) ? brightness : 0;
        uint8_t b = (win_b > 0) ? brightness : 0;

        strip_fill(winner_pin, r, g, b);
        strip_fill(loser_pin, 0, 0, 0);

        _delay_ms(80);
    }

    /*
     * PAS 4: efect de celebrare pe ambele benzi
     * Castigatorul ramane colorat, cealalta banda clipeste slab.
     */
    for (uint8_t i = 0; i < 4; i++) {
        strip_fill(winner_pin, win_r, win_g, win_b);
        strip_fill(loser_pin, 15, 15, 15);
        _delay_ms(170);

        strip_fill(winner_pin, win_r, win_g, win_b);
        strip_fill(loser_pin, 0, 0, 0);
        _delay_ms(130);
    }

    /*
     * PAS 5: flash final + fade out
     */
    for (uint8_t i = 0; i < 3; i++) {
        strip_fill(winner_pin, 60, 60, 60);
        strip_fill(loser_pin, 20, 20, 20);
        _delay_ms(120);

        leds_clear();
        _delay_ms(100);
    }

    for (uint8_t brightness = 50; brightness >= 5; brightness -= 5) {
        uint8_t r = (win_r > 0) ? brightness : 0;
        uint8_t g = (win_g > 0) ? brightness : 0;
        uint8_t b = (win_b > 0) ? brightness : 0;

        strip_fill(winner_pin, r, g, b);
        strip_fill(loser_pin, 0, 0, 0);

        _delay_ms(90);

        if (brightness == 5) {
            break;
        }
    }

    leds_clear();
}