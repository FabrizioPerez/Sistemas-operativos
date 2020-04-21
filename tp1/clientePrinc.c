#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TAM 256


int main(int argc, char *argv[])
{
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int terminar = 0;

	char buffer[TAM];
	if (argc < 3)
	{
		fprintf(stderr, "Uso %s host puerto\n", argv[0]);
		exit(0);
	}

	puerto = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(argv[1]);

	memset((char *)&serv_addr, '0', sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(puerto);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		perror("conexion");
		exit(1);
	}

	while (1)
	{
		
		fflush(stdin);

		printf("Ingrese el mensaje a transmitir: ");
		memset(buffer, '\0', TAM);
		fgets(buffer, TAM - 1, stdin);

//		n = write(sockfd, buffer, strlen(buffer));
	    n = send(sockfd, buffer, strlen(buffer), 0);


		// Verificando si se escribió: fin
		printf("el tamaño del buffer es: %d \n", strlen(buffer));
	
		buffer[strlen(buffer) - 1] = '\0';
		if (!strcmp("fin", buffer))
		{
			terminar = 1;
			printf("ponemos terminar en 1 \n");
		}

		memset(buffer, '\0', TAM);
		  
		n =  recv(sockfd, buffer, TAM, 0);

	//	n = read(sockfd, buffer, TAM);

		buffer[strlen(buffer) - 1] = '\0';
		if (!strcmp("fabrizio perez", buffer))
		{
			terminar = 1;
			printf("var terminar en 1 \n");
		}

		printf("Respuesta: %s\n", buffer);
		if (terminar)
		{
			printf("Finalizando ejecución\n");
			exit(0);
		}
		//printf( "vamos a cerrar el socket...\n" );
		//fclose(sockfd);
	}
	return 0;
}
