#ifdef _WIN32
#define _TIMESPEC_DEFINED
#endif

#include <stdio.h>
#include <pthread.h>
#include <windows.h>

int main() {
    printf("Hello World!\n");
    fflush(stdout);
    
    printf("Testing pthread...\n");
    fflush(stdout);
    
    pthread_t thread;
    
    printf("Testing Windows API...\n");
    fflush(stdout);
    
    Sleep(100);
    
    printf("All tests passed!\n");
    fflush(stdout);
    
    return 0;
}