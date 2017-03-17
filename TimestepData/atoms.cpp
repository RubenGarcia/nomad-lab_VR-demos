#include <stdio.h>
#include <string.h>
#include <vector>

#include "atoms.hpp"

const char * const atomNames[] =

{
	"H", "He", 
	"Li", "Be", "B", "C", "N", "O", "F", "Ne", 
	"Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", 
	"K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr", 
	"Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe", 
	"Cs", "Ba", 
		"La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu", 
			"Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn", 
	"Fr", "Ra", 
		"Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr", 
			"Rf", "Ha", "Sg", "Ns", "Hs", "Mt", "Ds", "Rg", "Cn", "Nh", "Fl", "Mc", "Lv", "Ts", "Og"
};

//rgb radius for easy transfer to texture. In AAngstrom
//not const as users may want to scale the radius
float atomColours[][4] = 
{
{1.000000, 1.000000, 1.000000, 0.310000},//H
{0.851000, 1.000000, 1.000000, 0.280000},//He
{0.800000, 0.502000, 1.000000, 1.280000},//Li
{0.761000, 1.000000, 0.000000, 0.960000},//Be
{1.000000, 0.710000, 0.710000, 0.840000},//B
{0.565000, 0.565000, 0.565000, 0.760000},//C
{0.188000, 0.314000, 0.973000, 0.710000},//N
{1.000000, 0.051000, 0.051000, 0.660000},//O
{0.565000, 0.878000, 0.314000, 0.570000},//F
{0.702000, 0.890000, 0.961000, 0.580000},//Ne
{0.671000, 0.361000, 0.949000, 1.660000},//Na
{0.541000, 1.000000, 0.000000, 1.410000},//Mg
{0.749000, 0.651000, 0.651000, 1.210000},//Al
{0.941000, 0.784000, 0.627000, 1.110000},//Si
{1.000000, 0.502000, 0.000000, 1.070000},//P
{1.000000, 1.000000, 0.188000, 1.050000},//S
{0.122000, 0.941000, 0.122000, 1.020000},//Cl
{0.502000, 0.820000, 0.890000, 1.060000},//Ar
{0.561000, 0.251000, 0.831000, 2.030000},//K
{0.239000, 1.000000, 0.000000, 1.760000},//Ca
{0.902000, 0.902000, 0.902000, 1.700000},//Sc
{0.749000, 0.761000, 0.780000, 1.600000},//Ti
{0.651000, 0.651000, 0.671000, 1.530000},//V
{0.541000, 0.600000, 0.780000, 1.390000},//Cr
{0.612000, 0.478000, 0.780000, 1.390000},//Mn
{0.878000, 0.400000, 0.200000, 1.320000},//Fe
{0.941000, 0.565000, 0.627000, 1.260000},//Co
{0.314000, 0.816000, 0.314000, 1.240000},//Ni
{0.784000, 0.502000, 0.200000, 1.320000},//Cu
{0.490000, 0.502000, 0.690000, 1.220000},//Zn
{0.761000, 0.561000, 0.561000, 1.220000},//Ga
{0.400000, 0.561000, 0.561000, 1.200000},//Ge
{0.741000, 0.502000, 0.890000, 1.190000},//As
{1.000000, 0.631000, 0.000000, 1.200000},//Se
{0.651000, 0.161000, 0.161000, 1.200000},//Br
{0.361000, 0.722000, 0.820000, 1.160000},//Kr
{0.439000, 0.180000, 0.690000, 2.200000},//Rb
{0.000000, 1.000000, 0.000000, 1.950000},//Sr
{0.580000, 1.000000, 1.000000, 1.900000},//Y
{0.580000, 0.878000, 0.878000, 1.750000},//Zr
{0.451000, 0.761000, 0.788000, 1.640000},//Nb
{0.329000, 0.710000, 0.710000, 1.540000},//Mo
{0.231000, 0.620000, 0.620000, 1.470000},//Tc
{0.141000, 0.561000, 0.561000, 1.460000},//Ru
{0.039000, 0.490000, 0.549000, 1.420000},//Rh
{0.000000, 0.412000, 0.522000, 1.390000},//Pd
{0.753000, 0.753000, 0.753000, 1.450000},//Ag
{1.000000, 0.851000, 0.561000, 1.440000},//Cd
{0.651000, 0.459000, 0.451000, 1.420000},//In
{0.400000, 0.502000, 0.502000, 1.390000},//Sn
{0.620000, 0.388000, 0.710000, 1.390000},//Sb
{0.831000, 0.478000, 0.000000, 1.380000},//Te
{0.580000, 0.000000, 0.580000, 1.390000},//I
{0.259000, 0.620000, 0.690000, 1.400000},//Xe
{0.341000, 0.090000, 0.561000, 2.440000},//Cs
{0.000000, 0.788000, 0.000000, 2.150000},//Ba
{0.439000, 0.831000, 1.000000, 2.070000},//La
{1.000000, 1.000000, 0.780000, 2.040000},//Ce
{0.851000, 1.000000, 0.780000, 2.030000},//Pr
{0.780000, 1.000000, 0.780000, 2.010000},//Nd
{0.639000, 1.000000, 0.780000, 1.990000},//Pm
{0.561000, 1.000000, 0.780000, 1.980000},//Sm
{0.380000, 1.000000, 0.780000, 1.980000},//Eu
{0.271000, 1.000000, 0.780000, 1.960000},//Gd
{0.188000, 1.000000, 0.780000, 1.940000},//Tb
{0.122000, 1.000000, 0.780000, 1.920000},//Dy
{0.000000, 1.000000, 0.612000, 1.920000},//Ho
{0.000000, 0.902000, 0.459000, 1.890000},//Er
{0.000000, 0.831000, 0.322000, 1.900000},//Tm
{0.000000, 0.749000, 0.220000, 1.870000},//Yb
{0.000000, 0.671000, 0.141000, 1.870000},//Lu
{0.302000, 0.761000, 1.000000, 1.750000},//Hf
{0.302000, 0.651000, 1.000000, 1.700000},//Ta
{0.129000, 0.580000, 0.839000, 1.620000},//W
{0.149000, 0.490000, 0.671000, 1.510000},//Re
{0.149000, 0.400000, 0.588000, 1.440000},//Os
{0.090000, 0.329000, 0.529000, 1.410000},//Ir
{0.816000, 0.816000, 0.878000, 1.360000},//Pt
{1.000000, 0.820000, 0.137000, 1.360000},//Au
{0.722000, 0.722000, 0.816000, 1.320000},//Hg
{0.651000, 0.329000, 0.302000, 1.450000},//Tl
{0.341000, 0.349000, 0.380000, 1.460000},//Pb
{0.620000, 0.310000, 0.710000, 1.480000},//Bi
{0.671000, 0.361000, 0.000000, 1.400000},//Po
{0.459000, 0.310000, 0.271000, 1.500000},//At
{0.259000, 0.510000, 0.588000, 1.500000},//Rn
{0.259000, 0.000000, 0.400000, 2.600000},//Fr
{0.000000, 0.490000, 0.000000, 2.210000},//Ra
{0.439000, 0.671000, 0.980000, 2.150000},//Ac
{0.000000, 0.729000, 1.000000, 2.060000},//Th
{0.000000, 0.631000, 1.000000, 2.000000},//Pa
{0.000000, 0.561000, 1.000000, 1.960000},//U
{0.000000, 0.502000, 1.000000, 1.900000},//Np
{0.000000, 0.420000, 1.000000, 1.870000},//Pu
{0.329000, 0.361000, 0.949000, 1.800000},//Am
{0.471000, 0.361000, 0.890000, 1.690000},//Cm
{0.541000, 0.310000, 0.890000, MISSINGRADIUS},//Bk
{0.631000, 0.212000, 0.831000, MISSINGRADIUS},//Cf
{0.702000, 0.122000, 0.831000, MISSINGRADIUS},//Es
{0.702000, 0.122000, 0.729000, MISSINGRADIUS},//Fm
{0.702000, 0.051000, 0.651000, MISSINGRADIUS},//Md
{0.741000, 0.051000, 0.529000, MISSINGRADIUS},//No
{0.780000, 0.000000, 0.400000, MISSINGRADIUS},//Lr
{0.800000, 0.000000, 0.349000, MISSINGRADIUS},//Rf
{0.820000, 0.000000, 0.310000, MISSINGRADIUS},//Ha
{0.851000, 0.000000, 0.271000, MISSINGRADIUS},//Sg
{0.878000, 0.000000, 0.220000, MISSINGRADIUS},//Ns
{0.902000, 0.000000, 0.180000, MISSINGRADIUS},//Hs
{0.922000, 0.000000, 0.149000, MISSINGRADIUS},//Mt
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Ds
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Rg
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Cn
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Nh
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Fl
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Mc
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Lv
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Ts
{MISSINGR, MISSINGG, MISSINGB, MISSINGRADIUS},//Og
};

void discardline (FILE *F)
{
int c;
do {
	c = fgetc(F);
} while (c != EOF && c != '\n');
}

int findAtom(const char *const s)
{
	//rgh FIXME, add caching
	for (int i=0;i<sizeof(atomNames)/sizeof(const char *);i++)
		if (!strcmp(s, atomNames[i]))
			return i;
	return -1;
}

char * readAtomsXYZErrors[] = {
	"All Ok",//0
	"could not open file", //-1
	"error loading atom type and position line", //-2
	"atom type unknown", //-3
};

int readAtomsXYZ(const char *const file, int **numatoms, int *timesteps, float ***pos) 
{
	int mynumatoms;
	std::vector<float*> mypos;
	std::vector<int> mynum;
	FILE *f=fopen (file, "r");
	int r;
	char s[100];
	if (f==0)
		return -1;
	*timesteps=0;
	while (!feof(f)) {
		r=fscanf(f, "%d", &mynumatoms);
		if (r<1)
			continue; //there may be a blank line at the end of the file
		(*timesteps)++;
		discardline (f);
		mypos.push_back(new float[mynumatoms*4]);
		mynum.push_back(mynumatoms);
		discardline (f); //comment
		for (int i=0;i<mynumatoms;i++) {
			r=fscanf (f, "%s %f %f %f", s, mypos.back()+4*i+0, mypos.back()+4*i+1,mypos.back()+4*i+2);
			if (r<4)
				return -2;
			int a=findAtom(s);
			if (a==-1)
				return -3;
			(mypos.back())[4*i+3]=a;
		}
	}

	*pos=new float*[*timesteps];
	*numatoms=new int[*timesteps];
	for (int i=0;i<*timesteps;i++) {
		(*pos)[i]=mypos[i];
		(*numatoms)[i]=mynum[i];
	}

	return 0;
}

char * readAtomsCubeErrors [] ={
	"All Ok", //0
	"could not open file", //-1
	"could not read number of atoms or translation", //-2
	"could not read voxel size or unit cell", //-3
	"error loading atom type and position line", //-4
};
//We will use Angstrom, convert from bohr if needed
//rgh FIXME, untested code
int readAtomsCube(const char *const file, int **numatoms, int *timesteps, float ***pos)
{
	float isoTrans[3];
	int voxelSize[3];
	float abc[9];
	FILE *f = fopen(file, "r");
	int r;
	if (f == 0)
		return -1;

	*timesteps = 1;
	*pos = new float*[*timesteps];
	*numatoms = new int[*timesteps];

	discardline(f); //two comments
	discardline(f);
	r = fscanf(f, "%d %f %f %f", *numatoms, isoTrans + 0, isoTrans + 1, isoTrans + 2);
	if (r < 4)
		return -2;

	**pos = new float[4 * **numatoms];

	for (int i = 0; i < 3; i++) {
		r = fscanf(f, "%d %f %f %f", voxelSize+i, abc + 0 + 3 * i, abc + 1 + 3 * i, abc + 2 + 3 * i);
		if (r < 4)
			return -3;
		//positive then the units are Bohr, if negative then Angstroms.
		if (voxelSize[i] < 0)
			voxelSize[i] = -voxelSize[i];
		else {
			for (int j = 0; j < 3; j++)
				abc[j + 3 * i] *= 0.52918;
		}
	}

	for (int i = 0; i < **numatoms; i++) {
		int a;
		float unused;
		r = fscanf(f, "%d %f %f %f %f", &a, &unused, &((**pos)[4 * i + 0]), &((**pos)[4 * i + 1]), &((**pos)[4 * i + 2]));
		if (r < 5)
			return -4;
		(**pos)[4 * i + 3] = a - 1;
	}

//rgh FIXME, discard the volumetric data for now

	fclose(f);
	return 0;
}