/*  create_process.c
*   A simple piece of code to create a process using win32 API
*   Gabriel Lepinay
*   2025-05-05
*/

#include <windows.h>
#include <stdio.h>

int main(void) {
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;

    if (!CreateProcessW(
        L"C:\\Windows\\System32\\notepad.exe",
        NULL,
        NULL,
        NULL,
        FALSE,
        BELOW_NORMAL_PRIORITY_CLASS,
        NULL,
        NULL,
        &si,
        &pi
    )) {
        printf("Error while creating process: %ld", GetLastError());
    }
    
    printf("Process sucessfully created with PID: %ld", pi.dwProcessId);
    return 0;
}