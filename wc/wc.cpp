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
    std::size_t     lines   = 0;
    std::size_t     words   = 0;
    std::size_t     chars   = 0;
    std::size_t     bytes   = 0;
};

void scanLine(std::string line, Result& result)
{

    bool inWord     = line.size() != 0 && !std::isspace(line[0]);
    int  charSize   = 0;

    for (char c: line) {
        bool isSpace = std::isspace(c);
        if (inWord && isSpace) {
            inWord = false;
            result.words += 1;
        }
        else if (!inWord && !isSpace) {
            inWord = true;
        }
        if (charSize == 0) {
            result.chars += 1;
            // 0yyyzzzz => 0yyy zzzz
            if ((c & 0x80) == 0) {
                charSize = 0;
            }
            // 110xxxyy => 110x xxyy
            else if ((c & 0xE0) == 0xC0) {
                charSize = 1;
            }
            // 1110wwww => 1110 wwww
            else if ((c & 0xF0) == 0xE0) {
                charSize = 2;
            }
            // 11110uvv => 1111 0uyy
            else if ((c & 0xF8) == 0xF0) {
                charSize = 3;
            }
        }
        else {
            --charSize;
        }
    }

    result.lines    += 1;
    result.chars    += 1;
    result.bytes    += (line.size() + 1);
    if (inWord) {
        result.words    += 1;
    }
}

Result getData(std::istream& file)
{
    Result          result;
    std::string     line;
    while (std::getline(file, line)) {
        scanLine(line, result);
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
    for (; loop < argc; ++loop) {a
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
            std::cout << "Unknown file: " << fileName << "\n";
        }
        else {
            display(file, fileName, options);
        }
    }
}
