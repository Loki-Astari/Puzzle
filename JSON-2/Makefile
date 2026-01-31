
#
# Prefer flex/bison over lex/yacc
LEX					= flex
YACC				= bison

CXXFLAGS			= -std=c++20 -O3 -Werror -Wall -Wextra -Wno-unused-but-set-variable -Wno-sign-compare -Wno-uninitialized-const-reference


all:  json2
clean:
	$(RM) json2 json.lex.cpp json.tab.hpp json.tab.cpp location.hh position.hh stack.hh

json2:	json2.cpp json.lex.cpp json.tab.cpp

#
# LEX/YACC for C++ (Built-In rules only handle C)
%.lex.cpp: %.l
	$(LEX) $(LFLAGS) -o $@ $<
%.tab.cpp:	%.y
	$(YACC) $(YFLAGS) -o $@ $<
