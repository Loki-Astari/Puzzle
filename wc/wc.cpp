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

    void operator+=(Result const& rhs) {
        lines += rhs.lines;
        words += rhs.words;
        chars += rhs.chars;
        bytes += rhs.bytes;
    }
};

static int newLineCheck[256] =            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int unicodeSize[256] =             { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                            2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                                            3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                                            4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0};


Result getData(std::istream& file)
{
    Result      result;
    bool        inWord = false;

    // We will read chunks of 'bufferSize' from the stream.
    // But if we hit a multi-byte character as the last character in the buffer we will read that
    // into the buffer so we need a capacity 'bufferCapacity' that is slightly larger in case we
    // need it.
    static constexpr int bufferSize = 4096;
    static constexpr int bufferCapacity = bufferSize + 3;

    unsigned char buffer[bufferCapacity];

    file.read(reinterpret_cast<char*>(buffer), bufferSize);
    std::streamsize count = file.gcount();
    while (count != 0) {

        int increment;
        for (auto loop = 0; loop < count; loop += increment) {

            unsigned int index = buffer[loop];

            increment = unicodeSize[index];
            if (loop + increment > count) {
                // If the last character extends beyond the buffer then read it into
                // the buffer. We have made sure the buffer capacity is enough to hold
                // these extra characters.
                int read = loop + increment - count;
                file.read(reinterpret_cast<char*>(&buffer[count]), read);
                count += read;
            }

            // Assumption that we only care about UTF-8 stream and not other
            // multi-byte character systems. And yes that is true. I don't care.
            // One standard to cover them all stop using other multi-byte systems.
            // Rant over.
            std::wint_t  ch;
            switch (increment) {
                case 0:     throw std::runtime_error("Bad Input");
                case 1:     ch = index;break;
                case 2:     ch = ((static_cast<int>(buffer[loop + 0]) & 0x1F) <<  6)
                               | ((static_cast<int>(buffer[loop + 1]) & 0x3F) <<  0);
                            break;
                case 3:     ch = ((static_cast<int>(buffer[loop + 0]) & 0x0F) << 12)
                               | ((static_cast<int>(buffer[loop + 1]) & 0x3F) <<  6)
                               | ((static_cast<int>(buffer[loop + 2]) & 0x3F) <<  0);
                            break;
                case 4:     ch = ((static_cast<int>(buffer[loop + 0]) & 0x07) << 18)
                               | ((static_cast<int>(buffer[loop + 1]) & 0x3F) << 12)
                               | ((static_cast<int>(buffer[loop + 2]) & 0x3F) <<  6)
                               | ((static_cast<int>(buffer[loop + 3]) & 0x3F) <<  0);
                            break;
            }

            // Count the number of new line characters.
            result.lines += newLineCheck[index];

            // Words are "white space" separated.
            // Increment the counter when we are not in a word and hit one.
            // We are not in a word when there is white space.
            bool isSpace = std::iswspace(ch);
            result.words += (!inWord && !isSpace) ? 1 : 0;

            // Keep track if we are in the word.
            inWord = !isSpace;

            // We are parsing one character at a time in this loop.
            result.chars += 1;

            // The character may be multiple bytes.
            result.bytes += increment;
        }
        file.read(reinterpret_cast<char*>(buffer), bufferSize);
        count = file.gcount();
    }

    return result;
}

void display(std::string const& fileName, Options const& options, Result const& data)
{
    if (options.any || options.lines) {
        std::cout << " " << std::setw(7) << data.lines;
    }
    if (options.any || options.words) {
        std::cout << " " << std::setw(7) << data.words;
    }
    if (options.any || options.chars) {
        std::cout << " " << std::setw(7) << data.chars;
    }
    if (options.any || options.bytes) {
        std::cout << " " << std::setw(7) << data.bytes;
    }
    std::cout << " " << fileName << "\n";
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
        Result data = getData(std::cin);
        display("", options, data);
    }
    /* Loop over all the specified files */
    Result total;
    for (auto fileName: files) {
        std::ifstream   file(fileName);
        if (!file) {
            std::cerr << "Failure to open file: " << fileName << "\n";
        }
        else {
            Result data = getData(file);
            display(fileName, options, data);

            total += data;
        }
    }
    if (files.size() > 1) {
        display("total", options, total);
    }
}

