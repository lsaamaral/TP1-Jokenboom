#include "common.h"

#include <stdlib.h>
#include <arpa/inet.h>

/**
 * @brief Exibe uma mensagem para auxiliar a criar um servidor ou cliente. Funcao retirada da videoaula do professor Italo Cunha
 * 
 * @param argc Numero de argumentos
 * @param argv Vetor de argumentos
 */
void usage(int argc, char **argv);

/**
 * @brief Retorna o nome da acao de 0 a 4
 * 
 * @param action Codigo de 0 a 4
 * @return const char* Nome da acao
 */
const char* action_name(int action);

/**
 * @brief Determina se a jogada foi ganha pelo cliente ou pelo servidor
 * 
 * @param player Acao do cliente (jogador)
 * @param server Acao do servidor
 * @return int Retorna -1 para empate, 0 para vitoria do servidor, 1 para vitoria do jogador
 */
int winner(int player, int server);

/**
 * @brief Logica principal do jogo. O que acontece dependendo da mensagem do cliente alem mensagens de aviso no terminal
 * 
 * @param csock Socket do cliente conectado
 */
void game(int csock);

/**
 * @brief Loop principal, verifica se a conexao esta correta, prepara o socket e atende os clientes. Repete o jogo para novos clientes em um loop infinito
 * 
 * @param argc Numero de argumentos
 * @param argv Vetor de argumento
 * @return Retorna EXIT_SUCCESS quando o codigo nao teve nenhum problema
 */
int main(int argc, char **argv);