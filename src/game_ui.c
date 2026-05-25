#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "game_ui.h"

#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <stdio.h>

#include "twi.h"
#include "display.h"
#include "input.h"
#include "cards.h"

static void game_ui_select(uint8_t channel)
{
    tca_select(channel);
    _delay_ms(10);
}

static void game_ui_init_display(uint8_t channel)
{
    game_ui_select(channel);
    _delay_ms(50);

    display_init();
    _delay_ms(50);

    display_clear();
}

static void game_ui_print_lines(uint8_t channel,
                                const char *line1,
                                const char *line2,
                                const char *line3,
                                const char *line4)
{
    game_ui_select(channel);
    display_clear();

    if (line1 != 0) {
        display_set_cursor(0, 0);
        display_print(line1);
    }

    if (line2 != 0) {
        display_set_cursor(0, 2);
        display_print(line2);
    }

    if (line3 != 0) {
        display_set_cursor(0, 4);
        display_print(line3);
    }

    if (line4 != 0) {
        display_set_cursor(0, 6);
        display_print(line4);
    }
}

void game_ui_init(void)
{
    game_ui_init_display(GAME_UI_STATUS_DISPLAY);
    game_ui_init_display(GAME_UI_P1_CARD_DISPLAY);
    game_ui_init_display(GAME_UI_P2_CARD_DISPLAY);
    game_ui_init_display(GAME_UI_P1_MSG_DISPLAY);
    game_ui_init_display(GAME_UI_P2_MSG_DISPLAY);

    game_ui_clear_all();
}

void game_ui_clear_all(void)
{
    game_ui_select(GAME_UI_STATUS_DISPLAY);
    display_clear();

    game_ui_select(GAME_UI_P1_CARD_DISPLAY);
    display_clear();

    game_ui_select(GAME_UI_P2_CARD_DISPLAY);
    display_clear();

    game_ui_select(GAME_UI_P1_MSG_DISPLAY);
    display_clear();

    game_ui_select(GAME_UI_P2_MSG_DISPLAY);
    display_clear();
}

void game_ui_show_status(const char *line1,
                         const char *line2,
                         const char *line3,
                         const char *line4)
{
    game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                        line1,
                        line2,
                        line3,
                        line4);
}

void game_ui_show_p1_message(const char *line1,
                             const char *line2,
                             const char *line3,
                             const char *line4)
{
    game_ui_print_lines(GAME_UI_P1_MSG_DISPLAY,
                        line1,
                        line2,
                        line3,
                        line4);
}

void game_ui_show_p2_message(const char *line1,
                             const char *line2,
                             const char *line3,
                             const char *line4)
{
    game_ui_print_lines(GAME_UI_P2_MSG_DISPLAY,
                        line1,
                        line2,
                        line3,
                        line4);
}

void game_ui_show_start_screen(void)
{
    game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                        "STRATEGIC WAR GAME!",
                        0,
                        "PRESS CONFIRM",
                        "TO START.");

    game_ui_print_lines(GAME_UI_P1_MSG_DISPLAY,
                        "PLAYER 1,",
                        0,
                        "ARE YOU READY?",
                        0);

    game_ui_print_lines(GAME_UI_P2_MSG_DISPLAY,
                        "PLAYER 2,",
                        0,
                        "ARE YOU READY?",
                        0);
}

void game_ui_show_mode(uint8_t mode)
{
    if (mode == MODE_AI) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "MODE:",
                            0,
                            "PLAYER VS AI",
                            0);
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "MODE:",
                            0,
                            "PLAYER VS PLAYER",
                            0);
    }
}

void game_ui_show_difficulty(uint8_t difficulty)
{
    if (difficulty == DIFF_EASY) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "AI LEVEL:",
                            "EASY (LOW RISK)",
                            0, 
                            "PRESS CONFIRM.");
    } else if (difficulty == DIFF_MEDIUM) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "AI LEVEL:",
                            "MEDIUM (BALANCED)",
                            0,
                            "PRESS CONFIRM.");
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "AI LEVEL:",
                            "HARD (AGGRESSIVE)",
                            0,
                            "PRESS CONFIRM.");
    }
}

void game_ui_show_turn(uint8_t player)
{
    if (player == 1) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "PLAYER 1",
                            "TURN!",
                            "HIT OR",
                            "STAND?");
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "PLAYER 2",
                            "TURN!",
                            "HIT OR",
                            "STAND?");
    }
}

void game_ui_show_waiting(uint8_t player)
{
    if (player == 1) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "PLAYER 1",
                            "WAITING.",
                            0,
                            0);
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "PLAYER 2",
                            "WAITING.",
                            0,
                            0);
    }
}

void game_ui_show_player_card(uint8_t player, uint8_t card_index)
{
    if (card_index >= CARD_COUNT) {
        return;
    }

    uint8_t channel;

    if (player == 1) {
        channel = GAME_UI_P1_CARD_DISPLAY;
    } else {
        channel = GAME_UI_P2_CARD_DISPLAY;
    }

    game_ui_select(channel);
    display_clear();

    const uint8_t *bitmap =
        (const uint8_t *)pgm_read_ptr(&cards_bitmaps[card_index]);

    display_draw_bitmap_128x64(bitmap);
}

void game_ui_show_player_stand(uint8_t player)
{
    if (player == 1) {
        game_ui_print_lines(GAME_UI_P1_MSG_DISPLAY,
                            "PLAYER 1",
                            "STAND.",
                            0,
                            0);
    } else {
        game_ui_print_lines(GAME_UI_P2_MSG_DISPLAY,
                            "PLAYER 2",
                            "STAND.",
                            0,
                            0);
    }
}

void game_ui_show_round_result(uint8_t winner)
{
    if (winner == 1) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "ROUND",
                            "WINNER:",
                            "PLAYER 1!",
                            0);
    } else if (winner == 2) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "ROUND",
                            "WINNER:",
                            "PLAYER 2!",
                            0);
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "ROUND",
                            "RESULT:",
                            "DRAW.",
                            0);
    }
}

void game_ui_show_final_winner(uint8_t winner)
{
    if (winner == 1) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "GAME OVER.",
                            "WINNER:",
                            "PLAYER 1!",
                            0);
    } else if (winner == 2) {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "GAME OVER.",
                            "WINNER:",
                            "PLAYER 2!",
                            0);
    } else {
        game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                            "GAME OVER.",
                            "RESULT:",
                            "DRAW.",
                            0);
    }
}

void game_ui_show_error(const char *message)
{
    game_ui_print_lines(GAME_UI_STATUS_DISPLAY,
                        "ERROR!",
                        message,
                        0,
                        0);
}

void game_ui_show_hits_remaining(uint8_t player, uint8_t hits_left)
{
    if (hits_left > 9) {
        hits_left = 9;
    }

    char hits_str[20];
    snprintf(hits_str, sizeof(hits_str), "HITS LEFT: %d", hits_left);

    if (player == 1) {
        game_ui_show_p1_message("PLAYER 1",
                                0,          
                                hits_str,
                                0);
    } else {
        game_ui_show_p2_message("AI",
                                0,
                                hits_str,
                                0);
    }
}

void game_ui_show_no_hits_left(uint8_t player)
{
    if (player == 1) {
        game_ui_show_p1_message("PLAYER 1",
                                0,          
                                "NO HITS LEFT...",
                                "STAND?");
    } else {
        game_ui_show_p2_message("AI",
                                0,
                                "NO HITS LEFT...",
                                "STAND?");
    }
}

void game_ui_show_score_status(uint8_t p1_score,
                               uint8_t p2_score,
                               const char *line2,
                               const char *line3,
                               const char *line4)
{
    char score_line[16];

    sprintf(score_line, "SCORE: %u-%u", p1_score, p2_score);

    game_ui_show_status(score_line,
                        line2,
                        line3,
                        line4);
}

void game_ui_show_waiting_status(uint8_t p1_score, uint8_t p2_score)
{
    game_ui_show_score_status(p1_score,
                              p2_score,
                              "WAITING.",
                              0,
                              0);
}