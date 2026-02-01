#include "Lexer.h"
#include "Parser.h"

#include <fstream>
#include <iostream>


bool checkJson(std::istream& file, std::function<void(bool)>&& errorMsg)
{
    ThorsAnvil::Json::Lexer     lexer(file);
    yy::Parser                  parser(lexer);

    bool valid = (parser.parse() == 0);
    errorMsg(valid);
    return valid;
}

void errorPrinter(bool valid, std::string const& fileName)
{
    if (!valid) {
        std::cerr << "Error: " << fileName << ":\t\tNot valid JSON.\n";
    }
}

int main(int argc, char* argv[])
{
    bool result = true;
    if (argc == 1) {
        result = checkJson(std::cin, [](bool valid){errorPrinter(valid, "std::cin");});
    }
    else {
        for (int loop = 1; loop < argc; ++loop) {
            std::ifstream   file(argv[loop]);
            if (!file) {
                std::cerr << "Error: " << argv[loop] <<":\t\tCould not open file.\n";
                result = false;
                continue;
            }
            if (!checkJson(file, [filename = argv[loop]](bool valid){errorPrinter(valid, filename);})) {
                result = false;
            }
        }
    }
    return result ? 0 : 1;
}

