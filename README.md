# LZ77/78 Compression

## Short Description:

Implementation of Lempel-Ziv compression algorithm.


## Files:

#### encode.c

Interface to encode a file. Performs Lempel-Ziv algorithm.

#### decode.c

Interface to decode file encoded by `encode.c`.

#### io.c

Defines functions to read and write raw and encoded  
files.

#### io.h

Header for `io.c`.

#### trie.c

Implementation of trie ADT used by encoding algorithm.

#### trie.h

Header for `trie.c`.

#### word.c

Implementation of word ADT for words and word tables.

#### word.h

Header file for `word.c`.

#### endian.h

Interface to check and swap Endian order of a multi-byte number.

#### encode.h

Definitions of several constants.

#### set.c

Implementation of a set used in this context for command line options.

#### set.h

Header for `set.c`.

#### WRITEUP.pdf

Describes and shows what was learned and the main takeaways  
from this project.

#### DESIGN.pdf

Design document for the program that explains the layout  
of the project using pseudocode and some diagrams.

#### README.me

*This* document; describes briefly what each file does.

## Build:

To build all executables, run `make` or `make all`.  
If only a specific executables is desired run  
`make [encode|encrypt]` for the respective executable.

## Clean:

To remove all the `.o` and executable files, run  
```sh
$ make clean
```
.

## Format:

To format all `.c` and `.h` files, execute
```sh
$ make format
```
.

## Scan-Build

To scan-build, run  
```sh
$ make scan-build
```
.

## Running:

For encoding:  
Once building, you may run with:
```sh
$ ./encode
```
in the working  
directory. No options will just use `stdin` and `stdout`  
for input/output respectively.  
To specify input and outfile files use options `-i` and `-o`  
respectively. For compression statistics, use `-v`. To see  
more specific usage run 
```sh
$ ./encode -h
```
.

For decoding:  
After building, again, you can run with no options:  
```sh
$ ./decode
```
.
default being `stdin` and `stdout`. For input/output files  
you again specify `-i` and `-o`, respectively. Use `-h` for specific  
options.
