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

#include "PeriodicTable.h"

inline void printinfobox (FILE * i, float *pos, float scaling, int atom)
{
fprintf (i, "info %f %f %f %f -1 \"El-%d.png\"\n", pos[0], pos[1]+scaling/2.0f, pos[2], scaling/10.0f, atom+1);
}

void CreatePeriodicTable (float * atoms, float scaling, FILE *infobox)
{

atoms[0]=0; //H
atoms[1]=0;
atoms[2]=0;
atoms[3]=0;
if (infobox)
	printinfobox(infobox, atoms, scaling, 0);
atoms[4]=17*scaling; //He
atoms[5]=0;
atoms[6]=0;
atoms[7]=1;
if (infobox)
	printinfobox(infobox, atoms+4, scaling, 1);

int currentAtom=2;

for (int row=1;row<3;row ++) {
	for (int j=0; j<2;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;
	}

	for (int j=0; j<6;j++) {//right
		atoms[currentAtom*4]=(j+12)*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;
	}

}

for (int row=3;row<5;row ++) {
	for (int j=0; j<18;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;
	}

}

for (int row=5;row<7;row ++) {
	for (int j=0; j<3;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;
	}

	currentAtom+=14;

	for (int j=0; j<15;j++) {//right
		atoms[currentAtom*4]=(j+3)*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;
	}

}

//rare earths
currentAtom=57;
for (int row=0;row<2;row ++) {
	for (int j=0; j<14;j++) {
		atoms[currentAtom*4]=(j+4)*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=(-row-8)*scaling;
		atoms[currentAtom*4+3]=currentAtom;
		if (infobox)
			printinfobox(infobox, atoms+currentAtom*4, scaling, currentAtom);
		currentAtom++;	
	}
	currentAtom=89;

}

}

