# Naive Rainbow

Rainbow Table implementation with maximality factor support, using SHA1 as the hash function. Works on one CPU core.

This should be a "correct" implementation of rainbow tables, as the success rate is very close to what is given in rainbow tables papers.
That is, a single table has a success rate of 86.5%, and 4 tables have a success rate of 99.96%. You can check the success rate using `examples/coverage.c`.

For a GPU accelerated implementation, check https://github.com/gpucrack/GPUCrack.

Password cracking example (`examples/demo.c`) :
```
Generating 4 table(s)
Password length = 3
Chain length (t) = 10000
Maximality factor (alpha) = 0.952
Optimal number of starting chains given alpha (m0) = 1031

Generating table 1
progress: 97.09% DONE
Sorting table... DONE
Deleting duplicate endpoints... DONE

Generating table 2
progress: 97.09% DONE
Sorting table... DONE
Deleting duplicate endpoints... DONE

Generating table 3
progress: 97.09% DONE
Sorting table... DONE
Deleting duplicate endpoints... DONE

Generating table 4
progress: 97.09% DONE
Sorting table... DONE
Deleting duplicate endpoints... DONE

Table 1:
0DO -> ... -> -mN
05u -> ... -> -qK
0Cy -> ... -> 0No
06S -> ... -> 19D
07D -> ... -> 2Hn
0AB -> ... -> 5Df
05Z -> ... -> 5XQ
02t -> ... -> 5eP
...

Looking for password 'pwd', hashed as 37fa265330ad83eaa879efb1e2db6380896cf639.
Starting attack...
Password 'pwd' found for the given hash!
```

## Features

- Offline phase (rainbow tables generation)
- Online phase (password cracking)
- inference of the number of starting chains using the maximality factor
- simple storage and loading of rainbow tables from disk
- easily (GPU) parallelisable implementation (few mallocs, no complicated data structure, simple algorithms)


## Installation

- Install OpenSSL 1.1 (on Windows use a precompiled version, on Linux use `apt-get install libssl-dev`)
- Install cmake

```bash
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build .
```