#define _WIN32_WINNT 0x0501
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

typedef struct {
    char username[50];
    char password[50];
    char type[10];
} User;

User users[100];
int numUsers = 0;

void loadUsers() {
    FILE* file = fopen("comptes.txt", "r");
    if (file == NULL) {
        printf("Erreur lors de l'ouverture du fichier comptes.txt\n");
        return;
    }

    char line[100];
    while (fgets(line, sizeof(line), file) != NULL) {
        sscanf(line, "%s %s %s", users[numUsers].username, users[numUsers].password, users[numUsers].type);
        numUsers++;
    }

    fclose(file);
}

int authenticateUser(char* username, char* password) {
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return i;
        }
    }
    return -1;
}

void sendMenu(SOCKET clientSocket, char* userType) {
    if (strcmp(userType, "admin") == 0) {
        char menu[] = "*********************Menu*********************\n"
                      "1-Ajouter un contact\n"
                      "2-Rechercher un contact\n"
                      "3-Supprimer un contact\n"
                      "4-Modifier un contact\n"
                      "5-Afficher tous les contacts\n"
                      "6-Quitter\n"
                      "Entrer votre choix: ";
        send(clientSocket, menu, strlen(menu), 0);
    } else if (strcmp(userType, "invite") == 0) {
        char menu[] = "*********************Menu*********************\n"
                      "1-Rechercher un contact\n"
                      "2-Afficher tous les contacts\n"
                      "3-Quitter\n"
                      "Entrer votre choix: ";
        send(clientSocket, menu, strlen(menu), 0);
    }
}

void handleClient(SOCKET clientSocket, char* userType) {
    int choice;
    int quit = 0;

    while (!quit) {
        sendMenu(clientSocket, userType); // Envoyer le menu approprié

        recv(clientSocket, (char*)&choice, sizeof(int), 0);
        handleClientChoice(clientSocket, userType, choice);

        if ((strcmp(userType, "admin") == 0 && choice == 6) || (strcmp(userType, "invite") == 0 && choice == 3)) {
            quit = 1;
        }
    }
}

void handleClientChoice(SOCKET clientSocket, char* userType, int choice) {
    char message[BUFFER_SIZE];

    if (strcmp(userType, "admin") == 0) {
        switch (choice) {
            case 1:
                strcpy(message, "Vous avez choisi l'option 1 - Ajouter un contact");
                break;
            case 2:
                strcpy(message, "Vous avez choisi l'option 2 - Rechercher un contact");
                break;
            case 3:
                strcpy(message, "Vous avez choisi l'option 3 - Supprimer un contact");
                break;
            case 4:
                strcpy(message, "Vous avez choisi l'option 4 - Modifier un contact");
                break;
            case 5:
                strcpy(message, "Vous avez choisi l'option 5 - Afficher tous les contacts");
                break;
            case 6:
                strcpy(message, "Vous avez choisi l'option 6 - Quitter");
                break;
            default:
                strcpy(message, "Option invalide");
                break;
        }
    } else if (strcmp(userType, "invite") == 0) {
        switch (choice) {
            case 1:
                strcpy(message, "Vous avez choisi l'option 1 - Rechercher un contact");
                break;
            case 2:
                strcpy(message, "Vous avez choisi l'option 2 - Afficher tous les contacts");
                break;
            case 3:
                strcpy(message, "Vous avez choisi l'option 3 - Quitter");
                break;
            default:
                strcpy(message, "Option invalide");
                break;
        }
    }

    int bytesSent = send(clientSocket, message, strlen(message), 0);
    if (bytesSent != strlen(message)) {
        printf("Erreur lors de l'envoi du message au client\n");
    }
}

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Erreur lors de l'initialisation de Winsock: %d\n", iResult);
        return 1;
    }

    loadUsers();

    SOCKET listeningSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, "8000", &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    listeningSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listeningSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    iResult = bind(listeningSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        printf("Listen failed with error: %d\n", WSAGetLastError());
        closesocket(listeningSocket);
        WSACleanup();
        return 1;
    }

    printf("Serveur en attente de connexions...\n");

    SOCKET clientSocket;
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    while (1) {
        clientSocket = accept(listeningSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed: %d\n", WSAGetLastError());
            closesocket(listeningSocket);
            WSACleanup();
            return 1;
        }

        printf("Nouvelle connexion acceptée\n");

        char username[50], password[50];
        int attempts = 0;
        int userIndex;

        while (attempts < 3) {
            int bytesReceived = recv(clientSocket, username, sizeof(username), 0);
            if (bytesReceived <= 0) {
                break; // Connection closed or error occurred
            }
            username[bytesReceived] = '\0'; // Null-terminate the received string

            bytesReceived = recv(clientSocket, password, sizeof(password), 0);
            if (bytesReceived <= 0) {
                break; // Connection closed or error occurred
            }
            password[bytesReceived] = '\0'; // Null-terminate the received string

            userIndex = authenticateUser(username, password);
            if (userIndex != -1) {
                char type[10];
                strcpy(type, users[userIndex].type);
                send(clientSocket, type, sizeof(type), 0);
                printf("Utilisateur %s (%s) connecté\n", username, type);

                handleClient(clientSocket, type); // Handle client requests

                break;
            } else {
                char message[] = "Échec de l'authentification";
                send(clientSocket, message, sizeof(message), 0);
                attempts++;
            }
        }

        if (attempts == 3) {
            printf("Trop de tentatives pour %s\n", username);
            closesocket(clientSocket);
        }
    }

    closesocket(listeningSocket);
    WSACleanup();

    return 0;
}

