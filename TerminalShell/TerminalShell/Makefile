#Elisabeth Frischknecht


#MACROS
CXX = c++
CFLAGS = -std=c++11
CXXSOURCE = shelpers.cpp main.cpp
HEADERS = shelpers.h
CCOMPILE = -c
CLINK = -o

#myProgram is the executable that will be created. It will use the .o targets as prerequisites
zshell: main.o shelpers.o
	$(CXX) $(CFLAGS) *.o $(CLINK) zshell
	
#build: this builds the executable
build: zshell

#each .o file target will create the .o file of a given .cpp and .h file.
shelpers.o: shelpers.cpp shelpers.h
	$(CXX) $(CFLAGS) $(CCOMPILE) shelpers.cpp
	
main.o: main.cpp
	$(CXX) $(CFLAGS) $(CCOMPILE) main.cpp

#clean will remove all the files created by this make file, including the executable
clean:
	rm *.o zshell

run:
	./zshell

#this tells make that these targets are not associated with files
.PHONY: clean run printM build test

#printM will print the values of the Macros that I created
printM:
	echo CXX $(CXX)
#	echo CFLAGS
	echo CXXSOURCE $(CXXSOURCE)
	echo HEADERS $(HEADERS)
	echo CCOMPILE $(CCOMPILE)
	echo CLINK $(CLINK)


