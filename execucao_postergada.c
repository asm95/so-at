#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/signal.h>

#include "msgQueue.h"

/** \file execucao_postergada.c
 *  
 *  \brief Programa para envio de job de execução postergada
 *  
 *  O programa "execucao_postergada" deve receber exatamente dois argumentos:
 *  nome do programa e delay (em segundos).
 *  O programa inicia verificando quantos argumentos foram inseridos na CLI e,
 *  caso o número seja diferene de dois, o programa imprime que o número de argumentos
 *  está errado e encerra. Caso contrário, a execução segue.
 * 
 *  Após a checagem de número de argumentos, o programa verifica se o segundo argumento
 *  (delay) contém apenas números. Caso o argumento contenha outros caracteres, o programa
 *  imprime que o argumento deve conter apenas números e encerra. Caso contrário, a execução
 *  continua.
 * 
 *  Feito isso, o programa verifica se obteve um identificador de fila de mensagens e, em caso
 *  negativo, encerra o programa. Em caso positivo, o programa busca a mensagem na fila
 *  que irá conter o PID do escalonador para o envio da mensagem contendo o programa e o
 *  delay para execução. Após a recepção dessa mensagem, um sinal é enviado para escalonador
 *  para que uma nova mensagem, contendo seu PID, seja enviado para futura utilização do 
 *  "execucao_postergada".
 * 
 *  O programa monta a mensagem contendo o nome do programa e o delay de execução e envia
 *  para a fila de mensagens para que o escalonador inicie o job.
 *  
 *  Caso o identificador não seja obtido, o programa imprime um erro e encerra.Caso ocorra 
 *  um erro de envio, o programa informa com um print.
 * 
 *  \param int argc; Quantidade de argumentos passados na CLI
 *  \param char *argv[]; Argumentos passados na CLI
 *  \return int;
 */
int main(int argc, char *argv[]){
    int msg_id = get_channel(MQ_SD);
    int status, verify = 0;
    msg_packet p;
    pid_packet ppkg;

    if(argc == 3){
        for(int i = 0; i < strlen(argv[2]); i++){
            if(((int)argv[2][i] < 48) || ((int)argv[2][i] > 57)){
                verify = 1;
                break;
            }
        }

        if(verify == 0){
            if(msg_id >= 0){
                msgrcv(msg_id, &ppkg, sizeof(pid_packet)-sizeof(long), 0x1, 0);
                kill(ppkg.pid, SIGUSR1);

                p.type = 0x1;
                strcpy(p.name, argv[1]);
                p.delay = atoi(argv[2]);

                status = msgsnd(msg_id, &p, sizeof(msg_packet)-sizeof(long), 0);
                if(status == 0){
                    printf("Message successfully sent!\n");
                    kill(ppkg.pid, SIGUSR2);
                }
                else{
                    printf("Error while sending message...\n");
                    exit(1);
                }
            } else {
                printf("Theres no open channel...\n");
                exit(2);
            }
        } else {
            printf("Argument #2 must contain only numbers...\n");
        }
    } else {
        printf("Wrong number of arguments...\n");
    }

    exit(0);
}