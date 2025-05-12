#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "game.h"
#include "network.h"

// Game constants
#define WALL '#'
#define SURVIVOR 'S'
#define KILLER 'K'
#define EXIT 'E'
#define EMPTY ' '

// Maze dimensions
#define HEIGHT 10
#define WIDTH 25

// Direction constants for player movement
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

// Player turn constants
#define SURVIVOR_TURN 0
#define KILLER_TURN 1

//Turn Counter
int turnCounter = 0; 
int lastRelocatedTurn = -10;

// Maze structure
typedef struct {
    char grid[HEIGHT][WIDTH];
    int startX, startY;
    int exitX, exitY;
} Maze;

// Check if a coordinate is valid
bool isValid(int y, int x) {
    return y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH;
}

// Swap two integers
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Generate maze using randomized DFS
void generateMazeRecursive(Maze* maze, int y, int x) {
    // Mark current cell as visited
    maze->grid[y][x] = EMPTY;
    
    // Directions: up, right, down, left
    int dy[4] = {-2, 0, 2, 0};
    int dx[4] = {0, 2, 0, -2};
    
    // Shuffle directions
    for (int i = 0; i < 4; i++) {
        int r = rand() % 4;
        swap(&dy[i], &dy[r]);
        swap(&dx[i], &dx[r]);
    }
    
    // Explore in each direction
    for (int i = 0; i < 4; i++) {
        int ny = y + dy[i];
        int nx = x + dx[i];
        
        if (isValid(ny, nx) && maze->grid[ny][nx] == WALL) {
            // Create passage
            maze->grid[y + dy[i]/2][x + dx[i]/2] = EMPTY;
            generateMazeRecursive(maze, ny, nx);
        }
    }
}

// Initialize and generate a new maze
void initializeMaze(Maze* maze) {
    // Fill maze with walls
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            maze->grid[y][x] = WALL;
        }
    }
    
    // Set start position
    maze->startY = 1;
    maze->startX = 1;
    
    // Generate maze
    generateMazeRecursive(maze, maze->startY, maze->startX);
    
    // Place exit at a random position on the edge
    do {
        int side = rand() % 4;
        switch (side) {
            case 0: // Top
                maze->exitY = 0;
                maze->exitX = 1 + 2 * (rand() % ((WIDTH-1)/2));
                break;
            case 1: // Right
                maze->exitY = 1 + 2 * (rand() % ((HEIGHT-1)/2));
                maze->exitX = WIDTH - 1;
                break;
            case 2: // Bottom
                maze->exitY = HEIGHT - 1;
                maze->exitX = 1 + 2 * (rand() % ((WIDTH-1)/2));
                break;
            case 3: // Left
                maze->exitY = 1 + 2 * (rand() % ((HEIGHT-1)/2));
                maze->exitX = 0;
                break;
        }
    } while (maze->grid[maze->exitY][maze->exitX] == WALL);
    
    // Mark the exit
    maze->grid[maze->exitY][maze->exitX] = EXIT;
}

// Try to move player in a direction
bool movePlayer(Maze* maze, int* playerY, int* playerX, int direction, int* movesLeft) {
    int newY = *playerY;
    int newX = *playerX;
    
    switch (direction) {
        case UP:    newY--; break;
        case DOWN:  newY++; break;
        case LEFT:  newX--; break;
        case RIGHT: newX++; break;
    }
    
    if (!isValid(newY, newX) || maze->grid[newY][newX] == WALL) {
        return false;
    }
    
    // Update player position
    *playerY = newY;
    *playerX = newX;
    
    (*movesLeft)--; // Decrement remaining moves
    return true;
}

// Roll dice to determine moves
int rollDice() {
    int die1 = rand() % 6 + 1;
    int die2 = rand() % 6 + 1;
    return die1 + die2;
}

// Draw the entire maze
void drawMaze(Maze* maze, int survivorY, int survivorX, int killerY, int killerX,
              int survivorMovesLeft, int killerMovesLeft, int currentTurn) {
    clear();
    
    // Display maze
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            char cell = maze->grid[y][x];
            
            if (y == survivorY && x == survivorX) {
                attron(A_BOLD | COLOR_PAIR(1));
                mvaddch(y, x, SURVIVOR);
                attroff(A_BOLD | COLOR_PAIR(1));
            } else if (y == killerY && x == killerX) {
                attron(A_BOLD | COLOR_PAIR(3));
                mvaddch(y, x, KILLER);
                attroff(A_BOLD | COLOR_PAIR(3));
            } else if (cell == EXIT) {
                attron(COLOR_PAIR(2));
                mvaddch(y, x, EXIT);
                attroff(COLOR_PAIR(2));
            } else if (cell == WALL) {
                attron(COLOR_PAIR(4));
                mvaddch(y, x, cell);
                attroff(COLOR_PAIR(4));
            } else {
                mvaddch(y, x, cell);
            }
        }
    }
    
    // Display status info
    attron(COLOR_PAIR(5));
    if (currentTurn == SURVIVOR_TURN) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(HEIGHT + 1, 0, "SURVIVOR'S TURN");
        attroff(COLOR_PAIR(1) | A_BOLD);
        attron(COLOR_PAIR(5));
        mvprintw(HEIGHT + 1, 17, " (Use arrow keys) - Moves left: %d", survivorMovesLeft);
    } else {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(HEIGHT + 1, 0, "KILLER'S TURN");
        attroff(COLOR_PAIR(3) | A_BOLD);
        attron(COLOR_PAIR(5));
        mvprintw(HEIGHT + 1, 14, " (Use WASD keys) - Moves left: %d", killerMovesLeft);
    }
    
    mvprintw(HEIGHT + 2, 0, "Survivor: Arrow keys | Killer: WASD | End Turn: Space | Quit: q");
    attroff(COLOR_PAIR(5));
    
    refresh();
}

// Display game over message and wait for input
bool gameOverScreen(bool survivorWon) {
    clear();
    
    if (survivorWon) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(HEIGHT/2 - 1, WIDTH/2 - 10, "SURVIVOR ESCAPED!");
        attroff(COLOR_PAIR(1) | A_BOLD);
    } else {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(HEIGHT/2 - 1, WIDTH/2 - 8, "KILLER WINS!");
        attroff(COLOR_PAIR(3) | A_BOLD);
    }
    
    attron(COLOR_PAIR(5));
    mvprintw(HEIGHT/2 + 1, WIDTH/2 - 13, "Press 'P' to play again");
    mvprintw(HEIGHT/2 + 2, WIDTH/2 - 13, "Press any other key to exit");
    attroff(COLOR_PAIR(5));
    
    refresh();
    
    int ch = getch();
    return (ch == 'p' || ch == 'P');
}

// Display waiting message during opponent's turn
void displayTurnChange(int newTurn) {
    clear();
    if (newTurn == SURVIVOR_TURN) {
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(HEIGHT/2, WIDTH/2 - 18, "SURVIVOR'S TURN - PRESS ANY KEY TO START");
        attroff(COLOR_PAIR(1) | A_BOLD);
    } else {
        attron(COLOR_PAIR(3) | A_BOLD);
        mvprintw(HEIGHT/2, WIDTH/2 - 17, "KILLER'S TURN - PRESS ANY KEY TO START");
        attroff(COLOR_PAIR(3) | A_BOLD);
    }
    refresh();
    getch(); // Wait for key press
}

// Relocates the Exit every 5 rounds
void relocateExit(Maze* maze) {
    // Remove the old exit if it's still marked
    if (maze->grid[maze->exitY][maze->exitX] == EXIT) {
        maze->grid[maze->exitY][maze->exitX] = EMPTY;
    }

    // Pick a new random empty location anywhere in the maze 
    do {
        maze->exitY = rand() % HEIGHT;
        maze->exitX = rand() % WIDTH;
    } while (maze->grid[maze->exitY][maze->exitX] != EMPTY ||
             (maze->exitY == maze->startY && maze->exitX == maze->startX));

    maze->grid[maze->exitY][maze->exitX] = EXIT;
}

// Add these new functions for network play
bool isNetworkMode = false;
int networkSocket = -1;
bool isServer = false;

void initializeNetworkMode() {
    char choice;
    clear();
    mvprintw(0, 0, "Select network mode:");
    mvprintw(1, 0, "1. Host game (Server)");
    mvprintw(2, 0, "2. Join game (Client)");
    mvprintw(3, 0, "3. Local play");
    mvprintw(4, 0, "Enter choice (1-3): ");
    refresh();
    
    choice = getch();
    switch(choice) {
        case '1':
            isServer = true;
            isNetworkMode = true;
            networkSocket = createServer();
            if (networkSocket < 0) {
                mvprintw(5, 0, "Failed to create server. Press any key to exit.");
                getch();
                endwin();
                exit(1);
            }
            break;
        case '2':
            isServer = false;
            isNetworkMode = true;
            char ip[16];
            mvprintw(5, 0, "Enter server IP: ");
            echo();
            getstr(ip);
            noecho();
            networkSocket = connectToServer(ip);
            if (networkSocket < 0) {
                mvprintw(6, 0, "Failed to connect to server. Press any key to exit.");
                getch();
                endwin();
                exit(1);
            }
            break;
        default:
            isNetworkMode = false;
            break;
    }
}

// Modify the runGame function to handle network play
void runGame() {
    bool playAgain = true;
    
    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Hide cursor
    
    // Initialize colors if terminal supports them
    if (has_colors()) {
        start_color();
        use_default_colors(); // Use terminal default colors
        
        init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Survivor
        init_pair(2, COLOR_CYAN, COLOR_BLACK);   // Exit
        init_pair(3, COLOR_RED, COLOR_BLACK);    // Killer
        init_pair(4, COLOR_WHITE, COLOR_BLACK);  // Wall
        init_pair(5, COLOR_YELLOW, COLOR_BLACK); // Status text
    }
    
    initializeNetworkMode();

    while (playAgain) {
        Maze maze;
        int survivorY, survivorX;
        int killerY, killerX;
        bool gameOver = false;
        int survivorMovesLeft = 0;
        int killerMovesLeft = 0;
        bool survivorWon = false;
        int currentTurn = SURVIVOR_TURN; // Survivor goes first
        
        // Initialize maze
        initializeMaze(&maze);
        
        // Initialize player positions
        survivorY = maze.startY;
        survivorX = maze.startX;
        
        // Place killer at a random valid position far from survivor
        do {
            killerY = 1 + 2 * (rand() % ((HEIGHT-1)/2));
            killerX = 1 + 2 * (rand() % ((WIDTH-1)/2));
        } while (maze.grid[killerY][killerX] == WALL || 
                (abs(killerY - survivorY) + abs(killerX - survivorX) < 10));
        
        // Initial dice rolls
        survivorMovesLeft = rollDice();
        killerMovesLeft = rollDice();
        
        // Reset turn counter
        turnCounter = 0;
        lastRelocatedTurn = -10;
                
        // Announce first turn
        displayTurnChange(currentTurn);
        
        // Game loop
        while (!gameOver) {
            drawMaze(&maze, survivorY, survivorX, killerY, killerX, 
                    survivorMovesLeft, killerMovesLeft, currentTurn);
                    
            // Checks if turn counter is multiple by 10 or greater than 10 and if turn counter is not equal to lastRelocatedTurn        
            if (turnCounter > 0 && turnCounter % 10 == 0 && turnCounter != lastRelocatedTurn) {
                relocateExit(&maze);
                lastRelocatedTurn = turnCounter;
            }
                        
            int ch = getch();
            
            // Handle input based on whose turn it is
            if (currentTurn == SURVIVOR_TURN) {
                // Survivor's turn - process survivor controls
                switch (ch) {
                    case KEY_UP:
                        if (survivorMovesLeft > 0) {
                            movePlayer(&maze, &survivorY, &survivorX, UP, &survivorMovesLeft);
                        }
                        break;
                    case KEY_DOWN:
                        if (survivorMovesLeft > 0) {
                            movePlayer(&maze, &survivorY, &survivorX, DOWN, &survivorMovesLeft);
                        }
                        break;
                    case KEY_LEFT:
                        if (survivorMovesLeft > 0) {
                            movePlayer(&maze, &survivorY, &survivorX, LEFT, &survivorMovesLeft);
                        }
                        break;
                    case KEY_RIGHT:
                        if (survivorMovesLeft > 0) {
                            movePlayer(&maze, &survivorY, &survivorX, RIGHT, &survivorMovesLeft);
                        }
                        break;
                    case ' ': // End turn manually with spacebar
                        if (survivorMovesLeft > 0) {
                            // End turn, forfeiting remaining moves
                            survivorMovesLeft = 0;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        gameOver = true;
                        playAgain = false;
                        break;
                }
                
                // Check if survivor reached the exit
                if (survivorY == maze.exitY && survivorX == maze.exitX) {
                    gameOver = true;
                    survivorWon = true;
                    continue;
                }
                
                // Check if survivor's turn is over
                if (survivorMovesLeft <= 0 || (ch == ' ')) {
                    // Switch to killer's turn
                    currentTurn = KILLER_TURN;
                    // Roll dice for killer's new turn
                    killerMovesLeft = rollDice();
                    // Announce killer's turn
                    displayTurnChange(currentTurn);
                    turnCounter++; //Increment turn counter
                }
            } else {
                // Killer's turn - process killer controls
                switch (ch) {
                    case 'w':
                    case 'W':
                        if (killerMovesLeft > 0) {
                            movePlayer(&maze, &killerY, &killerX, UP, &killerMovesLeft);
                        }
                        break;
                    case 's':
                    case 'S':
                        if (killerMovesLeft > 0) {
                            movePlayer(&maze, &killerY, &killerX, DOWN, &killerMovesLeft);
                        }
                        break;
                    case 'a':
                    case 'A':
                        if (killerMovesLeft > 0) {
                            movePlayer(&maze, &killerY, &killerX, LEFT, &killerMovesLeft);
                        }
                        break;
                    case 'd':
                    case 'D':
                        if (killerMovesLeft > 0) {
                            movePlayer(&maze, &killerY, &killerX, RIGHT, &killerMovesLeft);
                        }
                        break;
                    case ' ': // End turn manually with spacebar
                        if (killerMovesLeft > 0) {
                            // End turn, forfeiting remaining moves
                            killerMovesLeft = 0;
                        }
                        break;
                    case 'q':
                    case 'Q':
                        gameOver = true;
                        playAgain = false;
                        break;
                }
                
                // Check if killer caught survivor
                if (killerY == survivorY && killerX == survivorX) {
                    gameOver = true;
                    survivorWon = false;
                    continue;
                }
                
                // Check if killer's turn is over
                if (killerMovesLeft <= 0 || (ch == ' ')) {
                    // Switch to survivor's turn
                    currentTurn = SURVIVOR_TURN;
                    // Roll dice for survivor's new turn
                    survivorMovesLeft = rollDice();
                    // Announce survivor's turn
                    displayTurnChange(currentTurn);
                    turnCounter++; //Increment Turn
                }
            }

            // Network synchronization after each move or turn change
            if (isNetworkMode) {
                GameState state;
                if (isServer) {
                    // Server sends state first
                    state.survivorY = survivorY;
                    state.survivorX = survivorX;
                    state.killerY = killerY;
                    state.killerX = killerX;
                    state.currentTurn = currentTurn;
                    state.survivorMovesLeft = survivorMovesLeft;
                    state.killerMovesLeft = killerMovesLeft;
                    memcpy(state.maze, maze.grid, sizeof(maze.grid));
                    
                    if (sendGameState(networkSocket, &state) < 0) {
                        mvprintw(HEIGHT + 3, 0, "Network error. Game will exit.");
                        getch();
                        gameOver = true;
                        playAgain = false;
                        break;
                    }
                    
                    // Receive client's state
                    if (receiveGameState(networkSocket, &state) < 0) {
                        mvprintw(HEIGHT + 3, 0, "Network error. Game will exit.");
                        getch();
                        gameOver = true;
                        playAgain = false;
                        break;
                    }
                } else {
                    // Client receives state first
                    if (receiveGameState(networkSocket, &state) < 0) {
                        mvprintw(HEIGHT + 3, 0, "Network error. Game will exit.");
                        getch();
                        gameOver = true;
                        playAgain = false;
                        break;
                    }
                    
                    // Update local state
                    survivorY = state.survivorY;
                    survivorX = state.survivorX;
                    killerY = state.killerY;
                    killerX = state.killerX;
                    currentTurn = state.currentTurn;
                    survivorMovesLeft = state.survivorMovesLeft;
                    killerMovesLeft = state.killerMovesLeft;
                    memcpy(maze.grid, state.maze, sizeof(maze.grid));
                    
                    // Send updated state back
                    if (sendGameState(networkSocket, &state) < 0) {
                        mvprintw(HEIGHT + 3, 0, "Network error. Game will exit.");
                        getch();
                        gameOver = true;
                        playAgain = false;
                        break;
                    }
                }
            }
        }

        if (gameOver) {
            // Show game over screen and check if player wants to play again
            playAgain = gameOverScreen(survivorWon);
            
            // If player doesn't want to play again, exit game mode
            if (!playAgain) {
                break;
            }
        }
    }
    
    // Clean up ncurses before returning to title screen
    endwin();
    
    // Set state back to title screen
    currentState = STATE_TITLE;

    if (isNetworkMode) {
        closeConnection(networkSocket);
    }
}
