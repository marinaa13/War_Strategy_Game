#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

#define MODE_PVP 0
#define MODE_AI  1

#define DIFF_EASY   0
#define DIFF_MEDIUM 1
#define DIFF_HARD   2

#define BTN_CONFIRM  PC7  // D30
#define BTN_P1_HIT   PC6  // D31
#define BTN_P1_STAND PC5  // D32
#define BTN_P2_HIT   PC4  // D33
#define BTN_P2_STAND PC3  // D34

void input_init(void);

uint8_t button_pressed(uint8_t pin);
uint8_t button_confirm_pressed(void);
uint8_t button_p1_hit_pressed(void);
uint8_t button_p1_stand_pressed(void);
uint8_t button_p2_hit_pressed(void);
uint8_t button_p2_stand_pressed(void);

uint8_t read_game_mode(void);
uint8_t read_difficulty(void);

#endif