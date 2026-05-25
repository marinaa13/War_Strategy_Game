#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>

#define LEDS_WINNER_TIE 0
#define LEDS_WINNER_P1  1
#define LEDS_WINNER_P2  2

void leds_init(void);
void leds_clear(void);

void leds_start_animation(void);
void leds_mode_selected_animation(uint8_t mode);
void leds_round_winner_animation(uint8_t winner);
void leds_game_over_animation(uint8_t winner);

#endif