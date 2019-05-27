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
 *  <b>Visão Geral</b>
 * 
 *  A função "managerProcess" executa um laço infinito para toda a execução.
 *  Dentro, todos os tratamentos de mensagem e execução dos gerentes são realizados.
 *  
 *  Ao receber uma mensagem, é verificado o destino da mesma. Se o destino for "-1",
 *  O gerente sabe que a mensagem está fluindo no sentido do escalonador. Caso o destino
 *  seja >= 0, o gerente sabe que a mensagem está fluindo no sentido dos gerentes.
 *  
 *  Caso a mensagem recebida tenha como destino um outro gerente - que não o gerente que 
 *  executou a recepção -, a mensagem é redirecionada para o gerente da lista de conexão mais 
 *  próximo do destino, ou o próprio destino. Caso a mensagem tenha como destino o gerente que 
 *  a recebeu, ele faz o tratamento. 
 * 
 *  Como dito anteriormente, as mensagens fluem em dois sentidos. No caso das mensagens que fluem
 *  do escalonador para os gerentes, ao encontrarem os destinos certos, os gerentes dão início
 *  a execução do programa solicitado pelo usuário e, após a execução ou erro da mesma, gerente
 *  envia as informações resultantes para o escalonador.
 * 
 *  A execução é realizada da seguinte forma: o gerente executa um fork. O filho irá executar o programa de fato,
 *  enquanto que o gerente salva a hora de inicio da execução, aguarda a saída do filho, salva
 *  a hora de término da execução e envia a mensagem de conclusão da execução para o escalonador.
 * 
 *  No caso das mensagens fluindo na direção do escalonador, os gerentes apenas fazem o redirecionamento
 *  sem maiores tratamentos.
 * 
 *  <b>Cálculo de rotas:</b>
 *  As rotas são traçadas em dois sentidos: escalonador -> gerentes; gerentes -> escalonador. Para
 *  ambos, exite uma subdivisão entre estruturas. Fat Tree e Hypercube executam um cálculo, Torus
 *  executa um cálculo diferente. Inicialmente havia sido desenvolvido apenas um tipo de cálculo para
 *  cada sentido porém, isso ocasionou congestionamento na estrutura Torus.
 * 
 *  - <b>Fat Tree e Hybercube</b>
 *  
 *   - <b>Escalonador => Gerentes:</b> A estratégia é verificar se a i-ésima conexão do gerente é menor que
 *   o destino e diferente de -1 (identificador do escalonador). Caso as duas condições sejam verdadeiras
 *   "aux" recebe o valor da i-ésima conexão. Caso a i-ésima conexão seja o destino, "aux" recebe o valor
 *   e sai do loop FOR. Feito isso, a mensagem é redirecionada para o próximo nó da estrutura ou para o
 *   nó destino.
 * 
 *   - <b>Gerentes => Escalonador:</b> Inicialmente a variável "aux" é setada para um valor alto arbitrário.
 *   Um for é executado para verificar qual das conexões do nó é a menor (mais próxima do escalonador).
 *   "aux" recebe a menor conexão. Caso alguma das conexões do nó seja -1 (escalonador), "aux" recebe o
 *   o valor da conexão com o escalonador e uma saída do for é executada. Ao final da execução do for
 *   a mensagem é enviada ao escalonador ou é redirecionada para um nó mais próxima do escalonador.
 *   obs: A estratégia pode ser resumida como uma tentativa de se alcançar o nó "0" para, então, enviar
 *   ao escalonador. A menor conexão é escolhida pois será numéricamente mais próxima de "0".
 * 
 *  - <b>Torus</b>
 * 
 *   - <b>Escalonador => Gerentes:</b> Inicialmente a variável "aux" recebe o valor do destino módulo "4" e
 *   "aux2" recebe "0". A ideia é verificar em qual coluna o destino se encontra. Um for é executado para verificar 
 *   se alguma das n conexões do nó está na mesma coluna. Caso esteja ou seja o nó destino, "aux2" recebe o valor da 
 *   conexão. Após o for, é verificado se "aux2" ainda é "0". Em caso positivo, "aux2" recebe o ID do nó acrescido de "1".
 *   É verificado se o valor de "aux2" módulo "4" é igual a "0". Em caso positivo, "aux2" recebe o resto da divisão
 *   de "aux2" por "4". A ideia é manter o redirecionamento na mesma coluna mas, caso não seja possível, a mensagem é
 *   redirecionada para o nó a direita do nó atual.
 * 
 *   - <b>Gerentes => Escalonador:</b> Inicialmente a variável "aux" começa com um valor alto arbitrário.
 *   Um laço for é executado para verificar todas as conexões do nó. O nó verifica se a i-ésima conexão que faz
 *   está na coluna "0", por meio de um módulo "4". Caso esteja e caso a conexão seja um ID menor que o seu próprio,
 *   "aux" recebe o ID dessa conexão. Caso a conexão tenha o destino, "aux" é setada para o destino final.
 *   Ao final do laço for, caso a variável "aux" ainda contenha o valor arbitrário inicial, o nó irá definí-la como
 *   a conexão diretamente a esquerda do nó. Caso o nó seja "0", aux é setado para o valor "-1" que simboliza o
 *   escalonador.
 * 
 *  \param int _id; ID do gerente (entre 0 e 15);
 *  \param pid_t *connections; Array com os id's dos gerentes ao qual se conecta;
 *  \param char *option; Define a opção de estrutura utilizada pelo escalonador;
 *  \return void;
 */
void manager_process(int _id, pid_t *connections, char *option);