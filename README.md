# Malloc
This repo contains the file of a working malloc library createad as a class assignment.<br>
The malloc library supports all the basic features that are supported by lib.c malloc.<br>
The malloc supports multiple threads, forking and much more.<br>

### How To Use 

- Download in the current folder
- Open Terminal
- Before Running the executable include this library using LD_PRELOAD
```sh
$ LD_PRELOAD=./lib.so ./executableCode</li>
```
### Working
In this design the malloc creates an arena for any new request if the no_of_arena are less than the max arena size
if the no_of_arena are equal to max arena, malloc check for any free arena to assign this request 
if no free arena found malloc forwards the request to mmap to handle

>------------------------------------------------------- initial sbrk
>
>
>		Arena 1
>
>			mybuddy
>
> -------------------------------------------------------
>
>
>		Arena 2
>			
>			mybuddy
>
> ------------------------------------------------------
>
>
>		Arena 3
>
>			mybuddy
>
>
> ------------------------------------------------------
>
>
>		Arena 4
>
>			mybuddy
>
>
> ------------------------------------------------------  sbrk



Each arena has its own buddy implementation that handles the malloc and free request for that particular arena

Before describing the library that handles the malloc,
let me explain the working of mybuddy
- the struct my_buddy takes 
  - level - the size of the buddy in power of two
  - startAddress - the memory start address of the free arena associated with that buddy
  - nodes - the nodes of the binary buddy tree

- create_buddy
	- this method not only creates a buddy but also get the memory from system using sbrb for the arena to use
- buddy_malloc
	- this method calls the buddy_alloc_memory that finds and returns a suitable nodeid to used for the allocation
	given the node id we calculate the offset from startAddress of that arena 
	and return the user that adress+2
    we also store the nodeid and size before giving the address</p>
- buddy_free 
	- given a ptr to free we check if its is null first
	(encountered this is issue because the glibc sends ptr null to free after calling printf function)
	check if the ptr is valid ie. in the range of area associated with the buddy 
	if yes read the node id from the previous address of ptr to get the node to free
	call the buddy_free_memory to set that node unused and check for parents merges and other sanity checks
### the working of lib.c 

the struct arena takes 
	- my_buddy* - its own buddy 
	- next* - address of next area, null for last 
	- free_calls - count of free calls of that arena 
	- mem_calls  - count of malloc call of that arena 
	- lock - arena lock 
- malloc 
	- any request greater than page size or if the arena are full then that request is handled by mmap when there is a first request of malloc it see if the head and tail ptr that is my linked list is empty yes then create a arena and set the thread specific arena to this any subsequent call thread can call its thread specific areana to allocate memory use the global lock before creating a buddy and free the lock after creating a buddy to make sure only one thread at time can create arena use the newly created arena for use and forward the malloc request to the buddy of that arena 
	- if the no_of_arena is equal to max arena check all arena to see if there is a free arena if yes associate that arena to the calling thread and forward the request to its buddy
	- if no return null to the calling malloc request 
	- if possible the size of arena is less than 4 then for a new thread create a new arena and add it to the link list
	and forward the malloc call to its buddy
- free 
	- find the arena associated with the thread calling free forward the request to that arena's buddy
