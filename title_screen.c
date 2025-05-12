#include <stdio.h>
#include <stdlib.h>
#include "game.h"

// Global state variable definition
int currentState = STATE_TITLE;

void bg_music() {
    system("nohup mpg123 -q bg-music.mp3 > /dev/null 2>&1 &"); // Fully detach music
}

// Function to display the title screen
void showTitleScreen() {
    const char *RED = "\033[31m";
    const char *RESET = "\033[0m";
    printf("\n\n");
    printf("%s▓█████▄ ▓█████ ▄▄▄██▀▀▀▄▄▄      %s\n", RED, RESET);
    printf("%s▒██▀ ██▌▓█   ▀   ▒██  ▒████▄    %s\n", RED, RESET);
    printf("%s░██   █▌▒███     ░██  ▒██  ▀█▄  %s\n", RED, RESET);
    printf("%s░▓█▄   ▌▒▓█  ▄▓██▄██▓ ░██▄▄▄▄██ %s\n", RED, RESET);
    printf("%s░▒████▓ ░▒████▒▓███▒   ▓█   ▓██▒%s\n", RED, RESET);
    printf("%s ▒▒▓  ▒ ░░ ▒░ ░▒▓▒▒░   ▒▒   ▓▒█░%s\n", RED, RESET);
    printf("%s ░ ▒  ▒  ░ ░  ░▒ ░▒░    ▒   ▒▒ ░%s\n", RED, RESET);
    printf("%s ░ ░  ░    ░   ░ ░ ░    ░   ▒   %s\n", RED, RESET);
    printf("%s   ░       ░  ░░   ░        ░  ░%s\n", RED, RESET);
    printf("%s ░                              %s\n", RED, RESET);
    printf("\n\n");
    printf("1. Start Game        \n");
    printf("2. Instructions      \n");
    printf("3. Exit              \n");
    printf("=================================\n");
    printf("Choose an option (1-3): ");
}

void startGame() {
    // Set the game state to playing
    currentState = STATE_PLAYING;
    printf("\nStarting the game...\n");
    // This will call the game logic from main.c
}

void showInstructions() {
    currentState = STATE_INSTRUCTIONS;
    printf("\n===== GAME INSTRUCTIONS =====\n");
    printf("SURVIVOR: Use arrow keys to move\n");
    printf("KILLER: Use WASD keys to move\n");
    printf("End your turn with spacebar\n");
    printf("Survivor's goal: Reach the exit (E)\n");
    printf("Killer's goal: Catch the survivor\n");
    printf("Each player rolls dice to determine movement points\n");
    printf("The exit relocates every 10 turns\n");
    printf("Press Q to quit the game\n");
    printf("=============================\n");
    
    printf("\nPress Enter to return to the title screen...");
    getchar(); // Wait for Enter key
    currentState = STATE_TITLE;
}

int main() {
    int choice;
    
    bg_music(); // Start music once when program runs
    
    // Main menu loop
    while (1) {
        // If the state is STATE_PLAYING, run the game
        if (currentState == STATE_PLAYING) {
            runGame(); // Call the game logic from main.c
            currentState = STATE_TITLE; // Return to title screen after game ends
            system("clear || cls"); // Clear screen
            continue;
        }
        
        // Show title screen and get user input
        showTitleScreen();
        if (scanf("%d", &choice) != 1) {
            // If input is invalid (not a number)
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear invalid input
            continue;
        }
        
        while (getchar() != '\n'); // Clear input buffer after valid scanf
        
        switch (choice) {
            case 1:
                startGame();
                break;
            case 2:
                showInstructions();
                break;
            case 3:
                printf("Exiting the game. Goodbye!\n");
                system("killall mpg123"); // Stop the background music
                exit(0);
            default:
                printf("Invalid choice. Try again.\n");
                printf("\nPress Enter to continue...");
                getchar();
        }
        
        if (currentState != STATE_INSTRUCTIONS) {
            system("clear || cls"); // Clear screen (Linux/Windows)
        }
    }
    
    return 0;
}
