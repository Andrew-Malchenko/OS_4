#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define BUFFER_SIZE 1024

void error(const char *message) {
    perror(message);
    printf("%s", message);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    int client_socket;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE];

    // Create client socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("Failed to create socket");
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Failed to connect to the server");
    }
    if (send(client_socket, "bee", strlen("bee"), 0) < 0) {
        error("Failed to send message to the server");
    }

    while (1) {
        printf("Press ENTER to collect honey or type 'exit' to quit: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove newline character
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Send "collect_honey" message to the server
        if (send(client_socket, "collect_honey", strlen("collect_honey"), 0) < 0) {
            error("Failed to send message to the server");
        }
    }

    close(client_socket);

    return 0;
}
