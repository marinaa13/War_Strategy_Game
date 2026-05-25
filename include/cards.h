#ifndef CARDS_H
#define CARDS_H

#include <stdint.h>
#include <avr/pgmspace.h>

#define CARD_COUNT 52

extern const unsigned char epd_bitmap_2_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_2_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_2_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_2_spades[] PROGMEM;
extern const unsigned char epd_bitmap_3_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_3_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_3_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_3_spades[] PROGMEM;
extern const unsigned char epd_bitmap_4_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_4_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_4_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_4_spades[] PROGMEM;
extern const unsigned char epd_bitmap_5_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_5_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_5_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_5_spades[] PROGMEM;
extern const unsigned char epd_bitmap_6_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_6_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_6_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_6_spades[] PROGMEM;
extern const unsigned char epd_bitmap_7_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_7_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_7_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_7_spades[] PROGMEM;
extern const unsigned char epd_bitmap_8_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_8_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_8_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_8_spades[] PROGMEM;
extern const unsigned char epd_bitmap_9_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_9_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_9_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_9_spades[] PROGMEM;
extern const unsigned char epd_bitmap_10_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_10_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_10_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_10_spades[] PROGMEM;
extern const unsigned char epd_bitmap_jack_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_jack_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_jack_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_jack_spades[] PROGMEM;
extern const unsigned char epd_bitmap_queen_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_queen_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_queen_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_queen_spades[] PROGMEM;
extern const unsigned char epd_bitmap_king_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_king_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_king_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_king_spades[] PROGMEM;
extern const unsigned char epd_bitmap_ace_clubs[] PROGMEM;
extern const unsigned char epd_bitmap_ace_diamonds[] PROGMEM;
extern const unsigned char epd_bitmap_ace_hearts[] PROGMEM;
extern const unsigned char epd_bitmap_ace_spades[] PROGMEM;

extern const unsigned char* const cards_bitmaps[CARD_COUNT] PROGMEM;

#endif