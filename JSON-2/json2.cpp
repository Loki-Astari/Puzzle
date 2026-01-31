#include "Lexer.h"
#include "Parser.h"

#include <fstream>
#include <iostream>


bool checkJson(std::string const& fileName, std::istream& file)
{
    ThorsAnvil::Json::Lexer     lexer(file);
    yy::Parser                  parser(lexer);

    bool valid = parser.parse() == 0;
    std::cout << fileName << ":\t\t" << ((valid ? "Valid" : "In Valid")) << "\n";
    return valid;
}

int main(int argc, char* argv[])
{
    bool result = true;
    if (argc == 1) {
        result = checkJson("", std::cin);
    }
    else {
        for (int loop = 1; loop < argc; ++loop) {
            std::ifstream   file(argv[loop]);
            if (!file) {
                std::cerr << "Invalid File: " << argv[loop] << "\n";
            }
            if (!checkJson(argv[loop], file)) {
                result = false;
            }
        }
    }
    return result ? 0 : 1;
}

