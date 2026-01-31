%option debug
%option nodefault
%option noyywrap
%option c++
%option yyclass="ThorsAnvil::Json::Lexer"

%{
#include "Lexer.h"
#include "json.tab.hpp"
#undef yylex
%}

WhiteSpace                  [ \t\r\n]+

Hex                         [0-9a-fA-F]
UnicodeChar                 \\u{Hex}{4}
EscapeCharValue             [/"bfnrt\\]
EscapeChar                  \\{EscapeCharValue}
NormalChar                  [^\\"]
Char                        {NormalChar}|{EscapeChar}|{UnicodeChar}
String                      \"{Char}*\"


Digit                       [0-9]
Sign                        [+-]
TheE                        [Ee]
Exp                         {TheE}?{Sign}?{Digit}+
Fract                       \.{Digit}+
BaseNumber                  0|[1-9]{Digit}*
SignNeg                     -
Number                      {SignNeg}?{BaseNumber}{Fract}?{Exp}?

%%

\{                          {return '{';}
\}                          {return '}';}
\[                          {return '[';}
\]                          {return ']';}
\:                          {return ':';}
\,                          {return ',';}
true                        {return yy::Parser::token::True;}
false                       {return yy::Parser::token::False;}
null                        {return yy::Parser::token::Null;}
{String}                    {return yy::Parser::token::String;}
{Number}                    {return yy::Parser::token::Number;}


{WhiteSpace}                { /* No Action */ }
.                           {return yy::Parser::token::Error; /* Error No matching */}

%%

