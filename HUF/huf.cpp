#include "Huffman.h"

#include <iostream>
#include <fstream>
#include <string>

using ThorsAnvil::Puzzle::HuffmanEncoder;
using ThorsAnvil::Puzzle::HuffmanDecoder;

int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Usage: huf [+-] <filename>\n";
        return 1;
    }
    if (argv[1][0] != '+' && argv[1][0] != '-') {
        std::cerr << "Usage: huf [+-] <filename>\n";
        return 1;
    }
    std::ifstream   file(argv[2]);
    if (!file) {
        std::cerr << "File: " << argv[2] << " could not be opened\n";
        return 1;
    }

    if (argv[1][0] == '+') {
        HuffmanEncoder  encoder;
        if (encoder.buildTree(file)) {
            std::string     outName(std::string(argv[2]) + ".huf");
            std::ofstream   out(outName);
            if (!out) {
                std::cerr << "File: " << outName << " can not be opened for output\n";
                return 1;
            }
            file.clear();
            file.seekg(0);
            encoder.exportTree(out);
            encoder.encode(file, out);
            return 0;
        }
    }
    else {
        HuffmanDecoder  decoder;
        if (decoder.buildTree(file)) {
            std::string     outName(std::string(argv[2]) + ".dec");
            std::ofstream   out(outName);
            if (!out) {
                std::cerr << "File: " << outName << " can not be opened for output\n";
                return 1;
            }
            decoder.decode(file, out);
            return 0;
        }
    }
    return 1;
}
