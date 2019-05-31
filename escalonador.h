#include <stdio.h>          // Includes the Stardard I/O Library
#include <stdlib.h>         // Includes the Standard C Library
#include <unistd.h>         // Includes the POSIX operating system API
#include <sys/syscall.h>    // Includes the System Calls numbers
#include <sys/types.h>      // Includes the System Primitive Data Types
#include <sys/wait.h>       // Includes the System Wait definitions
#include <sys/stat.h>       // Includes POSIX File Characteristics Library
#include <sys/shm.h>
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
 *  colocando-os na lista de "ready" e, feito isso, o escalonador fica bloqueado esperando
 *  novos jobs.
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
 *  \brief Função para finalização do escalonador
 * 
 *  A função shutdown é a responsável por finalizar a execução do escalonador. Ao receber um
 *  sinal do tipo <b>SIGINT</b>, o escalonador deve printar as informações de resumo da execução,
 *  bem como quaisquer jobs não executados.
 * 
 *  Além disso, a flag de finalização é setada para "1" e a execução do escalonador é concluída,
 *  retornando para função main.
 * 
 *  No sumário constam as seguintes informações:
 *  - Processos que não foram executados (número do job, nome do programa e delay);
 *  - Todos os processos executados por todos os gerentes (PID, nome do programa, hora de recepção,
 *  hora de início da execução, hora de término da execução e makespan);
 * 
 *  \return void
 */
void shutdown();

/** \fn void new_schedule()
 *  \brief Função responsável por agendar uma nova execução
 *  
 *  A função recebe uma mensagem do programa "execucao_postergada". O recebimento é bloqueado de forma que,
 *  se a mensagem não estiver disponível, o escalonador fica bloqueado. Após a recepção da mensagem, caso a fila
 *  de jobs esteja vazia, um alarme inicial é setado. Caso contrário, o novo job é inserido na fila, os delays
 *  são atualizados de acordo, e a lista de jobs é imprimida. Caso o job sendo inserido tenha um delay menor do que
 *  o primeiro job que estava em primeiro da fila anteriormente, o alarme é atualizado. Caso o delay seja 0, a
 *  função executa um kill, enviando um SIGALRM para dar inicio imediato a execução do job.
 * 
 *  \return void
 */
void new_schedule();

/** \fn void send_pid()
 *  \brief Função responsável por enviar o PID do escalonador
 * 
 *  Ao iniciar a execução do escalonador, a função main() envia o PID do escalonador para a fila que fará a comunicação
 *  com o programa "execucao_postergada". Toda vez que uma nova ordem de job é recebida, o escalonador recebe um sinal e
 *  deverá reenviar o seu PID para tal fila, de forma que quando a próxima execução do "execucao_postergada" ocorrer, terá
 *  o PID para a comunicação disponível.
 * 
 *  \return void
 */
void send_pid();

/** \fn void execute_job()
 *  \brief Função responsável por executar jobs
 * 
 *  A função é a que, de fato, executa os jobs escalonados. Inicialmente um contador é setado para zero e um laço é executado
 *  enquanto houver processos prontos para executar. Para cada processo "ready", o escalonador envia uma mensagem para o gerente
 *  "zero" redirecioná-la até o nó destino. A mensagem contém a ordem de execução do job.
 *  
 *  Após o envio de todas as ordens de execução, o escalondor executa um laço infinito para receber as respostas dos gerentes,
 *  avisando que terminaram a execução do job. Cada execução é inserida em uma fila - que guarda pid, programa, hora de envio,
 *  hora de início da execução, hora de conclusão de execução -, e quando o contador atinge a marca de 15/16 (a depender da estrutura
 *  escolhida para execução do escalonador), é imprimida a informação de resumo do job e o mesmo é removido da lista de jobs.
 *  O laço executa um break para concluir a execução e aguardar a próxima execução.
 * 
 *  O escalonador ficará bloqueado esperando a próxima execução, ou inserção de jobs, caso a fila de execução esteja vazia.
 * 
 *  \return void
 */
void execute_job();

/*! \mainpage Escalonador de Execução Postergada
    
    \section introducao Introdução
 
    Projeto de Sistemas Operacionais 1/2019

    Professora: Dra. Alba Cristina Magalhães Alves de Melo

    Aluno: Bernardo Costa Nascimento 140080279

    Aluno: Cristano

    \section instrucoes_basicas Instruções Básicas

    O presente trabalho é o desenvolvimento de um escalonador de execução postergada.
    Para compilar o projeto, basta executar o seguinte comando:
    
    > sh compile.sh
   
    A compilação irá gerar 4 executáveis com os seguintes nomes:
   
    - escalonador
    - execucao_postergada
    - shutdown
    - hello
   
    Para utilizar o sistema, basta executar o 'escalonador'. O programa deve ser iniciado
    com um dos três argumentos -h, -t ou -f, como exemplificador abaixo:
   
    > ./escalonador -h&
   
    O comando acima irá iniciar a execução do escalonador em modo background (&). Feito isso
    o usuário deve utilizar o programa execucao_postergada para passar algum programa para
    escalonamento.
   
    Para executar o programa execucao_postergada, o usuário deve passar 2 argumentos: nome do 
    programa e delay. Segue um exemplo:
   
    > ./execucao_postergada hello 4
   
    O comando acima irá selecionar o programa hello para ser executado em 4 segundos. O usuário
    poderá executar quantos programas desejar dessa maneira.
    
    Quando o usuário quiser concluir a execução do sistema, basta executar o programa shutdown.
   
    > ./shutdown
   
    Uma documentação extensa a respeito de todo código pode ser encontrada neste site. Basta seguir
    para Ficheiros --> Lista de Ficheiros. Feito isso, basta escolher o arquivo para ler a documentação.
 */