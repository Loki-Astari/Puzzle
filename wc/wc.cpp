#include <cstddef>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

/*
 * Command line options.
 * If no options are specified then any is true and we print all values.
 */
struct Options
{
    bool    any         = true;
    bool    lines       = false;
    bool    words       = false;
    bool    chars       = false;
    bool    bytes       = false;
};

/*
 * Collect values from a file.
 */
struct Result
{
    std::uintmax_t     lines   = 0;
    std::uintmax_t     words   = 0;
    std::uintmax_t     chars   = 0;
    std::uintmax_t     bytes   = 0;
};

Result getData(std::istream& file)
{
    Result      result;
    bool        inWord   = false;

    for (int c = file.get(); c != std::char_traits<char>::eof(); c = file.get()) {

        // A line must have at least one character on it.
        // The new line character counts as a character for this purpose.
        if (c == '\n') {
            result.lines += 1;
        }

        // Words are "white space" separated.
        // Increment the counter when we hit a space when inside a word.
        bool isSpace = std::isspace(c);

        if (!inWord && !isSpace) {
            result.words += 1;
        }
        inWord = !isSpace;

        // Ignore extra characters in multi byte character;
        if ((c & 0xC0) != 0x80) {
            result.chars += 1;
        }

        // Increment for each char read from the stream
        result.bytes += 1;
    }

    return result;
}

void display(std::istream& file, std::string const& fileName, Options const& options)
{
    Result data = getData(file);
    std::cout << "\t";
    if (options.any || options.lines) {
        std::cout << data.lines << "\t";
    }
    if (options.any || options.words) {
        std::cout << data.words << "\t";
    }
    if (options.any || options.chars) {
        std::cout << data.chars << "\t";
    }
    if (options.any || options.bytes) {
        std::cout << data.bytes << "\t";
    }
    std::cout << fileName << "\n";
}

int main(int argc, char* argv[])
{
    Options                     options;
    std::vector<std::string>    files;

    int loop = 1;
    for (; loop < argc; ++loop) {
        /*
         * If this is not a flag then we have reached the files.
         */
        if (argv[loop][0] != '-') {
            break;
        }

        /* Allow old style unix flags */
        for (int flag = 1; argv[loop][flag]; ++flag) {
            if (argv[loop][flag] == 'l') {
                options.any     = false;
                options.lines   = true;
            }
            else if (argv[loop][flag] == 'w') {
                options.any     = false;
                options.words   = true;
            }
            else if (argv[loop][flag] == 'm') {
                options.any     = false;
                options.chars   = true;
            }
            else if (argv[loop][flag] == 'c') {
                options.any     = false;
                options.bytes   = true;
            }
            else {
                std::cerr << "Usage: wc [-lwmc] <files>*\n";
                return 1;
            }
        }
    }

    /* Any remaining command line values are files */
    for (; loop < argc; ++loop) {
        files.emplace_back(argv[loop]);
    }

    /* If no files are explicitly set then use std::cin */
    if (files.size() == 0) {
        display(std::cin, "", options);
    }
    /* Loop over all the specified files */
    for (auto fileName: files) {
        std::ifstream   file(fileName);
        if (!file) {
            std::cerr << "Failure to open file: " << fileName << "\n";
        }
        else {
            display(file, fileName, options);
        }
    }
}

