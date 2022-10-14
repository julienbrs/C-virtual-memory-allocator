/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long CHUNKPOOL_SIZE;

void *
emalloc_small(unsigned long size)
{
    /* if the list of small allowances is empty, or there is no more space, we reallocate a dedicated block */
    if (arena.chunkpool == NULL || CHUNKPOOL_SIZE <= 96) {
        arena.chunkpool = NULL;
        CHUNKPOOL_SIZE = mem_realloc_small();

        /* we "chain" the blocks by writing the address of the next one every CHUNKSIZE bytes */
        int nb_chunks = CHUNKPOOL_SIZE / CHUNKSIZE;
        unsigned long *current_ptr = arena.chunkpool;
        for (int i = 0; i < nb_chunks; i++) {
            *current_ptr = (unsigned long)current_ptr + CHUNKSIZE;
            current_ptr = (unsigned long *)(*current_ptr);
        }
    }
    /* we take the address of the space we will allocate to the user */
    void *chunkpool_head = arena.chunkpool;

    /* indicates that this block of size CHUNKSIZE is no longer free */
    unsigned long new_head = *(unsigned long *)arena.chunkpool;
    arena.chunkpool = (void *) new_head;

    /* we update the remaining size for small allowances */
    CHUNKPOOL_SIZE -= CHUNKSIZE;

    /* we mark the allocated memory space */
    void *ptr_alloc = mark_memarea_and_get_user_ptr(chunkpool_head, CHUNKSIZE, SMALL_KIND);
    return ptr_alloc;
}

void efree_small(Alloc a) {
    
    /* inside the zone to be freed, we write the address of the next free block
    which is the current head of the chunkpool */
    unsigned long *addr_to_free = a.ptr;
    *addr_to_free = (unsigned long) arena.chunkpool;

    /* the head of the chunkpool becomes the free zone */
    arena.chunkpool = a.ptr;

    /* we update the available space after release */
    CHUNKPOOL_SIZE += CHUNKSIZE;
}
