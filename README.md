# Naive Rainbow

Implémentation des rainbow tables, en utilisant la fonction de hachage SHA1 avec OpenSSL. Marche sur un CPU.

Exemple de crackage de mot de passe :
```
0 -> ... -> ZJ
1 -> ... -> e3
2 -> ... -> VS
3 -> ... -> eS
4 -> ... -> qy
5 -> ... -> 4j
6 -> ... -> CN
7 -> ... -> 1T
8 -> ... -> cI
9 -> ... -> e3
10 -> ... -> 0B
11 -> ... -> 4R
12 -> ... -> VS
13 -> ... -> e3
14 -> ... -> I2
15 -> ... -> fY
16 -> ... -> 4R
17 -> ... -> 0B
18 -> ... -> 4R
19 -> ... -> cI
20 -> ... -> qy
21 -> ... -> X0
22 -> ... -> eS
23 -> ... -> VS
24 -> ... -> e3
25 -> ... -> cI
26 -> ... -> fY
27 -> ... -> eS
28 -> ... -> ZJ
29 -> ... -> fY

30 -> ... -> 4R
31 -> ... -> e3
32 -> ... -> eS
33 -> ... -> 4j
34 -> ... -> eS
35 -> ... -> 4R
36 -> ... -> ZJ
37 -> ... -> 1T
38 -> ... -> fY
39 -> ... -> 4R
40 -> ... -> Gh
41 -> ... -> cI
42 -> ... -> eS
43 -> ... -> eS
44 -> ... -> qy
45 -> ... -> _W
46 -> ... -> ta
47 -> ... -> 0B
48 -> ... -> _W
49 -> ... -> Gh
50 -> ... -> 4R
51 -> ... -> 1T
52 -> ... -> fY
53 -> ... -> ta
54 -> ... -> 0B
55 -> ... -> 4j
56 -> ... -> Gh
57 -> ... -> qy
58 -> ... -> 4R
59 -> ... -> 4j

60 -> ... -> CN
61 -> ... -> 4j
62 -> ... -> eS
63 -> ... -> 4R
64 -> ... -> VS
65 -> ... -> 1T
66 -> ... -> 0B
67 -> ... -> ta
68 -> ... -> 4R
69 -> ... -> VS
70 -> ... -> ZJ
71 -> ... -> qy
72 -> ... -> 4R
73 -> ... -> I2
74 -> ... -> 4j
75 -> ... -> Gh
76 -> ... -> X0
77 -> ... -> ta
78 -> ... -> ta
79 -> ... -> I2
80 -> ... -> ta
81 -> ... -> 0B
82 -> ... -> e3
83 -> ... -> Gh
84 -> ... -> cI
85 -> ... -> cI
86 -> ... -> X0
87 -> ... -> CN
88 -> ... -> 4R
89 -> ... -> 4R

Password 'll' found for the given hash!
```

- Fonctionnalités

    - Phase offline et online: cracke un hash avoir avoir précalculé des tables.

- A faire
    - Elimination des chaines redondantes. Pour un mot de passe de 2 charactères et une longueur de chaîne
    de 10 000, on voit que beaucoup de chaînes fusionnent (donc ont le même point de sortie)
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
