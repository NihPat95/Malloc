#include "mybuddy.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#define USED 0
#define UNUSED 1
#define SPLIT 2
#define FULL 3

extern int errno;

struct buddy_stats
{
	int free_node;
	int used_node;
	int used_size;
};

struct my_buddy
{
	int level;
	int *nodes;
	void *startAddress;
};

static int offset(int nodeid, int total_size, int parent_id, int parent_offset)
{
	if (nodeid == parent_id)
	{
		return parent_offset;
	}
	if (parent_id * 2 + 1 > nodeid)
		return -1;
	int x1 = offset(nodeid, total_size / 2, parent_id * 2 + 1, parent_offset);
	int x2 = offset(nodeid, total_size / 2, parent_id * 2 + 2, parent_offset + (total_size / 2));
	if (x1 == -1 && x2 == -1)
	{
		return -1;
	}
	return (x1 == -1) ? x2 : x1;
}

int node_size(int nodeid, int total_size, int parent_id)
{
	if (nodeid == parent_id)
	{
		return total_size;
	}
	if (parent_id * 2 + 1 > nodeid)
		return -1;
	int x1 = node_size(nodeid, total_size / 2, parent_id * 2 + 1);
	int x2 = node_size(nodeid, total_size / 2, parent_id * 2 + 2);
	if (x1 == -1 && x2 == -1)
	{
		return -1;
	}
	return (x1 == -1) ? x2 : x1;
}

static int no_of_node(int level)
{
	// minimum level must be 3
	// so to allocate a page of minimum 8 size bytes
	int sum = 1, c = 1;
	int size = 1 << level;
	while (size != 8)
	{
		c = c * 2;
		sum = sum + c;
		size = size / 2;
	}
	return sum;
}
static int get_nearest_power_two(int s)
{
	int i = 0, c = 0;
	while (1)
	{
		c = 1 << i;
		if (c >= s)
		{
			return c;
		}
		i++;
	}
}
static int get_page_size(int size, int total_size)
{
	if (size == 0)
	{
		return -1;
	}
	if (size <= 8)
	{
		return 8;
	}
	int align_size = get_nearest_power_two(size);
	if (align_size > total_size)
	{
		return -1;
	}
	else
		return align_size;
}

struct my_buddy *create_buddy(int l)
{
	int nodes_size = no_of_node(l);
	int s = (sizeof(struct my_buddy) + (sizeof(int) * nodes_size));
	struct my_buddy *buddy = sbrk(s);
	buddy->level = l;
	int i;
	buddy->nodes = sbrk(sizeof(int) * nodes_size);
	for (i = 0; i < nodes_size; i++)
	{
		buddy->nodes[i] = UNUSED;
	}
	void *sAddr = sbrk(1 << l);

	if (sAddr == (void *)-1)
	{
		return NULL;
	}
	buddy->startAddress = sAddr;
	return buddy;
}

void clean_buddy(struct my_buddy *b)
{
	int nodes_size = no_of_node(b->level);
	int i = 0;
	for (i = 0; i < nodes_size; i++)
	{
		b->nodes[i] = UNUSED;
	}
}

static int left_child(int p)
{
	return p * 2 + 1;
}
static int right_child(int p)
{
	return left_child(p) + 1;
}

static void check_parent_for_full(struct my_buddy *b, int i)
{
	if (i < 0)
	{
		return;
	}

	if (b->nodes[0] == USED)
	{
		b->nodes[0] = FULL;
		return;
	}

	int parent = (i % 2 == 0) ? (i / 2 - 1) : (i - 1) / 2;
	int left = left_child(parent);
	int right = right_child(parent);
	if ((b->nodes[left] == USED || b->nodes[left] == FULL) &&
		(b->nodes[right] == USED || b->nodes[right] == FULL))
	{
		b->nodes[parent] = FULL;
		check_parent_for_full(b, parent);
	}
}

void *buddy_malloc(struct my_buddy *b, int s)
{
	int size = s + (2 * sizeof(int));
	int x = buddy_alloc_memory(b, size);
	if (x == -1)
	{
		return NULL;
	}
	else
	{
		//write the value of node in the memory
		int *address = b->startAddress + offset(x, 1 << b->level, 0, 0);
		memcpy((void *)address, &x, sizeof(x));
		address = address + 1;
		memcpy((void *)address, &s, sizeof(x));
		address = address + 1;
		return (void *)address;
	}
}

int buddy_alloc_memory(struct my_buddy *b, int s)
{
	int page_size = get_page_size(s, 1 << b->level);
	if (page_size == -1)
		return -1;
	int i = 0;
	int size = 1 << b->level;
	while (i >= 0)
	{
		if (size == page_size)
		{
			if (b->nodes[i] == UNUSED)
			{
				// unused node make it used
				b->nodes[i] = USED;
				check_parent_for_full(b, i);
				return i;
			}
		}
		else
		{
			if (b->nodes[i] == USED)
			{
				// do nothing
			}
			else if (b->nodes[i] == SPLIT)
			{
				// check for child
				i = i * 2 + 1;
				size = size / 2;
				continue;
			}
			else if (b->nodes[i] == UNUSED)
			{
				// create two child
				b->nodes[i] = SPLIT;
				b->nodes[left_child(i)] = UNUSED;
				b->nodes[right_child(i)] = UNUSED;
				i = i * 2 + 1;
				size = size / 2;
				continue;
			}
		}
		if (i % 2 != 0)
		{
			// check right child
			i++;
			continue;
		}
		for (;;)
		{
			// check parent
			size = size * 2;
			i = (i + 1) / 2 - 1;
			if (i < 0)
				return -1;
			if (i % 2 != 0)
			{
				i++;
				break;
			}
		}
	}
	return -1;
}

static void check_for_combine(struct my_buddy *b, int i)
{
	if (i == 0)
	{
		return;
	}
	int parent = (i % 2 == 0) ? (i / 2 - 1) : (i - 1) / 2;
	int left = left_child(parent);
	int right = right_child(parent);
	if (b->nodes[left] == UNUSED && b->nodes[right] == UNUSED)
	{
		b->nodes[parent] = UNUSED;
		check_for_combine(b, parent);
	}
	else
	{
		return;
	}
}

static void set_parent_after_free(struct my_buddy *b, int i)
{
	if (i == 0)
	{
		if (b->nodes[0] == FULL)
		{
			b->nodes[0] = SPLIT;
		}
		return;
	}
	else
	{
		int parent = (i % 2 == 0) ? (i / 2 - 1) : (i - 1) / 2;
		if (b->nodes[parent] == FULL)
		{
			b->nodes[parent] = SPLIT;
			set_parent_after_free(b, parent);
		}
	}
}

void buddy_free(struct my_buddy *b, void *address)
{
	address = ((int *)address - 2);

	void *sp = b->startAddress;
	void *ep = sp + (1 << b->level);

	if (address >= sp && address <= ep)
	{
		int nodeid;
		memcpy(&nodeid, (void *)address, sizeof(nodeid));
		buddy_free_memory(b, nodeid);
	}
	else
	{
		//bad pointer
	}
}

void buddy_free_memory(struct my_buddy *b, int i)
{
	b->nodes[i] = UNUSED;
	check_for_combine(b, i);
	set_parent_after_free(b, i);
}

struct buddy_stats *getBuddyInfo(struct my_buddy *b)
{
	struct buddy_stats *bi = malloc(sizeof(struct buddy_stats));
	int i = 0;
	bi->free_node = 0;
	bi->used_node = 0;
	bi->used_size = 0;
	int total_nodes = no_of_node(b->level);
	for (i = 0; i < total_nodes; i++)
	{
		if (b->nodes[i] == USED)
		{
			bi->used_node++;
			int nodeSize = node_size(i, 1 << b->level, 0);
			bi->used_size = bi->used_size + nodeSize;
		}
		else if (b->nodes[i] == UNUSED)
		{
			bi->free_node++;
		}
	}
	return bi;
}

int buddy_free_node(struct buddy_stats *b)
{
	return b->free_node;
}
int buddy_used_node(struct buddy_stats *b)
{
	return b->used_node;
}
int buddy_used_size(struct buddy_stats *b)
{
	return b->used_size;
}

void print_stats(struct my_buddy *b)
{
	int nodes_size = no_of_node(b->level);
	int i;
	for (i = 0; i < nodes_size; i++)
	{
		if (b->nodes[i] == USED)
		{
			printf("%d USED\n", i);
		}
		if (b->nodes[i] == UNUSED)
		{
			printf("%d UNUSED\n", i);
		}
		if (b->nodes[i] == SPLIT)
		{
			printf("%d SPLIT\n", i);
		}
		if (b->nodes[i] == FULL)
		{
			printf("%d FULL\n", i);
		}
	}
}