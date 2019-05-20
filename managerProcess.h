#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef _SYS_LIBS_H
    #include <sys/signal.h>
    #include <sys/types.h>
    #include <sys/wait.h>
#endif

#ifndef _MSG_QUEUE_H
    #include "msgQueue.h"
#endif

/** \fn void manager_exit()
 *  \brief Executa a finalização do processo gerente
 * 
 *  A função "manager_process" executa um laço infinito, esperando sempre pela
 *  recepção de mensagens para redirecionamento ou execução. Dessa forma,
 *  ao receber um sinal <b>SIGQUIT</b> do escalonador, a execução é desviada
 *  para esta função, que executa um exit(0) para encerrar o gerente.
 * 
 *  \return void
 */
void manager_exit();

/** \fn void manager_process(int _id, pid_t *connections, char *option)
 *  \brief Função de gerenciamento dos processos
 * 
 *  A função "managerProcess" executa um laço infinito para toda a execução.
 *  Dentro do laço, há um segundo laço infinito para recepção de mensagens.
 *  
 *  Ao receber um dos tipos de mensagem ((0x2 + _id) ou (19 + _id)), é executado
 *  um break para tratar a mensagem recebida. Caso a mensagem recebida tenha como
 *  destino um outro gerente - que não o gerente que executou a recepção -, a
 *  mensagem é redirecionada para o gerente da lista de conexão mais próximo do destino,
 *  ou o próprio destino.
 * 
 *  Caso a mensagem tenha como destino o gerente que a recebeu, ele faz o tratamento. As
 *  mensagens fluem em dois sentidos: do escalonador para os gerentes (0x2 + _id) e dos
 *  gerentes para o escalonador (19 + _id). Caso a mensagem siga o sentido escalonador-gerentes,
 *  O tratamento será para execução de um dado programa do job que está sendo executado.
 *  Caso contrário, a mensagem será redirecionada de forma a chegar no gerente 0 que irá passar
 *  para o escalonador.
 * 
 *  No caso da execução, o gerente executa um fork. O filho irá executar o programa de fato,
 *  enquanto que o gerente salva a hora de inicio da execução, aguarda a saída do filho, salva
 *  a hora de término da execução e envia a mensagem de conclusão da execução para o escalonador.
 * 
 *  \param int _id; ID do gerente (entre 0 e 15);
 *  \param pid_t *connections; Array com os id's dos gerentes ao qual se conecta;
 *  \param char *option; Define a opção de estrutura utilizada pelo escalonador;
 *  \return void;
 */
void manager_process(int _id, pid_t *connections, char *option);