#include "umalloc.h"
#include "csbrk.h"
#include "ansicolors.h"
#include <stdio.h>
#include <assert.h>

const char author[] = ANSI_BOLD ANSI_COLOR_RED "Anushka Shah aks4562" ANSI_RESET;

/*
 * The following helpers can be used to interact with the memory_block_t
 * struct, they can be adjusted as necessary.
 */

// A sample pointer to the start of the free list.
memory_block_t *free_head;

/*
 * is_allocated - returns true if a block is marked as allocated.
 */
bool is_allocated(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & 0x1;
}

/*
 * allocate - marks a block as allocated.
 */
void allocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc |= 0x1;
}


/*
 * deallocate - marks a block as unallocated.
 */
void deallocate(memory_block_t *block) {
    assert(block != NULL);
    block->block_size_alloc &= ~0x1;
}

/*
 * get_size - gets the size of the block.
 */
size_t get_size(memory_block_t *block) {
    assert(block != NULL);
    return block->block_size_alloc & ~(ALIGNMENT-1);
}

/*
 * get_next - gets the next block.
 */
memory_block_t *get_next(memory_block_t *block) {
    assert(block != NULL);
    return block->next;
}

/*
 * put_block - puts a block struct into memory at the specified address.
 * Initializes the size and allocated fields, along with NUlling out the next 
 * field.
 */
void put_block(memory_block_t *block, size_t size, bool alloc) {
    assert(block != NULL);
    assert(size % ALIGNMENT == 0);
    assert(alloc >> 1 == 0);
    block->block_size_alloc = size | alloc;
    block->next = NULL;
}

/*
 * get_payload - gets the payload of the block.
 */
void *get_payload(memory_block_t *block) {
    assert(block != NULL);
    return (void*)(block + 1);
}

/*
 * get_block - given a payload, returns the block.
 */
memory_block_t *get_block(void *payload) {
    assert(payload != NULL);
    return ((memory_block_t *)payload) - 1;
}

/*
 *  STUDENT TODO:
 *      Describe how you select which free block to allocate. What placement strategy are you using?
 * 
 * I chose to allocate the first free block in the free list that is larger than size. I iterate through the 
 * free list until I find a free block that is larger than the size parameter, but if I don't find a 
 * free block large enough, I set the last free block's next to a new free block that is made by extending, which
 * is the block that is used in umalloc.
 */

/*
 * find - finds a free block that can satisfy the umalloc request.
 */
memory_block_t *find(size_t size) {
    memory_block_t *prev = free_head;
    // start of free list
    memory_block_t *curr = free_head->next;
    // iterate through free list
    while (curr) {
        // if free block from free list is larger than size, return the free block's previous block
        if (get_size(curr) >= size) {
            return prev;
        } else {
            prev = curr;
            curr = curr->next;
        }
    }
    // if no block in free list is big enough, set the last block in free list's next reference
    // to a new free block
    prev->next = extend(size);
    return prev;
}

/*
 * extend - extends the heap if more memory is required.
 */
memory_block_t *extend(size_t size) {
    memory_block_t *curr;
    // csbrk for a new memory chunk based on if size or PAGESIZE is bigger
    if (size > PAGESIZE) {
        curr = csbrk(size + 16);
        put_block(curr, size + 16, 0);
    } else {
        curr = csbrk(PAGESIZE);
        put_block(curr, PAGESIZE, 0);
    }
    return curr;
}

/*
 *  STUDENT TODO:
 *      Describe how you chose to split allocated blocks. Always? Sometimes? Never? Which end?
 * 
 * I chose to split allocated blocks only when the size of the free block is more than 64 plus the size 
 * needed for the block. I chose this so that I would split large blocks, but if there are not that many more
 * bits, then I would just give it to the allocated block to avoid another method call. I split my blocks with the 
 * free end on the left and the allocated end on the right. I do this so that I do not have to rearrange the pointers
 * to the free block in the free list but can instead just change the size of the free block.
*/

/*
 * split - splits a given block in parts, one allocated, one free.
 */
memory_block_t *split(memory_block_t *block, size_t size) {
    memory_block_t *curr = block->next;
    // set the allocated block's memory address
    memory_block_t *all_block = (memory_block_t*) ((uint64_t) curr + get_size(curr) - size);
    put_block(all_block, size, 1);
    // change size of free block
    curr->block_size_alloc = get_size(curr) - size;
    return all_block;
}

/*
 * coalesce - coalesces a free memory block with neighbors.
 */
memory_block_t *coalesce(memory_block_t *block) {
    return NULL;
}

/**
 * get_prev_block gets the previous block in the free list given a 
 * free memory block
 */
memory_block_t *get_prev_block(memory_block_t *block) {
    memory_block_t *prev_block = free_head;
    // start of free list
    memory_block_t *curr_block = free_head->next;
    // iterate through free list until curr_block arrives at block's previous in free list
    while (curr_block && (uint64_t) curr_block < (uint64_t) block) {
        prev_block = curr_block;
        curr_block = curr_block->next;
    }
    return prev_block;
}

/*
 * uinit - Used initialize metadata required to manage the heap
 * along with allocating initial memory.
 */
int uinit() {
    // initialize free_head to an allocated block that is never removed
    free_head = csbrk(16);
    put_block(free_head, 16, 1);
    // set free_head's next to the start of the actual free list
    free_head->next = csbrk(PAGESIZE * 4);
    put_block(free_head->next, PAGESIZE * 4, 0);
    return 0;
}

/*
 * umalloc -  allocates size bytes and returns a pointer to the allocated memory.
 */
void *umalloc(size_t size) {
    // align size and add 16 for header
    size = ALIGN(size) + 16;
    memory_block_t *prev = find(size);
    // call split if size of memory block is large enough
    if (get_size(prev->next) > size + 64) {
        prev = split(prev, size);
    } else {
        // set the pointers to remove prev from free list
        memory_block_t *curr = prev->next;
        prev->next = curr->next;
        prev = curr;
    }
    allocate(prev);
    return get_payload(prev);
}

/*
 *  STUDENT TODO:
 *      Describe your free block insertion policy.
 * 
 * My free block insertion policy is that after deallocated the memory block, I find the previous block in the free
 * list based on the memory address of the deallocated block. In the get_prev_block method, I iterate through the free
 * list until I get the last free block address that is less that the deallocated block's address. Then, in the free method,
 * I set the block's next to the previous block's next and set the previous block's next to the deallocated block, which adds
 * the deallocated block to the free list.
*/

/*
 * ufree -  frees the memory space pointed to by ptr, which must have been called
 * by a previous call to malloc.
 */
void ufree(void *ptr) {
    memory_block_t *block = get_block(ptr);
    deallocate(block);
    // get the previous memory block in free list
    memory_block_t *prev = get_prev_block(block);
    // add block back to free list
    block->next = prev->next;
    prev->next = block;
}