#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#include "game.h"

#include <stdint.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/io.h>

#include "adc.h"
#include "input.h"
#include "game_ui.h"
#include "cards.h"
#include "leds.h"

#define HITS_PER_PLAYER_GAME 7
#define TOTAL_GAME_ROUNDS ((CARD_COUNT - (2 * HITS_PER_PLAYER_GAME)) / 2)
#define INVALID_CARD 255

#define ROUND_WINNER_TIE 0
#define ROUND_WINNER_P1  1
#define ROUND_WINNER_AI  2

// D45 = PL4 pe Arduino Mega 2560
#define MODE_SWITCH_PIN PL4
#define MODE_SWITCH_IS_LOW() (!(PINL & (1 << MODE_SWITCH_PIN)))

typedef enum {
    GAME_STATE_START,
    GAME_STATE_SELECT_MODE,
    GAME_STATE_SHOW_MODE,
    GAME_STATE_SELECT_DIFFICULTY,
    GAME_STATE_SHOW_SELECTED_DIFFICULTY,
    GAME_STATE_ROUND_START,
    GAME_STATE_P1_TURN,
    GAME_STATE_P2_TURN,
    GAME_STATE_AI_TURN,
    GAME_STATE_ROUND_RESULT,
    GAME_STATE_GAME_OVER,
    GAME_STATE_PLAY_AGAIN
} GameState;

typedef struct {
    GameState state;
    uint8_t mode;
    uint8_t last_mode;

    uint8_t difficulty;
    uint8_t last_difficulty;

    uint8_t round;

    uint8_t p1_score;
    uint8_t ai_score;

    uint8_t p1_hits_left;
    uint8_t ai_hits_left;

    uint8_t p1_card;
    uint8_t ai_card;

    uint8_t used_cards[CARD_COUNT];
    uint8_t used_count;

    uint16_t rng_seed_counter;
} Game;

static void game_set_state(Game *game, GameState new_state)
{
    game->state = new_state;
}

static void game_reset_used_cards(Game *game)
{
    for (uint8_t i = 0; i < CARD_COUNT; i++) {
        game->used_cards[i] = 0;
    }

    game->used_count = 0;
}

static void game_seed_random(Game *game)
{
    uint16_t adc_noise = myAnalogRead(0);

    uint16_t seed = adc_noise ^
                    (game->rng_seed_counter * 31u) ^
                    (game->rng_seed_counter >> 3);

    srand(seed);
}

static void game_mode_switch_init(void)
{
    // D45 input + pull-up intern
    DDRL &= ~(1 << MODE_SWITCH_PIN);
    PORTL |= (1 << MODE_SWITCH_PIN);
}

static uint8_t game_read_mode_from_switch(void)
{
    if (MODE_SWITCH_IS_LOW()) {
        return MODE_AI;
    }

    return MODE_PVP;
}

static void game_init_data(Game *game)
{
    game->state = GAME_STATE_START;

    game_mode_switch_init();

    game->mode = MODE_AI;
    game->last_mode = 255;

    game->difficulty = DIFF_EASY;
    game->last_difficulty = 255;

    game->round = 1;

    game->p1_score = 0;
    game->ai_score = 0;

    game->p1_hits_left = HITS_PER_PLAYER_GAME;
    game->ai_hits_left = HITS_PER_PLAYER_GAME;

    game->p1_card = INVALID_CARD;
    game->ai_card = INVALID_CARD;

    game->rng_seed_counter = 0;

    game_reset_used_cards(game);
}

static uint8_t game_take_random_unused_card(Game *game)
{
    if (game->used_count >= CARD_COUNT) {
        return INVALID_CARD;
    }

    uint8_t available_cards = CARD_COUNT - game->used_count;
    uint8_t chosen_free_index = rand() % available_cards;

    for (uint8_t i = 0; i < CARD_COUNT; i++) {
        if (game->used_cards[i] == 0) {
            if (chosen_free_index == 0) {
                game->used_cards[i] = 1;
                game->used_count++;
                return i;
            }

            chosen_free_index--;
        }
    }

    return INVALID_CARD;
}

static uint8_t game_deal_card_to_player(Game *game, uint8_t player)
{
    uint8_t card = game_take_random_unused_card(game);

    if (card == INVALID_CARD) {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  "NO CARDS",
                                  "LEFT.",
                                  0);
        return 0;
    }

    if (player == 1) {
        game->p1_card = card;
    } else {
        game->ai_card = card;
    }

    game_ui_show_player_card(player, card);

    return 1;
}

static void game_show_mode_selection_intro(void)
{
    game_ui_show_status("SELECT THE GAMEMODE - ",
                        0,
                        "USE THE SWITCH",
                        "ON THE LEFT.");
}

static void game_show_selected_mode_for_confirm(uint8_t mode)
{
    if (mode == MODE_AI) {
        game_ui_show_status("GAME MODE:",
                            "PLAYER VS AI",
                            0,
                            "PRESS CONFIRM.");

        game_ui_show_p1_message("PLAYER 1 -",
                                0,
                                "VS AI",
                                0);

        game_ui_show_p2_message("AI -",
                                0,
                                "SELECTED.",
                                0);
    } else {
        game_ui_show_status("GAME MODE:",
                            "PLAYER VS PLAYER",
                            0,
                            "PRESS CONFIRM.");

        game_ui_show_p1_message("PLAYER 1 -",
                                0,
                                "READY.",
                                0);

        game_ui_show_p2_message("PLAYER 2 -",
                                0,
                                "READY.",
                                0);
    }
}

static void game_show_p1_turn_prompt(Game *game)
{
    char line[22];

    sprintf(line, "ROUND %u/%u - P1 TURN", game->round, TOTAL_GAME_ROUNDS);

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              line,
                              "HIT/STAND?");
}

static void game_show_ai_turn_prompt(Game *game)
{
    char line[22];

    sprintf(line, "ROUND %u/%u - AI TURN", game->round, TOTAL_GAME_ROUNDS);

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              line,
                              "THINKING...");
}

static void game_show_p2_turn_prompt(Game *game)
{
    char line[22];

    sprintf(line, "ROUND %u/%u - P2 TURN", game->round, TOTAL_GAME_ROUNDS);

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              line,
                              "HIT/STAND?");
}

static void game_show_difficulty_details(uint8_t difficulty)
{
    if (difficulty == DIFF_EASY) {
        game_ui_show_p2_message("AI LEVEL:", 0, "EASY", "(LOW RISK)");
    } else if (difficulty == DIFF_MEDIUM) {
        game_ui_show_p2_message("AI LEVEL:", 0, "MEDIUM", "(BALANCED)");
    } else {
        game_ui_show_p2_message("AI LEVEL:", 0, "HARD", "(AGGRESSIVE)");
    }
}

static void game_show_selected_difficulty(uint8_t difficulty)
{
    if (difficulty == DIFF_EASY) {
        game_ui_show_status("SELECTED:", "EASY", 0, "STARTING GAME...");
        game_ui_show_p2_message("AI - EASY", 0, "READY!", 0);
    } else if (difficulty == DIFF_MEDIUM) {
        game_ui_show_status("SELECTED:", "MEDIUM", 0, "STARTING GAME.");
        game_ui_show_p2_message("AI - MEDIUM", 0, "READY!", 0);
    } else {
        game_ui_show_status("SELECTED:", "HARD", 0, "STARTING GAME.");
        game_ui_show_p2_message("AI - HARD", 0, "READY!", 0);
    }
}

static const char *game_opponent_name(Game *game)
{
    if (game->mode == MODE_AI) {
        return "AI";
    }

    return "PLAYER 2";
}

static void game_show_hits_remaining(Game *game, uint8_t player)
{
    char hits_str[20];

    if (player == 1) {
        snprintf(hits_str, sizeof(hits_str), "HITS LEFT: %u", game->p1_hits_left);

        game_ui_show_p1_message("PLAYER 1",
                                0,
                                hits_str,
                                0);
    } else {
        snprintf(hits_str, sizeof(hits_str), "HITS LEFT: %u", game->ai_hits_left);

        game_ui_show_p2_message(game_opponent_name(game),
                                0,
                                hits_str,
                                0);
    }
}

static void game_show_no_hits_left(Game *game, uint8_t player)
{
    if (player == 1) {
        game_ui_show_p1_message("PLAYER 1",
                                0,
                                "NO HITS LEFT...",
                                "AUTO STAND.");
    } else {
        game_ui_show_p2_message(game_opponent_name(game),
                                0,
                                "NO HITS LEFT...",
                                "AUTO STAND.");
    }
}

static void game_begin_p1_turn(Game *game)
{
    game_show_p1_turn_prompt(game);

    game_show_hits_remaining(game, 1);

    if (game->mode == MODE_PVP) {
        game_ui_show_p2_message("PLAYER 2 -",
                                0,
                                "WAITING FOR P1...",
                                0);
    } else {
        game_ui_show_p2_message("AI -",
                                0,
                                "WAITING.",
                                0);
    }
}

static uint8_t game_prepare_round(Game *game)
{
    game_begin_p1_turn(game);

    if (!game_deal_card_to_player(game, 1)) {
        return 0;
    }

    if (!game_deal_card_to_player(game, 2)) {
        return 0;
    }

    return 1;
}

static uint8_t game_is_last_round(Game *game)
{
    return game->round >= TOTAL_GAME_ROUNDS;
}

static void game_handle_start(Game *game)
{
    game->rng_seed_counter++;

    if (button_confirm_pressed()) {
        game_seed_random(game);

        leds_start_animation();

        game_show_mode_selection_intro();
        _delay_ms(3000);

        game->last_mode = 255;

        game_set_state(game, GAME_STATE_SELECT_MODE);
    }
}

static void game_handle_select_mode(Game *game)
{
    uint8_t selected_mode = game_read_mode_from_switch();

    if (selected_mode != game->last_mode) {
        game->mode = selected_mode;

        game_show_selected_mode_for_confirm(game->mode);

        game->last_mode = selected_mode;
    }

    if (button_confirm_pressed()) {
        leds_mode_selected_animation(game->mode);
        game_set_state(game, GAME_STATE_SHOW_MODE);
    }

    _delay_ms(50);
}

static void game_handle_show_mode(Game *game)
{
    game_ui_show_mode(game->mode);
    _delay_ms(1500);

    if (game->mode == MODE_AI) {
        game_ui_show_status("SELECT THE DIFFICULTY",
                            0,
                            "USE THE RED KNOB",
                            "ON THE RIGHT.");

        _delay_ms(3000);

        game->last_difficulty = 255;

        game_set_state(game, GAME_STATE_SELECT_DIFFICULTY);
    } else {
        game_ui_show_status("PLAYER VS",
                            "PLAYER",
                            0,
                            "STARTING GAME.");

        _delay_ms(1500);

        game_set_state(game, GAME_STATE_ROUND_START);
    }
}

static void game_handle_select_difficulty(Game *game)
{
    uint8_t difficulty = read_difficulty();

    if (difficulty != game->last_difficulty) {
        game_ui_show_difficulty(difficulty);
        game_show_difficulty_details(difficulty);

        game->last_difficulty = difficulty;
    }

    if (button_confirm_pressed()) {
        game->difficulty = difficulty;
        game_set_state(game, GAME_STATE_SHOW_SELECTED_DIFFICULTY);
    }

    _delay_ms(50);
}

static void game_handle_show_selected_difficulty(Game *game)
{
    game_show_selected_difficulty(game->difficulty);

    leds_mode_selected_animation(MODE_AI);

    _delay_ms(1500);

    game_set_state(game, GAME_STATE_ROUND_START);
}

static void game_handle_round_start(Game *game)
{
    if (game_prepare_round(game)) {
        game_set_state(game, GAME_STATE_P1_TURN);
    } else {
        game_set_state(game, GAME_STATE_GAME_OVER);
    }
}

static void game_begin_second_player_turn(Game *game)
{
    if (game->mode == MODE_AI) {
        game_set_state(game, GAME_STATE_AI_TURN);
        return;
    }

    game_show_p2_turn_prompt(game);

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "WAITING FOR P2...",
                            0);

    game_ui_show_p2_message("PLAYER 2 -",
                            0,
                            "YOUR TURN.",
                            0);

    game_show_hits_remaining(game, 2);

    game_set_state(game, GAME_STATE_P2_TURN);
}

static void game_auto_stand_p1(Game *game)
{
    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "NO HITS LEFT.",
                              "AUTO STAND.");

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "NO HITS LEFT,",
                            "AUTO STAND.");

    _delay_ms(1500);

    game_begin_second_player_turn(game);
}

static void game_handle_p1_hit(Game *game)
{
    if (game->p1_hits_left == 0) {
        game_auto_stand_p1(game);
        return;
    }

    game->p1_hits_left--;

    if (!game_deal_card_to_player(game, 1)) {
        game_set_state(game, GAME_STATE_GAME_OVER);
        return;
    }

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "PLAYER 1 CHOSE",
                              "HIT!");

    game_show_hits_remaining(game, 1);

    _delay_ms(1500);

    if (game->p1_hits_left > 0) {
        game_show_p1_turn_prompt(game);
    } else {
        game_auto_stand_p1(game);
    }
}

static void game_handle_p1_stand(Game *game)
{
    if (game_is_last_round(game) && game->p1_hits_left > 0) {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  0,
                                  "LAST ROUND - MUST USE",
                                  "ALL HITS!");

        _delay_ms(1500);

        game_show_p1_turn_prompt(game);
        return;
    }

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "PLAYER 1 CHOSE STAND.",
                              (game->mode == MODE_AI) ? "AI TURN." : "P2 TURN.");

    game_ui_show_p1_message("PLAYER 1 - ",
                            0,
                            "YOU CHOSE STAND.",
                            0);

    _delay_ms(1000);

    game_begin_second_player_turn(game);
}

static void game_handle_p1_turn(Game *game)
{
    if (game->p1_hits_left == 0) {
        _delay_ms(1000);
        game_auto_stand_p1(game);
        return;
    }

    if (button_p1_hit_pressed()) {
        game_handle_p1_hit(game);
        return;
    }

    if (button_p1_stand_pressed()) {
        game_handle_p1_stand(game);
        return;
    }
}

static void game_auto_stand_p2(Game *game)
{
    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "P2 NO HITS LEFT.",
                              "AUTO STAND.");

    game_ui_show_p2_message("PLAYER 2 -",
                            0,
                            "NO HITS LEFT,",
                            "AUTO STAND.");

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "P2 ENDED TURN.",
                            0);

    _delay_ms(1500);

    game_set_state(game, GAME_STATE_ROUND_RESULT);
}

static void game_handle_p2_hit(Game *game)
{
    if (game->ai_hits_left == 0) {
        game_auto_stand_p2(game);
        return;
    }

    game->ai_hits_left--;

    if (!game_deal_card_to_player(game, 2)) {
        game_set_state(game, GAME_STATE_GAME_OVER);
        return;
    }

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "PLAYER 2 CHOSE",
                              "HIT!");

    game_show_hits_remaining(game, 2);

    _delay_ms(1500);

    if (game->ai_hits_left > 0) {
        game_show_p2_turn_prompt(game);
    } else {
        game_auto_stand_p2(game);
    }
}

static void game_handle_p2_stand(Game *game)
{
    if (game_is_last_round(game) && game->ai_hits_left > 0) {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  0,
                                  "LAST ROUND - P2 MUST",
                                  "USE ALL HITS!");

        _delay_ms(1500);

        game_show_p2_turn_prompt(game);
        return;
    }

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "PLAYER 2 CHOSE STAND.",
                              "ROUND END.");

    game_ui_show_p2_message("PLAYER 2 -",
                            0,
                            "YOU CHOSE STAND.",
                            0);

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "P2 ENDED TURN.",
                            0);

    _delay_ms(1500);

    game_set_state(game, GAME_STATE_ROUND_RESULT);
}

static void game_handle_p2_turn(Game *game)
{
    if (game->ai_hits_left == 0) {
        _delay_ms(1000);
        game_auto_stand_p2(game);
        return;
    }

    if (button_p2_hit_pressed()) {
        game_handle_p2_hit(game);
        return;
    }

    if (button_p2_stand_pressed()) {
        game_handle_p2_stand(game);
        return;
    }
}

static void game_handle_ai_hit(Game *game)
{
    if (game->ai_hits_left == 0) {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  "AI",
                                  "NO HITS LEFT.",
                                  0);

        game_show_no_hits_left(game, 2);
        return;
    }

    game->ai_hits_left--;

    if (!game_deal_card_to_player(game, 2)) {
        game_set_state(game, GAME_STATE_GAME_OVER);
        return;
    }

    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "AI CHOSE",
                              "HIT!");

    game_show_hits_remaining(game, 2);

    _delay_ms(1500);
}

static uint8_t game_get_card_value(uint8_t card)
{
    return (card / 4) + 2;
}

static uint8_t game_card_is_around_middle(uint8_t card)
{
    uint8_t value = game_get_card_value(card);

    /*
     * Zona de mijloc aproximativa:
     * 7, 8, 9, 10
     */
    if (value >= 7 && value <= 10) {
        return 1;
    }

    return 0;
}

static uint8_t game_probability_to_beat_player(Game *game)
{
    uint8_t p1_value = game_get_card_value(game->p1_card);

    uint8_t remaining_cards = 0;
    uint8_t winning_cards = 0;

    for (uint8_t i = 0; i < CARD_COUNT; i++) {
        if (game->used_cards[i] == 0) {
            remaining_cards++;

            if (game_get_card_value(i) > p1_value) {
                winning_cards++;
            }
        }
    }

    if (remaining_cards == 0) {
        return 0;
    }

    // Returnam probabilitatea ca procent intreg: 0-100.
    return (winning_cards * 100) / remaining_cards;
}

static uint8_t game_ai_should_hit(Game *game)
{
    uint8_t p1_value = game_get_card_value(game->p1_card);
    uint8_t ai_value = game_get_card_value(game->ai_card);

    if (game->ai_hits_left == 0) {
        return 0;
    }

    // Daca e ultima runda, AI-ul trebuie sa foloseasca toate HIT-urile ramase
    if (game_is_last_round(game)) {
        return 1;
    }

    if (game->difficulty == DIFF_EASY) {
        /*
         * EASY:
         * AI-ul se uita doar la cartea lui.
         * Daca are carte mica, incearca HIT.
         * Daca are carte ok/mare, se opreste.
         */
        if (ai_value <= 6) {
            return 1;
        }

        return 0;
    }

    if (game->difficulty == DIFF_MEDIUM) {
        /*
         * MEDIUM:
         * AI-ul se uita la cartea adversarului.
         * Face HIT daca:
         * - pierde sau e egal;
         * - cartea adversarului este aproximativ la mijloc.
         */
        if (ai_value <= p1_value && game_card_is_around_middle(game->p1_card)) {
            return 1;
        }

        return 0;
    }

    /*
     * HARD:
     * AI-ul foloseste informatia despre cartile ramase.
     * Daca deja castiga runda, se opreste.
     * Daca pierde sau e egal, face HIT doar daca sansa de a trage
     * o carte care bate cartea lui P1 este suficient de buna.
     */
    if (ai_value > p1_value) {
        return 0;
    }

    uint8_t win_chance = game_probability_to_beat_player(game);

    if (win_chance >= 35) {
        return 1;
    }

    return 0;
}

static void game_handle_ai_stand(Game *game)
{
    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              0,
                              "AI CHOSE STAND.",
                              0);

    
    game_ui_show_p1_message("AI CHOSE STAND.",
                            0,
                            "ROUND END.",
                            0);

    game_ui_show_p2_message("AI CHOSE STAND.",
                            0,
                            "ROUND END.",
                            0);

    _delay_ms(1500);

    game_set_state(game, GAME_STATE_ROUND_RESULT);
}

static void game_handle_ai_turn(Game *game)
{
    game_show_ai_turn_prompt(game);

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "WAITING FOR AI...",
                            0);

    game_ui_show_p2_message(0,
                            "AI THINKING...",
                            0,
                            0);

    _delay_ms(2500);

    while (game->ai_hits_left > 0) {
        if (!game_ai_should_hit(game)) {
            break;
        }

        game_handle_ai_hit(game);

        if (game->state == GAME_STATE_GAME_OVER) {
            return;
        }

        if (game->ai_hits_left == 0) {
            break;
        }

        _delay_ms(800);

        game_show_ai_turn_prompt(game);

        game_ui_show_p1_message("PLAYER 1 -",
                                0,
                                "WAITING FOR AI...",
                                0);

        game_ui_show_p2_message(0,
                                "AI THINKING...",
                                0,
                                0);

        _delay_ms(2500);
    }

    if (game->ai_hits_left == 0) {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  0,
                                  "AI HAS NO HITS LEFT.",
                                  "ROUND END.");

        game_ui_show_p1_message("AI ENDED TURN.",
                                0,
                                "ROUND END.",
                                0);

        game_show_no_hits_left(game, 2);

        _delay_ms(2500);

        game_set_state(game, GAME_STATE_ROUND_RESULT);
    } else {
        game_handle_ai_stand(game);
    }
}

static uint8_t game_get_round_winner(Game *game)
{
    uint8_t p1_value = game_get_card_value(game->p1_card);
    uint8_t ai_value = game_get_card_value(game->ai_card);

    if (p1_value > ai_value) {
        return ROUND_WINNER_P1;
    }

    if (ai_value > p1_value) {
        return ROUND_WINNER_AI;
    }

    return ROUND_WINNER_TIE;
}

static void game_handle_round_result(Game *game)
{
    uint8_t winner = game_get_round_winner(game);

    if (winner == ROUND_WINNER_P1) {
        game->p1_score++;

        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  0,
                                  "ROUND WINNER:",
                                  "PLAYER 1!");

        game_ui_show_p1_message(0,
                                "YOU WIN",
                                "THIS ROUND!",
                                0);

        if (game->mode == MODE_AI) {
            game_ui_show_p2_message(0,
                                    "AI LOSES",
                                    "THIS ROUND.",
                                    0);
        } else {
            game_ui_show_p2_message(0,
                                    "YOU LOSE",
                                    "THIS ROUND.",
                                    0);
        }
    } else if (winner == ROUND_WINNER_AI) {
        game->ai_score++;

        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  0,
                                  "ROUND WINNER:",
                                  game->mode == MODE_AI ? "AI!" : "PLAYER 2!");

        game_ui_show_p1_message(0,
                                "YOU LOSE",
                                "THIS ROUND.",
                                0);

        if (game->mode == MODE_AI) {
            game_ui_show_p2_message(0,
                                    "AI WINS",
                                    "THIS ROUND!",
                                    0);
        } else {
            game_ui_show_p2_message(0,
                                    "YOU WIN",
                                    "THIS ROUND!",
                                    0);
        }
    } else {
        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  "TIE!",
                                  0,
                                  "NO POINTS AWARDED.");

        game_ui_show_p1_message("TIE!",
                                0,
                                "NO POINTS AWARDED.",
                                0);

        game_ui_show_p2_message("TIE!",
                                0,
                                "NO POINTS AWARDED.",
                                0);
    }

    leds_round_winner_animation(winner);

    _delay_ms(2500);

    if (game->round >= TOTAL_GAME_ROUNDS) {
        if (game->p1_hits_left == 0 && game->ai_hits_left == 0) {
            game_set_state(game, GAME_STATE_GAME_OVER);
        } else {
            game_ui_show_score_status(game->p1_score,
                                      game->ai_score,
                                      "ERROR:",
                                      "HITS LEFT",
                                      "NOT ZERO.");

            _delay_ms(2000);
            game_set_state(game, GAME_STATE_GAME_OVER);
        }

        return;
    }

    game->round++;

    game_set_state(game, GAME_STATE_ROUND_START);
}

static void game_handle_game_over(Game *game)
{
    uint8_t final_winner;

    if (game->p1_score > game->ai_score) {
        final_winner = LEDS_WINNER_P1;

        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  "GAME OVER.",
                                  0,
                                  "PLAYER 1 WINS!");

        game_ui_show_p1_message("PLAYER 1 -",
                                0,
                                "YOU WIN!",
                                0);

        if (game->mode == MODE_PVP) {
            game_ui_show_p2_message("PLAYER 2 -",
                                    0,
                                    "YOU LOSE.",
                                    0);
        } else {
            game_ui_show_p2_message("AI -",
                                    0,
                                    "YOU LOSE.",
                                    0);
        }
    } else if (game->ai_score > game->p1_score) {
        final_winner = LEDS_WINNER_P2;

        if (game->mode == MODE_PVP) {
            game_ui_show_score_status(game->p1_score,
                                      game->ai_score,
                                      "GAME OVER.",
                                      0,
                                      "PLAYER 2 WINS!");

            game_ui_show_p1_message("PLAYER 1 -",
                                    0,
                                    "YOU LOSE.",
                                    0);

            game_ui_show_p2_message("PLAYER 2 -",
                                    0,
                                    "YOU WIN!",
                                    0);
        } else {
            game_ui_show_score_status(game->p1_score,
                                      game->ai_score,
                                      "GAME OVER.",
                                      0,
                                      "AI WINS!");

            game_ui_show_p1_message("PLAYER 1 -",
                                    0,
                                    "YOU LOSE.",
                                    0);

            game_ui_show_p2_message("AI -",
                                    0,
                                    "YOU WIN!",
                                    0);
        }
    } else {
        final_winner = LEDS_WINNER_TIE;

        game_ui_show_score_status(game->p1_score,
                                  game->ai_score,
                                  "GAME OVER.",
                                  0,
                                  "TIE!");

        game_ui_show_p1_message("PLAYER 1 -",
                                0,
                                "TIE!",
                                0);

        if (game->mode == MODE_PVP) {
            game_ui_show_p2_message("PLAYER 2 -",
                                    0,
                                    "TIE!",
                                    0);
        } else {
            game_ui_show_p2_message("AI -",
                                    0,
                                    "TIE!",
                                    0);
        }
    }
    leds_game_over_animation(final_winner);

    _delay_ms(5000);

    game_set_state(game, GAME_STATE_PLAY_AGAIN);
}

static void game_handle_play_again(Game *game)
{
    game_ui_show_score_status(game->p1_score,
                              game->ai_score,
                              "PLAY AGAIN?",
                              0,
                              "PRESS CONFIRM.");

    game_ui_show_p1_message("PLAYER 1 -",
                            0,
                            "PRESS CONFIRM",
                            "TO RESTART.");

    game_ui_show_p2_message(game->mode == MODE_AI ? "AI -" : "PLAYER 2 -",
                            0,
                            "WAITING.",
                            0);

    while (!button_confirm_pressed()) {
    }

    game_init_data(game);

    game_ui_show_start_screen();

    game_set_state(game, GAME_STATE_START);
}

void game_run(void)
{
    Game game;

    game_init_data(&game);

    game_ui_show_start_screen();

    while (1) {
        switch (game.state) {
            case GAME_STATE_START:
                game_handle_start(&game);
                break;

            case GAME_STATE_SELECT_MODE:
                game_handle_select_mode(&game);
                break;

            case GAME_STATE_SHOW_MODE:
                game_handle_show_mode(&game);
                break;

            case GAME_STATE_SELECT_DIFFICULTY:
                game_handle_select_difficulty(&game);
                break;

            case GAME_STATE_SHOW_SELECTED_DIFFICULTY:
                game_handle_show_selected_difficulty(&game);
                break;

            case GAME_STATE_ROUND_START:
                game_handle_round_start(&game);
                break;

            case GAME_STATE_P1_TURN:
                game_handle_p1_turn(&game);
                break;

            case GAME_STATE_P2_TURN:
                game_handle_p2_turn(&game);
                break;

            case GAME_STATE_AI_TURN:
                game_handle_ai_turn(&game);
                break;

            case GAME_STATE_ROUND_RESULT:
                game_handle_round_result(&game);
                break;

            case GAME_STATE_GAME_OVER:
                game_handle_game_over(&game);
                break;

            case GAME_STATE_PLAY_AGAIN:
                game_handle_play_again(&game);
                break;

            default:
                game_set_state(&game, GAME_STATE_START);
                break;
        }
    }
}