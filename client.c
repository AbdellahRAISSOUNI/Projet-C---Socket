#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

void handleServerMenu(SOCKET connectSocket, char* type) {
    char menu[BUFFER_SIZE];
    int choice;

    recv(connectSocket, menu, sizeof(menu), 0);
    printf("%s", menu);

    while (1) {
        if (scanf("%d", &choice) != 1) {
            printf("Entrée invalide. Veuillez entrer un nombre entier.\n");
            // Vidage du tampon d'entrée
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        // Vidage du tampon d'entrée
        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (choice >= 1 && choice <= 6) {
            break;
        } else {
            printf("Choix invalide. Veuillez entrer un nombre entre 1 et 6.\n");
        }
    }

    send(connectSocket, (char*)&choice, sizeof(int), 0);

    char message[BUFFER_SIZE];
    recv(connectSocket, message, sizeof(message), 0);
    printf("%s\n", message);
}


int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Erreur lors de l'initialisation de Winsock: %d\n", iResult);
        return 1;
    }

    SOCKET connectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo("127.0.0.1", "8000", &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %d\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (connectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    char username[50], password[50];
    int attempts = 0;
    char type[10]; // Declare type here

    while (attempts < 3) {
        printf("Nom d'utilisateur: ");
        scanf("%s", username);
        printf("Mot de passe: ");
        scanf("%s", password);

        send(connectSocket, username, strlen(username), 0);
        send(connectSocket, password, strlen(password), 0);

        recv(connectSocket, type, sizeof(type), 0);

        if (strcmp(type, "admin") == 0) {
            printf("*** Vous êtes admin ***\n");
            break;
        } else if (strcmp(type, "invite") == 0) {
            printf("*** Vous êtes invité ***\n");
            break;
        } else {
            printf("Échec de l'authentification\n");
            attempts++;
        }
    }

    if (attempts == 3) {
        printf("Trop de tentatives échouées\n");
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    int quit = 0;
    int choice; // Declare choice here

    while (!quit) {
        handleServerMenu(connectSocket, type);

        if ((strcmp(type, "admin") == 0 && choice == 6) || (strcmp(type, "invite") == 0 && choice == 3)) {
            quit = 1;
        }
    }

    closesocket(connectSocket);
    WSACleanup();

    return 0;
}
