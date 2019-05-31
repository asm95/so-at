#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/signal.h>

#include "msgQueue.h"

/** \brief Programa responsável por finalizar a execução do escalonador postergado
 *  
 *  O programa "shutdown" trabalha recebendo o PID do escalonador postergado, por meio de uma fila de mensagens,
 *  e, com o PID, envia um sinal <b>SIGINT</b> para o escalonador. Após o envio do sinal para encerramento
 *  da execução do escalonador, o programa irá acessar o arquivo "jobs.txt" e irá resetar o contador.
 *  
 *  O escalonador, por sua vez, trata o sinal, desviando a execução para rotina de tratamento que irá encerrar
 *  a execução do programa.
 * 
 *  Caso algum erro ocorra na recepção ou no envio, o usuário será avisado.
 * 
 *  IMPORTANTE: O programa shutdown só deve ser executado se o escalonador não estiver executando um job ou se estiver
 *  esperando para começar uma execução. Não utilize o comando caso exista uma execução em curso!
 * 
 *  \param int argc; Número de argumentos passados na CLI
 *  \param char *argv[]; Argumentos passados na CLI
 *  \return 0;
 */
int main(int argc, char *argv[]){
    FILE *file;
    int msgqid, recebido;
    pid_packet p;

    msgqid = get_channel(MQ_SD);                                                            // Loads the message queue ID
    if(msgqid >= 0){                                                                        // If message queue ID was loaded
        printf("Shuting down the scheduler...\n");

        recebido = msgrcv(msgqid, &p, sizeof(pid_packet)-sizeof(long), 0x1, 0);             // Receives the PID of the Scheduler
        if(recebido > 0)                                                                    // If there was no problem receiving the message
            kill(p.pid, SIGINT);                                                            // Sends a SIGINT to the Scheduler
        else                                                                                // If there was a problem while receiving the message
            printf("Error while receiving message...\n");                                   // Prints an error
    } else {                                                                                // If the message queue wasn't loaded
        printf("Error on getting the message queue...\n");                                  // Prints an error
    }

    file = fopen("jobs.txt", "w+");                                                         // Opens the "jobs.txt" file
    fputs("0", file);                                                                       // Resets the count
    fclose(file);                                                                           // Closes the "jobs.txt" file

    exit(0);
}