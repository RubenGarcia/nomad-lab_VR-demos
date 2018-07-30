#include <stdio.h>

#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"
#include "exportXYZ.h"

void exportXYZ(const char * file, const char *mat, int timesteps)
{

FILE * f=fopen (file, "w");
for (int i=0;i<timesteps;i++) {
	fprintf (f, "%d\n", numAtoms[i]);
	fprintf (f, "Comment: Material=%s\n", mat);

	for (int j=0;j<numAtoms[i];j++) {
		fprintf (f, "%s\t%f\t%f\t%f\n", atomNames[(int)(atoms[i][j*4+3])], 
			atoms[i][j*4+0], atoms[i][j*4+1], atoms[i][j*4+2]);


	}
}
fclose(f);
}
