#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "vars.h"

int verificarPass(char *, char *, int);
int reemplaPass(char *, char *, char *);
char *buscarImagenes();
int bloquearUsuario(char *);
char *obtenerFecha();

int main(int argc, char *argv[])
{

  int servFd, cliFd, newservFd, var, n, num;
  struct sockaddr_in *serv_addr, *cli_addr;
  socklen_t tamCli;
  ssize_t tamMjeRecibido;
  socklen_t tamano;
  FILE *usuarios; //puntero al archivo con usuarios y contraseñas.

  char usypas[1000];
  char auxUsypas[1000];
  char mjeRecibido[500];
  char mjeManipulable[500];
  int auxint = 2;
  int tamBuffer;
  char fin[] = "fin";
  char pedirContrasenia[] = "ingreseContra";
  char pedirUsuario[] = "ingreseUsu\n";
  char contraOk[] = "contraok";
  char operaciones[] = "1: recibir recibir saludo. 2: salir\n";
  char userBloqueado[] = "userBloqueado";

  char delim[] = " ;";
  char *splitAux;
  char *userEncontrado;
  char *passEncontrada;
  int i = 0;
  int encontrado = 0;
  int chancesToLog = CHANCESTOLOG;
  int verificado = 0;
  int recibeInstrucciones = 1;

  char delim2[] = " ";
  char *palabra1 = (char *)malloc(TAM);
  char *palabra2 = (char *)malloc(TAM);
  char *palabra3 = (char *)malloc(TAM);
  char *palabra4 = (char *)malloc(TAM);
  char *fechaString = (char *)malloc(TAM);


  char *imagesLs = (char *)malloc(2 * (TAM));

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
    perror("authservice log: no se bindeo bien: ");
    exit(-1);
  }

  printf("Proceso: %d - socket disponible en puerto: %d\n", getpid(), ntohs(serv_addr->sin_port));
  listen(servFd, 1);

  //preparo para aceptar conexiones
  cli_addr = calloc(1, sizeof(struct sockaddr_in));
  tamCli = sizeof(struct sockaddr_in);

  /*abrir archivo para lectura/escritura: */
  usuarios = fopen("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontras", "r+");
  if (usuarios == NULL)
  {
    perror("hubo problemas abriendo la base de datos... ");
    exit(1);
  }

  char *pn;
  fscanf(usuarios, "%[^\n]", usypas);
  fclose(usuarios);
  strcpy(auxUsypas, usypas);

  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);
  printf("The current date/time is: %s", asctime(timeinfo));
  strcpy(fechaString, asctime(timeinfo));
  printf("The current date is: %s", fechaString);
	//voy a usar esteo para lo de la fecha
  while (1)
  {

    fflush(stdout);
    fflush(stdin);
    memset(&mjeRecibido, 0, sizeof(mjeRecibido));

    newservFd = accept(servFd, (struct sockaddr *)&cli_addr, &tamCli);

    //checkear errores de accept. Retorna -1 en posible error.

    //si se produjo la conexión, espera por un usuario.

    tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);

    printf("---------- \n");
    printf("authService log: mensaje recibido: %s \n", mjeRecibido);

    strcpy(mjeManipulable, mjeRecibido);

    /*acabo de recibir un nombre de usuario, lo busco en base de datos y veo si existe.
      tambien obtengo su contraseña para una posterior llamada*/

    char *ptr = strtok(auxUsypas, delim); //primera iteracion para split

    while ((ptr != NULL) && (encontrado == 0))
    {

      printf("%d: '%s'\n", i, ptr);
      splitAux = ptr;
      ptr = strtok(NULL, delim);

      printf("palabra encontrada: %s \n", splitAux);
      printf("tam palabra es: %ld \n", strlen(splitAux));

      if ((!strcmp(mjeManipulable, splitAux)) && ((i % 2) == 0))
      { /*lo ultimo checkea que sea par, es decir, que sea un user y no una pass.*/
        userEncontrado = splitAux;
        passEncontrada = ptr;
        encontrado = 1;
        printf("se encontro el usuario: %s \n", userEncontrado);
      }

      i = i + 1;
    }

    /*Se encontro usuario hasta aca. variables:
    ahora tengo que pedirle al cliente la pass
    
    pero tambien puede que no lo haya encontrado, en cuyo caso
    hay que pedirle que reingrese el usuario (mas adelante, ahora no)
    */

    if (encontrado)
    {
      do
      {
        //El blucle envia "ingreseContra" 3 veces. Al 3er intento mal ingresado, bloquea usuario
        printf("auth-serv log: usuario valido. Esperando contrasenia...\n");
        memset(&mjeRecibido, 0, sizeof(mjeRecibido));       // limpio el buffer para recibir
        memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir
        n = send(newservFd, pedirContrasenia, strlen(pedirContrasenia), 0);

        /*se recibio contrasenia. Se la verifica:*/
        tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);

        strcpy(mjeManipulable, mjeRecibido);
        printf("contrasenia recibida: %s \n", mjeManipulable);

        loginOk = verificarPass(passEncontrada, mjeManipulable, encontrado);
        if (chancesToLog == 0)
        {
          printf("auth-serv log: usuario anulado. Terminando programa...\n");
          n = send(newservFd, userBloqueado, strlen(userBloqueado), 0);
          close(newservFd); //ver el tema de anular usuario
          bloquearUsuario(userEncontrado);
        }
        chancesToLog = chancesToLog - 1;

      } while ((loginOk == 0) && (chancesToLog));
    }

    /*desde aqui el login se hizo correctamente*/

    if (loginOk)
    { //llega en caso de login correcto. Simplemente avisa logueo exitoso//
      printf("auth-serv log: acceso correcto. Esperando que ingresen operacion... \n");
      memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
      memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir
      n = send(newservFd, contraOk, strlen(contraOk), 0);
    }

    /*El login se hizo correctamente y se envio el contraok al cliente.
      Se espera respuesta con un comando*/

    if (loginOk)
    {
      do
      {

        tamMjeRecibido = recv(newservFd, mjeRecibido, (size_t)200, 0);
        char *comSplit = strtok(mjeRecibido, delim2); //tengo la primera palabra
        strcpy(palabra1, comSplit);
        palabra1[strlen(palabra1) - 1] = '\0';
        printf("palabra1 es: %s de tamano: %ld\n", palabra1, strlen(palabra1));
        if ((comSplit = strtok(NULL, delim2)) != NULL)
        {
          strcpy(palabra2, comSplit);
          printf("palabra2 es: %s de tamano: %ld\n", palabra2, strlen(palabra2));
        }
        if ((comSplit = strtok(NULL, delim2)) != NULL)
        {
          strcpy(palabra3, comSplit);
          printf("palabra3 es: %s de tamano: %ld\n", palabra3, strlen(palabra3));
        } //obtuve tercera palabra recibida. Es para el caso de recibit options despues
        if ((comSplit = strtok(NULL, delim2)) != NULL)
        {
          strcpy(palabra4, comSplit);
          printf("palabra4 es: %s de tamano: %ld\n", palabra4, strlen(palabra4));
        } //obtuve cuarta palabra recibida

        if (strncmp(palabra1, "file_down", strlen(palabra1)) == 0 || (strncmp(palabra1, "File_down", strlen(palabra1)) == 0))
        {
          printf("auth-serv log: comando recibido file_down. Se quiere bajar la imagen: %s\n", palabra2);
          memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
          memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir

          n = send(newservFd, "buscando imagen...", strlen("buscando imagen..."), 0); //
        }

        else if (strncmp(palabra1, "file_ls", strlen(palabra1)) == 0 || ((strncmp(palabra1, "File_ls", strlen(palabra1)) == 0)))
        {
          printf("auth-serv log: comando recibido file_ls %s\n", palabra2);
          memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
          memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir
          imagesLs = buscarImagenes();

          n = send(newservFd, imagesLs, strlen(imagesLs), 0);
        }

        else if (strncmp(palabra1, "exit", strlen(palabra1)) == 0 || ((strncmp(palabra1, "Exit", strlen(palabra1)) == 0)))
        {
          printf("auth-serv log: comando recibido exit %s\n", palabra2);
          memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
          memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir
          n = send(newservFd, "cerrando conexion", strlen("cerrando conexion"), 0);
          recibeInstrucciones = 0;
          close(newservFd);
        }

        else if (strncmp(palabra1, "new_pass", strlen(palabra1)) == 0 || ((strncmp(palabra1, "New_pass", strlen(palabra1)) == 0)))
        {
          printf("auth-serv log: comando recibido new pass. Nueva pass: %s\n", palabra2);
          memset(&mjeRecibido, 0, sizeof(mjeRecibido));       //entonces limpio el buffer para recibir
          memset(&mjeManipulable, 0, sizeof(mjeManipulable)); // limpio el buffer para recibir

          printf("auth-serv log: la pass actual es: %s de tamano: %ld\n", passEncontrada, strlen(passEncontrada));
          printf("auth-serv log: pertenece al user: %s de tamano: %ld\n", userEncontrado, strlen(userEncontrado));
          printf("auth-serv log: deseo cambiarla por: %s de tamano: %ld\n", palabra2, strlen(palabra2));
          printf("auth-serv log: ahora deseo cambiarla por: %s de tamano: %ld\n", palabra2, strlen(palabra2));
          printf("auth-serv log: el arreglo completo es:\n %s\n", usypas);

          if (reemplaPass(palabra2, passEncontrada, userEncontrado))
          {
            n = send(newservFd, "Cambio de contra correcto.", strlen("Cambio de contra correcto."), 0);
          }
        }
        /*en caso de no matchear con ninguna instruccion:...*/
        else
        {
          printf("auth-serv log: esperaba recibir comando, se recibio: %s\n", mjeRecibido);
          n = send(newservFd, "comando no valido. Reingrese", strlen("comando no valido. Reingrese"), 0);
          close(newservFd);
        }

        printf("limpiando buffer...\n");
        printf("\n");
        memset(&mjeRecibido, 0, sizeof(mjeRecibido));
        printf("tamBuffer es: %ld \n", strlen(mjeRecibido));
        printf("buffer: %s \n", mjeRecibido);
        printf("---------- \n");

      } while (recibeInstrucciones);
    }
  }

  return 0;
}

int verificarPass(char *pass, char *buffer, int userValido)
{
  /*retorna 1 si la contrasenia es ok. 0 en caso contrario*/
  if ((!strcmp(pass, buffer)) && (userValido))
  {
    printf("el usuario fue encontrado");
    return 1;
  }
  else
  {
    return 0;
  }
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

  users = fopen("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontras", "r+");
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
    printf("tam palabra es: %ld \n", strlen(splitAux));
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

  printf("el arreglo leido en aux es: %s\n", usAndPas);
  printf("el arreglo final es aux es: %s\n", arregloFinal);
  status = remove("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontras");

  if (status == 0)
    printf("%s file deleted successfully.\n", "usuariosycontras");
  else
  {
    printf("Unable to delete the file\n");
    perror("Following error occurred");
  }
  fclose(users);

  fPtr = fopen("/home/fabrizio/Escritorio/materias/so/tp1/usuariosycontras", "w");

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

char *buscarImagenes()
{

  /*los va a ir buscando uno por uno desde el archivo "images". 
  Siempre le queda guardada la imagen en split aux y su md5
  en md5aux, en caso de haberlo. Cuando la iteración sea par, se agregan estos dos valores
  a un arreglo (arreglo final) que es el que se retorna y se envía al cliente*/

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
  char *arregloFinal = (char *)malloc(2 * (TAM));

  images = fopen("/home/fabrizio/Escritorio/materias/so/tp1/images", "r+");
  if (images == NULL)
  {
    perror("hubo problemas abriendo la base de datos de imagenes... ");
    exit(1);
  }

  fscanf(images, "%[^\n]", imagesAndMd5);
  strcpy(auxImages, imagesAndMd5);

  int i = 0;
  char *pn = strtok(auxImages, delim); //primera iteracion para split

  while (pn != NULL)
  {
    printf("%d: '%s'\n", i, pn);
    strcpy(splitAux, pn);                          //en splitaux va la imagen encontrada
    printf("palabra encontrada: %s \n", splitAux); //o sea, el checksum encontrado
    printf("tam palabra es: %ld \n", strlen(splitAux));
    pn = strtok(NULL, delim);
    if (pn == NULL)
    {
      printf("llego al final \n");
      break; //al llegar al final, se sale.
    }
    strcpy(md5Aux, pn);                          //en splitaux a imagen encontrado
    printf("palabra encontrada: %s \n", md5Aux); //o sea, el checksum encontrado
    printf("tam palabra es: %ld \n", strlen(md5Aux));

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

int bloquearUsuario(char *usuario)
{
  printf("se bloqueo exitosamente el usuario\n");
  return 1;
}

char *obtenerFecha()
{
}

/*
http://es.tldp.org/Tutoriales/PROG-SOCKETS/prog-sockets.html
http://man7.org/linux/man-pages/man2/execve.2.html
https://es.wikipedia.org/wiki/Execve



*/
