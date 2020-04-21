
/*creacion y prueba del metodo que cambia contrasenias*/


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

int reemplaPass(char *, char *, char *);

int main()
{
    char nuevapass[] = "coquilo123";
    char viejapass[] = "coco123";
    char user[] = "coco";
    int ok = 0;
    ok = reemplaPass(nuevapass, viejapass, user);
    printf("el valor de ok es: %d\n", ok);
}

int reemplaPass(char *nuevaPass, char *viejaPass, char *usuario)
{
    int status;
    FILE *users; //puntero al archivo con usuarios y contraseñas.
    FILE *fPtr;  //puntero al nuevo archivo
    char delim[] = " ;";
    char usAndPas[1000];
    char auxUsAndPas[1000];
    char *passEncontrada = (char *)malloc(TAM);
    char *userEncontrado = (char *)malloc(TAM);
    char *splitAux = (char *)malloc(TAM);
    char *passAux = (char *)malloc(TAM);
    char *auxBuffer = (char *)malloc(TAM);
    char *arregloFinal = (char *)malloc(TAM);
    char pruebaUser[] = "simurdiera";
    char pruebaPass[] = "simurdieraFry";

    users = fopen("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontrasAux2", "r+");
    if (users == NULL)
    {
        perror("hubo problemas abriendo la base de datos... ");
        exit(1);
    }

    fscanf(users, "%[^\n]", usAndPas);
    strcpy(auxUsAndPas, usAndPas);

    /*los va a ir buscando uno por uno. Cada vez que encuentra uno, lo concatena sea cual sea,
        pero cuando sea ESA pass, ahi lo reemplaza*/
    int i = 0;
    char *pn = strtok(auxUsAndPas, delim); //primera iteracion para split

    while (pn != NULL)
    {
        printf("%d: '%s'\n", i, pn);
        strcpy(splitAux, pn); //en splitaux va el user encontrado
        pn = strtok(NULL, delim);
        if (pn == NULL)
        {
            printf("llego al final \n");
            break; //al llegar al final, se sale.
        }
        printf("palabra encontrada: %s \n", splitAux);
        printf("tam palabra es: %d \n", strlen(splitAux));
        strcpy(passAux, pn); //en splitaux va el user encontrado

        if ((!strcmp(usuario, splitAux)) && ((!strcmp(viejaPass, passAux)) && ((i % 2) == 0)))
        { /*lo ultimo checkea que sea par, es decir, que sea un user y no una pass.*/
            strcpy(userEncontrado, splitAux);
            strcpy(passEncontrada, passAux);

            printf("se encontro el usuario y la pass: %s, %s \n", userEncontrado, passEncontrada);
            strcat(arregloFinal, userEncontrado);
            strcat(arregloFinal, " ");
            strcat(arregloFinal, nuevaPass);
            strcat(arregloFinal, ";");
        }
        else if ((i % 2) == 0)
        {
            strcat(arregloFinal, splitAux);
            strcat(arregloFinal, " ");
            strcat(arregloFinal, passAux);
            strcat(arregloFinal, ";");
        }

        i = i + 1;
    }

    strcat(arregloFinal, pruebaUser);
    strcat(arregloFinal, " ");
    strcat(arregloFinal, pruebaPass);
    strcat(arregloFinal, ";");
    printf("el arreglo leido en aux es: %s\n", usAndPas);
    printf("el arreglo final es aux es: %s\n", arregloFinal);
    status = remove("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontrasAux2");

    if (status == 0)
        printf("%s file deleted successfully.\n", "usuariosycontrasAux2");
    else
    {
        printf("Unable to delete the file\n");
        perror("Following error occurred");
    }
    fclose(users);

    fPtr = fopen("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontrasAux2", "w");

    if (fPtr == NULL)
    {
        /* File not created hence exit */
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

    printf("El archiv se creo bien, vamos a ingresarle el contenido: %s\n", arregloFinal);
    fputs(arregloFinal, fPtr);

    fclose(fPtr);

    printf("File created and saved successfully. :) \n");
    return 1;
}
