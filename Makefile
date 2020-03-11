PACKAGE = ezusb-gpif-compiler

CXX	= g++

STD	= -std=c++11

.cpp.o:
	$(CXX) -Wall -c -g $(STD) $< -o $*.o

.PHONY: all
all: gpif_compiler gpif_decompiler

gpif_compiler: gpif_compiler.o
	$(CXX) $(STD) $^ -o $@

gpif_decompiler: gpif_decompiler.o
	$(CXX) $(STD) $^ -o $@

.PHONY: clean
clean:
	rm -f *.o *~
	rm -f examples/*.inc

.PHONY: clobber
clobber: clean
	rm -f gpif_compiler gpif_decompiler *.deb

.PHONY: test
test: compilertest decompilertest

compilertest: gpif_compiler
	./gpif_compiler < testwave.wvf

decompilertest: gpif_decompiler
	./gpif_decompiler testgpif.c

.PHONY: examples
examples: gpif_compiler
	cd examples; ./COMPILE_GPIF.sh

.PHONY: install
install: gpif_compiler gpif_decompiler
	install $? /usr/local/bin
	cp -r examples doc-pak

.PHONY: deb
deb: all
	fakeroot checkinstall --pkgname $(PACKAGE) --pkgversion 0.2 --pkggroup=electronics --requires libusb-1.0-0 --default --fstrans=yes --install=no --backup=no --deldoc=yes
	-rm -r doc-pak
