%option debug
%option nodefault
%option noyywrap
%option c++
%option yyclass="ThorsAnvil::Json::Lexer"

%{
#include "Lexer.h"
%}

WhiteSpace                  [ \t\r\n]+

Hex                         [0-9a-fA-F]
UnicodeChar                 \\u{Hex}
EscapeCharValue             [/"bfnrt\\]
EscapeChar                  \\{EscapeCharValue}
NormalChar                  [^\\"]
Char                        {NormalChar}|{EscapeChar}|{UnicodeChar}
String                      \"{Char}*\"


Sign                        [+-]
Exp                         [Ee]{Sign}?{Digit}+
Fract                       .{Digit}+
Digit                       [0-9]
BaseNumber                  0|[1-9]{Digit}*
OptNeg                      -
Number                      {OptNeg}?{BaseNumber}{Fract}?{Exp}?

%%

\{                          {return '{';}

\}                          {return '}';}
\[                          {return '[';}
\]                          {return ']';}
\:                          {return ':';}
\,                          {return ',';}
true                        {return 250;}
false                       {return 251;}
null                        {return 252;}
{String}                    {return 253;}
{Number}                    {return 254;}


{WhiteSpace}                { /* No Action */ }
.                           {return 255; /* Error No matching */}

%%

