/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <assert.h>
#include "mem.h"
#include "mem_internals.h"

unsigned long CHUNKPOOL_SIZE;
// TODO: enlever les printf

void *
emalloc_small(unsigned long size)
{
    /* si la liste de petites allocs est vide, ou qu'il n'y a plus de place, on realloc un bloc dedié */
    if (arena.chunkpool == NULL || CHUNKPOOL_SIZE <= 96) {
        arena.chunkpool = NULL;
        CHUNKPOOL_SIZE = mem_realloc_small();

        /* on "chaîne" les blocs en écrivant l'adresse du suivant tous les CHUNKSIZE octets */
        int nb_chunks = CHUNKPOOL_SIZE / CHUNKSIZE;
        unsigned long *current_ptr = arena.chunkpool;
        for (int i = 0; i < nb_chunks; i++) {
            *current_ptr = (unsigned long)current_ptr + CHUNKSIZE;
            current_ptr = (unsigned long *)(*current_ptr);
        }
    }
    /* on prend l'adresse de l'espace qu'on va allouer à l'utilisateur */
    void *chunkpool_head = arena.chunkpool;
    //printf("adresse allouée : %p\n",chunkpool_head);

    /* on indique que ce bloc de taille CHUNKSIZE n'est plus libre */
    unsigned long new_head = *(unsigned long *)arena.chunkpool;
    //printf("prochaine adresse libre : %lu\n", new_head);
    arena.chunkpool = (void *) new_head;
    //printf("prochaine adresse libre : %p\n", arena.chunkpool);

    /* on met à jour la taille restante pour petites allocs */
    CHUNKPOOL_SIZE -= CHUNKSIZE;
    //printf("chunkpool_size : %lu\n",CHUNKPOOL_SIZE);

    /* on marque l'espace mémoire alloué */
    void *ptr_alloc = mark_memarea_and_get_user_ptr(chunkpool_head, CHUNKSIZE, SMALL_KIND);
    return ptr_alloc;
}

void efree_small(Alloc a) {
    
    /* à l'intérieur de la zone à libérer, on écrit l'adresse du prochain bloc libre */
    /* qui est la tête actuelle du chunkpool */
    unsigned long *addr_to_free = a.ptr;
    *addr_to_free = (unsigned long) arena.chunkpool;

    /* la tête du chunkpool devient la zone liberée */
    arena.chunkpool = a.ptr;
    //printf("chunkpool head : %p\n", arena.chunkpool);

    /* on met à jour la place disponible après libération */
    CHUNKPOOL_SIZE += CHUNKSIZE;
}
