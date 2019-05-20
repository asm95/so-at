#include <stdio.h>          // Includes the Stardard I/O Library
#include <stdlib.h>         // Includes the Standard C Library
#include <unistd.h>         // Includes the POSIX operating system API
#include <sys/syscall.h>    // Includes the System Calls numbers
#include <sys/types.h>      // Includes the System Primitive Data Types
#include <sys/wait.h>       // Includes the System Wait definitions
#include <sys/stat.h>       // Includes POSIX File Characteristics Library
#include <signal.h>         // Includes the Signal definitions
#include <string.h>         // Includes the String Library

#ifdef _MSG_QUEUE_H
    #include "msgQueue.h"
#endif
#include "dataStructures.h"
#include "managerProcess.h"

/** \fn delayed_scheduler(int managers)
 *  \brief Função base do escalonador
 * 
 *  A função delayed scheduler é a responsável pelo controle do escalonador.
 *  Aqui o escalonador inicialmente seta todos os processos gerentes como prontos,
 *  colocando-os na lista de "ready" e, feito isso, executa um "pause" enquanto espera
 *  pelo primeiro job.
 *  Após a recepção do primeiro job, o escalonador fica em busy waiting ao final de cada
 *  job.
 * 
 *  A função prepara o tratamento de quatro sinais distintos (SIGINT, SIGUSR1, SIGUSR2 e SIGALRM)
 *  cada qual responsável por uma tarefa distinta: shutdown, send_pid, new_scheduler e execute_job
 *  respectivamente.
 * 
 *  Ao executar a função, caso não seja possível obter a fila, o escalonador é finalizado.
 * 
 *  \param int managers;
 *  \return void
 */
void delayed_scheduler(int managers);

/** \fn void shutdown()
 *  \brief Função de finalização do escalonador
 * 
 *  TODO
 * 
 *  \return void
 */
void shutdown();

/** \fn void new_schedule()
 *  \brief Função responsável por agendar uma nova execução
 *  
 *  TODO
 * 
 *  \return void
 */
void new_schedule();

/** \fn void send_pid()
 *  \brief Função responsável por enviar o PID do escalonador
 * 
 *  TODO
 * 
 *  \return void
 */
void send_pid();

/** \fn void execute_job()
 *  \brief Função responsável por executar jobs
 * 
 *  TODO
 * 
 *  \return void
 */
void execute_job();

/** \fn int main(int argc, char *argv[])
 *  
 *  TODO
 * 
 *  \param argc; TODO
 *  \param *argv[]; TODO
 */