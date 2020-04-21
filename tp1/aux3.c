/*creacion y prueba del metodo que busca las imagenes y su md5*/

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

int verificarPass(char *, char *, int);
int reemplaPass(char *, char *, char *);
char *buscarImagenes();

int main(int argc, char *argv[])
{
    char *final = (char *)malloc(TAM);

    final = buscarImagenes();
    printf("el arreglo tornado fue: \n %s", final);


}

char *buscarImagenes()
{

    int status;
    FILE *images; //puntero al archivo con imagenes y sus md5;
    FILE *fPtr;   //puntero al nuevo archivo
    char delim[] = " ;";
    char imagesAndMd5[1000];
    char auxImages[1000];
    char *passEncontrada = (char *)malloc(TAM);
    char *userEncontrado = (char *)malloc(TAM);
    char *splitAux = (char *)malloc(TAM);
    char *md5Aux = (char *)malloc(TAM);
    char *auxBuffer = (char *)malloc(TAM);
    char *arregloFinal = (char *)malloc(TAM);

    images = fopen("/home/fabrizio/Escritorio/materias/so/tp1/images", "r+");
    if (images == NULL)
    {
        perror("hubo problemas abriendo la base de datos de imagenes... ");
        exit(1);
    }

    fscanf(images, "%[^\n]", imagesAndMd5);
    strcpy(auxImages, imagesAndMd5);

    /*los va a ir buscando uno por uno. Cada vez que encuentra uno, lo concatena sea cual sea,
        pero cuando sea ESA pass, ahi lo reemplaza*/
    int i = 0;
    char *pn = strtok(auxImages, delim); //primera iteracion para split

    while (pn != NULL)
    {
        printf("%d: '%s'\n", i, pn);
        strcpy(splitAux, pn);                          //en splitaux va la imagen encontrada
        printf("palabra encontrada: %s \n", splitAux); //o sea, el checksum encontrado
        printf("tam palabra es: %d \n", strlen(splitAux));
        pn = strtok(NULL, delim);
        if (pn == NULL)
        {
            printf("llego al final \n");
            break; //al llegar al final, se sale.
        }
        strcpy(md5Aux, pn);                          //en splitaux a imagen encontrado
        printf("palabra encontrada: %s \n", md5Aux); //o sea, el checksum encontrado
        printf("tam palabra es: %d \n", strlen(md5Aux));

        if ((i % 2) == 0)
        {
            strcat(arregloFinal, splitAux);
            strcat(arregloFinal, " ");
            strcat(arregloFinal, md5Aux);
            strcat(arregloFinal, "\n");
        }

        i = i + 1;
    }

    printf("el arreglo al final es:\n %s", arregloFinal);
    return arregloFinal;
}
