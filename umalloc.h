#include <stdlib.h>
#include <stdbool.h>

#define ALIGNMENT 16 /* The alignment of all payloads returned by umalloc */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

/*
 * memory_block_t - Represents a block of memory managed by the heap. The 
 * struct can be left as is, or modified for your design.
 * In the current design bit0 is the allocated bit
 * bits 1-3 are unused.
 * and the remaining 60 bit represent the size.
 */
typedef struct memory_block_struct {
    size_t block_size_alloc;
    struct memory_block_struct *next;
} memory_block_t;

// Helper Functions, this may be editted if you change the signature in umalloc.c

/*
*  STUDENT TODO:
*      Write 1-2 sentences for each function explaining what it does. Don't just repeat the name of the function back to us.
*/

/*
is_allocated:
This method compares the 64 bits from block_size_alloc to 0x1 using the & bitwise operator. This means that
if the last bit from block_size_alloc is 1, this method returns true, indicating that the memory block is 
allocated, or else it will return false.
*/
bool is_allocated(memory_block_t *block);

/*
allocate:
This method compares he 64 bits from block_size_alloc to 0x1 using the | bitwise operator and sets the
result back to block_size_alloc. This changes the last bit from block_size_alloc to a 1, making the
block allocated.
*/
void allocate(memory_block_t *block);

/*
deallocate:
This method compares he 64 bits from block_size_alloc to ~0x1 using the & bitwise operator and sets the
result back to block_size_alloc. This changes the last bit from block_size_alloc back to a 0 because of the
~ operator in front of 0x1, and makes the block not allocated.
*/
void deallocate(memory_block_t *block);

/*
get_size:
Alignment - 1 is equal to 01111 and so the 4 least significant bits are 1. ~(Alignment - 1) changes all the
0s to 1s and the 1s to 0s, and using the & operator makes the 4 least significant bits in block_size_alloc into
0, so we can get the size of the memory block without the bit stating if it allocated.
*/
size_t get_size(memory_block_t *block);

/*
get_next:
Returns the next pointer in the memory block. This is defined in the memory_block_t struct and is
called by using -> from block.
*/
memory_block_t *get_next(memory_block_t *block);

/*
put_block:
Set the last bit in block_size_alloc to 1 using the | operator because the last four
least significant digits in size are 0 are alloc is 1, so this makes the least significant
digit 1, indicating the block is allocated. Also, set the next block to null.
*/
void put_block(memory_block_t *block, size_t size, bool alloc);

/*
get_payload:
Get a pointer to where the actual data is and skip over the header information in block_size_alloc
and the next pointer. Adding block + 1 skips over the 16 bytes in the block and straight to the data.
*/
void *get_payload(memory_block_t *block);

/*
get_block:
Goes back from the header pointer and gets a pointer to the block. Subtracting block - 1 goes back 16 bytes to
retrieve the block information.
*/
memory_block_t *get_block(void *payload);

/*
find:
Iterates through the free list and if the size of the free memory block in the free list is greater than the
size needed then using another if loop, check if the size of the free memory block is big enough that it is
worth splitting, else just return pointer to memory block. If no memory block in the free list is large enough,
return null.
*/
memory_block_t *find(size_t size);
/*
extend:
If size is greater than PAGESIZE * 3, call put_block with (size/PAGESIZE) * 3; else call put_block with PAGESIZE * 3
Then, get to the end of the free list and set the next value to the new memory block created by csbrk.
*/
memory_block_t *extend(size_t size);
/*
split:
Align size, set the position of the free_block from the block that is being split. Add the block with correct
size into memory. Set the allocated block to the smaller size. Set pointers if the previous block is free_head.
Return the smaller block that is being allocated.
*/
memory_block_t *split(memory_block_t *block, size_t size);
memory_block_t *coalesce(memory_block_t *block);
/*
Iterate through the free list while the memory address of the block in the free list is less than the memory
address of the block parameter. 
*/
memory_block_t *get_prev_block(memory_block_t *block);


// Portion that may not be edited
int uinit();
void *umalloc(size_t size);
void ufree(void *ptr);