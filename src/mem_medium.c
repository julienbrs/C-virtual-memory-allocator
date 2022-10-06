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

void *recursive_buddy(int index, unsigned long size) {
    if (index >= FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {
        mem_realloc_medium();
        return recursive_buddy(index - 1, size);
    }
    else if (arena.TZL[index] != NULL && size == 1<<index) { // cas d'arrêt on a trouvé ce qu'on cherchait
        unsigned long next_elt = *(unsigned long *)arena.TZL[index];
        void *ptr = mark_memarea_and_get_user_ptr(arena.TZL[index], 1<<index, MEDIUM_KIND);
        arena.TZL[index] = (void *)next_elt;
        return ptr;
    }
    else if (arena.TZL[index] == NULL) { // on va chercher plus loin
        return recursive_buddy(index + 1, size);
    }
    else { //TZL[index] n'est pas NULL : on divise le bloc et on récurre
        /* on conserve le bloc dispo suivant dans la liste */
        unsigned long next_elt = *(unsigned long *)arena.TZL[index];
        /* on calcule le buddy du bloc qu'on va scinder */
        unsigned long buddy = (unsigned long)arena.TZL[index] ^ 1 << (index - 1);
        /* comme on a rencontré aucun bloc plus petit on sait que le suivant de buddy est NULL */
        *(unsigned long *)buddy = 0;
        /* le bloc coupé en deux pointe sur son buddy */
        *(unsigned long *)arena.TZL[index] = buddy;
        /* puis on l'insère dans la liste correspondant à sa nouvelle taille */
        arena.TZL[index - 1] = arena.TZL[index];
        /* on met le bloc dispo suivant du début en tête de liste */
        arena.TZL[index] = (void *)next_elt;
        /* on récurre ! */
        return recursive_buddy(index - 1, size);
    }
}


void *
emalloc_medium(unsigned long size)
{
    assert(size < LARGEALLOC);
    assert(size > SMALLALLOC);
    int index_tzl = puiss2(size + 32);
    void *ptr = recursive_buddy(index_tzl, 1<<index_tzl);
    return ptr;
}


void efree_medium(Alloc a) {
    int index_tzl = puiss2(a.size);
    unsigned long addr_buddy = (unsigned long) a.ptr ^ a.size;
    void *ptr_buddy = (void *) addr_buddy;
    void *ptr_list_elt = arena.TZL[index_tzl];
    void * temp;
    while (ptr_list_elt != NULL) {
        if ((void*)*(unsigned long *)ptr_list_elt == ptr_buddy) {
            //suppression de buddy
            *(unsigned long *)ptr_list_elt = *(unsigned long *)ptr_buddy;
            //calcul de l'adresse a fusionner
            temp = ((unsigned long) a.ptr < (unsigned long) ptr_buddy) ?  a.ptr : ptr_buddy;
            //on fusionne
            *(unsigned long *)temp = (unsigned long) arena.TZL[index_tzl + 1];
            arena.TZL[index_tzl + 1] = temp;
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


