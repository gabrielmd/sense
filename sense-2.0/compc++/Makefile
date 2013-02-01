CC=g++
CCFLAGS=-Wall -g
OBJS= token.o lexer.o parser.o

../bin/cxx:   $(OBJS)
	$(CC) -o ../bin/cxx  $(OBJS)

lexer.o: lex.yy.c token.h
	$(CC) $(CCFLAGS) -c -o lexer.o lex.yy.c

lex.yy.c: lexer.l
	flex lexer.l

token.o: token.cpp token.h
	$(CC) $(CCFLAGS) -c -o token.o token.cpp

parser.o: token.h parser.cpp
	$(CC) $(CCFLAGS) -c -o parser.o parser.cpp

clean:
	rm $(OBJS) lex.yy.c 

