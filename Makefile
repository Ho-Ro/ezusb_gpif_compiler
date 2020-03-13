PACKAGE = ezusb-gpif-compiler

CXX	= g++

STD	= -std=c++11

#.cpp.o:
#	$(CXX) -Wall -c -g $(STD) $< -o $*.o

.PHONY: all
all: gpif_compiler gpif_decompiler gpif_show

gpif_compiler: gpif_compiler.cpp gpif.h
	$(CXX) $(STD) $< -o $@

gpif_decompiler: gpif_decompiler.cpp gpif.h
	$(CXX) $(STD) $< -o $@

gpif_show: gpif_show.cpp
	$(CXX) $(STD) $< -o $@

.PHONY: clean
clean:
	rm -f *~
	rm -f examples/*.inc

.PHONY: clobber
clobber: clean
	rm -f gpif_compiler gpif_decompiler gpif_show *.deb

.PHONY: test
test: compilertest decompilertest showtest

compilertest: gpif_compiler
	./gpif_compiler < testwave.wvf | tee testwave.inc

decompilertest: gpif_decompiler
	./gpif_decompiler testgpif.c

showtest: gpif_show
	./gpif_show < testwave.inc

.PHONY: examples
examples: gpif_compiler
	cd examples; ./COMPILE_GPIF.sh

.PHONY: install
install: gpif_compiler gpif_decompiler gpif_show
	install $? /usr/local/bin
	cp -r examples doc-pak

.PHONY: deb
deb: all
	fakeroot checkinstall --pkgname $(PACKAGE) --pkgversion 0.2 --pkggroup=electronics --requires libusb-1.0-0 --default --fstrans=yes --install=no --backup=no --deldoc=yes
	-rm -rf doc-pak
