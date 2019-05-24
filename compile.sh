gcc -Wall -c dataStructures.c managerProcess.c msgQueue.c

gcc -Wall escalonador.c dataStructures.o managerProcess.o msgQueue.o -o escalonador

gcc -Wall execucao_postergada.c msgQueue.o -o execucao_postergada

gcc -Wall shutdown.c msgQueue.o -o shutdown

gcc -Wall hello.c -o hello

rm *.o