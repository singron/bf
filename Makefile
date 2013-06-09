CPPFLAGS := $(CPPFLAGS) -I.
CFLAGS := $(CFLAGS) -ggdb3 -Wall -Wextra

bf: bf.o bf-lexer.yy.o ast.o asm.o

bf.o: bf.c ast.h asm.h

ast.o: ast.c ast.h

asm.o: asm.c asm.h ast.h

bf-parser.tab.h: bf-parser.y ast.h
	bison -o $@ $<

bf-lexer.yy.c: bf-lexer.l bf-parser.tab.h

%.yy.c: %.l
	flex -o $@ $<

clean:
	rm ast.o bf-lexer.yy.c bf-lexer.yy.o bf-parser.tab.h bf.o bf asm.o
