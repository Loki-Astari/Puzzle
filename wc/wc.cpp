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
    std::streampos     lines   = 0;
    std::streampos     words   = 0;
    std::streampos     chars   = 0;
    std::streampos     bytes   = 0;
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
        bool isSpace = std::isspace(c & 0x7F);

        if (!inWord && !isSpace) {
            result.words += 1;
        }
        inWord = !isSpace;

        // Ignore UTF-8 continuation bytes for the character count:
        // Assumption that we only care about UTF-8 stream and not other
        // multi-byte character systems. And yes that is true. I don't care.
        // One standard to cover them all stop using other multi-byte systems.
        // Rant over.
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
            switch (argv[loop][flag]) {
                case 'l': options.any = false; options.lines = true; break;
                case 'w': options.any = false; options.words = true; break;
                case 'm': options.any = false; options.chars = true; break;
                case 'c': options.any = false; options.bytes = true; break;
                default:
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

