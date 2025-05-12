
#ifndef GAME_H
#define GAME_H

#include <stdbool.h>

// Game state constants
#define STATE_TITLE 0
#define STATE_PLAYING 1
#define STATE_INSTRUCTIONS 2
#define STATE_EXIT 3

// Function declarations for game logic
void runGame();

// Function declarations for title screen
void bg_music();
void showTitleScreen();
void startGame();
void showInstructions();

// Global state variable
extern int currentState;

#endif /* GAME_H */
