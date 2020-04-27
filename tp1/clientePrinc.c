#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#include "vars.h"

void prompt();
int validarComando(char *);

int main(int argc, char *argv[])
{
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	int chancesToLog = CHANCESTOLOG;
	int operacion = 0;
	int fin = 0;
	time_t t = time(NULL);

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

		struct tm tm = *localtime(&t);
		printf("hoy es: %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		
		
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
					loginOk = 1;
				}
				else if (!strcmp("ingreseContra", buffer))
				{
					printf("Contrasenia incorrecta. Reingrese, por favor \n");
				}
				chancesToLog = chancesToLog - 1;
			} while (chancesToLog && (loginOk == 0));
		}
		else
		{
			printf("el usuario no era valido: %s", buffer);
			printf("Finalizando ejecución\n");
			exit(0);
		}

		if ((chancesToLog == 0) && (loginOk == 0))
		{
			printf("El usuario ha sido bloqueado por ingresar mal\n");
			printf("la contrasenia %d veces...\n", CHANCESTOLOG);
		}

		if (loginOk)
		{
			do
			{
				//poner en una funcion (prompt + fgets )así lo uso de nuevo:
				prompt(); //muestro prompt y espero ingreso de datos//
				memset(buffer, '\0', TAM);
				fgets(buffer, TAM - 1, stdin);
				buffer[strlen(buffer) - 1] = '\0';
				operacion = validarComando(buffer); //Verifica si es un comando de operacion valido
				if (operacion > 0)
				{
					//concatenar con código de operación
					printf("lo que tengo en buffer (post validacion): %s\n", buffer);
					n = send(sockfd, buffer, strlen(buffer), 0);
					memset(buffer, '\0', TAM);
					n = recv(sockfd, buffer, TAM, 0);
					printf("respuesta de serv:\n %s\n", buffer);
					if (operacion == 0)
					{
						fin = 1;
					}
				}
			} while (!fin);
		}

		//printf( "vamos a cerrar el socket...\n" );
		//fclose(sockfd);
	}
	return 0;
}

void prompt()
{

	printf("Login valido. Ingrese operacion: \n"); //muestro prompt y espero ingreso de datos//
	printf("1- Exit (cerrar conexion)  \n");
	printf("2- User_ls (lista de usuarios)  \n");
	printf("3- New_pass <newpass> (cambiar contraseña de usuario actual)  \n");
	printf("4- File ls (listado de imagenes disponibles)  \n");
	printf("5- File_down <image_name> [...opt1...][...opt2...](listado de imagenes disponibles)  \n");
}

int validarComando(char *comando)
{
	/*la funcion recibe un comando y verifica que sea valido
	  En caso de ser valido, devuelve el numero de operacion
	  que se debe enviar antecediendo el comando*/

	char delim[] = " ";
	char *aux = (char *)malloc(TAM);
	char *aux2 = (char *)malloc(TAM);
	char *aux3 = (char *)malloc(TAM);
	aux = comando;
	int minTamPass = MINTAMANOPASS;

	if (!(strcmp(aux, "Exit")) || !(strcmp(aux, "exit")))
	{
		printf("el comando es exit...\n");
		return 1;
	}
	else if (!(strcmp(aux, "User_ls")) || !(strcmp(aux, "user_ls")))
	{
		printf("el comando es user ls...\n");
		return 2;
	}
	else if (!(strncmp(comando, "New_pass", strlen("New_pass"))) || !(strncmp(comando, "new_pass", strlen("new_pass"))))
	{
		printf("el comando es new passs...\n");
		if (strlen(comando) < minTamPass)
		{
			printf("la contrasenia debe contener al menos 7 caracteres..\n");
			return -3;
		}
		else
		{
			char *ptr = strtok(comando, delim); //primera iteracion para split. obtendria nombre de comando
			strcpy(aux2, ptr);					//aux2 se queda con el nombre del comando
			ptr = strtok(NULL, delim);			//segunta iteracion. obtendría la nueva contraseña
			strcpy(aux3, ptr);					//aux 3 se queda con la contrasenia
			if (strlen(aux3) < minTamPass)
			{
				printf("La contrasenia debe contener al menos 7 caracteres..\n");
				printf("\n");
				return -3;
			}
			strcat(comando, " ");  //espacio para imitar el formato de comando inicial
			strcat(comando, aux3); //se agrega nuevamente pass al final

			return 3;
		}
	}
	else if (!(strcmp(aux, "File_ls")) || !(strcmp(aux, "file_ls")))
	{
		printf("el comando es file ls...\n");
		return 4;
	}
	else if (!(strncmp(comando, "File_down", strlen("File_down"))) || !(strncmp(comando, "file_down", strlen("file_down"))))
	{
		char *ptr = strtok(aux, delim); //primera iteracion para split (obtengo el comando)
		strcpy(aux2, ptr);				//aux2 se queda con el nombre del comando
		ptr = strtok(NULL, delim);		//segunta iteracion. obtendría la imagen a bajar
		strcpy(aux3, ptr);				//aux 3 se queda con el nombre de la imagen
		strcat(comando, " ");			//espacio para imitar el comando inicial
		strcat(comando, aux3);			//le vuelvo a agregar ubuntu al final
		printf("ahora comando es: %s de tamanio %ld\n", comando, strlen(comando));
		return 5;
	}
	else
	{
		return 0;
	}
}