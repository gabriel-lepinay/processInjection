/*  injector_lin.c
*   A simple memory injector using ptrace 
*   Gabriel Lepinay
*   2025-05-08
*/

#include <stdio.h>
#include <sys/ptrace.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

union data_chunk {
    long val;
    char bytes[sizeof(long)];
} chunk;

char payload[] = "";

int get_pid(char *av[]) {
    int pid = atoi(av[1]);

    if (pid == 0)
        return -1;
    return pid; 
}

void read_mem(pid_t target_pid, long addr, char *buffer, int len) {
    int i = 0;
    int remaining = 0;

    while (i < len / sizeof(long)) {
        chunk.val = ptrace(PTRACE_PEEKDATA, target_pid, addr + i * sizeof(long), NULL);
        memcpy(buffer + i * sizeof(long), chunk.bytes, sizeof(long));
        i++;
    }

    remaining = len % sizeof(long);
    if (remaining) {
        chunk.val = ptrace(PTRACE_PEEKDATA, target_pid, addr + i * sizeof(long), NULL);
        memcpy(buffer + i * sizeof(long), chunk.bytes, remaining);
    }
}

void write_mem(pid_t target_pid, long addr, char *buffer, int len) {
    int i = 0;
    int remaining = 0;

    while (i < len / sizeof(long)) {
        memcpy(chunk.bytes, buffer + i * sizeof(long), sizeof(long));
        ptrace(PTRACE_POKEDATA, target_pid, addr + i * sizeof(long), chunk.val);
        i++;
    }
    remaining = len % sizeof(long);

    if (remaining) {
        memcpy(chunk.bytes, buffer + i * sizeof(long), remaining);
        ptrace(PTRACE_POKEDATA, target_pid, addr + i * sizeof(long), chunk.val);
    }
}
int main(int ac, char *av[]) {
    size_t payload_len = sizeof(payload) - 1;
    char mem_buffer[payload_len];
    struct user_regs_struct old_regs;

    if (ac != 2) {
        fprintf(stderr, "[-] Invalid command\nUsage: injector <pid>\n\t pid: int\n");
        return 1;
    }

    int pid = get_pid(av);
    if (pid == -1) {
        fprintf(stderr, "[-] Invalid PID\n");
        return 1;
    }

    if (mem_buffer == NULL) {
        perror("[-] Memory allocation failed");
        return 1;
    }

    printf("[i] Attaching to process.. ", pid);
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
        perror("KO\nError PTRACE_ATTACH");
        return 1;
    }
    printf("OK (PID:%d)\n", pid);
    waitpid(pid, NULL, 0);
    
    printf("[i] Loading target process registers.. ");
    if (ptrace(PTRACE_GETREGS, pid, NULL, &old_regs) == -1) {
        perror("KO\nError PTRACE_GETREGS");
        return 1;
    }
    printf("OK (RIP: %lx)\n", old_regs.rip);

    printf("[i] Backing up target process memory.. ");
    read_mem(pid, old_regs.rip, mem_buffer, payload_len);
    printf("OK\n");
    
    printf("[i] Writing payload to target process memory.. ");
    write_mem(pid, old_regs.rip, payload, payload_len);
    printf("OK\n");

    printf("[i] Executing payload.. ");
    if (ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
        perror("[-] Error PTRACE_CONT");
        return 1;
    }
    wait(NULL);
    printf("DONE\n");

    printf("[i] Restoring original process memory.. ");
    write_mem(pid, old_regs.rip, mem_buffer, payload_len);
    printf("OK\n");

    printf("[+] Injection completed\n");
    return 0;
}
