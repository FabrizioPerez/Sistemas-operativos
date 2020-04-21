
/*lee en binario la imagen de instalación (ok)
La envía a través de un  socket (TODO))
*/

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
#define TAMENVIOFILE 10000

int main(int argc, char *argv[])
{
    /*socket cosas*/
    int servFd, cliFd, newservFd, var, n, num;
    struct sockaddr_in *serv_addr, *cli_addr;
    socklen_t tamCli;
    ssize_t tamMjeRecibido;
    socklen_t tamano;
    char mjeRecibido[4000];
    char mjeManipulable[4000];
    char palabra[4000];


    // char *buffer = (char *)malloc(20000);
    char buffer[4000];
    char bufferAux[4000];

    int leidos = 0;
    int escritos = 0;
    int fileLen = 0;
    int seguirEnviando = 1;
    FILE *fich;
    fich = fopen("/home/fabrizio/Escritorio/materias/So_archivos/imagenes/bionicpup32-8.0-uefi.iso", "rb");
    if (fich == NULL)
    {
        perror("hubo problemas abriendo la imagen original ");
        exit(1);
    }

    fseek(fich, 0, SEEK_END);
    fileLen = ftell(fich); // Obtiene el tamanio del archivo. Por las dudas para usar en algun lado
    fseek(fich, 0, SEEK_SET);
    printf("El tamaño total del archivo a enviar es: %d", fileLen);
/*
    FILE *fptr2 = fopen("/home/fabrizio/Escritorio/materias/So_archivos/imagenes2/fabripiola2.iso", "wb");
    if (fptr2 == NULL)
    {
        perror("hubo problemas abriendo el segundo archivo ");
        exit(1);
    }
*/
    servFd = socket(AF_INET, SOCK_STREAM, 0); //definicion del socket;

    serv_addr = calloc(1, sizeof(struct sockaddr_in)); //alloca e inicialiaza un lugar de memoria de ese tamaño. Devuelve puntero.
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons((uint16_t)atoi(argv[1]));
    serv_addr->sin_addr.s_addr = INADDR_ANY;
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

    while (1)
    {

        fflush(stdout);
        fflush(stdin);
        memset(&mjeRecibido, 0, sizeof(mjeRecibido));

        newservFd = accept(servFd, (struct sockaddr *)&cli_addr, &tamCli);
        //checkear errores de accept. Retorna -1 en posible error.

        //si se produjo la conexión, espera por un usuario.

        tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);

        strcpy(mjeManipulable, mjeRecibido);

        // mjeManipulable[strlen(mjeManipulable) - 1] = '\0'; /****************/

        printf("tamBuffer es: %d \n", strlen(mjeManipulable));
        printf("tam mje recibido: %d \n", tamMjeRecibido);
        printf("mensaje: %s \n", mjeManipulable);

        if (strncmp(mjeManipulable, "file_down", strlen(mjeManipulable)) == 0 || (strncmp(mjeManipulable, "File_down", strlen(mjeManipulable)) == 0))
        {
            printf("me llego el pedido");
            memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
            memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir
            n = send(newservFd, "imagenes: 1- tiny core; 2- archlinux", TAM, 0);  
            printf("se enviaron %d elementos\n", n);
            tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0); //esperamos el numero de imagen

            if (strncmp(mjeRecibido, "1", strlen(mjeRecibido)) == 0)
            { //se recibio que se desea bajar la imagen 1
                do
                {
                    memset(&mjeRecibido, 0, sizeof(mjeRecibido));
                    memset(&buffer, 0, sizeof(buffer));
                    leidos = fread(buffer, 1, TAMENVIOFILE, fich);   //The maximum send buffer size is 1,048,576 bytes.
                    if (leidos != 0)
                    {
                        n = send(newservFd, buffer, leidos, 0); //envio de datos
                        printf("------se enviaron: %d bytes----- \n", n );
                        memset(&buffer, 0, sizeof(buffer));       // limpio el buffer para recibir
                        memset(&bufferAux, 0, sizeof(bufferAux)); // limpio el buffer para recibir
                        n = recv(newservFd, buffer, TAM, 0);      //espera el guion
                     
                     /*   if ((strcmp(buffer, "-") != 0))
                        {
                            printf("aux4-log: llego algo distinto de -, llego %s \n", buffer);
                            seguirEnviando = 0; //o sea, llego algo distinto del "-". Dejar de enviar
                        }*/
                        
                        memset(&buffer, 0, sizeof(buffer));       // limpio el buffer para recibir
                    }
                    else if (leidos == 0)
                    { //probablemente sea el el EOF y no un error, pero te dice si fue exitoso
                       // perror("Estado de lectura de bytes");
                       printf("aux4 log: leidos = 0;\n");
                    }
                    if (ferror(fich))
                    { //checkeo el error
                        printf("Hubo un error en el copiado...");
                    }
                    if (feof(fich))
                    { //checkeo si llego al final del archivo eof
                        printf("el eof se alcanzo con exito dentro del while...\n");

                        memset(&buffer, 0, sizeof(buffer));
                        n = send(newservFd, "salir", strlen("salir"), 0); //envio de datos
                        seguirEnviando = 0;
                        fclose(fich);                             //cierro archivos
                        //fclose(fptr2);
                       
                    }

                } while ((leidos != 0) && seguirEnviando);
                printf("terminado el envio y cerrado");
            }
        }

        ////////////////////////////////Secuencia de pruebas para ver que llegue todo bien.
        /////////////////////////////////////////////
        //respondió bien. Hay que buclear esto nomás y cuidar la finalizacion de la secuencia

        /*

        printf("me llego el pedido");
        memset(&mjeRecibido, 0, sizeof(mjeRecibido));                      //entonces limpio el buffer para recibir
        memset(&mjeManipulable, 0, sizeof(mjeManipulable));                // limpio el buffer para recibir
        n = send(newservFd, "imagenes: 1- tiny core; 2- archlinux", 1, 0); //envio de datos
        memset(&mjeRecibido, 0, sizeof(mjeRecibido));                      //entonces limpio el buffer para recibir
        tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);
        mjeRecibido[strlen(mjeRecibido) - 1] = '\0';
        if (strncmp(mjeRecibido, "1", strlen(mjeRecibido)) == 0)
        { //llego queriendo descargar la imagen de tinycore
            //aca iria lo del do-while. pero para simular voy a hacer esto.
            memset(&mjeRecibido, 0, sizeof(mjeRecibido)); //entonces limpio el buffer para recibir
            leidos = fread(buffer, 1, 2000, fich);
            //  printf("bytes leidos: %d", leidos);
            if (leidos == 0)
            { //probablemente sea el el EOF y no un error, pero te dice si fue exitoso
                perror("Estado de lectura de bytes");
            }
            n = send(newservFd, buffer, leidos, 0); //envio de datos
            memset(&buffer, 0, sizeof(buffer));     //entonces limpio el buffer para recibir

            n = recv(newservFd, buffer, TAM, 0);
            if (strncmp(mjeRecibido, "-", strlen(mjeRecibido)) == 0)
            {
                leidos = fread(buffer, 1, 2000, fich);
                n = send(newservFd, buffer, leidos, 0); //envio de segundos datos
                memset(&buffer, 0, sizeof(buffer));     //entonces limpio el buffer para recibir

                n = recv(newservFd, buffer, TAM, 0); //ver si es necesario cambiar tam por 1
                if (strncmp(mjeRecibido, "x", strlen(mjeRecibido)) == 0)
                {
                    printf("aux4 log: recibiio bien la primer parte y la segunda. Cerrando.. \n");
                    n = send(newservFd, "a", 1, 0); //envio de datos
                    fclose(fich);                   //cierro archivos
                    fclose(fptr2);
                }
            }
        }
        exit(0);

*/

        ///////////////////////////////fin secuencia de prueba
        /////////////////////////////////////////

        /*
        do
        {
            leidos = fread(buffer, 1, 2000, fich);
            //  printf("bytes leidos: %d", leidos);
            if (leidos == 0)
            { //probablemente sea el el EOF y no un error, pero te dice si fue exitoso
                perror("Estado de lectura de bytes");
            }

            escritos = fwrite(buffer, 1, leidos, fptr2);
            memset(&buffer, 0, 20000);

            if (feof(fich))
            { //checkeo si llego al final del archivo eof
                printf("el eof se alcanzo con exito dentro del while...");
            }

        } while (leidos != 0);
*/
    }

    //fclose(fich);
    //fclose(fptr2);
}

/*
int main()
{
    FILE *fptr = fopen("cara.jpg", "rb");           // open existing binary picture
    char buffer[10000] = {0};                       // the pic is 6kb or so, so 10k bytes will hold it
    FILE *fptr2 = fopen("new_small_pic.jpg", "wb"); // open a new binary file
                                                    // for our copy of the pic
    unsigned long fileLen;
    unsigned long counter;
    char buffer2[10000]; // the pic is 6kb or so, so 10k bytes will hold it

    fseek(fptr, 0, SEEK_END);
    fileLen = ftell(fptr); // get the exact size of the pic
    fseek(fptr, 0, SEEK_SET);

    for (counter = 0; counter < fileLen; counter++)
    {
        // fputc(fgetc(fptr),fptr2);  // read each byte of the small_pic.jpg and make
        // a new pic from it
        strncat(buffer2,fgetc(fptr), 1);
        fputc(buffer2[counter], fptr2);
    }

    fclose(fptr);
    fclose(fptr2);
    return 0;
}

*/
/*
int main()
{
    char buffer = (char *)malloc(4000);
    int leidos = 0;
    FILE *fich;
    fich = fopen("/home/martin/Documentos/DocumentosFacultad/SO2/2020/filesService/img/bionic.iso", "rb");

    leidos = fread(buffer, 1, 2000, fich);
    while (leidos != 0)
    {
        fwrite(buffer, 1, leidos, fptr2);
        //  memset(&bufMensajeEnviar.mtext, 0, 2000);
        // strcpy(bufMensajeEnviar.mtext, buffer);

        msgsnd(idenColaMensajes, (struct msgbuf *)&bufMensajeEnviar,
               sizeof(bufMensajeEnviar.datoNumerico) +
                   sizeof(bufMensajeEnviar.mtext),
               IPC_NOWAIT);

        memset(&buffer, 0, 2000);
        leidos = fread(buffer, 1, 2000, fich);
        printf("Mando mensaje 1\n");
    }
    fclose(fich);
}
*/
