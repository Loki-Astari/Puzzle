
# Challenge

This challenge is to build your own json parser

https://codingchallenges.fyi/challenges/challenge-json-parser

# Requirements

You will need to install `flex` and `bison`

# Building

````
> make
````

# Usage

````
> ./json2 <fileNames>*
````

## FileNames

If no files are specified it will read the std::cin, otherwise it will parse each file specified.

For each input print the file name and "Valid" or "In Valid". Validation is done as per [JSON](https://www.json.org/json-en.html).





