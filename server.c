#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>

struct message_t
{
    char state[6];
    char board[10];
};

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
        error(1, 0, "ERROR on binding\n");

    listen(sockfd, 2); // El servidor hablilita su socket para poder recibir conexiones, llamando a la función listen(). Los parametros son: el descriptor del socket y el numero maximo de conexiones en la cola de entrada

    return sockfd;
}

int exists_winner(int turn, char *board[], int last_to_play)
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
            winner = last_to_play;
        }
        else
        {
            //Si luego del turno 9 no hay ganadores tenemos un empate
            if (turn == 9) 
            {
                winner = 0;
            }
        }
    }
    return winner;
}

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    int sockfd = init(port);

    while (1)
    {
        int player_sock[2];

        printf("Esperando por jugadores...\n");

        player_sock[0] = wait_for_player(sockfd);
        printf("Jugador 1 conectado\n");
        write(player_sock[0], "UR P1", 6);

        player_sock[1] = wait_for_player(sockfd);
        printf("Jugador 2 conectado\n");
        write(player_sock[1], "UR P2", 6);

        printf("Empezando partida...\n");

        struct message_t msg = {"", "000000000"};
        int winner = 0;
        int turn = 0;

        while (!winner)
        {
            int player_playing = turn % 2;
            int player_waiting = (turn + 1) % 2;
            printf("Turno de Jugador %d...\n", player_playing + 1);

            memcpy(msg.state, "PLAY", 6);
            write(player_sock[player_playing], &msg, sizeof(struct message_t));

            memcpy(msg.state, "WAIT", 6);
            write(player_sock[player_waiting], &msg, sizeof(struct message_t));

            read(player_sock[player_playing], &msg, sizeof(struct message_t));
            printf("Jugada de jugador %d: %s\n", player_playing + 1, msg.state);

            turn++;
            char *jugada_recibida = msg.state;

            /* el tablero esta en msg.board
                0 0 0
                0 0 0
                0 0 0
                un 1 representa la ficha del jugador 1
                un 2 representa la ficha del jugador 2
            */

            /*
            Acá falta la logica:
                si pasaron muchos turnos es empate
                si la jugada es inválida, pedir otra
                si alguien ganó, avisar
                etc...
            */

           //Guardamos en winner si la partida sigue, es empate o alguien gano
            winner = exists_winner(turn, *msg.board,player_playing);
        }
    }

    return 0;
}
