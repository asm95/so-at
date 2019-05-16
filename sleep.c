#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/signal.h>

void exits(){
    // exit(0);
}

int main(){
    signal(SIGUSR1, exits);
    sleep(900);
    exit(0);
}