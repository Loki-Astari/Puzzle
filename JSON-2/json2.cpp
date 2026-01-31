#include "Lexer.h"

#include <iostream>


int main()
{
    ThorsAnvil::Json::Lexer     lexer(std::cin);
    while (int t = lexer.yylex()) {
        std::cout << "Token: " << t << "\n";
    }
}
