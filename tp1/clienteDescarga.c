#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TAM 4000
#define TAMRECEPCION 10000

int validarComando(char *);

int main(int argc, char *argv[])
{
    int sockfd, puerto, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int terminar = 0;
    int loginOk = 0;
    int retry = 3;
    int operacion = 0;
    int final = 0;

    char buffer[TAM];
    char bufferAux[1];
    char arregloFinal[TAM];
    int leidos = 0;
    int escritos = 0;
    int fileLen = 0;
    int finRecepcion = 0;
    int tamUltimoBloque = 1200;

    // char buffer[TAM];
    if (argc < 3)
    {
        fprintf(stderr, "Uso %s host puerto\n", argv[0]);
        exit(0);
    }

    FILE *fptr2 = fopen("/home/fabrizio/Escritorio/materias/So_archivos/imagenes3/imagenpiola2.iso", "wb");
    if (fptr2 == NULL)
    {
        perror("hubo problemas abriendo el segundo archivo ");
        exit(1);
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

        printf("Enviando automaticamente file_down para comenzar... \n");

        n = send(sockfd, "file_down", strlen("file_down"), 0);

        // Verificando si se escribió: fin
        //	printf("el tamaño del buffer es: %d \n", strlen(buffer));
        buffer[strlen(buffer) - 1] = '\0';
        if (!strcmp("fin", buffer))
        {
            terminar = 1;
            printf("ponemos terminar en 1 \n");
        }

        /*ya hemos enviado el usuario acá. esperamos respuesta de servidor*/

        //		buffer[strlen(buffer) - 1] = '\0';
        /*creo que el \n lo tengo que leer o tener en cuenta
												cuando el ingreso fue de consola, es decir, con un enter.
												si no, fijate que lo manda asi nomas. O sea, mande derecho*/
        memset(buffer, '\0', TAM);
        n = recv(sockfd, buffer, TAMRECEPCION, 0); //recibo algo. espero recibir a
        if (strcmp("imagenes: 1- tiny core; 2- archlinux", buffer) == 0)
        {
            printf("llegó: %s de %d elementos. Pero tam buffer es %ld \n", buffer, n, strlen(buffer));
            printf("Ingrese el numero de la imagen que desea bajar:\n");
            memset(buffer, '\0', TAM);
            fgets(buffer, TAM - 1, stdin);
            buffer[strlen(buffer) - 1] = '\0';
            n = send(sockfd, buffer, strlen(buffer), 0); //le envío el numero de imagen deseadas
        }
        else
        {
            printf("llegó: %s \n", buffer);
            exit(0);
        }

        while (finRecepcion != 1)
        {
            printf("Descargando imagen...   ");
            memset(buffer, '\0', TAM);
            n = recv(sockfd, buffer, TAMRECEPCION, 0); //empiezo a recibir datos. Bucle
            if (strcmp(buffer, "salir") == 0)
            {
                //me llego x. Letra de salida
                finRecepcion = 1;
                printf("aux5-log: recibi comando de finalizacion. cerrando... \n");
                fclose(fptr2);
                break;
            }
            else
            {
                escritos = fwrite(&buffer, 1, n, fptr2); //escribi en el archivo la parte recibida. 
                                                        // se usa n para que siempre se escriba la misma cantidad recibida.
                if (escritos == 0)
                {
                    perror("estado de la escritura: ");
                }
                memset(buffer, '\0', TAM);
                memset(bufferAux, '\0', TAM);
               
               
                if (escritos != 0)
                {
                    n = send(sockfd, "-", 1, 0); //respondo con un guion para indicar que ok
                }
                else
                {
                    perror("hubo un error en la escritura");
                    fclose(fptr2);
                    n = send(sockfd, "error", strlen("error"), 0); //respondo con un guion para indicar que ok
                }      
            
     
            
            }
        }

        exit(0);

        ////////////////////////////////Secuencia de pruebas para ver que llegue todo bien.
        /////////////////////////////////////////////
        //respondió bien. Hay que buclear esto nomás y cuidar la finalizacion de la secuencia

        printf("Ingrese el numero de la imagen que desea bajar:\n");
        memset(buffer, '\0', TAM);
        fgets(buffer, TAM - 1, stdin);
        n = send(sockfd, buffer, strlen(buffer), 0); //le envío el numero de imagen

        memset(buffer, '\0', TAM);
        n = recv(sockfd, buffer, TAMRECEPCION, 0);            //recibo una parte de la imagen.
        strcat(bufferAux, buffer);                            //la guardo en bufferrecibido
        escritos = fwrite(bufferAux, 1, TAMRECEPCION, fptr2); //escribi la primera parte

        memset(buffer, '\0', TAM);    //limpio buffer
        memset(bufferAux, '\0', TAM); //limpio bufferAux

        n = send(sockfd, "-", strlen("-"), 0);
        n = recv(sockfd, buffer, TAMRECEPCION, 0); //recibo una parte de la imagen.
        strcat(bufferAux, buffer);                 //guardo la segunda parte de la imagen en bufferAux

        escritos = fwrite(bufferAux, 1, TAMRECEPCION, fptr2); //escribo la segunda parte de la imagen.
        n = send(sockfd, "x", strlen("x"), 0);
        memset(buffer, '\0', TAM);
        memset(bufferAux, '\0', TAM); //limpio los dos buffers

        n = recv(sockfd, buffer, 1, 0); //recibo una parte de la imagen.

        if (!strcmp("a", buffer))
        { //recibi el final. cerrando

            printf("aux5- log: recibi comando de finalizacion. cerrando... \n");
            fclose(fptr2);
            exit(0);
        }

        ///////////////////////////////fin secuencia de prueba
        /////////////////////////////////////////

        if (!strcmp("ingreseContra", buffer))
        {
            printf("%s\n", buffer);
            memset(buffer, '\0', TAM);
            fgets(buffer, TAM - 1, stdin);
            n = send(sockfd, buffer, strlen(buffer), 0);
        }
        else
        {

            printf("lo que llego fue: %s \n", buffer);
            printf("ahora empieza la recepcion de la imagen \n");
            do
            {
                // memset(buffer, '\0', TAM);
                n = recv(sockfd, buffer, TAMRECEPCION, 0);
                if (!strcmp("a", buffer))
                {
                    printf("se llego al final del copiado");
                    final = 1;
                }

                strcat(arregloFinal, buffer);

                n = send(sockfd, "-", strlen("-"), 0);

                memset(buffer, '\0', TAM);

            } while (final != 0);

            fclose(fptr2);
            exit(0);

            //tengo que guardar el archivo.

            //despues de escribir, envio aca el ok para que siga enviando el otro.
        }
    }
    return 0;
}
