/*  process.c
*   Just a simple infinite print process 
*   Gabriel Lepinay
*   2025-05-08
*/
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

int main(int ac, char *av[]) {
    int seconds = 0;

    while (true) {
        printf("[%d] Process (%d) is running...\n", seconds, getpid());
        seconds++;
        sleep(5);
    }
}