%skeleton "lalr1.cc"
%require  "2.3"
%debug
%defines
%define "parser_class_name" "Parser"

%parse-param                {ThorsAnvil::Json::Lexer  &lexer}

%{
#include "Lexer.h"
#undef  yylex
#define yylex lexer.yylex
%}


%token                      True
%token                      False
%token                      Null
%token                      String
%token                      Number

%token                      Error

%%

Json                :       JsonValue

JsonValue           :       True
                    |       False
                    |       Null
                    |       String
                    |       Number
                    |       JsonArray
                    |       JsonObject

JsonArray           :       '[' JsonArrayListOpt ']'
JsonArrayListOpt    :                                           {/* No Values */}
                    |       JsonArrayList
JsonArrayList       :       JsonValue
                    |       JsonArrayList ',' JsonValue

JsonObject          :       '{' JsonObjectListOpt '}'
JsonObjectListOpt   :                                           {/* No Values */}
                    |       JsonObjectList
JsonObjectList      :       JsonObjectValue
                    |       JsonObjectList ',' JsonObjectValue
JsonObjectValue     :       String ':' JsonValue


%%

void yy::Parser::error(yy::location const&, std::string const& /*msg*/)
{
    //std::cerr << "Error: " << msg << "\n";
}
