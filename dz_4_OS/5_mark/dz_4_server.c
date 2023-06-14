#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 12345

int main(int argc, char **argv) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress, clientAddressBear;
    socklen_t addrLen = sizeof(clientAddress);
    char buffer[MAX_MESSAGE_LENGTH];
    int H = 10; // Вместимость горшка
    int honey = 0; // Текущее количество меда в горшке

    // Создание UDP-сокета для сервера
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return 1;
    }

    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddress.sin_port = htons(atoi(argv[2]));

    // Привязка сокета к адресу сервера
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("Failed to bind socket");
        return 1;
    }

    printf("Server is running...\n");
    printf("Waiting connected bear client\n");
    while (1) {
    ssize_t recvLen = recvfrom(serverSocket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddress, &addrLen);
    if (recvLen < 0) {
        perror("Failed to receive message");
        break;
    }

    // Завершение строки сообщения
    buffer[recvLen] = '\0';

    if (strcmp(buffer, "bear_client") == 0) {
        clientAddressBear = clientAddress;
        break;
    }

    }
    printf("Client Bear connected\n");
    while (1) {
        // Получение сообщения от клиента
        ssize_t recvLen = recvfrom(serverSocket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddress, &addrLen);
        if (recvLen < 0) {
            perror("Failed to receive message");
            break;
        }

        // Завершение строки сообщения
        buffer[recvLen] = '\0';

        // Логика поведения пчел
        if (strcmp(buffer, "bee") == 0) {
            if (honey < H) {
                honey++;
                printf("Bee %s:%d added honey (%d/%d)\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port), honey, H);

                if (honey == H) {
                    // Отправка сообщения медведю
                    ssize_t sentLen = sendto(serverSocket, "wake up", strlen("wake up"), 0, (struct sockaddr *)&clientAddressBear, addrLen);
                    if (sentLen < 0) {
                        perror("Failed to send message");
                        break;
                    }
                }
            } else {
                printf("Bee %s:%d tried to add honey but the pot is full\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
            }
        }
        // Логика поведения медведя
        else if (strcmp(buffer, "bear") == 0) {
            if (honey == H) {
                honey = 0;
                printf("Bear woke up and ate all the honey\n");
            } else {
                printf("Bear woke up but the pot is not full\n");
            }
        }
    }

    // Закрытие сокета сервера
    close(serverSocket);

    return 0;
}
