all: PeriodicTable SimpleMove Perovskite Gromacs2NOMADVR Gro2xyz TestArchive

TestArchive: Archive.o TestArchive.o NOMADVRLib/atoms.o eprintf.o NOMADVRLib/ConfigFileAtoms.o \
	happyhttp/happyhttp.o
	g++ -g $^ -o $@

SimpleMove: SimpleMove.o PeriodicTable.o exportXYZ.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o \
	happyhttp/happyhttp.o eprintf.o myrecv.o
	g++ -g $^ -o $@

Perovskite: Perovskite.o PeriodicTable.o exportXYZ.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o \
	happyhttp/happyhttp.o eprintf.o myrecv.o Archive.o
	g++ -g $^ -o $@

PeriodicTable: PeriodicTable.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o exportXYZ.o happyhttp/happyhttp.o \
	eprintf.o PeriodicTableMain.o
	g++ -g $^ -o $@

Gromacs2NOMADVR: Gromacs2NOMADVR.o exportXYZ.o NOMADVRLib/ConfigFileAtoms.o NOMADVRLib/atoms.o \
	happyhttp/happyhttp.o eprintf.o myrecv.o
	g++ -g $^ -o $@

Gro2xyz: Gro2xyz.o Gro2xyzMain.o
	g++ -g $^ -o $@

.cpp.o:
	g++ -g -I. -std=c++11 $< -c -o $*.o

clean:
	rm -f *.o PeriodicTable SimpleMove Gromacs2NOMADVR Perovskite

