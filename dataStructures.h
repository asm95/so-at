#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

//! Número de Gerentes para Hypercube e Torus
#define CHILDS 16
//! Número de Gerentes para Fat Tree
#define FATCHILDS 15

//! Define a estrutura da lista de jobs
typedef struct execq {
    //! Número do Job a ser executado
    int  job;
    //! Nome do programa a ser executado
    char *name;
    //! Delay do programa (definido pelo usuário)
    int  rDelay;
    
    //! Delay atualizado conforme os jobs são executados
    time_t uDelay;
    //! Hora de envio do job
    time_t sent;

    //! Próximo Job da lista
    struct execq *prox;
} execq;

//! Define a estrutura de jobs que já foram executados
typedef struct execd{
    //! PID do processo que realizou a n-ésima execução do programa
    pid_t  pid;
    //! Programa executado
    char*  program;
    //! Hora do envio do job
    time_t sent;
    //! Hora que o programa iniciou a execução
    time_t begin;
    //! Hora que o programa concluíu a execução
    time_t end;
    //! Tempo total de execução do programa
    double makespan;

    //! Próximo processo da lista de execuções concluídas
    struct execd *next;
} execd;

//! Define a estrutura da Fat Tree (foi implementada uma Binary Tree)
typedef struct fTree {
    //! ID do Gerente
    pid_t id;                                       // ID of the process

    //! Tamanho do vetor de conexões
    int   lenght;
    //! Vetor de conexões
    pid_t *connects;

    //! Próximo nó da árvore posicionado a esquerda
    struct fTree *left;                             // Left node
    //! Próximo nó da árvore posicionado a direita
    struct fTree *right;                            // Right node
} fTree;

//! Define a estrutura do Hybercube e Torus
typedef struct hyperTorus{
    //! ID do Gerente
    pid_t id;

    //! Tamanho do vetor de conexões
    int   length;
    //! Vetor de conexões
    pid_t *connects;

    //! Próximo elemento do grafo Hybercube ou Torus
    struct hyperTorus *next;
} hyperTorus;

//! Define a estrutura de processos livres
typedef struct manq{
    //! ID do Gerente
    pid_t _id;
    
    //! Próximo Gerente da fila
    struct manq *next;
} manq;

/** \fn void createQueue(execq **queue)
 *  \brief Inicia a estrutura de dados do tipo execq
 * 
 *  Inicializador padrão de uma estrutura de dados do tipo execq.
 * 
 *  \param execq **queue; Ponteiro para uma estrutura do tipo execq
 *  \return void;
 */
void createQueue(execq **queue);

/** \fn void insertProcess(execq **queue, char *_name, int _delay)
 *  \brief Insere um novo job na fila
 * 
 *  Realiza a inserção de um novo job recebido pelo escalonador. Inserção
 *  é ordenada pelo delay. Dessa forma, não segue uma estrutura típica de fila, e
 *  sim de uma lista ordenada.
 *  
 *  \param execq **queue; Ponteiro para estrutura do tipo execq;
 *  \param char *_name; Ponteiro para o nome do programa;
 *  \param int _delay; Delay, em segundos, para execução do job;
 *  \return void;
 */
void insertProcess(execq **queue, char *_name, int _delay);

/** \fn void removeProcess(execq **queue)
 *  \brief Remove um job da lista.
 * 
 *  Realiza a remoção de um job da lista de jobs. A remoção deve ocorrer após a
 *  conclusão do job pelos processos gerentes. Sempre o primeiro job da lista é
 *  removido.
 * 
 *  \param execq **queue; Ponteiro para estrutura do tipo execq;
 *  \return void;
 */
void removeProcess(execq **queue);

/** \fn void listProcesses(execq *queue)
 *  \brief Lista todos os jobs não concluídos
 * 
 *  Realiza a impressão de todos os jobs que ainda estão para serem executados.
 * 
 *  \param execq *queue; Cópia da estrutura do tipo execq
 *  \return void;
 */
void listProcesses(execq *queue);

/** \fn void updateDelays(execq **queue)
 *  \brief Realiza update do delay dos jobs na lista
 * 
 *  Após a execução de um job ou a inserção de um novo, é necessário atualizar
 *  o delay dos jobs na lista de execução pois algum tempo se passou.
 *  A função irá realizar a atualização dos valores de acordo com o a hora que o 
 *  job foi enviado e quanto tempo se passou desde então.
 * 
 *  \param execq **queue; Ponteiro para estrutura do tipo execq;
 *  \return void;
 */
void updateDelays(execq **queue);

/** \fn void createExecD(execd **done)
 *  \brief Inicializador da estrutura execd
 * 
 *  Inicializador padrão da estrutura do tipo execd
 * 
 *  \param execd **queue; Ponteiro para estrutura do tipo execd;
 *  \return void;
 */
void createExecD(execd **done);

/** \fn void insertExecD(execd **done, pid_t pid, char* program, time_t sent, time_t begin, time_t end)
 *  \brief Insere informação sobre todos os processos que executaram um dado job
 * 
 *  Inserção das informações resultantes da execuçaõ de um job qualquer pelo sistema. A inserção
 *  segue o padrão de fila.
 * 
 *  \param execd **done; Ponteiro para estrutura do tipo execd;
 *  \param pid_t pid; PID do processo que realizou a n-ésima execução do programa;
 *  \param char *program; Nome do programa executado;
 *  \param time_t sent; Hora de envio do job;
 *  \param time_t begin; Hora do início da execução do programa pelo processo;
 *  \param time_t end; Hora de conclusão da execução do programa pelo processo;
 */
void insertExecD(execd **done, pid_t pid, char* program, time_t sent, time_t begin, time_t end);

/** \fn void deleteExecD(execd **done)
 *  \brief Deleta toda a fila de processos executados
 * 
 *  Realiza a deleção e posterior "free" dos dados da fila de processos executados.
 * 
 *  \param execd **done; Ponteiro para estrutura do tipo execd;
 *  \return void;
 */
void deleteExecD(execd **done);

/** \fn listExecD(execd *done)
 *  \brief Lista todos os processos na fila de processos executados
 * 
 *  Realiza a impressão da informação sobre todos os processos que executaram algum programa
 *  durante toda a execução do escalonador.
 * 
 *  \param execd *done; Cópia da estrutura do tipo execd;
 *  \return void;
 */
void listExecD(execd *done);

/** \fn void createFTree(fTree **_tree)
 *  \brief Inicializador da estrutura do tipo fTree
 * 
 *  Inicializador padrão da estrutura do tipo fTree.
 * 
 *  \param fTree **_tree; Ponteiro para estrutura do tipo fTree;
 *  \return void;
 */
void createFTree(fTree **_tree);

/** \fn void definesTree(fTree **_tree, int _parent, int *_node, int _level)
 *  \brief Cria a estrtura da Fat Tree (Binary Tree)
 * 
 *  Função recursiva responsável pela criação da estrutura completa da árvore
 *  binária simbólica dos processos gerentes. Aqui serão calculadas todas as
 *  conexões que cada nó da árvore realiza, e cada nó irá receber um ID que,
 *  posteriormente, será associado a um processo gerente.
 * 
 *  \param fTree **_tree; Ponteiro para estrutura fTree;
 *  \param int _parent; ID do nó pai do nó sendo inserido na árvore;
 *  \param int *_node; Ponteiro para o número do nó sendo inserido na árvore;
 *  \param int _level; Level atual da árvore sendo construído.
 */
void definesTree(fTree **_tree, int _parent, int *_node, int _level);

/** \fn void readTree(fTree *_tree)
 *  \brief Função de leitura da árovre simbólica
 * 
 *  A função realiza a leitura de toda a arvóre simbólica criada. Esta função
 *  tem o intuito de ser apenas para debug e verificação da estrutura da árvore.
 * 
 *  \param fTree *_tree; Cópia da árvore simbólica;
 *  \return void;
 */
void readTree(fTree *_tree);

/** \fn void deleteTree(fTree **_tree)
 *  \brief Função padrão de deleção para estrutura Fat Tree (Binary Tree)
 * 
 *  \param fTree **_tree; Ponteiro para estrutura do tipo fTree;
 *  \return void;
 */
void deleteTree(fTree **_tree);

/** \fn pid_t* get_fTreeConnection(fTree *_tree, int _id)
 *  \brief Define as conexões de um dado processo gerente
 * 
 *  A função recebe a árvore simbólica gerada e o <_id> do processo gerente.
 *  Com isso, o processo gerente percorre a árvore simbólica até achar o nó com mesmo
 *  número que seu <_id> e, ao achar este nó, recupera as conexões que ele faz em forma
 *  de um vetor de tipo pid_t.
 * 
 *  \param fTree *_tree; Cópia da árvore simbólica;
 *  \param int _id; ID do processo gerente;
 *  \return pid_t*; Vetor com todas as conexões que o n-ésimo gerente faz;
 */
pid_t* get_fTreeConnection(fTree *_tree, int _id);

/** \fn void createHyperTorus(hyperTorus **_ht)
 *  \brief Inicializador da estrutura do tipo hyperTorus
 * 
 *  Inicializador padrão da estrutura do tipo hyperTorus
 * 
 *  \param hyperTorus **_ht; Ponteiro para estrutura do tipo hyperTorus;
 *  \return void;
 */
void createHyperTorus(hyperTorus **_ht);

/** \fn void definesHyper(hyperTorus **_hyper, int _id)
 *  \brief Define a estrutura simbólica Hypercube
 * 
 *  A função tem como responsabilidade definir a estrutura simbólica do tipo
 *  Hypercube. A função é chamada N vezes (no caso desse sistema, N=16), e
 *  Passa o número da chamada como argumento. O número é associado ao <_id>
 *  do gerente.
 * 
 *  Na estrutura Hypercube, cada nó realiza 4 conexões (com exceção do nó 0 que
 *  realiza uma 5a conexão com o escalonador). Essa conexões são feitas com base
 *  no número do nó. Cada nó só se conecta com nós com <_id> que tenha apenas 1
 *  bit de diferença. Para isso, cada chamada da função roda um for para todos os
 *  16 ID's possíveis na estrutura e realiza um XOR entre o <_id> do gerente e cada
 *  outro ID da estrutura. Caso o resultado seja 1, 2, 4 ou 8, significa que apenas
 *  1 bit mudou e, portanto, há conexão.
 * 
 *  As conexões são salvas em um vetor alocado em cada nó da estrutura simbólica.
 * 
 *  \param hyperTorus **_hyper; Ponteiro para estrutura do tipo hyperTorus;
 *  \param int _id; ID do n-ésimo gerente;
 */
void definesHyper(hyperTorus **_hyper, int _id);

/** \fn void definesTorus(hyperTorus **_torus, int _id)
 *  \brief Define a estrutura simbólica Torus
 * 
 *  A função, de forma semelhante a definesHyper(), tem como objetivo define a estrutura
 *  simbólica Torus. A função será chamada N vezes (no caso desse sistema, N=16), e passa
 *  o número da chamada como argumento <_id>.
 * 
 *  A regra da estrutura Torus é a seguinte:
 *  - A estrutura é composta de 4 linhas com 4 gerentes em cada linha;
 *  - A estrutura é composta de 4 colunas com 4 gerentes em cada coluna;
 *  - Dessa forma, determina-se a coluna pela conta: col = _id%4. Se o nó
 *  estiver na coluna 0, ele se conecta com o primeiro nó a direita e com o
 *  terceiro nó a direita. Se o nó estiver na coluna 1 ou 2, se conecta com
 *  o primeiro nó a esquerda e o primeiro nó a direita. Se o nó estiver na
 *  coluna 3, se conecta com o primeiro nó a esquerda e o primeiro nó da coluna.
 *  - Ainda, se o <_id> do nó subtraído de 4 for menor que 0 (nós na primeira linha)
 *  Eles se conectam com os nós na ultima linha ((_id+12)%16). Caso contrário, se
 *  conectam com o nó da linha de cima.
 *  - Por fim, se o <_id> acrescido de 4 for maior que 15 (nós da última linha),
 *  eles se conectam com os nós da primeira linha ((_id+4)%16). Caso contrário, se
 *  conectam com o nó da linha de baixo.
 * 
 *  As conexões são salvas em um vetor de cada nó da estrutura simbólica.
 * 
 *  \param hyperTorus **_torus; Ponteiro para estrutura do tipo hyperTorus;
 *  \param int _id; ID do n-ésimo gerente;
 *  \return void;
 */
void definesTorus(hyperTorus **_torus, int _id);

/** \fn void readHyperTorus(hyperTorus *ht)
 *  \brief Função de leitura da estrutura simbólica Hypercube ou Torus
 * 
 *  A função realiza a leitura de toda estrutura Hypercube ou Torus. Esta
 *  função tem por intuito apenas debug e verificação das estruturas simbólicas.
 * 
 *  \param hyperTorus *ht; Cópia da estrutura simbólica Hypercube ou Torus;
 *  \return void;
 */
void readHyperTorus(hyperTorus *ht);

/** \fn void deleteHyperTorus(hyperTorus **_ht)
 *  \brief Função padrão de deleção das estruturas Hypercube e Torus
 * 
 *  \param hyperTorus **_ht; Ponteiro para estrutura do tipo hyperTorus;
 *  \return void;
 */
void deleteHyperTorus(hyperTorus **ht);

/** \fn pid_t* get_htConnection(hyperTorus *_hyper, int _id)
 *  \brief Define as conexões de um dado processo processo gerente
 * 
 *  A função busca na estrutura simbólica, usando o <_id> como base, o nó simbólico
 *  e as conexões que o mesmo faz no Hypercube ou Torus. Após encontrar o nó simbólico
 *  de mesmo ID que <_id> a função retorna o vetor de conexões do processo gerente.
 * 
 *  \param hyperTorus *_hyper; Cópia da estrutura simbólica hyperTorus;
 *  \param int _id; ID do processo gerente;
 *  \return pid_t*; Vetor com as conexões do n-ésimo processo gerente;
 */
pid_t* get_htConnection(hyperTorus *_hyper, int _id);

/** \fn void readHTConnections(pid_t *connectios, int _id)
 *  \brief Realiza a leitura do vetor de conexões na estrutura Hypercube ou Torus
 * 
 *  A função recebe um vetor de conexões do n-ésimo procesos gerentes
 *  e imprime todas as conexões contidas no vetor. Essa função tem como objetivo apenas
 *  debug e verificação das conexões.
 * 
 *  \param pid_t *connections; Vetor de conexões de um dado processo gerente;
 *  \param int _id; ID do n-ésimo processo gerente;
 *  \return void;
 */
void readHTConnections(pid_t *connections, int _id);

/** \fn void readFTConnections(pid_t *connections, int _id)
 *  \brief Realiza a leitura do vetor de conexões na estrutura Fat Tree
 * 
 *  A função recebe um vetor de conexões do n-ésimo processos gerentes
 *  e imprime todas as conexões contidas no vetor. Essa função te m como objetivo apenas
 *  debug e verificação das conexões.
 * 
 *  \param pid_t *connections; Vetor de conexões de um dado processo gerente;
 *  \param int _id; ID do n-ésimo processo gerente;
 *  \return void;
 */
void readFTConnections(pid_t *connections, int _id);

/** \fn void createManQ(manq **_manq)
 *  \brief Inicializador padrão para estrutura do tipo manq
 * 
 *  \param manq **_manq; Ponteiro para a estrutura do tipo manq;
 *  \return void;
 */
void createManQ(manq **_manq);

/** \fn void insertManQ(manq **_manq, int _id)
 *  \brief Insere processos, na ordem, de execução.
 * 
 *  O escalonador utiliza um escalonamento bastante simples: FIFO (First In First Out).
 *  Dessa forma, a estrutura "manq" guarda, na ordem, os processos prontos para execução de
 *  um dado job.
 * 
 *  \param manq **_manq; Ponteiro para a fila do escalonamento;
 *  \param int _id; ID do n-ésimo processo gerente;
 *  \return void;
 */
void insertManQ(manq **_manq, int _id);

/** \fn manq* removeManQ(manq **_manq)
 *  \brief Remove um processo da fila de escalonamento
 * 
 *  Os processo em estado "ready" são guardados na estrutura manq. Dessa forma,
 *  quando chega a vez de um processo gerente receber a ordem de execução, o ID do processo
 *  gerente é removido da lista manq e retornado para o escalonador.
 * 
 *  \param manq **_manq; Ponteiro para estrutura do tipo manq;
 *  \return manq*; Retorna o primeiro item da fila manq;
 */
manq* removeManQ(manq **_manq);

/** \fn void readManQ(manq *_manq)
 *  \brief Realiza a leitura da fila de escalonamento.
 * 
 *  Essa função tem por objetivo apenas a debug e verificação da estrutura.
 * 
 *  \param manq *_manq; Cópia da fila manq;
 *  \return void;
 */
void readManQ(manq *_manq);