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
    
    int index_tzl = puiss2(size + 32);
    if (arena.TZL[index_tzl] != NULL) {
        unsigned long next_elt = *(unsigned long *)arena.TZL[index_tzl];
        ptr = mark_memarea_and_get_user_ptr(arena.TZL[index_tzl], 1<<index_tzl, MEDIUM_KIND);
        arena.TZL[index_tzl] = (void *)next_elt;

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
            *ptr_buddy = 0;
            arena.TZL[next_index - i - 1] = (void *) ptr_buddy;
        }
        unsigned long next_elt = *(unsigned long *)arena.TZL[next_index];
        ptr = mark_memarea_and_get_user_ptr(arena.TZL[next_index], 1<<index_tzl, MEDIUM_KIND);
        arena.TZL[next_index] = (void *)next_elt;
    }
    return ptr;
}


void efree_medium(Alloc a) {
    printf("enter free\n");
    int index_tzl = puiss2(a.size);
    printf("index tzl : %d\n", index_tzl);
    unsigned long addr_buddy = ((unsigned long) arena.TZL[index_tzl]) ^ (1 << index_tzl);
    printf("addr buddy : %lu\n", addr_buddy);
    void *ptr_buddy = (void *) addr_buddy;
    printf("ptr buddyszhisizhsz : %p\n", ptr_buddy);
    void *ptr_list_elt = arena.TZL[index_tzl];
    printf("ptr list elt : %p\n", ptr_list_elt);
    while (ptr_list_elt != NULL) {
        if (ptr_list_elt == ptr_buddy) {
            arena.TZL[index_tzl] = ((unsigned long) a.ptr < (unsigned long) ptr_buddy) ? (void *)*(unsigned long *) a.ptr : (void *)*(unsigned long *) ptr_buddy;
            *(unsigned long *)a.ptr = (unsigned long) arena.TZL[index_tzl + 1];
            arena.TZL[index_tzl + 1] = a.ptr;
            return;
        }
        else {
            unsigned long next_addr = *(unsigned long *)ptr_list_elt;
            ptr_list_elt = (void *) next_addr;
        }
    }
    *(unsigned long *)a.ptr = (unsigned long) arena.TZL[index_tzl];
    arena.TZL[index_tzl] = a.ptr;
    return;
}


