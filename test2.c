#include <stdio.h>
#include <stdlib.h>

// realloc
int main()
{
    int i;
    int *x = malloc(sizeof(int) * 10);
    x = realloc(x, 0);
    x = realloc(x, sizeof(int) * 10);
    for (i = 0; i < 10; i++)
    {
        *(x + i) = i;
    }

    int *y = realloc(x, sizeof(int) * 5);
    for (i = 0; i < 5; i++)
    {
        *(y + i) = i;
    }

    int *z = realloc(y, sizeof(int) * 15);
    for (i = 0; i < 15; i++)
    {
        *(z + i) = i;
    }

    free(z);

    exit(0);
}