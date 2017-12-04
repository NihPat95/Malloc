#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// test case with multithread
// created 4 threads that each try to get and free some memory
// atleast one will fail because of the system design

void *f(int size)
{
    int *x = malloc(size);
    printf("\t\t\t\t%p\n", x);
    if (x == NULL)
    {
        printf("Malloc Size %d Failed\n", size);
        return;
    }
    int i, j;
    for (i = 0; i < 10000; i++)
    {
        for (j = 0; j < 10000; j++)
        {
            //delay loop
        }
    }
    free(x);
}

int main()
{
    pthread_t thread1, thread2, thread3, thread4;

    printf("=== Creating Thread 1\n");
    if (pthread_create(&thread1, NULL, f, 100))
    {
        printf("Error Creating Thread 1\n");
    }

    printf("=== Creating Thread 2\n");
    if (pthread_create(&thread2, NULL, f, 200))
    {
        printf("Error Creating Thread 2\n");
    }

    printf("=== Creating Thread 3\n");
    if (pthread_create(&thread3, NULL, f, 300))
    {
        printf("Error Creating Thread 3\n");
    }

    printf("=== Creating Thread 4\n");
    if (pthread_create(&thread4, NULL, f, 400))
    {
        printf("Error Creating Thread 4\n");
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    pthread_join(thread3, NULL);
    pthread_join(thread4, NULL);

    printf("=== After All Thread Join\n");
    malloc_stats();

    exit(0);
}