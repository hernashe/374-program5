#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_BUF 100000

void strip_newline(char *s) {
    s[strcspn(s, "\n")] = '\0';
}

int valid_text(const char *s) {
    for (int i = 0; s[i] != '\0'; i++) {
        if ((s[i] < 'A' || s[i] > 'Z') && s[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int socketFD, portNumber;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;

    char ciphertext[MAX_BUF];
    char key[MAX_BUF];
    char buffer[200000];
    FILE *fp;

    if (argc != 4) {
        fprintf(stderr, "Usage: %s ciphertext key port\n", argv[0]);
        exit(1);
    }

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", argv[1]);
        exit(1);
    }
    memset(ciphertext, '\0', sizeof(ciphertext));
    fgets(ciphertext, sizeof(ciphertext), fp);
    fclose(fp);
    strip_newline(ciphertext);

    fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: cannot open %s\n", argv[2]);
        exit(1);
    }
    memset(key, '\0', sizeof(key));
    fgets(key, sizeof(key), fp);
    fclose(fp);
    strip_newline(key);

    if (!valid_text(ciphertext) || !valid_text(key)) {
        fprintf(stderr, "dec_client error: input contains bad characters\n");
        exit(1);
    }

    if (strlen(key) < strlen(ciphertext)) {
        fprintf(stderr, "Error: key '%s' is too short\n", argv[2]);
        exit(1);
    }

    portNumber = atoi(argv[3]);

    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        fprintf(stderr, "Error: could not contact dec_server on port %d\n", portNumber);
        exit(2);
    }

    serverHostInfo = gethostbyname("localhost");
    if (serverHostInfo == NULL) {
        fprintf(stderr, "Error: could not contact dec_server on port %d\n", portNumber);
        close(socketFD);
        exit(2);
    }

    memset((char*)&serverAddress, '\0', sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNumber);

    memcpy((char*)&serverAddress.sin_addr.s_addr,
           (char*)serverHostInfo->h_addr,
           serverHostInfo->h_length);

    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        fprintf(stderr, "Error: could not contact dec_server on port %d\n", portNumber);
        close(socketFD);
        exit(2);
    }

    /* handshake: identify as decryption client */
    memset(buffer, '\0', sizeof(buffer));
    strcpy(buffer, "DEC\n");
    send(socketFD, buffer, strlen(buffer), 0);

    /* wait for approval */
    memset(buffer, '\0', sizeof(buffer));
    recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    if (strcmp(buffer, "OK") != 0) {
        fprintf(stderr, "Error: could not contact dec_server on port %d\n", portNumber);
        close(socketFD);
        exit(2);
    }

    /* send ciphertext + key */
    memset(buffer, '\0', sizeof(buffer));
    sprintf(buffer, "%s\n%s", ciphertext, key);
    send(socketFD, buffer, strlen(buffer), 0);

    /* receive plaintext */
    memset(buffer, '\0', sizeof(buffer));
    recv(socketFD, buffer, sizeof(buffer) - 1, 0);

    printf("%s\n", buffer);

    close(socketFD);
    return 0;
}