#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

struct message_t 
{
    char state[6];
    char board[10];
};

int sv_sock;
int player_number;
char player_symbol[3] = {' ', 'x', 'o'};

struct message_t recv_message()
{
    struct message_t buffer = {"", ""};
    read(sv_sock, (void*)&buffer, sizeof(struct message_t));
    return buffer;
}

void take_side()
{
    struct message_t message = recv_message();
    if(strcmp(message.state, "UR P1")==0)
        player_number = 1;
    else if(strcmp(message.state, "UR P2")==0)
        player_number = 2;
    else 
        error(1, 0, "Unexpected server error.");
}

void print_row(char* board)
{
    printf("%c|%c|%c\n", player_symbol[board[0]-48], player_symbol[board[1]-48], player_symbol[board[2]-48]);
}

void print_board(char* board)
{
    for(int i=0; i<3; i++)
        print_row(board+(i*3));
}

/* retorna al jugador ganador*/
int play()
{
    int winner=0;
    while(!winner)
    {
        struct message_t message = recv_message();
        print_board(message.board);

        if(strcmp(message.state, "PLAY")==0)
        {
            printf("Ingrese la jugada: ");
            fgets(message.state, 6, stdin);
            printf("%s%s\n", message.state, message.board);
            write(sv_sock, &message, sizeof(struct message_t));
        }
        else if(strcmp(message.state, "WAIT")==0)
        {
            printf("Turno del otro jugador.\n");
        }
        else 
        {
            error(1, 0, "Unexpected server error.");
        }
    }
}

int main(int argc, char *argv[])
{
    char buffer[1024];

    if(argc < 3)
        error(1, 0, "Ingrese nombre del servidor y el numero de puerto");

    int portno = atoi(argv[2]);
    sv_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct hostent *server = gethostbyname(argv[1]);

    if(server == NULL)
        error(1, 0, "Servidor %s no encontrado.", argv[1]);

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char*)&serv_addr.sin_addr.s_addr, server->h_length); //ojo aqui puede ser server.sin_addr
    serv_addr.sin_port = htons(portno);
           
    printf("Connecting to %s:%d...\n", server->h_name, portno);
    if (connect(sv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Conectamos con el servidor
        error(1, 0, "No se puede conectar.");

    printf("\nTATETI\n");
    
    take_side();

    printf("Jugando como Jugador %d. (%c)\n", player_number, player_symbol[player_number]);

    play();

    close(sv_sock);
    return 0;
}
