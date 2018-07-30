#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"
#include "exportXYZ.h"
#include "PeriodicTable.h"

int main (int argc, char ** argv) 
{

//create periodic table
if (argc!=2) {
	fprintf (stderr, "Required argument, XYZ filename\n");
	return 1;
}

numAtoms=new int[1];
numAtoms[0]=118;

atoms=new float* [1];
atoms[0]=new float[118*4];

CreatePeriodicTable(atoms[0], 5);

exportXYZ(argv[1], "PeriodicTable", 1);

return 0;
}

