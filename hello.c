#include <stdio.h>
#include <unistd.h>

#define SLEEP_TIME 5

int main (){
    /*
    The program is meant to be used by the scheduler
    Sleep time can be adjusted by replacing the macro SLEEP_TIME

    Something interesting about printf: it just flushes[1] when new
    line is reached.

    source: https://stackoverflow.com/a/1716621
    [1] flush is to actually print something into the screen rather
    than waiting the buffer limit to reach
    */
    printf("Hello, hello, will wait for %d seconds\n", SLEEP_TIME);
    sleep(SLEEP_TIME);
}