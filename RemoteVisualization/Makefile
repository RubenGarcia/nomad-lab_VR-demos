all: NOMADVRLib/atoms.o eprintf.o happyhttp/happyhttp.o NOMADVRLib/ConfigFileAtoms.o main.o
	g++ $^ -o nomad2xyz

.cpp.o:
	g++ -I. -std=c++11 $< -c -o $*.o

clean:
	rm -f *.o nomad2xyz
