#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// test case with multithread
// created 4 threads that each try to get and free some memory
// atleast one will fail because of the system design

void *f(int size)
{
    int *x = malloc(size);
    if (x == NULL)
    {
        printf("Malloc Size %d Failed\n", size);
        return;
    }
    int i, j;
    for (i = 0; i < 10; i++)
    {
        *(x + i) = i;
    }
    printf("Pointer %p\n", x);
}

int main()
{

    int i = 0;
    for (i = 0; i < 30; i++)
    {
        pthread_t thread;
        if (pthread_create(&thread, NULL, f, 10 * sizeof(int)))
        {
            printf("Error Creating Thread 4\n");
        }
        pthread_join(thread, NULL);
    }
    printf("End Main\n");

    exit(0);
}