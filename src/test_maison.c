#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>
#include "mem_internals.h"
#include "mem.h"




int main(int argc, char **argv)
{
    printf("cc");
    void *ptr = malloc(sizeof(int));
    mark_memarea_and_get_user_ptr(ptr, 27, 0);
    return 0;
}
 