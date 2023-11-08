#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#include "tateti.h"

char symbols[3] = {' ', 'x', 'o'};
char* keys = "012345678";

int player_number = 0;
int sock;

void print_board(char* board)
{
    for(int i=0; i<3; i++)
    {
        int x = (i*3);
        int y = x+1;
        int z = y+1;
        printf("%c|%c|%c\t%c|%c|%c\n", symbols[board[x]-48], symbols[board[y]-48], symbols[board[z]-48], keys[x], keys[y], keys[z]);
    }
}

/* conecta con el servidor y retorna su socket */
int init(int port, char* server_name)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname(server_name);

    if(server == NULL)
        error(1, 0, "Servidor %s no encontrado.", server_name);

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char*)&serv_addr.sin_addr.s_addr, server->h_length); //ojo aqui puede ser server.sin_addr
    serv_addr.sin_port = htons(port);
           
    printf("Connecting to %s:%d...\n", server->h_name, port);
    if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Conectamos con el servidor
        error(1, 0, "No se puede conectar.");

    return sock;
}

void print_screen(char* board)
{
    system("clear");
    printf("\nTATETI\n\n");
    printf("Jugando como %c\n\n", symbols[player_number]);    
    print_board(board);
    printf("\n");
}

int main(int argc, char *argv[])
{
    if(argc < 3)
        error(1, 0, "Ingrese nombre del servidor y el numero de puerto");

    sock = init(atoi(argv[2]), argv[1]);
    struct message_t msg = {"", "000000000"};

    recv_message(sock, &msg);

    if (message_state_is(&msg, YOU_ARE_P1))
        player_number = 1;
    else if (message_state_is(&msg, YOU_ARE_P2))
        player_number = 2;
    else 
        error(1, 0, "Unexpected server error.");

    printf("Jugando como Jugador %d. (%c)\n", player_number, symbols[player_number]);

    int winner=0;

    while(!winner)
    {
        recv_message(sock, &msg);
        print_screen(msg.board);

        if (message_state_is(&msg, YOU_WON))
        {
            printf("Ganaste!\n");
            winner=1;
        }

        else if (message_state_is(&msg, YOU_LOST))
        {
            printf("Perdiste!\n");
            winner=2;
        }

        else if (message_state_is(&msg, GAME_IS_DRAW))
        {
            printf("Empataste!\n");
            winner = 3;
        }

        else if (message_state_is(&msg, IS_YOUR_TURN))
        {
            printf("Ingrese la jugada: ");
            fgets(msg.state, 6, stdin);
            write(sock, &msg, MSG_SIZE);
        }

        else if (message_state_is(&msg, WAIT_FOR_TURN))
        {
            printf("Turno del otro jugador.\n");
        }

        else 
        {
            error(1, 0, "Unexpected server error.");
        }
    }

    printf("God bye!\n");
    close(sock);

    return 0;
}
