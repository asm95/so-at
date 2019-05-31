#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/shm.h>

#include "msgQueue.h"

/** \brief Programa para envio de job de execução postergada
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
 *  para a fila de mensagens para que o escalonador inicie o job, acessa o arquivo "jobs.txt"
 *  para ler o contador de jobs e, por fim, imprime a informação do job enviado para execução.
 *  
 *  Caso o identificador não seja obtido, o programa imprime um erro e encerra.Caso ocorra 
 *  um erro de envio, o programa informa com um print.
 * 
 *  \param int argc; Quantidade de argumentos passados na CLI
 *  \param char *argv[]; Argumentos passados na CLI
 *  \return int;
 */
int main(int argc, char *argv[]){
    int msgsdid = get_channel(MQ_SD);                                                       // Gets the message queue ID
    int msgsjid = get_channel(MQ_SJ);
    int status, verify = 0, i = 0;
    char count[10], newcount[10], ch;
    FILE *file;
    msg_packet p;
    pid_packet ppkg;

    if(argc == 3){                                                                          // Checks if there are exectly 3 arguments on the CLI
        for(int i = 0; i < strlen(argv[2]); i++){                                           // Loops through the characters on the third argument
            if(((int)argv[2][i] < 48) || ((int)argv[2][i] > 57)){                           // If the nth-character is not a number
                verify = 1;                                                                 // Sets the flag
                break;
            }
        }

        if(verify == 0){                                                                    // If the arguments ara ok
            if(msgsdid >= 0){                                                               // If the message queue is loaded
                msgrcv(msgsdid, &ppkg, sizeof(pid_packet)-sizeof(long), 0x1, 0);            // Receives the PID of the Scheduler
                kill(ppkg.pid, SIGUSR1);                                                    // Sends a SIGUSR1 to the Scheduler

                p.type = 0x1;                                                               // Sets the message type
                strcpy(p.name, argv[1]);                                                    // Copies the program name
                p.delay = atoi(argv[2]);                                                    // Copies delay

                status = msgsnd(msgsjid, &p, sizeof(msg_packet)-sizeof(long), 0);           // Sends the message
                if(status == 0){                                                            // After the message was sent
                    file = fopen("jobs.txt", "r");                                          // Opens the jobs.txt file
                    while((ch = fgetc(file)) != EOF)
                        count[i++] = ch;                                                    // Reads the value of the counter
                    fclose(file);                                                           // Closes the jobs.txt file
                    printf("Job successfully sent!\n");
                    printf("job=%d, arquivo=%s, delay=%d\n", atoi(count), p.name, p.delay);
                    kill(ppkg.pid, SIGUSR2);                                                // Sends a SIGUSR2 to the Scheduler
                }
                else{                                                                       // If there was an error on sending the message
                    printf("Error while sending message...\n");
                    exit(1);
                }
            } else {                                                                        // If message queue ID wasn't loaded
                printf("Theres no open channel...\n");
                exit(2);
            }
        } else {                                                                            // If third argument is not a number
            printf("Argument #2 must contain only numbers...\n");
        }
    } else {
        printf("Wrong number of arguments...\n");
    }

    file = fopen("jobs.txt", "w+");
    sprintf(newcount, "%d", atoi(count)+1);
    fputs(newcount, file);
    fclose(file);

    exit(0);
}