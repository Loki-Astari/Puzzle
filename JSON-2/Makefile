
LEX					= flex
YACC				= bison

CXXFLAGS			= -std=c++20


all:  json2
clean:
	$(RM) json2 json.lex.cpp json.tab.hpp json.tab.cpp location.hh position.hh stack.hh

json2:	json2.cpp json.lex.cpp json.tab.cpp

json.lex.cpp:	json.l
	$(LEX) -d -o json.lex.cpp json.l

json.tab.cpp:	json.y
	$(YACC) -o json.tab.cpp json.y
