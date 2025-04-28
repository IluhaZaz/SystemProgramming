#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);

    inet_pton(AF_INET, argv[1], &serv_addr.sin_addr);

    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    read(sock, buffer, BUFFER_SIZE);
    printf("%s\n", buffer);

    int low = 1;
    int high = 100;
    int guess;
    int attempts = 0;

    while (low <= high) {
        guess = (low + high) / 2;
        attempts++;

        char guess_str[10];
        sprintf(guess_str, "%d", guess);
        send(sock, guess_str, strlen(guess_str), 0);
        printf("Guessing %d\n", guess);

        memset(buffer, 0, BUFFER_SIZE);
        read(sock, buffer, BUFFER_SIZE);
        printf("Server response: %s\n", buffer);

        if (strstr(buffer, "Correct") != NULL) {
            break;
        } else if (strstr(buffer, "low") != NULL) {
            low = guess + 1;
        } else if (strstr(buffer, "high") != NULL) {
            high = guess - 1;
        }
    }

    printf("Guessed the number %d in %d attempts\n", guess, attempts);
    close(sock);
    return 0;
}