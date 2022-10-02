/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <stdint.h>
#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned int puiss2(unsigned long size) {
    unsigned int p=0;
    size = size -1; // allocation start in 0
    while(size) {  // get the largest bit
	p++;
	size >>= 1;
    }
    if (size > (1 << p))
	p++;
    return p;
}


void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    void *ptr;
    int index_tzl = puiss2(size);
    if (arena.TZL[index_tzl] != NULL) {
        ptr = mark_memarea_and_get_user_ptr(arena.TZL[index_tzl], size, MEDIUM_KIND);
    }
    else {
        int next_index = index_tzl + 1;
        while (arena.TZL[next_index] == NULL) {
            next_index++;
            if (next_index > FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {
                mem_realloc_medium();
                next_index = index_tzl + 1;
            }
        }
        int decal = next_index - index_tzl;
        for (int i = 0; i < decal; i++) {
            unsigned long addr_buddy = ((unsigned long) arena.TZL[next_index]) ^ (1 << (next_index - i - 1));
            unsigned long *ptr_buddy = (unsigned long *) addr_buddy;
            *ptr_buddy = (unsigned long) arena.TZL[next_index - i - 1];
            arena.TZL[next_index - i - 1] = (void *) ptr_buddy;
        }
        //TODO : retirer l'élément alloué next_index du tableau et le marquer
        ptr = mark_memarea_and_get_user_ptr(arena.TZL[next_index], size, MEDIUM_KIND);
        arena.TZL[next_index] = (void *)*(unsigned long *)arena.TZL[next_index];
    }
    return ptr;
}


void efree_medium(Alloc a) {
    /* ecrire votre code ici */
}


