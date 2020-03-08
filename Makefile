CXX	= g++

STD	= -std=c++11

.cpp.o:
	$(CXX) -Wall -c -g $(STD) $< -o $*.o

gpif_compiler: gpif_compiler.o
	$(CXX) $(STD) $^ -o $@

clean:
	rm -f *.o *~

clobber: clean
	rm -f gpif_compiler

test: gpif_compiler
	./gpif_compiler < testwave.wvf
	./gpif_compiler testgpif.c
