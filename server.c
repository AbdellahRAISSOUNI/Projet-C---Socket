#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENTS 5
#define BUFFER_SIZE 1024

// Prototypes des fonctions
void authentifier_utilisateur(SOCKET client_socket, char *username, char *password);
void menu_admin(SOCKET client_socket);
void menu_invite(SOCKET client_socket);

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    SOCKET server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    int client_len;
    char buffer[BUFFER_SIZE];
    SOCKET clients[MAX_CLIENTS];
    int num_clients = 0;

    // Créer le socket serveur
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        printf("Error creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Lier le socket serveur à une adresse
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8888);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error binding socket: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Écouter les connexions entrantes
    if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Error listening on socket: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("Le serveur écoute sur le port 8888...\n");

    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket == INVALID_SOCKET) {
            printf("Error accepting connection: %d\n", WSAGetLastError());
            continue;
        }

        printf("Nouveau client connecté\n");

        // Ajouter le client à la liste des clients connectés
        clients[num_clients++] = client_socket;

        // Authentifier l'utilisateur
        char username[BUFFER_SIZE];
        char password[BUFFER_SIZE];
        authentifier_utilisateur(client_socket, username, password);

        // Vérifier si l'utilisateur est admin ou invité
        if (strcmp(username, "admin") == 0 && strcmp(password, "admin123") == 0) {
            menu_admin(client_socket);
        } else {
            menu_invite(client_socket);
        }

        // Supprimer le client de la liste des clients connectés
        closesocket(client_socket);
        for (int i = 0; i < num_clients; i++) {
            if (clients[i] == client_socket) {
                clients[i] = clients[--num_clients];
                break;
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

void authentifier_utilisateur(SOCKET client_socket, char *username, char *password) {
    char buffer[BUFFER_SIZE];

    // Recevoir le nom d'utilisateur du client
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(username, buffer);

    // Recevoir le mot de passe du client
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(password, buffer);

    printf("Nom d'utilisateur reçu : %s, mot de passe : %s\n", username, password);
}

void menu_admin(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];

    // Envoyer le menu admin au client
    strcpy(buffer, "Menu administrateur :\n1. Ajouter un contact\n2. Modifier un contact\n3. Supprimer un contact\n4. Rechercher un contact\n5. Quitter\n");
    send(client_socket, buffer, strlen(buffer), 0);

    while (1) {
        // Recevoir le choix du client
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        int choix = atoi(buffer);

        switch (choix) {
            case 1:
                // Ajouter un contact
                printf("Le client a choisi d'ajouter un contact\n");
                break;
            case 2:
                // Modifier un contact
                printf("Le client a choisi de modifier un contact\n");
                break;
            case 3:
                // Supprimer un contact
                printf("Le client a choisi de supprimer un contact\n");
                break;
            case 4:
                // Rechercher un contact
                printf("Le client a choisi de rechercher un contact\n");
                break;
            case 5:
                // Quitter
                printf("Le client a choisi de quitter\n");
                return;
            default:
                printf("Choix invalide\n");
                break;
        }
    }
}

void menu_invite(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];

    // Envoyer le menu invité au client
    strcpy(buffer, "Menu invité :\n1. Afficher les contacts\n2. Rechercher un contact\n3. Quitter\n");
    send(client_socket, buffer, strlen(buffer), 0);

    while (1) {
        // Recevoir le choix du client
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        int choix = atoi(buffer);

        switch (choix) {
            case 1:
                // Afficher les contacts
                printf("Le client a choisi d'afficher les contacts\n");
                break;
            case 2:
                // Rechercher un contact
                printf("Le client a choisi de rechercher un contact\n");
                break;
            case 3:
                // Quitter
                printf("Le client a choisi de quitter\n");
                return;
            default:
                printf("Choix invalide\n");
                break;
        }
    }
}
