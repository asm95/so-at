#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>

#include "msgQueue.h"

/** \brief Programa responsável por finalizar a execução do escalonador postergado
 *  
 *  O programa "shutdown" trabalha recebendo o PID do escalonador postergado, por meio de uma fila de mensagens,
 *  e, com o PID, envia um sinal <b>SIGINT</b> para o escalonador.
 *  
 *  O escalonador, por sua vez, trata o sinal, desviando a execução para rotina de tratamento que irá encerrar
 *  a execução do programa.
 * 
 *  Caso algum erro ocorra na recepção ou no envio, o usuário será avisado.
 * 
 *  \param int argc; Número de argumentos passados na CLI
 *  \param char *argv[]; Argumentos passados na CLI
 *  \return 0;
 */
int main(int argc, char *argv[]){
    int msgqid, recebido;
    pid_packet p;

    msgqid = get_channel(MQ_SD);
    if(msgqid >= 0){
        printf("Shuting down the scheduler...\n");

        recebido = msgrcv(msgqid, &p, sizeof(pid_packet)-sizeof(long), 0x1, 0);
        if(recebido > 0)
            kill(p.pid, SIGINT);
        else
            printf("Error while receiving message...\n");
    } else {
        printf("Error on getting the message queue...\n");
    }

    exit(0);
}