#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int listenSocket, connectionSocket, portNumber;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t sizeOfClientInfo = sizeof(clientAddress);

    if (argc < 2) {
        fprintf(stderr, "USAGE: %s port\n", argv[0]);
        exit(1);
    }

    portNumber = atoi(argv[1]);

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        perror("socket");
        exit(1);
    }

    memset((char *)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        perror("bind");
        exit(1);
    }

    listen(listenSocket, 5);

    while (1) {
        while (waitpid(-1, NULL, WNOHANG) > 0);

        connectionSocket = accept(listenSocket,
                                  (struct sockaddr *)&clientAddress,
                                  &sizeOfClientInfo);

        if (connectionSocket < 0) {
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            close(listenSocket);

            char buffer[200000];
            memset(buffer, '\0', sizeof(buffer));

            recv(connectionSocket, buffer, sizeof(buffer) - 1, 0);

            char *plaintext = strtok(buffer, "\n");
            char *key = strtok(NULL, "\n");

            char ciphertext[200000];
            memset(ciphertext, '\0', sizeof(ciphertext));

            for (int i = 0; plaintext[i] != '\0'; i++) {
                int p, k, c;

                if (plaintext[i] == ' ')
                    p = 26;
                else
                    p = plaintext[i] - 'A';

                if (key[i] == ' ')
                    k = 26;
                else
                    k = key[i] - 'A';

                c = (p - k + 27) % 27;

                if (c == 26)
                    ciphertext[i] = ' ';
                else
                    ciphertext[i] = 'A' + c;
            }

            send(connectionSocket, ciphertext, strlen(ciphertext), 0);
            close(connectionSocket);
            exit(0);
        } else {
            close(connectionSocket);
        }
    }

    close(listenSocket);
    return 0;
}