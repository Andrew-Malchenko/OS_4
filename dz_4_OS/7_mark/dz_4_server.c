#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#define MAX_MESSAGE_LENGTH 1024
#define SERVER_PORT 12345

int main(int argc, char **argv) {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress, clientAddressBear, clientAddressViewer, clientAddressBee;
    socklen_t addrLen = sizeof(clientAddress);
    char buffer[MAX_MESSAGE_LENGTH];
    int H = atoi(argv[3]); // Вместимость горшка
    int honey = 0; // Текущее количество меда в горшке
    int counts_bee = atoi(argv[4]); // Количество пчёл в улье
    
    if(counts_bee <= 0){
        perror("counts bee <= 0");
        return 1;
    }

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
    printf("Client Bear connected, waiting connected viewer client\n");
    
    while (1) {
    ssize_t recvLen = recvfrom(serverSocket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddress, &addrLen);
    if (recvLen < 0) {
        perror("Failed to receive message");
        break;
    }

    // Завершение строки сообщения
    buffer[recvLen] = '\0';

    if (strcmp(buffer, "viewer_client") == 0) {
        clientAddressViewer = clientAddress;
        break;
    }

    }
    printf("Client Viewer connected, waiting connected bee client\n");
    
    while (1) {
    ssize_t recvLen = recvfrom(serverSocket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddress, &addrLen);
    if (recvLen < 0) {
        perror("Failed to receive message");
        break;
    }

    // Завершение строки сообщения
    buffer[recvLen] = '\0';

    if (strcmp(buffer, "bee_client") == 0) {
        clientAddressBee = clientAddress;
        break;
    }

    }
    printf("Client Bee connected\n");
    
    char str[MAX_MESSAGE_LENGTH];
    snprintf(str, MAX_MESSAGE_LENGTH - 1, "%d", counts_bee);
    ssize_t sentLen = sendto(serverSocket, str, strlen(str), 0, (struct sockaddr *)&clientAddressBee, addrLen);
    if (sentLen < 0) {
       perror("Failed to send message");
    }
    
    while (1) {
        // Получение сообщения от клиента
        ssize_t recvLen = recvfrom(serverSocket, buffer, MAX_MESSAGE_LENGTH, 0, (struct sockaddr *)&clientAddress, &addrLen);
        if (recvLen < 0) {
            perror("Failed to receive message");
            break;
        }

        // Завершение строки сообщения
        buffer[recvLen] = '\0';
        
        // Логика поведения медведя
        if (strcmp(buffer, "bear") == 0) {
            if (honey == H) {
                honey = 0;
                printf("Bear woke up and ate all the honey\n");
                ssize_t sentLen = sendto(serverSocket, "bear sleeping", strlen("bear sleeping"), 0, (struct sockaddr *)&clientAddressViewer, addrLen);
                if (sentLen < 0) {
                    perror("Failed to send message");
                    break;
                }
            } else {
                printf("Bear woke up but the pot is not full\n");
            }
        }
        // Логика поведения пчел
            else {
            if (honey < H) {
                honey++;
                printf("Bee number %d added honey\n", atoi(buffer));
                ssize_t sentLen3 = sendto(serverSocket, "Bee added honey", strlen("Bee added honey"), 0,
                (struct sockaddr *)&clientAddressViewer, addrLen);
                if (sentLen3 < 0) {
                    perror("Failed to send message");
                    break;
                }

                if (honey == H) {
                    // Отправка сообщения медведю
                    ssize_t sentLen = sendto(serverSocket, "bear wake up", strlen("bear wake up"), 0, (struct sockaddr *)&clientAddressBear, addrLen);
                    if (sentLen < 0) {
                        perror("Failed to send message");
                        break;
                    }
                    ssize_t sentLen2 = sendto(serverSocket, "bear wake up", strlen("bear wake up"), 0, (struct sockaddr *)&clientAddressViewer, addrLen);
                    if (sentLen2 < 0) {
                        perror("Failed to send message");
                        break;
                    }
                }
            } else {
                printf("Bee number %d tried to add honey but the pot is full\n", atoi(buffer));
                ssize_t sentLen = sendto(serverSocket, "Bee tried to add honey but the pot is full", strlen("Bee tried to add honey but the pot is full"), 0,
                (struct sockaddr *)&clientAddressViewer, addrLen);
                if (sentLen < 0) {
                    perror("Failed to send message");
                    break;
                }
            }
        }
    }

    // Закрытие сокета сервера
    close(serverSocket);

    return 0;
}
