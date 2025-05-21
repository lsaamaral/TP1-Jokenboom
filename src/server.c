#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

const char* action_name(int action) {
    const char* names[] = {
        "Nuclear Attack",
        "Intercept Attack",
        "Cyber Attack",
        "Drone Strike",
        "Bio Attack"
    };

    return names[action];
}

int winner(int player, int server) {

    int win[5][5] = {
        {-1, 0, 1, 1, 0}, // -1 empata, 1 jogador vence e 0 servidor vence
        {1, -1, 0, 0, 1},
        {0, 1, -1, 1, 0},
        {0, 1, 0, -1, 1},
        {1, 0, 1, 0, -1}
    };

    return win[player][server];
}

void game(int csock) {
    GameMessage msg;
    int players_count = 0;
    int servers_count = 0;
    int keep_playing = 1;

    srand(time(NULL) ^ getpid());

    while (keep_playing) {
        msg.type = MSG_REQUEST;
        snprintf(msg.message, MSG_SIZE, "Escolha sua jogada:\n0 - Nuclear Attack\n1 - Intercept Attack\n2 - Cyber Attack\n3 - Drone Strike\n4 - Bio Attack\n");
        printf("Apresentando as opções para o cliente.\n");
        send(csock, &msg, sizeof(msg), 0);

        if (recv(csock, &msg, sizeof(msg), 0) <= 0) {
            break;
        }

        printf("Cliente escolheu %d.\n", msg.client_action);

        if (msg.type != MSG_RESPONSE || msg.client_action < 0 || msg.client_action > 4) {
            msg.type = MSG_ERROR;
            strncpy(msg.message, "Por favor, selecione um valor de 0 a 4.\n", sizeof(msg.message));
            printf("Erro: opção inválida de jogada.\n");
            send(csock, &msg, sizeof(msg), 0);
            continue;
        }

        msg.server_action = rand() % 5;
        printf("Servidor escolheu aleatoriamente %d.\n", msg.server_action);
        msg.result = winner(msg.client_action, msg.server_action);

        if (msg.result == 1) {
            players_count++;
        } else if (msg.result == 0) {
            servers_count++;
        }

        msg.client_wins = players_count;
        msg.server_wins = servers_count;

        msg.type = MSG_RESULT;
        snprintf(msg.message, MSG_SIZE, "Você escolheu: %s\nServidor escolheu: %s\nResultado: %s\n", action_name(msg.client_action), action_name(msg.server_action), msg.result == -1 ? "Empate!" : msg.result == 1 ? "Vitória!" : "Derrota!");

        send(csock, &msg, sizeof(msg), 0);

        if (msg.result == -1) {
            printf("Jogo empatado.\nSolicitando ao cliente mais uma escolha.\n");
            continue;
        } else {
            printf("Placar atualizado: Cliente %d x %d Servidor\n", players_count, servers_count);
        }

        msg.type = MSG_PLAY_AGAIN_REQUEST;
        strncpy(msg.message, "Deseja jogar novamente?\n1 - Sim\n0 - Não\n", sizeof(msg.message));
        printf("Perguntando se o cliente deseja jogar novamente.\n");
        send(csock, &msg, sizeof(msg), 0);

        int valid_response = 0;
        while (!valid_response) {
            if (recv(csock, &msg, sizeof(msg), 0) <= 0) {
                keep_playing = 0;
                break;
            }

            if (msg.type != MSG_PLAY_AGAIN_RESPONSE || (msg.client_action != 0 && msg.client_action != 1)) {
                msg.type = MSG_ERROR;
                strncpy(msg.message, "Por favor, digite 1 para jogar novamente ou 0 para encerrar.\n", sizeof(msg.message));
                printf("Erro: resposta inválida para jogar novamente.\n");
                send(csock, &msg, sizeof(msg), 0);
            } else {
                valid_response = 1;
            }
        }

        if (!valid_response) break;

        keep_playing = msg.client_action;

        if (msg.client_action == 1) {
            printf("Cliente deseja jogar novamente.\n");
        } else {
            printf("Cliente não deseja jogar novamente.\nEnviando placar final.\nEncerrando conexão.\n");
        }
    }

    msg.type = MSG_END;
    snprintf(msg.message, MSG_SIZE, "Fim de jogo!\nPlacar final: Você %d x %d Servidor\nObrigado por jogar!", players_count, servers_count);
    msg.client_wins = players_count;
    msg.server_wins = servers_count;
    send(csock, &msg, sizeof(msg), 0);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s = 0;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }


    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[MSG_SIZE];
    addrtostr(addr, addrstr, MSG_SIZE);
    int wichip = 0;
    if (strcmp(argv[1], "v4") == 0) {
        wichip = 4;
    } else {
        wichip = 6;
    }
    printf("Servidor iniciado em modo IPv%d na porta %s. Aguardando conexão...\n", wichip, argv[2]);

    while(1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[MSG_SIZE];
        addrtostr(caddr, caddrstr, MSG_SIZE);
        printf("Cliente conectado.\n");

        game(csock);
        close(csock);
        printf("Cliente desconectado.\n");
    }

    exit(EXIT_SUCCESS);
}