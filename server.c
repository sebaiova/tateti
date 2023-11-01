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

    while((player_sock = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)) < 0)
        close(player_sock);  

    return player_sock;
}

int init(int portno)
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serv_addr;
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR para que el sistema operativo asigne al servidor el ip local
    serv_addr.sin_port = htons(portno);
     
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error(1, 0, "ERROR on binding\n");

    listen(sockfd, 2); //El servidor hablilita su socket para poder recibir conexiones, llamando a la función listen(). Los parametros son: el descriptor del socket y el numero maximo de conexiones en la cola de entrada
    
    return sockfd;
}

int main(int argc, char *argv[])
{
    int port = atoi(argv[1]);
    int sockfd = init(port);

    while(1)
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

        struct message_t msg = { "", "000000000" };
        int winner = 0;
        int turn = 0; 

        while(!winner)
        {
            int player_playing = turn%2;
            int player_waiting = (turn+1)%2;
            printf("Turno de Jugador %d...\n", player_playing+1);
        
            memcpy(msg.state, "PLAY", 6);
            write(player_sock[player_playing], &msg, sizeof(struct message_t));
        
            memcpy(msg.state, "WAIT", 6);
            write(player_sock[player_waiting], &msg, sizeof(struct message_t));

            read(player_sock[player_playing], &msg, sizeof(struct message_t));
            printf("Jugada de jugador %d: %s\n", player_playing+1, msg.state);
        
            turn++;
            char* jugada_recibida = msg.state;

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
        }
    }

    return 0;
}

