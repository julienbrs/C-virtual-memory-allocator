/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include <sys/mman.h>
#include <assert.h>
#include <stdint.h>
#include "mem.h"
#include "mem_internals.h"

#define OCTET 8

unsigned long knuth_mmix_one_round(unsigned long in)
{
    return in * 6364136223846793005UL % 1442695040888963407UL;
}

void *mark_memarea_and_get_user_ptr(void *ptr, unsigned long size, MemKind k)
{
    // on ecrit la taille totale du bloc alloué à l'adresse ptr
    *(unsigned long *)ptr = size;

    // on ecrit la valeur magique à l'adresse ptr + 8 octets
    unsigned long magic = knuth_mmix_one_round((unsigned long) ptr) & 0b00UL | (unsigned long) k;
    *(unsigned long *)(ptr + 8) = magic;

    // on ecrit la taille totale du bloc alloué à l'adresse ptr + size - 16 octets
    *(unsigned long *)(ptr + size - 16) = size;

    // on ecrit la valeur magique à l'adresse ptr + size - 8 octets
    *(unsigned long *)(ptr + size - 8) = magic;

    // on retourne l'adresse de début du bloc utilisable ie l'adresse ptr + 16 octets
    return ptr + 16*OCTET;
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    /* On récupère les différentes valeurs */
    void **adr_marquage_debut = ptr - 16*8;
    unsigned long size = (unsigned long) *adr_marquage_debut;
    MemKind k = (MemKind) *(adr_marquage_debut + 8*8) & 0b11UL;
    /* On vérifie que magic est cohérent */
    unsigned long magic_theoric = (knuth_mmix_one_round((unsigned long) ptr) & 0b00UL) | (unsigned long) k;
    assert (magic_theoric == (unsigned long) *(adr_marquage_debut + 8));
    /* On vérifie que les marquages sont cohérents */
    assert (*(adr_marquage_debut + 16) == *(adr_marquage_debut + size - 16));

    Alloc a = {adr_marquage_debut, size, k};
    return a;
}


unsigned long
mem_realloc_small() {
    assert(arena.chunkpool == 0);         
    unsigned long size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = mmap(0,
			   size,
			   PROT_READ | PROT_WRITE | PROT_EXEC,
			   MAP_PRIVATE | MAP_ANONYMOUS,
			   -1,
			   0);
    if (arena.chunkpool == MAP_FAILED)
	handle_fatalError("small realloc");
    arena.small_next_exponant++;
    return size;
}

unsigned long
mem_realloc_medium() {
    uint32_t indice = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;
    assert(arena.TZL[indice] == 0);
    unsigned long size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    assert( size == (1 << indice));
    arena.TZL[indice] = mmap(0,
			     size*2, // twice the size to allign
			     PROT_READ | PROT_WRITE | PROT_EXEC,
			     MAP_PRIVATE | MAP_ANONYMOUS,
			     -1,
			     0);
    if (arena.TZL[indice] == MAP_FAILED)
	handle_fatalError("medium realloc");
    // align allocation to a multiple of the size
    // for buddy algo
    arena.TZL[indice] += (size - (((intptr_t)arena.TZL[indice]) % size));
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free
}


// used for test in buddy algo
unsigned int
nb_TZL_entries() {
    int nb = 0;
    
    for(int i=0; i < TZL_SIZE; i++)
	if ( arena.TZL[i] )
	    nb ++;

    return nb;
}
