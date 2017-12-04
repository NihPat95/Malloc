#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// call simple malloc and free

int main()
{

	int i = 0;
	int *x = malloc(sizeof(int) * 10);
	for (i = 0; i < 10; i++)
	{
		*(x + i) = i;
	}
	free(x);

	int *y = malloc(1 << 20);
	free(y);

	int *z = calloc(5, sizeof(int));

	for (int k = 0; k < 5; k++)
	{
		printf("%d ", (*(z + k)));
	}

	free(z);

	return 0;
}
