#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

int main() {
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }

    SOCKET client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Créer le socket client
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        printf("Error creating socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    // Configurer l'adresse du serveur
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);

    // Se connecter au serveur
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error connecting to server: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Demander le nom d'utilisateur et le mot de passe
    printf("Entrez votre nom d'utilisateur : ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0'; // Supprimer le caractère de nouvelle ligne
    if (send(client_socket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("Error sending data: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    printf("Entrez votre mot de passe : ");
    fgets(buffer, BUFFER_SIZE, stdin);
    buffer[strcspn(buffer, "\n")] = '\0';
    if (send(client_socket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
        printf("Error sending data: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // Recevoir le menu du serveur
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
        printf("Error receiving data: %d\n", WSAGetLastError());
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    printf("%s", buffer);

    while (1) {
        printf("Entrez votre choix : ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        if (send(client_socket, buffer, strlen(buffer), 0) == SOCKET_ERROR) {
            printf("Error sending data: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }

        // Recevoir la réponse du serveur
        if (recv(client_socket, buffer, BUFFER_SIZE, 0) == SOCKET_ERROR) {
            printf("Error receiving data: %d\n", WSAGetLastError());
            closesocket(client_socket);
            WSACleanup();
            return 1;
        }
        printf("%s\n", buffer);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
