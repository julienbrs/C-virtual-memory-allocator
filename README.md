Allocateur Mémoire en C (malloc)
==================

Tous les fichiers sont couverts par la licence GPLv3+

## Introduction


Implémentation d'un allocateur dynamique de mémoire semblable à `malloc` sous Linux (GNU libc) en langage C.

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


Description de l'implémentation
----------
Gère les allocations pour 3 tailles différentes (petites, moyennes, grandes).

Les petites tailles sont gérées avec une liste simplement chaînée (*pool*) de *chunks* disponibles de taille identique. Lorsqu'il n'y a plus de *chunk* disponible, une demande est faite au système, avec un mécanisme de recursive doubling pour amortir le coût.

Utilisation de l'agorithme du *Buddy* pour gérer les tailles moyennes

Jeux de tests et squelette fournis (Google Test)

