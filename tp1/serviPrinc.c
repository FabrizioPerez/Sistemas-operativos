#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h> 
#include <errno.h> 


#define TAM 256

int main( int argc, char *argv[] ) {

  int servFd, cliFd, newservFd, var, n;
  struct sockaddr_in* serv_addr, *cli_addr;
  socklen_t tamCli;
  ssize_t tamMjeRecibido ;
  socklen_t tamano; 
  char mjeRecibido [500];
  char mjeRecibido2 [500];
  int auxint = 2;
  int tamBuffer;
  char usuario1[] = "fabry 2\n";
  char mjeconsola[] = "fabrizio perez, tenes que poner\n";
  char usuario2[] = "fabrizio perez\n";
  char fin[] = "fin";

  servFd= socket(AF_INET, SOCK_STREAM, 0); //definicion del socket;

  serv_addr = calloc(1, sizeof(struct sockaddr_in)); //alloca e inicialiaza un lugar de memoria de ese tamaño. Devuelve puntero.
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_port = htons((uint16_t)atoi(argv[1]));
  //serv_addr->sin_port = htons(6666);
  serv_addr->sin_addr.s_addr = INADDR_ANY; //esta puesto en 0000. creo que para poner una tengo que usar htonl(argv(1)) y cambiar el puerto al argv(2)
  //tambien, para probar todo junto puedo probar con el loopback 127.0.0.1
  var = bind(servFd, (struct sockaddr *) serv_addr, (socklen_t) sizeof(struct sockaddr) );
  
  if (var < 0){
    perror( "no se bindeo bien: " );
    exit(-1);
  }

  printf( "Proceso: %d - socket disponible en puerto: %d\n", getpid(), ntohs(serv_addr->sin_port) );
  listen( servFd, 1 );

  //preparo para aceptar conexiones 
  cli_addr = calloc(1, sizeof(struct sockaddr_in)); 
  tamCli = sizeof(struct sockaddr_in); 
  
 // newservFd = accept(servFd, (struct sockaddr *) &cli_addr, &tamCli);

  while(1){

 
  fflush(stdout);
  fflush(stdin);
  memset(&mjeRecibido, 0, sizeof(mjeRecibido));   

  newservFd = accept(servFd, (struct sockaddr *) &cli_addr, &tamCli);
  //checkear errores de accept. Retorna -1 en posible error. 
  tamMjeRecibido =  recv(newservFd, mjeRecibido, (size_t) 200, 0);
  //read(newservFd, mjeRecibido, 500);

  printf("---------- \n");

  printf("tamBuffer es: %d \n",strlen(mjeRecibido));
  printf("tam mje recibido: %d \n", tamMjeRecibido);
  printf("mensaje: %s \n", mjeRecibido); 
 
     strncpy(mjeRecibido2, mjeRecibido, tamMjeRecibido );
      if (strncmp(mjeRecibido, usuario1, tamMjeRecibido) == 0){
        printf("usuario valido. \n");
        n = send(newservFd, mjeconsola, strlen(mjeconsola), 0);

      //  n = write( newservFd, usuario2, strlen(usuario2) );
      }else if (strncmp(mjeRecibido, usuario2, tamMjeRecibido) == 0){
        printf("usuario valido. \n");
        n = send(newservFd, usuario2, strlen(usuario2), 0);
      }else{
        printf("usuario NO valido, reingrese... \n");
      }

  printf("limpiando buffer...\n");
  printf("\n");
  memset(&mjeRecibido, 0, sizeof(mjeRecibido));   
  printf("tamBuffer es: %d \n",strlen(mjeRecibido));
  printf("buffer: %s \n", mjeRecibido);
  printf("---------- \n");

    
 
/*
http://es.tldp.org/Tutoriales/PROG-SOCKETS/prog-sockets.html
http://man7.org/linux/man-pages/man2/execve.2.html
https://es.wikipedia.org/wiki/Execve



*/




  }




  return 0;
}
