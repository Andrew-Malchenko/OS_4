#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define MAX_MESSAGE_LENGTH 1024

int main(int argc, char **argv) {
    int clientSocket;
    struct sockaddr_in serverAddress;
    char message[] = "bee";
    char buffer[MAX_MESSAGE_LENGTH];

    // Создание UDP-сокета для клиента
    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (clientSocket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(argv[2]));

    while (1) {
        printf("Press Enter to send a honey request (or 'q' to quit): ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strcmp(buffer, "q\n") == 0) {
            break;
        }

        // Отправка сообщения на сервер
        ssize_t sentLen = sendto(clientSocket, message, strlen(message), 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
        if (sentLen < 0) {
            perror("Failed to send message");
            break;
        }

        printf("Bee sent a honey request\n");
    }

    // Закрытие сокета клиента
    close(clientSocket);

    return 0;
}
