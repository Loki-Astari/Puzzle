%option debug
%option nodefault
%option noyywrap
%option c++
%option yyclass="ThorsAnvil::Json::Lexer"

%{
#include "Lexer.h"
#include "Parser.h"
%}

WhiteSpace                  [ \t\r\n]+

HexDigit                    [0-9a-fA-F]
UnicodeChar                 \\u{HexDigit}{4}
EscapeCharValue             [/"bfnrt\\]
EscapeChar                  \\{EscapeCharValue}
NormalChar                  [^"\\]
Char                        {NormalChar}|{EscapeChar}|{UnicodeChar}
String                      \"{Char}*\"


Digit                       [0-9]
Sign                        [+-]
E                           [Ee]
Exp                         {E}{Sign}?{Digit}+
Fract                       \.{Digit}+
BaseNumber                  0|[1-9]{Digit}*
Minus                     -
Number                      {Minus}?{BaseNumber}{Fract}?{Exp}?

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

