//compilar con cc -lrt al final para la real time library

// puedo ver la cola en: /dev/mqueue

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>

/* !!!!!!!!!!!!!!!!!!!!!!!!!! */
#define QUEUEPATH "/colapos" /* <--- add some name to que queue in the following format : /qname */

#define MSG_SIZE 50
#define MAXMJEQUE 5

int main(int argc, char *argv[])
{

    printf("Cliente tiene en argv[1]: %s \n", argv[1]);

    struct mq_attr queue_atributes = {0};
    /* Setting queue atributes.
         *
         * Default and max values can be viewed/modified through /proc interfaces
         * explained in /proc interfaces section of man 7 mq_overview
         */
    queue_atributes.mq_msgsize = MSG_SIZE;
    queue_atributes.mq_maxmsg = MAXMJEQUE;

    /* permission macros(the ones starting with S_) can vi viewed in man 2 open */
    char *aEnviar = (char *)malloc(MSG_SIZE);
    memset(&aEnviar, 0, sizeof(aEnviar));
    char *buffer = (char *)malloc(MSG_SIZE);
    memset(&buffer, 0, sizeof(buffer));

    mqd_t qd = mq_open(QUEUEPATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &queue_atributes);
    if (qd == -1)
    {
        perror("Creating queue");
        exit(EXIT_FAILURE);
    }

    char msg[MSG_SIZE] = "";
    unsigned int prio = 1;

    if (mq_receive(qd, msg, MSG_SIZE, &prio) == -1)
    {
        perror("cliente error lectura");
        exit(EXIT_FAILURE);
    }

    printf("Cliente recibio: %s \n", msg);
    printf("es de tamaño %d \n", (int)strlen(msg));
    if (strcmp(msg, "1holahola") == 0)
    {
        char envio[] = "2chauchau";
        printf("Cliente envia: %s \n", envio);
        printf("es de tamaño %d \n", (int)strlen(envio));
        if (mq_send(qd, envio, MSG_SIZE, (unsigned int)1) == -1)
        {
            perror("Sending");
            exit(EXIT_FAILURE);
        }else
        {
            printf("cliente recibio y envio correctamente... \n");
        }
        
    }
    else
    {

        printf("Cliente esta por enviar: %s \n", argv[1]);
     //   strcpy(aEnviar, (char *)argv[1]);
        char aEnviar[] = "buenoop";
        printf("Cliente envia: %s \n", aEnviar);
      //  printf("es de tamaño %d \n", (int)strlen(aEnviar));
        if (mq_send(qd, aEnviar, MSG_SIZE, (unsigned int)1) == -1)
        {
            perror("Sending");
            exit(EXIT_FAILURE);
        }
    }

    /*
    if (mq_close(qd) == -1)
    {
        perror("Closing queue");
    }
*/
    /*
    if (mq_unlink(QUEUEPATH) == -1)
    {
        perror("Destroying queue");
    }
    */
    exit(EXIT_SUCCESS);

    return 0;
}
