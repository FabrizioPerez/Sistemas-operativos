/*este es el cliente que va dentro del servidor principal.
Su funcionamiento es conectarse con el authService */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TAM 256

int validarComando(char *);

int main(int argc, char *argv[])
{
	int sockfd, puerto, n;
	struct sockaddr_in serv_addr2;
	struct hostent *server;
	int terminar = 0;
	int loginOk = 0;
	int retry = 3;
	int operacion = 0;

	char buffer[TAM];
	if (argc < 3)
	{
		fprintf(stderr, "Uso %s host puerto\n", argv[0]);
		exit(0);
	}

	puerto = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	server = gethostbyname(argv[1]);

	memset((char *)&serv_addr2, '0', sizeof(serv_addr2));
	serv_addr2.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr2.sin_addr.s_addr, server->h_length);
	serv_addr2.sin_port = htons(puerto);

	if (connect(sockfd, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0)
	{
		perror("conexion");
		exit(1);
	}

	while (1)
	{

		fflush(stdin);

		printf("Bienvenido, nombre de usuario para enviar: ");
		memset(buffer, '\0', TAM);
		fgets(buffer, TAM - 1, stdin);
		buffer[strlen(buffer) - 1] = '\0';		//quito el \n del ingreso
		n = send(sockfd, buffer, strlen(buffer), 0);

		// Verificando si se escribió: fin
		if (!strcmp("fin", buffer))
		{
			terminar = 1;
			printf("ponemos terminar en 1 \n");
		}

		/*ya hemos enviado el usuario acá. esperamos respuesta de servidor*/

		memset(buffer, '\0', TAM);
		n = recv(sockfd, buffer, TAM, 0);

												
		if (!strcmp("ingreseContra", buffer))
		{
			printf("%s\n", buffer);
			memset(buffer, '\0', TAM);
			fgets(buffer, TAM - 1, stdin);
			buffer[strlen(buffer) - 1] = '\0';
			n = send(sockfd, buffer, strlen(buffer), 0);	//envio de contrasenia
		}
		else
		{

			printf("lo que llego fue: %s \n", buffer);
		}

		/*aca ya se envio el usuario y contraseña
		 se ingresa a al loop que envia y recibe 3 veces, en caso de ser incorrecta la contrasenia*/

		while ((loginOk == 0) && (retry != 0))
		{
			memset(buffer, '\0', TAM);
			n = recv(sockfd, buffer, TAM, 0);
			if (!strcmp("contraok", buffer))
			{
				do
				{
					//poner en una funcion (prompt + fgets )así lo uso de nuevo:
					printf("contrasenia ok. Ingrese operacion: \n"); /*muestro prompt y espero insgreso de datos*/
					printf("1- Exit (cerrar conexion)  \n");
					printf("2- User ls (lista de usuarios)  \n");
					printf("3- New_pass <newpass> (cambiar contraseña de usuario actual)  \n");
					printf("4- File ls (listado de imagenes disponibles)  \n");
					printf("5- File_down <image_name> [...opt1...][...opt2...](listado de imagenes disponibles)  \n");
					memset(buffer, '\0', TAM);
					fgets(buffer, TAM - 1, stdin);
					printf("lo que tengo en buffer (bf valida): %s\n", buffer);
					operacion = validarComando(buffer); //que se fije si el comando es valido. Si es valido, retornar el numero de la operacion
					//este numero va a ser el primer valor que envíe como codigo de operacion
					if (operacion > 0)
					{
						printf("la operacion a ejecutar es: %d\n", operacion);
						//concatenar con código de operación
						printf("lo que tengo en buffer (af valida): %s\n", buffer);
						n = send(sockfd, buffer, strlen(buffer), 0);
						loginOk = 1; //para salir del loop general, que no me vuelva a pedir pass;
					}
				} while (operacion <= 0);
			}
			else
			{
				printf("Contrasenia incorrecta, reingrese: \n"); 
				memset(buffer, '\0', TAM);
				fgets(buffer, TAM - 1, stdin);
				n = send(sockfd, buffer, strlen(buffer), 0);
				retry = retry - 1;
				//ver verificacion de salida y cierre de programa
			}
		}

		memset(buffer, '\0', TAM);
		n = recv(sockfd, buffer, TAM, 0);
		if (!strcmp("cerrando conexion", buffer))
		{
			printf("Cerrando conexion. Salu2... \n"); //estos prints deberian ser desde server,supongo
			memset(buffer, '\0', TAM);
			exit(0);
		}
		else
		{
			printf("esperaba recibir un comando, se recibio:\n%s", buffer);
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
	int minTamPass = 15;

	if (!(strcmp(aux, "Exit\n")) || !(strcmp(aux, "exit\n")))
	{
		printf("el comando es exit...\n");
		return 1;
	}
	else if (!(strcmp(aux, "User ls\n")) || !(strcmp(aux, "user ls\n")))
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
			char *ptr = strtok(comando, delim); //primera iteracion para split
			strcpy(aux2, ptr);					//aux2 se queda con el nombre del comando
			ptr = strtok(NULL, delim);			//segunta iteracion. obtendría la nueva contraseña
			strcpy(aux3, ptr);					//aux 3 se queda con la contrasenia
			strcat(comando, " ");				//espacio para imitar el comando inicial
			strcat(comando, aux3);				//le vuelvo a agregar ubuntu al final

			return 3;
		}
	}
	else if (!(strcmp(aux, "File_ls\n")) || !(strcmp(aux, "file_ls\n")))
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

		//printf("aux 2 tiene: %s\n", aux2);
		return 5;
	}
	else
	{
		return 0;
	}
}