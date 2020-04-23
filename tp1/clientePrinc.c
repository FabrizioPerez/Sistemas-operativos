#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "vars.h"

int main(int argc, char *argv[])
{
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int chancesToLog = CHANCESTOLOG;
	int logOk = 0;

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

		printf("Bienvenido. Ingrese el usuario: ");
		memset(buffer, '\0', TAM);
		fgets(buffer, TAM - 1, stdin);
		buffer[strlen(buffer) - 1] = '\0';

		n = send(sockfd, buffer, strlen(buffer), 0); //envio usuario

		// Verificando si se escribió: fin

		buffer[strlen(buffer) - 1] = '\0';
		if (!strcmp("fin", buffer))
		{
			printf("ponemos terminar en 1 \n");
		}

		memset(buffer, '\0', TAM);
		n = recv(sockfd, buffer, TAM, 0); //recibo respuesta del authservice

		if (!strcmp("ingreseContra", buffer))
		{
			do
			{
				printf("Ingrese la contrasenia: ");
				memset(buffer, '\0', TAM);
				fgets(buffer, TAM - 1, stdin);
				buffer[strlen(buffer) - 1] = '\0';
				n = send(sockfd, buffer, strlen(buffer), 0);

				memset(buffer, '\0', TAM);
				n = recv(sockfd, buffer, TAM, 0);

				if (!strcmp("contraok", buffer))
				{
					printf("Logueo correcto. ingrese la operacion: \n");
					logOk = 1;
				}
				else if (!strcmp("ingreseContra", buffer))
				{
					printf("Contrasenia incorrecta. Reingrese, por favor \n");
				}
				chancesToLog = chancesToLog - 1;  
			} while (chancesToLog && (logOk == 0));
		}
		else
		{
			printf("el usuario no era valido: %s", buffer);
			printf("Finalizando ejecución\n");
			exit(0);
		}

		if ((chancesToLog == 0) && (logOk == 0))
		{

			printf("El usuario ha sido bloqueado por ingresar mal\n");
			printf("la contrasenia %d veces...\n", CHANCESTOLOG);
		}

		printf("Logueo correcto. ingrese la operacion: \n");
		memset(buffer, '\0', TAM);
		fgets(buffer, TAM - 1, stdin);
		buffer[strlen(buffer) - 1] = '\0';
		n = send(sockfd, buffer, strlen(buffer), 0);	//envio de la operacion

		//printf( "vamos a cerrar el socket...\n" );
		//fclose(sockfd);
	}
	return 0;
}
