#include "PeriodicTable.h"

void CreatePeriodicTable (float * atoms, float scaling)
{

atoms[0]=0; //H
atoms[1]=0;
atoms[2]=0;
atoms[3]=0;
atoms[4]=17*scaling; //He
atoms[5]=0;
atoms[6]=0;
atoms[7]=1;

int currentAtom=2;

for (int row=1;row<3;row ++) {
	for (int j=0; j<2;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;

		currentAtom++;
	}

	for (int j=0; j<6;j++) {//right
		atoms[currentAtom*4]=(j+12)*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;

		currentAtom++;
	}

}

for (int row=3;row<5;row ++) {
	for (int j=0; j<18;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;

		currentAtom++;
	}

}

for (int row=5;row<7;row ++) {
	for (int j=0; j<3;j++) {//left
		atoms[currentAtom*4]=j*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;

		currentAtom++;
	}

	currentAtom+=14;

	for (int j=0; j<15;j++) {//right
		atoms[currentAtom*4]=(j+3)*scaling;
		atoms[currentAtom*4+1]=0;
		atoms[currentAtom*4+2]=-row*scaling;
		atoms[currentAtom*4+3]=currentAtom;

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

		currentAtom++;	
	}
	currentAtom=89;

}

}

