#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "vars.h"

int validarComando(char *);

int main(int argc, char *argv[])
{

  int servFd, cliFd, newservFd, var, n;
  struct sockaddr_in *serv_addr, *cli_addr; //correspondientes a la conexion clientePrinc-servPrinc
  socklen_t tamCli;
  ssize_t tamMjeRecibido;
  socklen_t tamano;
  char mjeRecibido[TAM];
  char mjeRecibido2[TAM];
  int auxint = 2;
  int tamBuffer;
  char fin[] = "fin";

  ///////////

  int sockfd, puerto;
  struct sockaddr_in serv_addr2;
  struct hostent *server;
  int terminar = 0;
  int retry = 3;
  int operacion = 0;
  char buffer[TAM];
  int chancesToLog = CHANCESTOLOG + 1;
  int conectado = 0;

  ////////

  servFd = socket(AF_INET, SOCK_STREAM, 0); //definicion del socket;

  serv_addr = calloc(1, sizeof(struct sockaddr_in)); //alloca e inicialiaza un lugar de memoria de ese tamaño. Devuelve puntero.
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_port = htons((uint16_t)atoi(argv[1]));
  //serv_addr->sin_port = htons(6666);
  serv_addr->sin_addr.s_addr = INADDR_ANY; //esta puesto en 0000. creo que para poner una tengo que usar htonl(argv(1)) y cambiar el puerto al argv(2)
  //tambien, para probar todo junto puedo probar con el loopback 127.0.0.1
  var = bind(servFd, (struct sockaddr *)serv_addr, (socklen_t)sizeof(struct sockaddr));

  if (var < 0)
  {
    perror("no se bindeo bien: ");
    exit(-1);
  }

  printf("Proceso: %d - socket disponible en puerto: %d\n", getpid(), ntohs(serv_addr->sin_port));
  listen(servFd, 1);

  //preparo para aceptar conexiones
  cli_addr = calloc(1, sizeof(struct sockaddr_in));
  tamCli = sizeof(struct sockaddr_in);

  while (1)
  {

    fflush(stdout);
    fflush(stdin);
    memset(&mjeRecibido, 0, sizeof(mjeRecibido));

    newservFd = accept(servFd, (struct sockaddr *)&cli_addr, &tamCli);

    //checkear errores de accept. Retorna -1 en posible error.
    tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);
    //se supone que llego un usuario

    printf("---------- \n");
    printf("ServPrinc log: Usuario recibido: %s \n", mjeRecibido);

    //se conectó un cliente. Llego un nombre de user.
    //se levanta el authService y se intenta comprobar el usuario
    pid_t chpid = fork();

    if (chpid == -1)
    {
      perror("Creating child process");
    }
    else if (chpid == 0)
    {
      char *argv[] = {"./_servAuth", "5565", NULL};
      char *envp[] = {NULL};
      if (execv("./_servAuth", argv) == -1)
      {
        perror("Could not execve");
      }
    }
    else if (chpid)
    {
      printf("serviPrinc log: proceso padre que ejecuta el princservice: %d \n", chpid);
      printf("serviPrinc log: proceso padre de id: %d \n", getpid());

      puerto = atoi("5565");
      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      server = gethostbyname("localhost");

      memset((char *)&serv_addr2, '0', sizeof(serv_addr2));
      serv_addr2.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&serv_addr2.sin_addr.s_addr, server->h_length);
      serv_addr2.sin_port = htons(puerto);

      do
      {
        if (connect(sockfd, (struct sockaddr *)&serv_addr2, sizeof(serv_addr2)) < 0)
        {
          //intenta la conexion hasta que la logra. Con esto se evita el
          //conexion refused. Intenta hasta que el otro socket esté listo.
          //perror("serviPrinc log: conexion");
        }
        else
        {
          printf("serviPrinc log: se logro la conexion con authservice  \n");
          conectado = 1;
        }

      } while (conectado == 0);

      //si la creacion fue exitosa, le mando el user recibido al auth service
      n = send(sockfd, mjeRecibido, strlen(mjeRecibido), 0);
      memset(mjeRecibido, '\0', TAM);

      n = recv(sockfd, mjeRecibido, TAM, 0);
      if (!strcmp("ingreseContra", mjeRecibido))
      {
        do
        {
          //en este caso, el auth service confirma que el usuario es valido
          //y solicita su contrasenia.
          printf("ServiPrinc log: user ok. solicitando al cliente la pass");

          n = send(newservFd, mjeRecibido, strlen(mjeRecibido), 0); //envio al cliente el pedido de password;
          memset(mjeRecibido, '\0', TAM);
          tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0); //recibo la password desde el cliente

          n = send(sockfd, mjeRecibido, strlen(mjeRecibido), 0); //envio de contrasenia al authservice
          memset(mjeRecibido, '\0', TAM);
          n = recv(sockfd, mjeRecibido, TAM, 0); //respuesta del authservice

          if (!strcmp("contraok", mjeRecibido))
          {
            //el authservice avisa que la contraseña es correcta.
            //cierro el ciclo de autenticacion y espero instrucciones
            loginOk = 1;
            n = send(newservFd, mjeRecibido, strlen(mjeRecibido), 0); //envio al cliente la respuesta del authser
          }
          chancesToLog = chancesToLog - 1;
        } while ((loginOk == 0) && (chancesToLog));
      }
      else
      {
        printf("lo que llego fue: %s \n", buffer);
      }

      //bloque que recibe instruccion del cliente, la pasa authservice
      //y a su respuesta la envía al cliente.
      memset(mjeRecibido, '\0', TAM);
      tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0); //esperando instruccion del cliente.
      n = send(sockfd, mjeRecibido, strlen(mjeRecibido), 0);         //envio instr al authservice
      memset(mjeRecibido, '\0', TAM);
      n = recv(sockfd, mjeRecibido, TAM, 0); //respuesta del authservice
      n = send(newservFd, mjeRecibido, strlen(mjeRecibido), 0); //envio al cliente la respuesta del authser
    







    }

    /*
    if (strncmp(mjeRecibido, usuario1, tamMjeRecibido) == 0)
    {
      printf("usuario valido. \n");
      n = send(newservFd, mjeconsola, strlen(mjeconsola), 0);
    }
    else
    {
      printf("usuario NO valido, reingrese... \n");
    }

    printf("limpiando buffer...\n");
    printf("\n");
    memset(&mjeRecibido, 0, sizeof(mjeRecibido));
    printf("tamBuffer es: %d \n", strlen(mjeRecibido));
    printf("buffer: %s \n", mjeRecibido);
    printf("---------- \n");
  }
*/

    return 0;
  }
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
      strcpy(aux2, ptr);                  //aux2 se queda con el nombre del comando
      ptr = strtok(NULL, delim);          //segunta iteracion. obtendría la nueva contraseña
      strcpy(aux3, ptr);                  //aux 3 se queda con la contrasenia
      strcat(comando, " ");               //espacio para imitar el comando inicial
      strcat(comando, aux3);              //le vuelvo a agregar ubuntu al final

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
    strcpy(aux2, ptr);              //aux2 se queda con el nombre del comando
    ptr = strtok(NULL, delim);      //segunta iteracion. obtendría la imagen a bajar
    strcpy(aux3, ptr);              //aux 3 se queda con el nombre de la imagen
    strcat(comando, " ");           //espacio para imitar el comando inicial
    strcat(comando, aux3);          //le vuelvo a agregar ubuntu al final
    printf("ahora comando es: %s de tamanio %ld\n", comando, strlen(comando));

    //printf("aux 2 tiene: %s\n", aux2);
    return 5;
  }
  else
  {
    return 0;
  }
}

