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
C -> ... -> (null)
D -> ... -> j6b
E -> ... -> 2p1
F -> ... -> TE
G -> ... -> Cgm
H -> ... -> NFU
I -> ... -> (null)
J -> ... -> lBp
K -> ... -> WFd
L -> ... -> (null)
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

### Windows

- Installer OpenSSL 1.1 (prendre une version précompilée)  
- Avec Visual Studio, ouvrir NaiveRainbow.sln
- Ajouter le répertoire `include/` dans les répertoires d'include additionnels
- Ajouter le répertoire `lib/` dans les répertoires de bibliothèques additionnelles
- Ajouter `libssl.lib` et `libcrypto.lib` dans les dépendances additionnelles

### Linux

- Installer OpenSSL 1.1 (avec un `apt-get install libssl-dev` j'imagine)
- Compiler le main.c avec `gcc -I [dossier /include] -L [dossier /lib] -lssl -lcrypto` je pense que ça devrait marcher.

A terme on devrait trouver un système de build qui satisfait tout le monde (Linux & Windows). Peut-être un fichier cmake.
