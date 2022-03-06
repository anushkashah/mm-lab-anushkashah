
#include "umalloc.h"
#include "csbrk.h"

#include <stdio.h>

//Place any variables needed here from umalloc.c or csbrk.c as an extern.
extern memory_block_t *free_head;
extern sbrk_block *sbrk_blocks;
int counter = 0;

/*
 * check_heap -  used to check that the heap is still in a consistent state.
 
 * STUDENT TODO:
 * Required to be completed for checkpoint 1:
 *      - Check that pointers in the free list point to valid free blocks. Blocks should be within the valid heap addresses: look at csbrk.h for some clues.
 *        They should also be allocated as free.
 *      - Check if any memory_blocks (free and allocated) overlap with each other. Hint: Run through the heap sequentially and check that
 *        for some memory_block n, memory_block n+1 has a sensible block_size and is within the valid heap addresses.
 *      - Ensure that each memory_block is aligned. 
 * 
 * Should return 0 if the heap is still consistent, otherwise return a non-zero
 * return code. Asserts are also a useful tool here.
 */
int check_heap() {
    memory_block_t *cur = free_head;
    while (cur) {
        // Check 1: check that all the blocks in the free list are marked as free
        // if the block is allocated, return -1
        if (is_allocated(cur)) {
            return -1;
        } 
        sbrk_block *curr_block = sbrk_blocks;
        while (curr_block) {
            // Check 2: check that the block is located within valid heap addresses and is not 
            // outside the sbrk block if block is outside every sbrk block, then return -1
            if ((uint64_t) cur >= curr_block->sbrk_start || (uint64_t) cur + get_size(cur) <= curr_block->sbrk_end) {
                break;
            } else if (curr_block->next == NULL) {
                return -1;
            } else {
                curr_block = curr_block->next;
            }
        }
        // Check 3: checks to make sure that free blocks don't overlap other free blocks
        // if the memory address of the end of one free block is greater than the memory address
        // of the beginning of the next free block, return -1
        if (cur->next && (uint64_t) cur + get_size(cur) + 16 > (uint64_t) cur->next) {
            return -1;
        }

        // Check 4: check that the memory blocks are 16 byte aligned
        // if not, return -1
        if ((uint64_t) cur % ALIGNMENT != 0) {
            return -1;
        }
        cur = cur->next;
    }
    return 0;
}