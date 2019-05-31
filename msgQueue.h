#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/stat.h>

//! Chave para o canal de comunicação entre escalonador, execucao_postergada e shutdown.
#define MQ_SD 140080279
//! Chave para o canal de comunicação entre escalonador e gerentes.
#define MQ_SM 0x8349
//! Chave para o canal de envio de Jobs
#define MQ_SJ 1400802792

//! Pacote de mensagem utilizado para comunicação entre escalonador, execucao_postergada e gerentes.
typedef struct msg_packet {
    //! Define o tipo da mensagem
    long type;

    //! Nome do programa a ser escalonado
    char name[256];
    //! Delay (in seconds) for execution
    int  delay;
    
    //! ID do gerente destino
    int _mdst;
    //! ID do gerente para utilização do escalonador
    int _id;
    //! Flag de finalização
    int finish;

    //! PID do filho do gerente
    pid_t  pid;
    //! Hora de início da execução do filho do gerente
    time_t begin;
    //! Hora de término da execução do filho do gerente
    time_t end;
} msg_packet;

//! Pacote de mensagem para comunicação entre escalonador, execucao_postergada e shutdown
typedef struct pid_packet{
    //! Tipo da mensagem
    long type;
    //! PID do escalonador
    pid_t pid;
} pid_packet;

/** \fn int create_channel(int key)
 *  \brief Função para criar filas de mensagens
 *  
 *  A função é responsável por criar canais de comunicação (fila de mensages) 
 *  com base em uma chave <key>. O msgget é executado utilizando a <key> fornecida
 *  e utilizando a flag de criação <b>IPC_CREAT</b> e <b>IPC_EXCL</b> que torna o 
 *  canal único para a chave fornecida.
 *  
 *  \param int key; Chave para criação de uma fila de mensagens.
 *  \return int; Retorna o identificador da fila ou -1 em caso de erro.
 */
int create_channel(int key);

/** \fn int get_channel(int key)
 *  \brief Retorna o identificador de uma fila de mensagens
 * 
 *  A função é responsável por obter o identificador de uma fila de mensagens com base
 *  na chave <key>. Caso a fila já tenha sido criada, é retornado o identificador da
 *  mesma. Caso contrário, é retornado -1.
 * 
 *  \param int key; Chave identificadora da fila de mensagens
 *  \return int; Retorna o identificador da fila ou -1 em caso de erro.
 */
int get_channel(int key);

/** \fn int delete_channel(int msg_id)
 *  \brief Delata uma fila de mensagens
 * 
 *  A função recebe um identificador <msg_id> para deletar uma fila. Caso o identificador
 *  seja negativo, a função retorna -1 pois a fila não existe. Caso contrário, é executado
 *  msgctl para remover a fila identificada por <msg_id>. Retorna 0 em caso de sucesso ou
 *  -1 em caso de erro.
 * 
 *  \param int msg_id; Identificador da fila de mensagens
 *  \return int; Retorna 0 em caso de sucesso ou -1 em caso de erro.
 */
int delete_channel(int msg_id);