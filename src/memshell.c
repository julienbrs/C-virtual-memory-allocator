/*****************************************************
 * Copyright Grégory Mounié 2013,2018                *
 *           Simon Nieuviarts 2008-2012              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mem.h"

/*
  ===============================================================================
  Macros
  ===============================================================================
*/

/*
  #define DEBUG
*/

/*
 * Maximum number of blocks allocated at the same time
 */
#define NB_MAX_ALLOC 5000
 
/*
 * Number of different commands for the interpreter
 * (not including erroneous commands (ERROR))
 */
#define NB_CMD 8
 
/*
 * Maximum number of characters for a command line
 * (including terminator - please check)
 */
#define MAX_CMD_SIZE 64

  
/* prompt */
#define PROMPT ">"

/*
  ===============================================================================
  Types
  ===============================================================================
*/

/*
 * Identification of the command typed
 * ERROR is added for erroneous commands
 */
typedef enum {INIT=0, SHOW, USED, ALLOC, FREE, DESTROY, HELP, EXIT, ERROR} COMMAND;

/*
 * type of block identifiers
 */
typedef unsigned long ID;
 
  
/*
 * Identification of the typed parameters
 */
typedef struct {
	ID id;
	size_t size;
} ARG;
 
 
/*
 * Data structure containing information about an allocated block
 */
typedef struct {
	/* identifier of the allocated block, is 0 if no block is allocated*/
	ID id;
	/* address of the allocated block, is NULL if no block is allocated*/
	void *address;
	/* block size */
	size_t size; 
} BLOCINFO;

/*
  ===============================================================================
  Global variables
  ===============================================================================
*/


/*
 * Counter to determine the Ids of the allocated blocks.
 * Must be (re)initialized to 1
 */
unsigned long id_count;

/*
 * List of recognized commands
 */
static char* commands[NB_CMD] = {"init", "show", "used","alloc", "free", "destroy", "help", "exit"};
	
	
/*
 * Table storing information about the allocated blocks
 * Searches in this table are done on the id field
 * BLOCINFO structures
 */
BLOCINFO bloc_info_table[NB_MAX_ALLOC];

static void *zone_memoire;

/*
  ===============================================================================
  Functions
  ===============================================================================
*/


/*
 * Display function of the occupied memory
 */
void used()
{
	unsigned long i;
 	
	for(i=0; i<NB_MAX_ALLOC; i++) {
		if ((bloc_info_table[i]).id != 0) {
			printf("%ld 0x%lX 0x%lX\n",
			       bloc_info_table[i].id,
			       (unsigned long)(bloc_info_table[i].address - (void*)zone_memoire),
			       (unsigned long)bloc_info_table[i].size);
		}
	}
	
}


/*
 * Display of the help
 */
void help()
{
	printf("Commandes disponibles :\n");
	printf("1) init : initialisation ou réinitialisation de l'allocateur\n");
	printf("2) alloc <taille> : allocation d'un bloc mémoire\n");
	printf("\tLa taille peut être en décimal ou en héxadécimal (préfixe 0x)\n");
	printf("\tretour : identificateur de bloc et adresse de départ de la zone\n");
	printf("3) free <identificateur> : libération d'un bloc\n");
	printf("4) destroy : libération de l'allocateur\n");
	printf("4) show : affichage la taille initiale et de l'adresse de départ\n");	
	printf("5) used : affichage de la liste des blocs occupés\n");
	printf("\tsous la forme {identificateur, adresse de départ, taille}\n");		
	printf("6) help : affichage de ce manuel\n");
	printf("7) exit : quitter le shell\n");
	
	printf("\nRemarques :\n");
	printf("1) Au lancement, le shell appelle mem_init\n");
	printf("2) Le shell supporte jusqu'à %d allocations entre deux initialisations\n", NB_MAX_ALLOC);			
}

 
/*
 * Initialization of the interpreter
 */
void init()
{

	int i;
 
	printf("**** Mini-shell de test pour l'allocateur mémoire ****\n");
	printf("\tTapez help pour la liste des commandes\n");
	
	id_count = 1;
	
	/* initialization of the info table : */
	for (i=0; i < NB_MAX_ALLOC; i++) {
		(bloc_info_table[i]).id = 0;
	}

	printf("\n");
}

/*
 * Determine the command typed
 * token : string corresponding to the command only
 * cmd : location where to put the command eventually identified
 */
void get_command(char *token, COMMAND *cmd)
{
	
	COMMAND i;
	
	for(i=INIT; i<NB_CMD; i++) {
		if (!strcmp(token, commands[i])) break;
	}
	if (i < NB_CMD) /* if the command has been found */
		*cmd = i;
	else *cmd=ERROR; /* otherwise we report the error*/
}


/*
 * Determine the arguments of the command line
 * args : typed arguments
 * pcmd : location where to put the arguments
 */
ARG get_args(char *args, COMMAND *pcmd)
{
	ARG our_args = {0,0};
	char *size_string, *id_string;
	long size, id;
	char *endptr= (char*)1;
		
	/* according to the desired command */
	switch(*pcmd) {
		
	case ALLOC:
		if (args == NULL) {
			/* error if no argument */
			*pcmd = ERROR;
			break;
		}
		/* recovering the <size> parameter */
		size_string = strtok(args, "\n");
		
		size = strtol(size_string, &endptr, 0);
		/* NB : last parameter at 0 to manage decimal and hexa*/

		if ((*endptr != '\0') || (size == 0) || (size < 0)) {
			/* error if the argument is not integer 
			   or if it is null, or if it is negative */
			*pcmd = ERROR;
			break;
		}
		/* otherwise the argument is correct */
		/* fill the structure with the value obtained*/
		our_args.size = (size_t)size;				
		break;
				
	case FREE:
		id_string = strtok(args, "\n");
		if (id_string == NULL) {
			/* error if no argument */
			*pcmd = ERROR;
			break;
		}
		id = strtol(id_string, &endptr, 10);

		if ((*endptr != '\0') || (id == 0) || (id < 0)) {
			/* error if the argument is not integer
			   or if it is null or negative */
			*pcmd = ERROR;
			break;
		}
		/* otherwise the argument is correct */
		/* fill the structure with the value obtained*/
		our_args.id = (ID)id;						
		break;
				
	default:;
	}
	return our_args;
}

 
/*
 * parses a typed line
 * args : location where to store the arguments structure
 * return : the typed command
 */
COMMAND read_command(ARG *args)
{
	//	char c;
	char cmd[MAX_CMD_SIZE]="";
	char *token;
	COMMAND our_cmd;
		
 
 	/* NB : there is no display of the prompt */
	scanf("%[^\n]", cmd); /* reading the command line */
        getc(stdin); /* recovery of \n */
	token = strtok(cmd," "); /* recovery of the order */
	get_command(token, &our_cmd); /* determination of the order */
		
	/* if the command has been correctly identified, we get the arguments :*/
	if (our_cmd != ERROR) {
		token = strtok(NULL, "\n");
		*args = get_args(token, &our_cmd);
	}
		
	return our_cmd;	
}


/*
 * obtains an identifier from an address and a block size
 * and stores the block information in the table
 * addr : address of the block
 * size : size of the block 
 * return : an id number or 0 if no id available
 */
ID get_id(void *addr, size_t size)
{

	unsigned long index = 0;
 		
	while ((index < NB_MAX_ALLOC) && ((bloc_info_table[index]).id != 0)) {
		index++;
	}
		
	if (index == NB_MAX_ALLOC) { /* the allocation limit has been reached */		
		return 0;
	}
	else {
			
		bloc_info_table[index].id = id_count;
		bloc_info_table[index].address = addr;
		bloc_info_table[index].size = size;
			
		return id_count++; /* NB: increment id_count */
	}	
}


/*
 * Get the size and address of a block from an id
 * addr : location to store the block address
 * size : location to store the size of the block 
 * return : 0 if ok, -1 if wrong id
 */
int get_info_from_id(ID id, void** addr, size_t* size)
{
 
	unsigned long index = 0;
 
	/* if invalid id, failure */
	if (id < 1) return -1;
		
	while ((bloc_info_table[index].id != id) && (index < NB_MAX_ALLOC)) {
		index++;
	}
					
	/* if id non repertorie, failure */
	if (index == NB_MAX_ALLOC) return -1;
				 	
	*addr = bloc_info_table[index].address;
	*size = bloc_info_table[index].size;		

	return 0;		
}


 
/*
 * Free the entry associated with an id in the info table
 * We assume that the id exists in the table
 * id : the id to free
 */
void remove_id(ID id)
{
	unsigned long index = 0;
		
	while (bloc_info_table[index].id != id) {
		index++;
	}
			
	bloc_info_table[index].id = 0;
	bloc_info_table[index].address = NULL;
}

	
int main() {
	
	COMMAND cmd;
	ARG args = {0,0};
	void *res, *addr;
	ID id;
	size_t size;	

  
	init(); /* initialization of the interpreter */
    	
	while(1) {
#ifdef DEBUG
		printf("memshell-main: debut de la boucle de l'interpreteur\n");
#endif
      		
		printf(PROMPT);
		
		cmd = read_command(&args);
		switch(cmd) {
			
		case INIT:
				
			printf("!!! Pas implanté dans ce sujet !!!\n");
			break;
			
		case SHOW:
		        printf("!!! Pas implanté dans ce sujet !!! Utilisez un débogueur !!!\n"); 
			break;

		case USED:
			used();
			break;
			
		case ALLOC:

			res = emalloc(args.size);
			/* if an error occurs, 0 is displayed */
			if (res == NULL) {
				printf("Erreur : échec de l'allocation (fonction emalloc, retour=NULL)\n");
			} else {
				id = get_id(res, args.size);
				if (id == 0) {
					/* if there is no free id left
					   we display 0 and release the block */
					printf("Erreur : nombre maximum d'allocations atteint/n");
					efree(res);
				} else { /* no problem, display of the allocated area */
					printf("%ld 0x%lX\n", id, (unsigned long)(res - (void*)zone_memoire));
				}
			}
			break;
				

		case DESTROY: 
			printf("!!! Pas implanté dans ce sujet !!!\n");
			break;

		case FREE:
						
			if (get_info_from_id(args.id, &addr, &size) == -1)
				/* error in the id value */
				printf("Erreur : identificateur de bloc incorrect\n"); 
			else {
							
							
				/* liberation du bloc concerne */
				efree(addr);
									
				/* release of the id */
				remove_id(args.id);
								
				/* NB: in the normal case, nothing is displayed */
			}
			break;
				
				
		case HELP:
			help();
			break;
				
		case EXIT:
			goto end;	
			
		case ERROR:
			
			printf("Commande incorrecte\n");
			break;
		}
			
	}
end: return 0;
}
