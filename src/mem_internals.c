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
    /* we write the total size of the block allocated at address ptr */
    *(unsigned long *)ptr = size;

    /* we write the magic value to the address ptr + 8 bytes */
    unsigned long magic = (knuth_mmix_one_round((unsigned long) ptr) & ~0b11UL) | (unsigned long) k;
    *(unsigned long *)(ptr + 8) = magic;

    /* we write the magic value to the address ptr + size - 16 bytes */
    *(unsigned long *)(ptr + size - 16) = magic;

    /* we write the total size of the block allocated to the address ptr + size - 8 bytes */
    *(unsigned long *)(ptr + size - 8) = size;

    /* we return the start address of the usable block ie the address ptr + 16 bytes */
    return (void *)(ptr + 16);
}

Alloc
mark_check_and_get_alloc(void *ptr)
{
    /* we get the address of the beginning of the allocated block */
    void *start_ptr = ptr - 16;

    /* we get the total size of the allocated block */
    unsigned long size = *(unsigned long *)start_ptr;

    /* we get the magic value */
    unsigned long magic = *(unsigned long *)(start_ptr + 8);

    /* get the type of memory allocation */
    int k =  (MemKind) (*(unsigned long *)(start_ptr + 8) & 0b11UL);

    /* We check that magic is coherent */
    unsigned long magic_theoric = (knuth_mmix_one_round((unsigned long) start_ptr) & ~0b11UL) | (unsigned long) k;
    assert(magic == magic_theoric);

    /* We can check that the size is identical at the beginning and at the end of the block */
    //unsigned long end_size = *(unsigned long *)(start_ptr + size - 8);
    //assert(size == end_size);

    /* we fill in the fields of the allowance we send back */
    Alloc a = {start_ptr, k, size};

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
