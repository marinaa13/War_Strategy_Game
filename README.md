# Strategic War Card Game

Embedded card game built on an Arduino Mega 2560. The project uses multiple I2C OLED displays connected through a TCA9548A multiplexer, buttons for player actions, a potentiometer for AI difficulty selection, and WS2812B LED strips for visual feedback.

## Features

- Player vs Player and Player vs AI game modes
- AI difficulty selection using a potentiometer
- Multiple OLED displays for cards, status, and player messages
- TCA9548A I2C multiplexer for handling several OLED displays with the same address
- WS2812B LED animations for start, mode selection, round winner, and game over
- State-machine based game logic

## Hardware Used

- Arduino Mega 2560
- OLED displays
- TCA9548A I2C multiplexer
- WS2812B LED strips
- Push buttons
- Potentiometer
- Resistors and wiring components

## Project Structure

- `main.c` initializes the hardware modules and starts the game
- `game.c` contains the main game logic
- `game_ui.c` manages the OLED interface
- `display.c` controls a single OLED display
- `twi.c` implements I2C communication
- `input.c` handles buttons and user input
- `adc.c` reads the potentiometer
- `leds.c` controls the WS2812B LED animations
- `cards.c` stores the card bitmaps

## Description

The game is organized as a state machine, with separate states for start, mode selection, player turns, AI turns, round results, and game over. The OLED displays are selected through the I2C multiplexer, while the LED strips provide visual feedback during important game events.