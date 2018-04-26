/*
# Copyright 2016-2018 Ruben Jesus Garcia Hernandez
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

#include "NOMADVRLib/eprintf.h"
#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"

	int timesteps;

void exportXYZ(const char * file, const char *mat)
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
}

void usage (const char * argv0) 
{
        eprintf ("Usage: \n%s e <basename for encyclopedia json> xyz", argv0);
        eprintf ("%s a <archive json> xyz", argv0);

}

int main (int argc, char **argv) {

	if (argc < 3) {
		usage(argv[0]);
		return 1;
	}

	std::vector<float> *clonedAtoms;
	if (argv[1][0]=='e') {
		readAtomsJson (argv[2], &numAtoms, &timesteps, &atoms, abc, &clonedAtoms);
	
		//now export xyz
		exportXYZ(argv[3], argv[2]);
	} else if (argv[1][0]=='a') {
		readAtomsAnalyticsJson (argv[2], &numAtoms, &timesteps, &atoms, abc, &clonedAtoms);
		exportXYZ (argv[3], argv[2]);
	} else {
		usage(argv[0]);	
		return 2;
	}
	return 0;
}
