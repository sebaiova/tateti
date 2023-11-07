#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>

#include "tateti.h"

int wait_for_player(int sockfd)
{
    int player_sock;
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr);

    while ((player_sock = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) < 0)
        close(player_sock);

    return player_sock;
}

int init(int portno)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR para que el sistema operativo asigne al servidor el ip local
    serv_addr.sin_port = htons(portno);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error(1, 0, "ERROR on binding.\n");

    listen(sockfd, 2); // El servidor hablilita su socket para poder recibir conexiones, llamando a la función listen(). Los parametros son: el descriptor del socket y el numero maximo de conexiones en la cola de entrada

    return sockfd;
}

int exists_winner(int turn, char *board)
{
    //turn: es el nro del ultimo turno
    //board: el estado actual del tablero
    //last_to_play: el nro del ultimo jugador en hacer una jugada
    //Este metodo retorna 0 si hay empate, -1 si se debe seguir jugando o el nro del jugador ganador en que caso de que exista un ganador
    
    int winner = -1;
    
    //Si no pasaron 3 turnos no puede haber ganadores 
    if (turn < 3) 
    {
        //Revisamos todas las filas , columnas y diagonales
        if (turn >= 3 && 
        (board[0] != 0 && board[0] == board[1] && board[1] == board[2]) ||
        (board[3] != 0 && board[3] == board[4] && board[4] == board[5]) ||
        (board[8] != 0 && board[6] == board[7] && board[7] == board[8]) ||
        (board[0] != 0 && board[0] == board[3] && board[6] == board[2]) ||
        (board[1] != 0 && board[1] == board[4] && board[4] == board[7]) ||
        (board[8] != 0 && board[2] == board[5] && board[5] == board[8]) ||
        (board[6] != 0 && board[6] == board[4] && board[4] == board[2]) ||
        (board[8] != 0 && board[8] == board[4] && board[4] == board[1]))
        {
            // El ultimo en jugar es ganador
            winner = 1;
        }
        else
        {
            //Si luego del turno 9 no hay ganadores tenemos un empate
            if (turn >= 9) 
            {
                winner = 0;
            }
        }
    }
    return winner;
}

int update_board(int player, char* state, char* board)
{
    /*  Retorna 0 si la jugada es válida, sino retorna -1 y no escribe el board */

    int move = state[0]-48; /* atoi */

    if(move < 0 || move > 9 || board[move]!=0)
    {
        return -1;
    }

    board[move] = player;
    return 0;
}

void send_message(int to_sockfd, char* new_state, struct message_t* msg)
{
    memcpy(msg->state, new_state, MSG_STATE_SIZE);
    write(to_sockfd, msg, MSG_SIZE);
}

void recv_message(int from_sockfd, struct message_t* msg)
{
    read(from_sockfd, (void*)&msg, MSG_SIZE);
}

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    int sockfd = init(port);

    while (1)
    {
        int player[2];
        struct message_t msg = {"", "000000000"};

        printf("Esperando por jugadores...\n");

        player[0] = wait_for_player(sockfd);
        printf("Jugador %d conectado.\n", player[0]);
        send_message(player[0], YOU_ARE_P1, &msg);

        player[1] = wait_for_player(sockfd);
        printf("Jugador %d conectado.\n", player[1]);
        send_message(player[1], YOU_ARE_P2, &msg);

        printf("Empezando partida...\n");

        int winner = -1;
        int turn = 0;

        while (winner != -1)
        {
            int player_playing = player[turn % 2];
            int player_waiting = player[(turn + 1) % 2];

            send_message(player_waiting, WAIT_FOR_TURN, &msg);

            printf("Turno de Jugador %d...\n", player_playing);

            do {
                send_message(player_playing, IS_YOUR_TURN, &msg);
                recv_message(player_playing, &msg);
            }
            /* hasta que se recibe una jugada valida */
            while ( update_board(player_playing, msg.state, msg.board) == 0 );
            
            turn++;
            printf("Jugada recibida: %s\n", player_playing, msg.state);

           //Guardamos en winner si la partida sigue, es empate o alguien gano
            winner = exists_winner(turn, msg.board);

            switch(winner)
            {
                case 0: send_message(player_playing, GAME_IS_DRAW, &msg);
                        send_message(player_waiting, GAME_IS_DRAW, &msg);
                        printf("El juego es Empate!\n");
                        break;
                case 1: send_message(player_playing, YOU_WON, &msg);
                        send_message(player_waiting, YOU_LOST, &msg);
                        printf("Gana el Jugador %d!\n", player_playing);
                        break;
                default: 
                        break;
            }
        }
    }

    return 0;
}
