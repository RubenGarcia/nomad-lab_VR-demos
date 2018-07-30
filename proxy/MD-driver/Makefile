all: PeriodicTable SimpleMove

SimpleMove: SimpleMove.o PeriodicTable.o exportXYZ.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o \
	happyhttp/happyhttp.o eprintf.o myrecv.o
	g++ -g $^ -o $@

PeriodicTable: PeriodicTable.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o exportXYZ.o happyhttp/happyhttp.o \
	eprintf.o PeriodicTableMain.o
	g++ -g $^ -o $@

.cpp.o:
	g++ -g -I. -std=c++11 $< -c -o $*.o

clean:
	rm -f *.o PeriodicTable SimpleMove

