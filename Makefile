CPPFLAGS := $(CPPFLAGS) -I.
CFLAGS := $(CFLAGS) -ggdb3 -Wall -Wextra

TESTS = hello benchmark benchmark2 mandelbrot

bf: bf.o bf-lexer.yy.o ast.o asm.o opt.o

bf.o: bf.c ast.h asm.h

ast.o: ast.c ast.h

asm.o: asm.c asm.h ast.h

opt.o: opt.c opt.h ast.h

bf-parser.tab.h: bf-parser.y ast.h
	bison -o $@ $<

bf-lexer.yy.c: bf-lexer.l bf-parser.tab.h

%.yy.c: %.l
	flex -o $@ $<

clean:
	rm -f ast.o bf-lexer.yy.c bf-lexer.yy.o bf-parser.tab.h bf.o bf asm.o $(TESTS)

$(TESTS): %: tests/%.bf bf
	./bf < $< | gcc -nostdlib -x assembler -o $@ -

test: bf $(TESTS)
	@for f in $(TESTS) ; do \
		time ./$$f ; \
	done


.PHONY: clean test
