#ifndef THORSANVIL_JSON_LEXER_H
#define THORSANVIL_JSON_LEXER_H

#include <istream>

#if !defined(yyFlexLexerOnce)
#include <FlexLexer.h>
#endif

namespace ThorsAnvil::Json
{

class Lexer : public yyFlexLexer
{
    public:
       Lexer(std::istream& in)
           : yyFlexLexer(&in)
       {}

       // This function is generated from json.l file
       // and its implementation is in the json.lex.cpp
       virtual int yylex() override;
};

}

#endif
