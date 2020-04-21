#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h> 
#include <errno.h> 
#include <sys/un.h> 


#define TAM 80

int main(int argc, char *argv[]) {
  int sockfd, newsockfd, servlen, clilen, n, buf, pid;
  struct sockaddr_un  cli_addr, serv_addr;
  char buffer[TAM];
  const char *SOCKNAME = "/tmp/sockete";

  
  /* Se toma el nombre del socket de la línea de comandos */
  if( argc != 2 ) {
    printf( "Uso: %s <nombre_de_socket>\n", argv[0] );
    exit( 1 );
  }
  
  if ( ( sockfd = socket( AF_UNIX, SOCK_STREAM, 0) ) < 0 ) {
    perror( "creación de  socket");
    exit(1);
  }
  
  /* Remover el nombre de archivo si existe */
  unlink(SOCKNAME);
  
  memset( &serv_addr, 0, sizeof(serv_addr) );
  serv_addr.sun_family = AF_UNIX;
  strncpy(serv_addr.sun_path, SOCKNAME, sizeof(serv_addr.sun_path) - 1);
  servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
  
 if( bind( sockfd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr_un))<0 ) {
    perror( "no se biendeo bien sUn" ); 
    exit(1);
  }
  
  printf( "Proceso: %d - socket disponible: %s\n", getpid(), serv_addr.sun_path );
  
  listen( sockfd, 1 );
  clilen = sizeof( cli_addr );
  
  while ( 1 )
    {
      newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
      
      
      pid = fork();
      
      if ( pid == 0 )
	{        // proceso hijo
	  close( sockfd );
	  
	  while(1) { 
	    memset( buffer, 0, TAM );
	    
	    n = read( newsockfd, buffer, TAM-1 );
	    
	    printf( "PROCESO: %d. ", getpid() );
	    printf( "Recibí: %s", buffer );
	    
	    n = write( newsockfd, "Obtuve su mensaje", 18 );
	    // Verificación de si hay que terminar
	    buffer[strlen(buffer)-1] = '\0';
	    if( !strcmp( "fin", buffer ) ) {
	      printf( "PROCESO %d. Como recibí 'fin', termino la ejecución.\n\n", getpid() );
	      exit(0);
	    }
	  }
	}
      else {
	printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
	close( newsockfd );
      }
    }
  return 0;
}
