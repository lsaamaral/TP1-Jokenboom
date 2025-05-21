#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int argc, char **argv) {
    printf("usage: %s <server IP> <server port>", argv[0]);
    printf("example: %s 127.0.0.1 51511", argv[0]);
    exit(EXIT_FAILURE);
}

void clean_n() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != addrparse(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s = 0;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if ( 0 != connect(s, addr, sizeof(storage))) {
        logexit("connect");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);

    printf("Conectado ao servidor.\n");

    GameMessage msg;
    int running = 1;

    while(running) {
        if (recv(s, &msg, sizeof(msg), 0) <= 0) {
            break;
        }

        switch (msg.type) {
            case MSG_REQUEST:
                printf("%s\n", msg.message);
                msg.type = MSG_RESPONSE;
                if (scanf("%d", &msg.client_action) != 1) {
                    msg.client_action = -1;
                }
                clean_n();
                send(s, &msg, sizeof(msg), 0);
                break;

            case MSG_RESULT:
                printf("\n%s\n", msg.message);
                printf("Placar atualizado: Você %d x %d Servidor\n", msg.client_wins, msg.server_wins);
                break;
            
            case MSG_PLAY_AGAIN_REQUEST:
                printf("%s\n", msg.message);
                msg.type = MSG_PLAY_AGAIN_RESPONSE;
                if (scanf("%d", &msg.client_action) != 1) {
                    msg.client_action = -1;
                }
                clean_n();
                send(s, &msg, sizeof(msg), 0);
                break;
            
            case MSG_ERROR:
                printf("\nErro: %s\n", msg.message);
                if (strstr(msg.message, "jogar novamente") != NULL) {
                    msg.type = MSG_PLAY_AGAIN_RESPONSE;
                    if (scanf("%d", &msg.client_action) != 1) {
                        msg.client_action = -1;
                    }
                    clean_n();
                    send(s, &msg, sizeof(msg), 0);
                }
                break;

            case MSG_END:
                printf("\n%s\n", msg.message);
                running = 0;
                break;
            
            default:
                printf("Mensagem não conhecida\n");
                running = 0;
        }
    }
    close(s);
    exit(EXIT_SUCCESS);
}
