#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <error.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[])
{
    int n;
    char buffer[1024];

   // if (argc < 3)
   //    error(1, 0, "Ingrese nombre del servidor y el numero de puerto");

    //int portno = atoi(argv[2]);
    int portno = 1040;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //struct hostent *server = gethostbyname(argv[1]);
    struct hostent *server = gethostbyname("155.10.2.1"); 

    if (server == NULL)
        error(1, 0, "Servidor %s no encontrado.", argv[1]);

/* Rellenamos con valores el socket del cliente */
    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr_list[0], (char*)&serv_addr.sin_addr.s_addr, server->h_length); //ojo aqui puede ser server.sin_addr
    serv_addr.sin_port = htons(portno);
           
    printf("Connecting to %s:%d...\n", server->h_name, portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) //Conectamos con el servidor
        error(1, 0, "No se puede conectar.");
    printf("Conexión establecida.");

    printf("Ingrese su mensaje: ");
    bzero(buffer,1024);
    fgets(buffer,1024,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
         error(0, 0, "Conexion no aceptada\n");
    bzero(buffer,1024);
    n = read(sockfd,buffer,1024);
    if (n < 0) 
         error(0, 0, "ERROR de lectura del socket\n");
    printf("%s\n",buffer);
    close(sockfd);
    return 0;
}
