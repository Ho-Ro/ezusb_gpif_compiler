all: gpif_compiler gpif_decompiler

PACKAGE = ezusb-gpif-compiler

CXX	= g++

STD	= -std=c++11

.cpp.o:
	$(CXX) -Wall -c -g $(STD) $< -o $*.o

gpif_compiler: gpif_compiler.o
	$(CXX) $(STD) $^ -o $@

gpif_decompiler: gpif_decompiler.o
	$(CXX) $(STD) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o *~
	rm -f WVF/*.inc

.PHONY: clobber
clobber: clean
	rm -f gpif_compiler gpif_decompiler

.PHONY: test
test: compilertest decompilertest

compilertest: gpif_compiler
	./gpif_compiler < testwave.wvf

decompilertest: gpif_decompiler
	./gpif_decompiler testgpif.c

wvf_files: gpif_compiler
	cd WVF; ./COMPILE_GPIF

.PHONY: install
install: gpif_compiler gpif_decompiler
	install $? /usr/local/bin

.PHONY: deb
deb: all
	fakeroot checkinstall --pkgname ezusb_gpif_compiler --pkgversion 0.2 --default --install=no --backup=no --deldoc=yes
