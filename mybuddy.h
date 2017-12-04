#ifndef BUDDY_ALLOCATION
#define BUDDY_ALLOCATION

struct buddy_stats;
struct my_buddy;

struct buddy_stats *getBuddyInfo(struct my_buddy *b);
struct my_buddy *create_buddy(int level);
void free_buddy(struct my_buddy *);
void buddy_free(struct my_buddy *, void *ptr);
int buddy_alloc_memory(struct my_buddy *, int size);
void *buddy_malloc(struct my_buddy *, int size);
void buddy_free_memory(struct my_buddy *, int i);
void clean_buddy(struct my_buddy *b);
int buddy_free_node(struct buddy_stats *b);
int buddy_used_node(struct buddy_stats *b);
int buddy_used_size(struct buddy_stats *b);

#endif