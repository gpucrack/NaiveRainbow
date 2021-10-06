# Naive Rainbow

Implémentation des rainbow tables, en utilisant la fonction de hachage SHA1 avec OpenSSL. Marche sur un CPU.

Exemple de crackage de mot de passe :
```
Generating tables...
0 -> ... -> IY
1 -> ... -> OXT
2 -> ... -> pnE
3 -> ... -> DbB
4 -> ... -> q_v
5 -> ... -> JjE
6 -> ... -> dDa
7 -> ... -> Hmh
8 -> ... -> Bqp
9 -> ... -> tE8
A -> ... -> KOT
B -> ... -> LFF
D -> ... -> j6b
E -> ... -> 2p1
F -> ... -> TE
G -> ... -> Cgm
...

Looking for password 'pls', hashed as b1432db0088f669789237117d35b0b8e14768d10.

Starting attack...
Password 'pls' found for the given hash!
```

- Fonctionnalités
    - Phase offline et online: cracke un hash avoir avoir précalculé des tables.
    - Elimination des chaînes redondantes

- A faire
    - stockage sur disque
    - Compression

## Installation

- Installer OpenSSL 1.1 (prendre une version précompilée sur Windows, `apt-get install libssl-dev` pour Linux)
- Installer cmake si ce n'est pas déjà fait

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```