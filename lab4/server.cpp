#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void log_message(const char *client_addr, const char *message) {
    printf("%s:%s\n", client_addr, message);
}

int main(int argc, char *argv[]) {

    int port = atoi(argv[1]);
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char client_addr_str[INET_ADDRSTRLEN];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    listen(server_fd, 1);

    printf("Server started on port %d. Waiting for connections...\n", port);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        inet_ntop(AF_INET, &client_addr.sin_addr, client_addr_str, INET_ADDRSTRLEN);
        log_message(client_addr_str, "Client connected");

        srand(time(NULL));
        int number_to_guess = rand() % 100 + 1;
        int attempts = 0;
        int guess;
        char response[BUFFER_SIZE];

        sprintf(response, "Guess the number between 1 and 100");
        send(client_fd, response, strlen(response), 0);
        log_message(client_addr_str, "Game started. Number to guess generated");

        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int valread = read(client_fd, buffer, BUFFER_SIZE);
            if (valread <= 0) {
                log_message(client_addr_str, "Client disconnected");
                break;
            }

            guess = atoi(buffer);
            attempts++;

            if (guess < number_to_guess) {
                strcpy(response, "Too low");
                log_message(client_addr_str, "Client guessed too low");
            } else if (guess > number_to_guess) {
                strcpy(response, "Too high");
                log_message(client_addr_str, "Client guessed too high");
            } else {
                sprintf(response, "Correct! You guessed the number in %d attempts", attempts);
                send(client_fd, response, strlen(response), 0);
                log_message(client_addr_str, "Client guessed correctly");
                break;
            }

            send(client_fd, response, strlen(response), 0);
        }

        close(client_fd);
        log_message(client_addr_str, "Client disconnected");
    }

    close(server_fd);
    return 0;
}