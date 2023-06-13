#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define MAX_CLIENTS 2
#define MAX_THREADS 1
#define BUFFER_SIZE 1024

typedef struct {
    int honey_level;
    int bear_sleeping;
    pthread_mutex_t mutex;
} HoneyPot;

typedef struct {
    int bee_socket;
    int bear_socket;
    HoneyPot *honey_pot;
} ClientData;

void error(const char *message) {
    perror(message);
    printf("%s", message);
    exit(EXIT_FAILURE);
}

void eats_honey(int bear_socket){
    printf("Bear is eating honey\n");
    if (send(bear_socket, "up_bear", strlen("up_bear"), 0) < 0) {
            error("Failed to send message to the server");
    }
    char response[BUFFER_SIZE];
    while (1) {
        memset(response, 0, sizeof(response));
        if (recv(bear_socket, response, BUFFER_SIZE, 0) < 0) {
            error("Failed to receive response from the server");
        }
        if(strcmp(response, "bear_sleep") == 0){
           break;
        }
        printf("Incorrect response: %s\n", response);
    }
}

void* handle_client(void *arg) {
    ClientData *client_data = (ClientData*)arg;
    int client_socket = client_data->bee_socket;
    int bear_socket = client_data->bear_socket;
    HoneyPot *honey_pot = client_data->honey_pot;
    char response[BUFFER_SIZE];
    int read_size;

    while (1) {
        memset(response, 0, sizeof(response));
        if (recv(client_socket, response, BUFFER_SIZE, 0) < 0) {
            error("Failed to receive response from the server");
        }

        if (strcmp(response, "collect_honey") == 0) {
            pthread_mutex_lock(&(honey_pot->mutex));

            honey_pot->honey_level++;
            printf("Bee added honey to the pot.\n");

            if (honey_pot->honey_level == 5) {
                honey_pot->bear_sleeping = 0;
                printf("The honey pot is full!\n");
                eats_honey(bear_socket);
                printf("Bear is sleep\n");
                honey_pot->honey_level = 0;
                honey_pot->bear_sleeping = 1;
            }

            pthread_mutex_unlock(&(honey_pot->mutex));
        }
    }

    close(client_socket);
    close(bear_socket);
}

int main(int argc, char **argv) {
    int server_socket, client_socket, opt = 1;
    struct sockaddr_in server_address, client_address;
    HoneyPot honey_pot = { 0, 10, PTHREAD_MUTEX_INITIALIZER };
    pthread_t threads[MAX_THREADS];

    // Create server socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("Failed to create socket");
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        error("Failed to set socket options");
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(argv[1]);
    server_address.sin_port = htons(atoi(argv[2]));

    // Bind the socket to the specified address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        error("Failed to bind");
    }

    // Start listening for incoming connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        error("Failed to listen");
    }

    printf("Server started. Waiting for clients...\n");

    int bee_socket, bear_socket;

    // Accept client connections
    int address_size = sizeof(client_address);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&address_size)) < 0) {
            error("Failed to accept client connection");
        }

        char response[BUFFER_SIZE];
        memset(response, 0, sizeof(response));
        if (recv(client_socket, response, BUFFER_SIZE, 0) < 0) {
            error("Failed to receive response from the server");
        }
        if (strcmp(response, "bee") == 0) {
            printf("Client bee connected. Waiting for honey collection...\n");
            bee_socket = client_socket;
        } else {
          if (strcmp(response, "bear") == 0) {
            printf("Client bear connected. Waiting for honey collection...\n");
            bear_socket = client_socket;
          } else {
             printf("Client %s connected. Waiting for honey collection...\n", response);
          }
        }

    }
    ClientData *client_data = (ClientData*)malloc(sizeof(ClientData));

    client_data->honey_pot = &honey_pot;
    client_data->bee_socket = bee_socket;
    client_data->bear_socket = bear_socket;

    if (pthread_create(&threads[0], NULL, handle_client, (void*)client_data) != 0) {
        error("Failed to create thread");
    }
    // Wait for all threads to finish
    for (int i = 0; i < MAX_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
