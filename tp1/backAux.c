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
    char usuario[] = "delia";  //serian los que entran por socket
    char contra[] = "delia456";
     int i = 0;    
     int salida = 0;;


	char *ptr = strtok(str, delim);
  
   
	while ((ptr != NULL) && (salida == 0))
	{
       
		printf("%d: '%s'\n",i,  ptr);
		splitAux = ptr;
        
        ptr = strtok(NULL, delim);

        printf("palabra tiene: %s\n", splitAux);
        if (!strcmp(usuario, splitAux)){

            userEncontrado = splitAux;
            printf("encontramos el usuario %s\n", userEncontrado);
            printf("yendo a comparar si su pwd es:%s\n", ptr);
            passEncontrada = ptr;
            printf("ahora su passs es:%s\n", passEncontrada);

            salida = 1;
        }
      
       i = i+1;
	}

       if (strcmp(contra, passEncontrada)== 0){
            printf("coincide su contraseña. Acceso ok para: %s", userEncontrado);
        }else{
            printf("contraseña incorrecta. Acceso incorrecto para: %s", userEncontrado);
        }
        

    
	return 0;
}