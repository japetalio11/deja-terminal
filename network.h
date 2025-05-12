#ifndef NETWORK_H
#define NETWORK_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Game state structure for network transmission
typedef struct {
    int survivorY;
    int survivorX;
    int killerY;
    int killerX;
    int currentTurn;
    int survivorMovesLeft;
    int killerMovesLeft;
    char maze[10][25];  // Using the same dimensions as the game
} GameState;

// Function declarations
int createServer();
int connectToServer(const char* serverIP);
int sendGameState(int socket, GameState* state);
int receiveGameState(int socket, GameState* state);
void closeConnection(int socket);

#endif // NETWORK_H 