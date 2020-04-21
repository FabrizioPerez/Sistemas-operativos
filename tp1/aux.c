#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

int main()
{
	char str[] = "fabry fabry123;coco coco123;jenny jenny123;delia delia456"; //de archivo
	int init_size = strlen(str);
	char delim[] = " ;";
    char *splitAux;
    char *userEncontrado;
    char *passEncontrada;
    char usuario[] = "jenny";  //serian los que entran por socket
    char contra[] = "jenny123";
    int i = 0;    
    int salida = 0;
    int chancesToLog = 3;


	char *ptr = strtok(str, delim);
  
   
	while ((ptr != NULL) && (salida == 0))
	{
       
		printf("%d: '%s'\n",i,  ptr);
		splitAux = ptr;
        
        ptr = strtok(NULL, delim);

        if ((!strcmp(usuario, splitAux)) && ((i%2) == 0)){ /*lo ultimo checkea que sea par, es decir, que sea un user y no una pass.*/
            userEncontrado = splitAux;
            passEncontrada = ptr;
            salida = 1;
        }
      
       i = i+1;
	}
    
       if ((strcmp(contra, passEncontrada)== 0) && (chancesToLog != 0)){
            printf("coincide su contraseña. Acceso ok para: %s", userEncontrado);
        }else{
            printf("contraseña incorrecta. Acceso incorrecto para: %s", userEncontrado);
            chancesToLog = chancesToLog -1;
        }
        

    
	return 0;
}