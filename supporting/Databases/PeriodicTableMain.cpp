/*
 # Copyright 2016-2018 Ruben Jesus Garcia-Hernandez
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
*/

#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"
#include "exportXYZ.h"
#include "PeriodicTable.h"

int main (int argc, char ** argv) 
{

//create periodic table
if (argc!=2 && argc!=3) {
	fprintf (stderr, "Required argument: XYZ filename, optional argument: scaling\n");
	return 1;
}

numAtoms=new int[1];
numAtoms[0]=118;

atoms=new float* [1];
atoms[0]=new float[118*4];

float scaling=5;
float extrascaling=1;
if (argc==3) {
	int read;
	read = sscanf (argv[2], "%f", &extrascaling);
	if (read!=1) {
		fprintf (stderr, "second argument (scaling) is not a float number, exiting\n");
		return 2;
	}
}

FILE *f=fopen ("PT.ncfg", "w");
CreatePeriodicTable(atoms[0], scaling*extrascaling, f);
fprintf (f, "xyzfile \"%s\"\n", argv[1]);
fclose(f);

exportXYZ(argv[1], "PeriodicTable", 1);

return 0;
}

