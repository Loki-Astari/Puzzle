#include "Lexer.h"
#include "Parser.h"

#include <iostream>


int main()
{
    ThorsAnvil::Json::Lexer     lexer(std::cin);
    yy::Parser                  parser(lexer);

    parser.parse();
}
