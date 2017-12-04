#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "mybuddy.h"
#include <signal.h>
#include <sys/mman.h>

#define MAXARENA 6

pthread_key_t tidKey;
pthread_mutex_t global_lock;

#define PAGE_SIZE 18

int no_of_arenas = 0;
int arena_used = 0;
__thread struct arena *my_arena = NULL;

struct arena
{
    pthread_t thread_id;
    pthread_mutex_t lock;
    struct my_buddy *b;
    struct arena *next;
    int used;
};

void des(void *b)
{
    pthread_mutex_lock(&global_lock);
    //    printf("Thread Ending\n");
    if (my_arena != NULL)
    {
        my_arena->used = 0;
        arena_used--;
        clean_buddy(my_arena->b);
    }
    pthread_mutex_unlock(&global_lock);
    //    printf("Arena ptr %p\n", my_arena);
}

struct arena *head = NULL, *tail = NULL;

struct arena *create_arena()
{
    //create a new arena
    struct arena *a = sbrk(sizeof(struct arena));
    pthread_mutex_init(&a->lock, NULL);
    a->b = create_buddy(PAGE_SIZE);
    a->next = NULL;
    a->used = 1;
    arena_used++;
    if (a->b == NULL)
    {
        return NULL;
    }
    return a;
}

void *try_mmap(size_t size)
{
    int *ptr;
    int s;
    ptr = mmap(NULL, size + (2 * sizeof(s)), PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr != MAP_FAILED)
    {
        *ptr = -1;
        ptr = ptr + 1;
        *ptr = (int)size;
        ptr = ptr + 1;
    }
    else
    {
        ptr = NULL;
    }
    return ptr;
}

void *malloc(size_t size)
{
    int flag = 1;
    int r;
    if (size == 0)
    {
        return NULL;
    }
    if (my_arena == NULL)
    {
        r = pthread_key_create(&tidKey, des);
    }
    if (pthread_getspecific(tidKey) == NULL)
    {
        pthread_setspecific(tidKey, (void *)(&flag));
    }

    void *ptr = NULL;
    if (size > 1 << PAGE_SIZE)
    {
        ptr = try_mmap(size);
        return ptr;
    }

    if (my_arena == NULL)
    {
        pthread_mutex_lock(&global_lock);
        if (no_of_arenas < MAXARENA)
        {
            my_arena = create_arena();
            if (head == NULL && tail == NULL)
            {
                head = my_arena;
                tail = my_arena;
            }
            else
            {
                tail->next = my_arena;
                tail = my_arena;
            }
            no_of_arenas++;
            pthread_mutex_lock(&my_arena->lock);
            ptr = buddy_malloc(my_arena->b, size);
            pthread_mutex_unlock(&my_arena->lock);
        }
        else
        {
            struct arena *current = head;
            while (current->next != NULL)
            {
                if (current->used == 0)
                {
                    my_arena = current;
                    my_arena->used = 1;
                    arena_used++;
                    pthread_mutex_lock(&my_arena->lock);
                    ptr = buddy_malloc(my_arena->b, size);
                    pthread_mutex_unlock(&my_arena->lock);
                }
                else
                {
                    current = current->next;
                }
            }
        }
        pthread_mutex_unlock(&global_lock);
    }
    else
    {
        pthread_mutex_lock(&my_arena->lock);
        ptr = buddy_malloc(my_arena->b, size);
        pthread_mutex_unlock(&my_arena->lock);
    }
    if (ptr == NULL)
    {
        ptr = try_mmap(size);
    }
    return ptr;
}

void free(void *p)
{
    if (p == NULL)
    {
        // do nothing
        return;
    }
    int *ptr = (int *)p;
    ptr = ptr - 1;
    int size = *ptr;
    ptr = ptr - 1;
    int t = *ptr;
    if (t == -1)
    {
        munmap(ptr, size + (2 * sizeof(size)));
    }
    else
    {
        if (my_arena != NULL)
            buddy_free(my_arena->b, p);
    }
}

void *realloc(void *p, size_t size)
{
    if (p == NULL)
    {
        return malloc(size);
    }
    if (size == 0)
    {
        free(p);
        return NULL;
    }

    int *oldptr = p;
    int s;
    oldptr--;
    memcpy(&s, (void *)oldptr, sizeof(int));
    oldptr++;
    if (s > size)
    {
        int *p = malloc(size);
        if (p == NULL)
        {
            return p;
        }
        else
        {
            memcpy(p, (void *)oldptr, size);
            free(oldptr);
            return p;
        }
    }
    else if (s <= size)
    {
        int *p = malloc(size);
        if (p == NULL)
        {
            return p;
        }
        else
        {
            memcpy(p, oldptr, s);
            free(oldptr);
            return p;
        }
    }
    return NULL;
}