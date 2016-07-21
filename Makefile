all : my-sh

my-sh : my-sh.c helper.o node.o mystring.o tokenizer.o
	clang -g -o my-sh my-sh.c helper.o node.o mystring.o tokenizer.o

helper.o : helper.c helper.h
	clang -g -c helper.c

node.o : node.c node.h
	clang -g -c node.c

mystring.o : mystring.c mystring.h
	clang -g -c mystring.c

tokenizer.o : tokenizer.c tokenizer.h
	clang -g -c tokenizer.c

clean : 
	rm -rf *.o

clobber : clean
	rm -rf ex2
