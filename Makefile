all: gpif_compiler gpif_decompiler

CXX	= g++

STD	= -std=c++11

.cpp.o:
	$(CXX) -Wall -c -g $(STD) $< -o $*.o

gpif_compiler: gpif_compiler.o
	$(CXX) $(STD) $^ -o $@

gpif_decompiler: gpif_decompiler.o
	$(CXX) $(STD) $^ -o $@

clean:
	rm -f *.o *~

clobber: clean
	rm -f gpif_compiler gpif_decompiler

test: compilertest decompilertest

compilertest: gpif_compiler
	./gpif_compiler < testwave.wvf

decompilertest: gpif_decompiler
	./gpif_decompiler testgpif.c

wvf_files: gpif_compiler
	cd WVF; ./COMPILE_GPIF
