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
    /* si la liste de petites allocs est vide, ou qu'il n'ya plus de place, on realloc un bloc dedié */
    if (arena.chunkpool == NULL || CHUNKPOOL_SIZE <= 96) {
        arena.chunkpool = NULL;
        CHUNKPOOL_SIZE = mem_realloc_small();
    }
    void *chunkpool_head = arena.chunkpool;
    //printf("%p\n",chunkpool_head);
    arena.chunkpool += CHUNKSIZE;
    //printf("%p\n",arena.chunkpool);
    CHUNKPOOL_SIZE -= CHUNKSIZE;
    //printf("%lu\n",CHUNKPOOL_SIZE);
    void *ptr_alloc = mark_memarea_and_get_user_ptr(chunkpool_head, CHUNKSIZE, SMALL_KIND);
    return ptr_alloc;
}

void efree_small(Alloc a) {
    /* ecrire votre code ici */
}
