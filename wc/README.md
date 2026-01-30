
# Challenge

This challenge is to build your own version of the Unix command line tool wc!  

https://codingchallenges.fyi/challenges/challenge-wc/

# Building

> make

# Usage

> ./wc <flags>? <fileNames>*

## Flags

If no flags are specified then all data is printed. Otherwise print only what is asked for.

* `-l`: Count the number of lines in the input file.
* `-w`: Count the number of space separated words in the input file.
* `-m`: Count the number of UTF-8 characters in the input file.
* `-c`: Count the number of bytes in the input file.

Note: The application supports UNIX like flags so they can be specified individually `-l -w` or in a single flag `-lw`.

## FileNames

A list of zero or more file to scan. If no files are specified then if will read from the standard input.




