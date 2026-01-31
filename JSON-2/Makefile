
LEX					= flex
CXXFLAGS			= -std=c++20


all:  json2
clean:
	$(RM) json2 json.lex.cpp

json2:	json2.cpp json.lex.cpp

json.lex.cpp:	json.l
	$(LEX) -d -o json.lex.cpp json.l
