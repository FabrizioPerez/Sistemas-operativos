#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/types.h>
#include <sys/wait.h>

/* !!!!!!!!!!!!!!!!!!!!!!!!!! */
#define QUEUEPATH "/colapos" /* <--- add some name to que queue in the following format : /qname */

#define MSG_SIZE 50
#define MAXMJEQUE 5

int main()
{

        struct mq_attr queue_atributes = {0};

        queue_atributes.mq_msgsize = MSG_SIZE;
        queue_atributes.mq_maxmsg = MAXMJEQUE; //por si vuelvo a tener invalid argumen, recorda que attr->mq_maxmsg debe ser menor o igual a msg_max, que está en
                                               // /proc/sys/fs/mqueue

        /* permission macros(the ones starting with S_) can vi viewed in man 2 open */
        mqd_t qd = mq_open(QUEUEPATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &queue_atributes);
        if (qd == -1)
        {
                perror("Creating queue");
                exit(EXIT_FAILURE);
        }

        /*

        char msg[MSG_SIZE] = "";
        unsigned int prio = 1;
        if (mq_receive(qd, msg, MSG_SIZE, &prio) == -1)
        {
                perror("Receiving");
                exit(EXIT_FAILURE);
        }
        if (mq_close(qd) == -1)
        {
                perror("Closing queue");
        }
        printf("Message received in procces: %s \n", msg);
        printf("es de tamaño %d\n", (int)strlen(msg));
        */

        pid_t chpid = fork();

        if (chpid == -1)
        {
                perror("Creating child process");
        }
        else if (chpid == 0)
        {
                printf("proceso hijo nro: %d \n", chpid);

                if (mq_close(qd) == -1)
                {
                        perror("Closing queue");
                }
                char *argv[] = {"./_posixC", "buenobueno", NULL};
                char *envp[] = {NULL};
                if (execv("./_posixC", argv) == -1)
                {
                        perror("Could not execve");
                }
        }
        else if (chpid)
        {
                /*
                if (mq_close(qd) == -1)
                {
                        perror("Closing queue padre");
                }

                mqd_t qd = mq_open(QUEUEPATH, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO, &queue_atributes);
                if (qd == -1)
                {
                        perror("Creating queue");
                        exit(EXIT_FAILURE);
                }
                
*/
                printf("proceso padre nro: %d \n", chpid);
                char id[] = "1";
                char envio[] = "holahola";
                strcat(id, envio);
                printf("proceso padre envia: %s \n", envio);
                if (mq_send(qd, id, MSG_SIZE, (unsigned int)1) == -1)
                {
                        perror("Sending");
                        exit(EXIT_FAILURE);
                }

                char *buffer = (char *)malloc(MSG_SIZE);
                memset(&buffer, 0, sizeof(buffer));

                /*
                char envio[] = "dePrueba";
                if (mq_send(qd, envio, MSG_SIZE, (unsigned int)1) == -1)
                {
                        perror("Sending");
                        exit(EXIT_FAILURE);
                }
                else
                {
                         printf("padre envio bien %s \n", envio);
                }
                */

                wait(200);
                char msg[MSG_SIZE] = "";
                unsigned int prio = 1;

                if (mq_receive(qd, msg, MSG_SIZE, &prio) == -1)
                {
                        perror("padre error lectura");
                        exit(EXIT_FAILURE);
                }

                printf("Message received en padre: %s \n", msg);
                printf("es de tamaño %d \n", (int)strlen(msg));
                char msgLimpio[MSG_SIZE] = "";
                strncpy(msgLimpio, msg, 1);
                if (strcmp(msgLimpio, "2") == 0)
                {
                        printf("padre eliminando queue...\n");
                        if (mq_unlink(QUEUEPATH) == -1)
                        {
                                perror("Destroying queue");
                        }
                }
                else if (strcmp(msgLimpio, "1") == 0)
                {
                        printf("padre robo el mensaje... reenviar\n");
                }
        }

        /*
        delete : if (mq_unlink(QUEUEPATH) == -1)
        {
                perror("Destroying queue");
        }
*/
        return 0;
}