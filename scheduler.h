#include <stdio.h>          // Includes the Stardard I/O Library
#include <stdlib.h>         // Includes the Standard C Library
#include <unistd.h>         // Includes the POSIX operating system API
#include <sys/syscall.h>    // Includes the System Calls numbers
#include <sys/types.h>      // Includes the System Primitive Data Types
#include <sys/wait.h>       // Includes the System Wait definitions
#include <sys/mman.h>       // Includes BSD Memory Management Library
#include <sys/stat.h>       // Includes POSIX File Characteristics Library
#include <signal.h>         // Includes the Signal definitions
#include <string.h>         // Includes the String Library
#include <fcntl.h>

typedef struct processTable {
    pid_t pid;              // Process ID
    uid_t uid;              // Group ID
    pid_t ppid;             // Process Parent ID
    // CP: Fator de utilização da CPU
    // PRI: Prioridade do processo
    // NI: Parâmetro para o escalonador
    // SZ: Tamanho do segmento de dados + pilha
    // RSS: Memória real utilizada pelo processo
    // WCHAN: Evento pelo qual o processo está esperando
    // STAT: Status de execução do processo
    // TT: Terminal associado ao processo
    // TIME: Tempo de CPU (user + system)
    // COMMAND: Arquivo executável que gerou o processo
} processTable;