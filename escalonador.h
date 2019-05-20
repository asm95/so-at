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
 *  colocando-os na lista de "ready" e, feito isso, o escalonador fica em busy waiting 
 *  esperando novs jobs.
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
 *  sinal do tipo <b>SIGINT</b>, o escalonador deve fechar os canais de comunicação previamente abertos
 *  e executar o sumário de execução.
 * 
 *  No sumário constam as seguintes informações:
 *  - Processos que não foram executados (número do job, nome do programa e delay);
 *  - Todos os processos executados por todos os gerentes (PID, nome do programa, hora de recepção,
 *  hora de início da execução, hora de término da execução e makespan);
 * 
 *  Feito isso, a função espera pela finalização da execução dos processos gerentes, de forma a evitar
 *  o aparecimento de processos zumbis.
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
 *  O escalonador ficará em busy waiting esperando a próxima execução.
 * 
 *  \return void
 */
void execute_job();

/** \fn int main(int argc, char *argv[])
 *  
 *  A função principal do escalonador. Aqui ocorre toda a preparação para a execução do escalonador e de seus processos gerentes.
 *  A execução começa recebendo o parâmetro de estrutura (-h, -t ou -f). Feito isso, os canais de comunicação são criados - um para
 *  comunicação com o programa "execucao_postergada" e "shutdown", e outro para os processos gerentes. O escalonador envia duas mensagens 
 *  iniciais para comunicação com "execucao_postergada" e com o "shutdown". O escalonador verifica o tipo de estrutura que foi escolhido para 
 *  execução - hypercube, torus ou fat tree -, e cria os grafos ou a árvore simbólica dos nós gerentes.
 * 
 *  Após a criação das estruturas simbólicas, o escalonador realiza os N forks para criação dos processos gerentes. Para cada criação
 *  o PID do filho é armazenado em um vetor, o "_id" é atualizado para o filho e, caso alguma chamada fork ocasione erro, os processos
 *  que já tenham sido criados são terminados enviando um SIGKILL a eles.
 * 
 *  Os filhos criados executam uma função para receber, da estrutura simbólica, as conexões que eles fazem e, a partir dai, seguem para
 *  execução da função de gerenciamento. O pai segue para execução da função de escalonador postergado.
 * 
 *  \param argc; TODO
 *  \param *argv[]; TODO
 */