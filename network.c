#include "network.h"

int createServer() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Setsockopt failed");
        return -1;
    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);  // Bind to all interfaces
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        return -1;
    }

    printf("Server waiting for connection on port %d...\n", PORT);
    printf("Your IP address is: %s\n", inet_ntoa(address.sin_addr));
    
    int client_socket = accept(server_fd, NULL, NULL);
    if (client_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        return -1;
    }

    printf("Client connected!\n");
    close(server_fd); // Close the listening socket
    return client_socket;
}

int connectToServer(const char* serverIP) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, serverIP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(sock);
        return -1;
    }

    printf("Connecting to server at %s:%d...\n", serverIP, PORT);
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to server!\n");
    return sock;
}

int sendGameState(int socket, GameState* state) {
    ssize_t sent = send(socket, state, sizeof(GameState), 0);
    if (sent < 0) {
        perror("Send failed");
    }
    return sent;
}

int receiveGameState(int socket, GameState* state) {
    ssize_t received = recv(socket, state, sizeof(GameState), 0);
    if (received < 0) {
        perror("Receive failed");
    }
    return received;
}

void closeConnection(int socket) {
    if (socket >= 0) {
        close(socket);
    }
} 