#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <error.h>

int main(int argc, char *argv[])
{
    int rval;
    char bufer[1024];

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    int portno = atoi(argv[1]);

    struct sockaddr_in serv_addr;
    bzero((char*) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; // INADDR para que el sistema operativo asigne al servidor el ip local
    serv_addr.sin_port = htons(portno);
     
    if(bind(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error(1, 0, "ERROR on binding\n");

    listen(sockfd, 1); //El servidor hablilita su socket para poder recibir conexiones, llamando a la función listen(). Los parametros son: el descriptor del socket y el numero maximo de conexiones en la cola de entrada
    printf("Escuchando...\n");
    
    struct sockaddr_in cli_addr;
    int clilen = sizeof(cli_addr); //Reservamos un espacio de direcciones para que el cliente se pueda contactar con el servidor

    while (1)
    {
        int newsockfd = accept(sockfd, (struct sockaddr *)	&cli_addr, &clilen); // la funcion aceppt() habilita al servidor, para que un cliente se pueda contactar con el. Los parametros son: el descriptor del servidor. El puntero a una estructura sockadd_in (Aquí se almacenará informacion de la conexión entrante. Se utiliza para determinar que host está llamando y desde qué número de puerto); por ultimo la estructura de dato -clilen- en donde se depositara para futura operaciones 
        if(newsockfd < 0) 
            perror("Conexion no aceptada\n"); //Aviso sobre posible error de conexion
        else do 
        {
		    memset(bufer, 0, sizeof(bufer));
         	rval=read(newsockfd,bufer,1024);
		if (rval<0) perror("Mensaje no leido\n");
		else{printf("%s",bufer); 
			write(newsockfd,"Recibi tu mensaje",17);
	  }
         } while (rval>0);
       printf("\nCerrando la conexion\n");
       close(newsockfd);  
     } /* end of while */
     
     return 0; /* we never get here */
}

