#ifndef GAME_UI_H
#define GAME_UI_H

#include <stdint.h>

#define GAME_UI_STATUS_DISPLAY 0
#define GAME_UI_P1_CARD_DISPLAY 1
#define GAME_UI_P2_CARD_DISPLAY 2
#define GAME_UI_P1_MSG_DISPLAY 3
#define GAME_UI_P2_MSG_DISPLAY 4

void game_ui_init(void);
void game_ui_clear_all(void);

void game_ui_show_start_screen(void);
void game_ui_show_mode(uint8_t mode);
void game_ui_show_difficulty(uint8_t difficulty);

void game_ui_show_status(const char *line1,
                         const char *line2,
                         const char *line3,
                         const char *line4);

void game_ui_show_p1_message(const char *line1,
                             const char *line2,
                             const char *line3,
                             const char *line4);

void game_ui_show_p2_message(const char *line1,
                             const char *line2,
                             const char *line3,
                             const char *line4);

void game_ui_show_turn(uint8_t player);

void game_ui_show_player_card(uint8_t player, uint8_t card_index);
void game_ui_clear_player_card(uint8_t player);

void game_ui_show_player_stand(uint8_t player);

void game_ui_show_round_result(uint8_t winner);
void game_ui_show_final_winner(uint8_t winner);

void game_ui_show_error(const char *message);

void game_ui_show_hits_remaining(uint8_t player, uint8_t hits_left);
void game_ui_show_no_hits_left(uint8_t player);
void game_ui_show_score_status(uint8_t p1_score,
                               uint8_t p2_score,
                               const char *line2,
                               const char *line3,
                               const char *line4);

void game_ui_show_waiting_status(uint8_t p1_score, uint8_t p2_score);

#endif