#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
// testcase with one fork call and one thread calling malloc and free some data multiple times

void *f()
{
    int i = 0;
    for (i = 0; i < 1000; i++)
    {
        int *x = malloc(100);
        if (x == NULL)
        {
            printf("Failed Thread Malloc\n");
            continue;
        }
        else
        {
            free(x);
        }
    }
}

int main()
{

    pthread_t thread1;

    printf("Before Creating Thread\n");

    if (pthread_create(&thread1, NULL, f, NULL))
    {
        printf("Error Creating Thread\n");
    }

    pthread_join(thread1, NULL);
    printf("=== Thread Created\n");
    malloc_stats();

    if (fork() == 0)
    {
        f();
    }
    else
    {
        f();
    }

    printf("=== After Fork\n");
    malloc_stats();

    exit(0);
}