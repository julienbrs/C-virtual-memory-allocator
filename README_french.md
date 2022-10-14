Allocateur Mémoire en C (malloc)
==================

Tous les fichiers sont couverts par la licence GPLv3+

## Introduction


Implémentation d'un allocateur dynamique de mémoire semblable à `malloc` sous Linux (GNU libc) en langage C.

Sujet pdf joint ou [ici](https://systemes.pages.ensimag.fr/sepc/TP/S1/) (version française).

## Comment l'utiliser
### Pour installer: 
Clone the project to your device and run these commands:

> `cd build` 

> `cmake ..`

> `cmake ..` // to move cmake files to build

> `make`

> `make test`

### Commandes disponibles:
Ces commandes sont à tapées dans le répertoire `build`.

Pour voir les résultats des différents tests:
> `make check`

Pour un résumé des tests:
> `make test`

Shell interactif pour tester les allocations, possibilité de lancer gdb pour débogger
> `./memshell`


Comment utiliser memshell
----------
![Proof usage of memshell](screen_running.png "Alloc of 3 different size")

Commandes disponibles :
1) **init** : initialisation ou réinitialisation de l'allocateur
2) **alloc** <taille> : allocation d'un bloc mémoire
	La taille peut être en décimal ou en héxadécimal (préfixe 0x)
	retour : identificateur de bloc et adresse de départ de la zone
3) **free** <identificateur> : libération d'un bloc
4) **destroy** : libération de l'allocateur
4) **show** : affichage la taille initiale et de l'adresse de départ
5) **used** : affichage de la liste des blocs occupés
	sous la forme {identificateur, adresse de départ, taille}
6) **help** : affichage de ce manuel
7) **exit** : quitter le shell

Remarques :
1) Au lancement, le shell appelle mem_init
2) Le shell supporte jusqu'à 5000 allocations entre deux initialisations

Description de l'implémentation
----------
Gère les allocations pour 3 tailles différentes (petites, moyennes, grandes).

Les petites tailles sont gérées avec une liste simplement chaînée (*pool*) de *chunks* disponibles de taille identique. Lorsqu'il n'y a plus de *chunk* disponible, une demande est faite au système, avec un mécanisme de recursive doubling pour amortir le coût.

Utilisation de l'agorithme du *Buddy* pour gérer les tailles moyennes

Jeux de tests et squelette fournis (Google Test)

