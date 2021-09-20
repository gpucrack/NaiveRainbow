# Naive Rainbow

Début d'une implémentation des rainbow tables, en utilisant la fonction de hachage SHA1 avec OpenSSL. Marche sur un CPU.

Exemple de table générée pour une taille de mot de passe de 4 et une longeur de chaine de 5
```
0 -> b6589fc6abdc82cf12099d1c2d4ab994e841c -> Qv2_ -> e54be83d5f56afc2470912a907fccb54a98254e -> Xz9T
1 -> 356a192b7913b04c54574d18c28d46e6395428ab -> tmP -> e74e5f70b7e4ac1e786c3e614cd5e9828207499 -> uSuz
2 -> da4b9237bacccdf19c760cab7aec4a8359010b0 -> ILB -> 7887b24bd30e8676c2f27cb9b2179d1932 -> _Pi
3 -> 77de68daecd823babbb58edb1c8e14d7106e83bb -> 4Tjr -> 9cf6c0b343e154a2e24eb68ec0c36975dc5e9b6d -> Jz6-
4 -> 1b6453892473a467d07372d45eb05abc2031647a -> BdZp -> bbcaf454b5fc5914f39c556b76a5d769bfe15e -> X3qz
5 -> ac3478d69a3c81fa62e6f5c3696165a4e5e6ac4 -> f672 -> 9e1e89d6c5a596b6442bc665894b93e7a8b152 -> IevK
6 -> c1dfd96eea8cc2b62785275bca38ac261256e278 -> gx9R -> 722a3e3ea4bfcf255a295d1799d8e68324c59b -> KgmI
7 -> 902ba3cda188381594b6e1b452790cc53948fda -> XwcC -> fe16ccfca067302782ee34b9688c1caac98d857f -> B1H0
8 -> fe5dbbcea5ce7e2988b8c69bcfdfde894aabc1f -> xdJw -> 8d19bfdf3745d2585345ff3b0c9f37dffaa836a -> kqzv
9 -> ade7c2cf97f75d09975f4d72d1fa6c19f4897 -> Vw5 -> 417342fe8961c8af7d4fdf996fe8edfb22d5fb -> xvPr
10 -> b1d5781111d84f7b3fe45a852e59758cd7a87e5 -> Yh0K -> d11b76ae754f734d09efd4591fe7f5d46d8e89f -> iTAs
```

- Fonctionnalités

    - Génère une table de M lignes et de taille de chaîne T

- A faire

    - Attaque online
    - Ne garder que les *starting points* et les *ending points*
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
