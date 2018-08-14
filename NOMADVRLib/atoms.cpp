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


#define NOMINMAX

#include <stdio.h>
#include <string.h>
#include <vector>
#include <math.h>

#ifdef _MSC_VER
#include <winsock2.h>
#endif

#include "eprintf.h"
#include "atoms.hpp"
#include "ConfigFileAtoms.h"
#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"
#include "happyhttp/happyhttp.h"

//FIXME, support more platforms
#ifdef WIN32
const char * TMPDIR="";
#else
//const char * TMPDIR="/sdcard/Oculus/NOMAD/";
const char * TMPDIR;//filled by main
//="/storage/540E-1AE2/";
#endif

bool inv_abc_init=false;
float inv_abc[3][3];
int TIMESTEPS;

std::vector<const char*> extraAtomNames;
std::vector<float*> extraAtomData;

int getAtomTimesteps() 
{
	if (fixedAtoms)
		return 1;
	else
		return TIMESTEPS;
}

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
{1.000000f, 1.000000f, 1.000000f, 0.310000f},//H
{0.851000f, 1.000000f, 1.000000f, 0.280000f},//He
{0.800000f, 0.502000f, 1.000000f, 1.280000f},//Li
{0.761000f, 1.000000f, 0.000000f, 0.960000f},//Be
{1.000000f, 0.710000f, 0.710000f, 0.840000f},//B
{0.565000f, 0.565000f, 0.565000f, 0.760000f},//C
{0.188000f, 0.314000f, 0.973000f, 0.710000f},//N
{1.000000f, 0.051000f, 0.051000f, 0.660000f},//O
{0.565000f, 0.878000f, 0.314000f, 0.570000f},//F
{0.702000f, 0.890000f, 0.961000f, 0.580000f},//Ne
{0.671000f, 0.361000f, 0.949000f, 1.660000f},//Na
{0.541000f, 1.000000f, 0.000000f, 1.410000f},//Mg
{0.749000f, 0.651000f, 0.651000f, 1.210000f},//Al
{0.941000f, 0.784000f, 0.627000f, 1.110000f},//Si
{1.000000f, 0.502000f, 0.000000f, 1.070000f},//P
{1.000000f, 1.000000f, 0.188000f, 1.050000f},//S
{0.122000f, 0.941000f, 0.122000f, 1.020000f},//Cl
{0.502000f, 0.820000f, 0.890000f, 1.060000f},//Ar
{0.561000f, 0.251000f, 0.831000f, 2.030000f},//K
{0.239000f, 1.000000f, 0.000000f, 1.760000f},//Ca
{0.902000f, 0.902000f, 0.902000f, 1.700000f},//Sc
{0.749000f, 0.761000f, 0.780000f, 1.600000f},//Ti
{0.651000f, 0.651000f, 0.671000f, 1.530000f},//V
{0.541000f, 0.600000f, 0.780000f, 1.390000f},//Cr
{0.612000f, 0.478000f, 0.780000f, 1.390000f},//Mn
{0.878000f, 0.400000f, 0.200000f, 1.320000f},//Fe
{0.941000f, 0.565000f, 0.627000f, 1.260000f},//Co
{0.314000f, 0.816000f, 0.314000f, 1.240000f},//Ni
{0.784000f, 0.502000f, 0.200000f, 1.320000f},//Cu
// This is the standard colour
{0.490000f, 0.502000f, 0.690000f, 1.220000f},//Zn
// This is Andris's colour
//{0.625f,0.125f,0.9375f, 1.220000f},//Zn

{0.761000f, 0.561000f, 0.561000f, 1.220000f},//Ga
{0.400000f, 0.561000f, 0.561000f, 1.200000f},//Ge
{0.741000f, 0.502000f, 0.890000f, 1.190000f},//As
{1.000000f, 0.631000f, 0.000000f, 1.200000f},//Se
{0.651000f, 0.161000f, 0.161000f, 1.200000f},//Br
{0.361000f, 0.722000f, 0.820000f, 1.160000f},//Kr
{0.439000f, 0.180000f, 0.690000f, 2.200000f},//Rb
{0.000000f, 1.000000f, 0.000000f, 1.950000f},//Sr
{0.580000f, 1.000000f, 1.000000f, 1.900000f},//Y
{0.580000f, 0.878000f, 0.878000f, 1.750000f},//Zr
{0.451000f, 0.761000f, 0.788000f, 1.640000f},//Nb
{0.329000f, 0.710000f, 0.710000f, 1.540000f},//Mo
{0.231000f, 0.620000f, 0.620000f, 1.470000f},//Tc
{0.141000f, 0.561000f, 0.561000f, 1.460000f},//Ru
{0.039000f, 0.490000f, 0.549000f, 1.420000f},//Rh
{0.000000f, 0.412000f, 0.522000f, 1.390000f},//Pd
{0.753000f, 0.753000f, 0.753000f, 1.450000f},//Ag
{1.000000f, 0.851000f, 0.561000f, 1.440000f},//Cd
{0.651000f, 0.459000f, 0.451000f, 1.420000f},//In
{0.400000f, 0.502000f, 0.502000f, 1.390000f},//Sn
{0.620000f, 0.388000f, 0.710000f, 1.390000f},//Sb
{0.831000f, 0.478000f, 0.000000f, 1.380000f},//Te
{0.580000f, 0.000000f, 0.580000f, 1.390000f},//I
{0.259000f, 0.620000f, 0.690000f, 1.400000f},//Xe
{0.341000f, 0.090000f, 0.561000f, 2.440000f},//Cs
{0.000000f, 0.788000f, 0.000000f, 2.150000f},//Ba
{0.439000f, 0.831000f, 1.000000f, 2.070000f},//La
{1.000000f, 1.000000f, 0.780000f, 2.040000f},//Ce
{0.851000f, 1.000000f, 0.780000f, 2.030000f},//Pr
{0.780000f, 1.000000f, 0.780000f, 2.010000f},//Nd
{0.639000f, 1.000000f, 0.780000f, 1.990000f},//Pm
{0.561000f, 1.000000f, 0.780000f, 1.980000f},//Sm
{0.380000f, 1.000000f, 0.780000f, 1.980000f},//Eu
{0.271000f, 1.000000f, 0.780000f, 1.960000f},//Gd
{0.188000f, 1.000000f, 0.780000f, 1.940000f},//Tb
{0.122000f, 1.000000f, 0.780000f, 1.920000f},//Dy
{0.000000f, 1.000000f, 0.612000f, 1.920000f},//Ho
{0.000000f, 0.902000f, 0.459000f, 1.890000f},//Er
{0.000000f, 0.831000f, 0.322000f, 1.900000f},//Tm
{0.000000f, 0.749000f, 0.220000f, 1.870000f},//Yb
{0.000000f, 0.671000f, 0.141000f, 1.870000f},//Lu
{0.302000f, 0.761000f, 1.000000f, 1.750000f},//Hf
{0.302000f, 0.651000f, 1.000000f, 1.700000f},//Ta
{0.129000f, 0.580000f, 0.839000f, 1.620000f},//W
{0.149000f, 0.490000f, 0.671000f, 1.510000f},//Re
{0.149000f, 0.400000f, 0.588000f, 1.440000f},//Os
{0.090000f, 0.329000f, 0.529000f, 1.410000f},//Ir
{0.816000f, 0.816000f, 0.878000f, 1.360000f},//Pt
{1.000000f, 0.820000f, 0.137000f, 1.360000f},//Au
{0.722000f, 0.722000f, 0.816000f, 1.320000f},//Hg
{0.651000f, 0.329000f, 0.302000f, 1.450000f},//Tl
{0.341000f, 0.349000f, 0.380000f, 1.460000f},//Pb
{0.620000f, 0.310000f, 0.710000f, 1.480000f},//Bi
{0.671000f, 0.361000f, 0.000000f, 1.400000f},//Po
{0.459000f, 0.310000f, 0.271000f, 1.500000f},//At
{0.259000f, 0.510000f, 0.588000f, 1.500000f},//Rn
{0.259000f, 0.000000f, 0.400000f, 2.600000f},//Fr
{0.000000f, 0.490000f, 0.000000f, 2.210000f},//Ra
{0.439000f, 0.671000f, 0.980000f, 2.150000f},//Ac
{0.000000f, 0.729000f, 1.000000f, 2.060000f},//Th
{0.000000f, 0.631000f, 1.000000f, 2.000000f},//Pa
{0.000000f, 0.561000f, 1.000000f, 1.960000f},//U
{0.000000f, 0.502000f, 1.000000f, 1.900000f},//Np
{0.000000f, 0.420000f, 1.000000f, 1.870000f},//Pu
{0.329000f, 0.361000f, 0.949000f, 1.800000f},//Am
{0.471000f, 0.361000f, 0.890000f, 1.690000f},//Cm
{0.541000f, 0.310000f, 0.890000f, MISSINGRADIUS},//Bk
{0.631000f, 0.212000f, 0.831000f, MISSINGRADIUS},//Cf
{0.702000f, 0.122000f, 0.831000f, MISSINGRADIUS},//Es
{0.702000f, 0.122000f, 0.729000f, MISSINGRADIUS},//Fm
{0.702000f, 0.051000f, 0.651000f, MISSINGRADIUS},//Md
{0.741000f, 0.051000f, 0.529000f, MISSINGRADIUS},//No
{0.780000f, 0.000000f, 0.400000f, MISSINGRADIUS},//Lr
{0.800000f, 0.000000f, 0.349000f, MISSINGRADIUS},//Rf
{0.820000f, 0.000000f, 0.310000f, MISSINGRADIUS},//Ha
{0.851000f, 0.000000f, 0.271000f, MISSINGRADIUS},//Sg
{0.878000f, 0.000000f, 0.220000f, MISSINGRADIUS},//Ns
{0.902000f, 0.000000f, 0.180000f, MISSINGRADIUS},//Hs
{0.922000f, 0.000000f, 0.149000f, MISSINGRADIUS},//Mt
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

float atomRadius (int i)
{
	if (i<atomsInPeriodicTable)
		return atomColours[i][3];
	else
		return extraAtomData[i-atomsInPeriodicTable][3];
}

void discardline (FILE *F)
{
int c;
do {
	c = fgetc(F);
} while (c != EOF && c != '\n');
}



int findAtom(const char *const s)
{
	//discard number at end
	//rgh: I have C1A, so better to go from the end to avoid this being confused with C
	char x[10];
	strcpy (x, s);
	char *p=x+strlen(x);
	p--;
	while (*p >= '0' && *p <= '9') {
		*p='\0';
		*p--;
		if (p==x)
			break;
	}
		
	/*
	char *p=x; 
	const char *q=s;
	while (*q!=0) {
		if (*q >='0' && *q <= '9')
			break;
		*p++=*q++;
	}
	*p=0;
	*/
	//rgh FIXME, add caching
	for (unsigned int i=0;i<sizeof(atomNames)/sizeof(const char *);i++)
		if (!strcmp(x, atomNames[i]))
			return i;
	//now check extra atoms, starting at atomsInPeriodicTable
	for (unsigned int i=0;i<extraAtomNames.size();i++)
		if (!strcmp(x, extraAtomNames[i]))
			return i+atomsInPeriodicTable;
	return -1;
}

const char * readAtomsXYZErrors[] = {
	"All Ok",//0
	"Could not open file", //-1
	"Error loading atom type and position line", //-2
	"Atom type unknown", //-3
	"Corrupt xyz file", //-4
	"Unable to download xyz file" //-5
};

void cleanAtoms (int **numatoms, int timesteps, float ***pos)
{
	for (int i=0;i<timesteps;i++) {
		delete[] ((*pos)[i]);
	}
	delete[] (*numatoms);
	*numatoms=nullptr;
	delete[] (*pos);
	*pos=nullptr;
}

int readAtomsXYZ(const char *const file, int **numatoms, int *timesteps, float ***pos) 
{
	const char *myfile=nullptr;
	const char *webdownload="material.xyz";
	//add http support
	if (!strncmp(file, "http:", 5) || !strncmp(file, "https:", 6)) {
		char cmd[2048];
		int ret;
		sprintf (cmd, "wget %s -O %s", file, webdownload);
		ret=system(cmd);
		if (ret!=0) {
			*numatoms=nullptr;
			*pos=nullptr;
			return (-5);
		}
		myfile=webdownload;
	} else {
		myfile = file;
	}
	int mynumatoms;
	std::vector<float*> mypos;
	std::vector<int> mynum;
	FILE *f=fopen (myfile, "r");
	int r;
	char s[100];
	if (f==0) {
		eprintf ("Error opening file %s", file);
		return -1;
	}
	*timesteps=0;
	int blanklines=0;
	while (!feof(f)) {
		r=fscanf(f, "%d", &mynumatoms);
		if (r<1) {
			blanklines++;
			if (blanklines>3) {
				eprintf("Corrupt xyz file %s. Error at %d atoms\n", file, mynumatoms);
				fclose(f);
				return -4;
			}
			continue; //there may be a blank line at the end of the file
		}
		blanklines=0;
		(*timesteps)++;
		discardline (f);
		//eprintf ("Getting atoms, mynumatoms=%d",mynumatoms);
		mypos.push_back(new float[mynumatoms*4]);
		mynum.push_back(mynumatoms);
		discardline (f); //comment
		for (int i=0;i<mynumatoms;i++) {
			float unused;
			//read by lines as otherwise it will swallow the new line to fill the unused
			char line[512];
			fgets(line, 512, f);
			r=sscanf (line, "%s %f %f %f %f", s, mypos.back()+4*i+0, mypos.back()+4*i+1,mypos.back()+4*i+2, &unused);
			if (r<4) {
				fclose (f);
				return -2;
			}
			int a=findAtom(s);
			if (a==-1) {
				eprintf ("Read atoms xyz, atom type unknown: %s", s);
				a=83; //rgh fixme, this is unknown, unknown unknown, make a real unknown at the end.
				//return -3;
			}
			(mypos.back())[4*i+3]=(float)a;
			//RGH FIXME, add cloned atoms
		}
	}

	*pos=new float*[*timesteps];
	*numatoms=new int[*timesteps];
	for (int i=0;i<*timesteps;i++) {
		(*pos)[i]=mypos[i];
		(*numatoms)[i]=mynum[i];
		//eprintf ("Getting atoms, numatoms=%d",(*numatoms)[i]);
	}
	fclose (f);
	return 0;
}

const char * readAtomsCubeErrors [] ={
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
	//float isoTrans[3];
	//float abc[9];
	has_abc=true;
	FILE *f = fopen(file, "r");
	int r;
	if (f == 0) {
		eprintf ("Error opening file %s", file);
		return -1;
	}
	if (!fixedAtoms) {
		*timesteps = 1;
		*pos = new float*[*timesteps];
		*numatoms = new int[*timesteps];
	} else {
		*pos = new float*[1];
		*numatoms = new int[1];
	}

	discardline(f); //two comments
	discardline(f);
	r = fscanf(f, "%d %f %f %f", *numatoms, cubetrans + 0, cubetrans + 1, cubetrans + 2);
	if (r < 4) {
		fclose (f);
		return -2;
	}
	//rgh FIXME. Is this always bohr?
	for (int i=0;i<3;i++)
		cubetrans[i]*= 0.52918f;

	**pos = new float[4 * **numatoms];
	bool bohr=true;

	// dnabok <dnabok@physik.hu-berlin.de>  2/6/2017, 3:36 PM: vectors are c, b, a
	for (int i = 0; i < 3; i++) {
		r = fscanf(f, "%d %f %f %f", voxelSize+(i), &(abc[i][0]), &(abc[i][1]), &(abc[i][2]));
		if (r < 4)
			return -3;
		//positive then the units are Bohr, if negative then Angstroms.
		if (voxelSize[i] < 0) { //angstrom, no change
			voxelSize[i] = -voxelSize[i];
			bohr=false;
		} else { //bohr
			for (int j = 0; j < 3; j++)
				abc[i][j] *= 0.52918f;
		}
	}

	for (int i = 0; i < **numatoms; i++) {
		int a;
		float tmp[3];
		float charge;
		r = fscanf(f, "%d %f %f %f %f", &a, &charge, tmp, tmp+1, tmp+2);
		if (r < 5)
			return -4;

		//atoms are not in abc coordinates, but in bohr (I suppose that also ansgrom if voxelsize < 0)
		if (bohr)
			for (int s=0;s<3;s++)
				(**pos)[4*i+s]=tmp[s]*0.52918f;
		else 
			for (int s=0;s<3;s++)
				(**pos)[4*i+s]=tmp[s];

		(**pos)[4 * i + 3] = float(a - 1);
	}


	for (int i=0;i<3;i++)
		for (int j=0;j<3;j++)
			abc[i][j]*=(voxelSize[i]-1)/supercell[i]; 

//rgh FIXME, discard the volumetric data for now

	fclose(f);
	return 0;
}

FILE *out;

void OnData( const happyhttp::Response* r, void* userdata, const unsigned char* data, int n )
{
	if (out!=nullptr)
		fwrite( data,1,n, out );
	else
		eprintf ("null file pointer when saving network data to disk");
}

const char * readAtomsJsonErrors[] = {
	"All Ok",//0
	"could not open file", //-1
	"error parsing json", //-2
	"error downloading json", //-3
	"error reading json position", //-4
	"error reading json vector", //-5
};

int readAtomsJsonURL (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],
					  std::vector<float>** clonedAtoms, const char *const token)
{
//eprintf ("readAtomsJsonURL start");

try {
char host[2048], page[2048], url[2048], file[2048];
int port=80;
int r;
//http://stackoverflow.com/questions/726122/best-ways-of-parsing-a-url-using-c
r=sscanf (f,  "http://%2047[^/:]:%d%2047[^\n]", host, &port, page);
if (r==1)
	r=sscanf (f,  "http://%2047[^/]%2047[^\n]", host, page);
if (r<2) { 
#if defined(WIN32) || defined(CAVE)
	//possibly https or other unsupported protocol, fall back to wget 
	return readAtomsJsonURLwget (f, numatoms, timesteps, pos, abc, clonedAtoms, token);
#else
	//GetUrl("https://www.google.de/", "google_index.html"); //rgh test
	return -3;
#endif
}
sprintf (url, "%s%s", page, "/cells?pagination=off");
sprintf (file, "%s%s", TMPDIR, "material_cells.json");
out=fopen(file , "w");
if (out==nullptr) {
	eprintf ("Could not open file for writing: %s", file);
	return -1;
}
happyhttp::Connection conn( host, port );
conn.setcallbacks( nullptr, OnData, nullptr, 0 );

//https://github.com/Zintinio/HappyHTTP/issues/9

const char*headers[3];
char base64[2048];
if (token) {
	headers[0]="Authorization";
	sprintf (base64, "Basic %s", token);
	headers[1]=base64;
	headers[2]=0;
	conn.request( "GET", url, headers, 0,0 );
} else {
	conn.request( "GET", url, 0, 0,0 );
}


while( conn.outstanding() )
	conn.pump();
fclose(out);
conn.close();
sprintf (url, "%s%s", page, "/elements?pagination=off");
sprintf (file, "%s%s", TMPDIR, "material_elements.json");
out=fopen(file , "w");

if (out==nullptr) {
	eprintf ("Could not open file for writing: %s", file);
	return -1;
}

if (token) {
	conn.request( "GET", url, headers, 0,0 );
} else {
	conn.request( "GET", url, 0, 0,0 );
}

while( conn.outstanding() )
	conn.pump();
} catch (const happyhttp::Wobbly& w) {
#ifdef _MSC_VER
	int e=	WSAGetLastError();
	eprintf ("error %s, wsa error %d\n", w.what(), e);
#else
	eprintf( "error %s\n", w.what());
#endif
	fclose(out);

#if defined(WIN32) || defined(CAVE)
	return readAtomsJsonURLwget (f, numatoms, timesteps, pos, abc, clonedAtoms, token);
#endif
	return -3;
}
fclose(out);

char file [2048];
sprintf (file, "%s%s", TMPDIR, "material");
//eprintf ("readAtomsJsonURL before return");
return readAtomsJson (file, numatoms, timesteps, pos, abc, clonedAtoms, token);
}

#if defined(WIN32) || defined(CAVE)
//base64 encoded token
int readAtomsJsonURLwget (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],
						  std::vector<float>** clonedAtoms, const char *const token)
{
char cmd[2048];
int ret;
if (token)
	sprintf (cmd, "wget --no-check-certificate --header \"Authorization:Basic %s\" %s/cells?pagination=off -O %smaterial_cells.json", token, f, TMPDIR);
else
	sprintf (cmd, "wget %s/cells?pagination=off -O %smaterial_cells.json", f, TMPDIR);
ret=system(cmd);
if (ret!=0) 
	return (-3);
if (token)
	sprintf (cmd, "wget --no-check-certificate --header \"Authorization:Basic %s\" %s/elements?pagination=off -O %smaterial_elements.json", token, f, TMPDIR);
else
	sprintf (cmd, "wget %s/elements?pagination=off -O %smaterial_elements.json", f, TMPDIR);
ret=system(cmd);
if (ret!=0) 
	return(-3);
sprintf (cmd, "%smaterial", TMPDIR);
return readAtomsJson (cmd, numatoms, timesteps, pos, abc, clonedAtoms, token);
}
#endif

bool  isAlmostZero(float coordinate) 
{   
	return (fabs(coordinate) < 1E-5);
}

void add (std::vector<float> *v, float x, float y, float z, float a)
{
	v->push_back(x);
	v->push_back(y);
	v->push_back(z);
	v->push_back(a);
}

const char * readAtomsAnalyticsJsonErrors[] = {
	"All Ok",//0
	"Could not open file", //-1
	"atom_species not in json", //-2
	"json parse error or no atom_positions", //-3
	"lattice_vectors not in json", //-4
};

int readAtomsAnalyticsJson(const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],
	std::vector<float>** clonedAtoms)
{
	float tmppos[3];

	FILE *fp = fopen(f, "r");
	if (fp == 0) {
		eprintf("readAtomsAnalyticsJson, could not open file %s", f);
		return -1;
	}
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document json;
	json.ParseStream(is);
	fclose(fp);
	*timesteps = 0;
	//later, when we know the timesteps
	//*pos = new float*[*timesteps];
	//*numatoms = new int[*timesteps];
	//*clonedAtoms=new std::vector<float>[*timesteps];
	if (!json.HasParseError()) {
		//section_system
		if (!json.HasMember("section_system"))
			return -3;
		const rapidjson::GenericValue<rapidjson::UTF8<> > &ss = json ["section_system"];
		if (!ss.IsArray())
			return -3;

		//first check how many of the arrrays have atoms, then fill the information

		for (rapidjson::SizeType i = 0; i < ss.Size(); i++) {
			const rapidjson::GenericValue<rapidjson::UTF8<> > & el=ss[i];
			if (el.HasMember("atom_positions")) 
				(*timesteps)++;
		}

		if (*timesteps==0) {
			eprintf ("No atom_positions in json");
			return -3;
		} 

		*pos = new float*[*timesteps];
		*numatoms = new int[*timesteps];
		*clonedAtoms=new std::vector<float>[*timesteps];
		for (int i=0;i<*timesteps;i++)
			(*clonedAtoms)[i].clear();

		//Rubén García: Per-timestep lattice vectors not supported. 
		//Using only the first ones

		int currenttime=0;
		for (rapidjson::SizeType i = 0; i < ss.Size(); i++) {
			const rapidjson::GenericValue<rapidjson::UTF8<> > & el=ss[i];

			if (!el.HasMember("lattice_vectors") && !el.HasMember("atom_positions")) 
				continue;

			if (currenttime==0 && el.HasMember("lattice_vectors")) {
				// take the lattice vectors only once; verify they are real vectors
				const rapidjson::GenericValue<rapidjson::UTF8<> > &lv = el["lattice_vectors"];
				const rapidjson::GenericValue<rapidjson::UTF8<> > &flatdata = lv["flatData"];
				bool allzeros=true;
				for (int i=0;i<3;i++)
					for (int j=0;j<3;j++) {
						abc[i][j]=flatdata[i*3+j].GetFloat()*1e10; // meter -> aangstrom
						if (abc[i][j]!=0)
							allzeros=false;
					}
				if (!allzeros)
					has_abc=true;
			}

			if (!el.HasMember("atom_positions")) 
				continue;
			const rapidjson::GenericValue<rapidjson::UTF8<> > &result = el["atom_positions"];
			if (!result.HasMember("shape"))
				return (-1);
			const rapidjson::GenericValue<rapidjson::UTF8<> > &shape = result["shape"]; //[numatoms, 3]
			(*numatoms)[currenttime] = shape[0].GetInt();
			(*pos)[currenttime] = new float[**numatoms * 4];
			if (el.HasMember("atom_species")) {
				const rapidjson::GenericValue<rapidjson::UTF8<> > &results = el["atom_species"];
				for (int i = 0; i < **numatoms ; i++)
					(*pos)[currenttime][i*4+3]=results[i].GetInt()-1;


				//atoms are stored in space coordinates, in meters
				const rapidjson::GenericValue<rapidjson::UTF8<> > &flatdata = result["flatData"];
				for (int i = 0; i < **numatoms ; i++) {
					for (int j=0;j<3;j++)
						(*pos)[currenttime][i*4+j] = flatdata[i*3+j].GetFloat()*1e10; //we store them in aangstrom

					if (has_abc)
						CloneSpatialAtoms((*pos)[currenttime], (*pos)[currenttime][3], (*clonedAtoms)+currenttime);
				}
			} // if atom_species
			else return (-2);
			currenttime++;
		} // for ss.size
	} // json parse error
	else return -3;

	if (has_abc)
		for (int i=0;i<*timesteps;i++)
			TransformAtoms((*clonedAtoms)+i, abc);
	return 0;
}

int readAtomsJson (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3], 
				   std::vector<float>** clonedAtoms, const char *const token)
{
//	eprintf ("readAtomsJson start");
	char file[512];
	int r;
	sprintf (file, "%s_cells.json", f);
	FILE *fcells=fopen (file, "r");
	if (fcells==0) {
		eprintf ("readAtomsJson, could not open file %s", file);
		return -1;
	}
	char readBuffer[65536];
	rapidjson::FileReadStream is(fcells, readBuffer, sizeof(readBuffer));
	rapidjson::Document json;
	json.ParseStream(is);
	fclose(fcells);

	int total_results;
	if (!json.HasParseError() && json.HasMember("total_results") && json["total_results"].IsInt())
		total_results=json["total_results"].GetInt();
	else
		return -2;

	if (!json.HasMember("results"))
		return -2;
	const rapidjson::GenericValue<rapidjson::UTF8<> > &results=json["results"];
	for (int i=0;i<total_results;i++) {
		const rapidjson::GenericValue<rapidjson::UTF8<> > &result=results[i];
		//rgh FIXME, both are the same structure.
		//encyclopaedia is displaying the non-primitive=Bravais, do the same for now
		if (!result.HasMember("is_primitive") || result["is_primitive"].IsBool()==false)
			return -2;
		if (result["is_primitive"].GetBool()==true)
			continue;
		if (!result.HasMember("a") || !result.HasMember("b") || !result.HasMember("c"))
			return -2;
		if (!result["a"].IsString() || !result["b"].IsString() || !result["c"].IsString())
			return -2;
		const char *myabc[3]={result["a"].GetString(), result["b"].GetString(), result["c"].GetString()};
		bool allzeros=true;
		for (int j=0;j<3;j++) {
			r=sscanf(myabc[j], "(%f,%f,%f)", &(abc[j][0]), &(abc[j][1]), &(abc[j][2]));
			if (r!=3)
				return -5;
			for (int k=0;k<3;k++) {
				abc[j][k]*=1e10; //using angstrom internally
				if (abc[j][k]!=0)
					allzeros=false;
			}
		}

		if (!allzeros)
			has_abc=true;

	}


	sprintf (file, "%s_elements.json", f);
	FILE *felements=fopen (file, "r");
	if (felements==0) {
		eprintf ("readAtomsJson, could not open file %s", file);
		return -1;
	}
	rapidjson::FileReadStream is2(felements, readBuffer, sizeof(readBuffer));
	json.ParseStream(is2);
	fclose(felements);

	*timesteps=1;
	*pos = new float*[*timesteps];
	*numatoms = new int[*timesteps];

	if (!json.HasMember("total_results") || !json["total_results"].IsInt())
		return -2;
	**numatoms=json["total_results"].GetInt();
	**pos=new float[4* **numatoms];

	*clonedAtoms=new std::vector<float>[*timesteps];
	const rapidjson::GenericValue<rapidjson::UTF8<> >& a = json["results"];
	float tmppos[3];
	for (int i=0;i< **numatoms;i++)
	{
		const rapidjson::GenericValue<rapidjson::UTF8<> > &result=a[i];
		if (!result.HasMember("label"))
			return -2;
		const rapidjson::GenericValue<rapidjson::UTF8<> > &label=result["label"];
		if (!label.IsInt())
			return -2;
		int k=label.GetInt()-1;
		(**pos)[4*i + 3]=(float)k;
		const char *stringpos=result["position"].GetString();
		
		r=sscanf(stringpos, "(%f,%f,%f)", tmppos+0, tmppos+1, tmppos+2);
		if (r!=3)
			return -4;
		//clone 
		//https://gitlab.mpcdf.mpg.de/nomad-lab/encyclopedia-gui/blob/lauri_viz/viztools/structure/src/typescript/structureviewer.ts
		//Rubén García, abc may be all 0 values; then we should not clone
		if (has_abc) {
			Clone (tmppos, (float)k, *clonedAtoms);
		//atom positions in the abc domain, must multiply.
			for (int s=0;s<3;s++)
				(**pos)[4*i+s]=tmppos[0]*abc[0][s]+tmppos[1]*abc[1][s]+tmppos[2]*abc[2][s];
		}
	
	}
	if (has_abc) 
		TransformAtoms(*clonedAtoms, abc);

//eprintf ("readAtomsJson, end");
return 0;
}

//from abc coordinates to spatial coordinates
void TransformAtoms(std::vector<float>* clonedAtoms, const float abc[3][3])
{
	float tmppos[3];
	for (unsigned int o=0;o<clonedAtoms->size()/4;o++) {
		for (int s=0;s<3;s++)
			tmppos[s]=(*clonedAtoms)[o*4+s];
		for (int s=0;s<3;s++)
			(*clonedAtoms)[o*4+s]=tmppos[0]*abc[0][s]+tmppos[1]*abc[1][s]+tmppos[2]*abc[2][s];
	}
}

//returns false if abc is not an invertible matrix (should not happen)
bool CloneSpatialAtoms (float tmppos[3], float k, std::vector<float>* clonedAtoms) 
{ //move atoms to abc coordinates, then Clone()

	if (!inv_abc_init) {
		inv_abc_init=true;
		//Contains code from OpenVR samples/shared/Matrices.cpp under the
		//BSD 3-clause "New" or "Revised" License
		//https://github.com/ValveSoftware/openvr/blob/master/LICENSE
		float tmp[9];
		float determinant;

    tmp[0] = abc[1][1] * abc[2][2] - abc[1][2] * abc[2][1];
    tmp[1] = abc[0][2] * abc[2][1] - abc[0][1] * abc[2][2];
    tmp[2] = abc[0][1] * abc[1][2] - abc[0][2] * abc[1][1];
    tmp[3] = abc[1][2] * abc[2][0] - abc[1][0] * abc[2][2];
    tmp[4] = abc[0][0] * abc[2][2] - abc[0][2] * abc[2][0];
    tmp[5] = abc[0][2] * abc[1][0] - abc[0][0] * abc[1][2];
    tmp[6] = abc[1][0] * abc[2][1] - abc[1][1] * abc[2][0];
    tmp[7] = abc[0][1] * abc[2][0] - abc[0][0] * abc[2][1];
    tmp[8] = abc[0][0] * abc[1][1] - abc[0][1] * abc[1][0];

    // check determinant if it is 0
    determinant = abc[0][0] * tmp[0] + abc[0][1] * tmp[3] + abc[0][2] * tmp[6];
    if(fabs(determinant) <= 1e-5)
    {
        return false; // cannot inverse, make it idenety matrix
    }

    // divide by the determinant
    float invDeterminant = 1.0f / determinant;
    inv_abc[0][0] = invDeterminant * tmp[0];
    inv_abc[0][1] = invDeterminant * tmp[1];
    inv_abc[0][2] = invDeterminant * tmp[2];
    inv_abc[1][0] = invDeterminant * tmp[3];
    inv_abc[1][1] = invDeterminant * tmp[4];
    inv_abc[1][2] = invDeterminant * tmp[5];
    inv_abc[2][0] = invDeterminant * tmp[6];
    inv_abc[2][1] = invDeterminant * tmp[7];
    inv_abc[2][2] = invDeterminant * tmp[8];
	}

	float t[3];
	for (int s=0;s<3;s++)
			t[s]=tmppos[0]*inv_abc[0][s]+tmppos[1]*inv_abc[1][s]+tmppos[2]*inv_abc[2][s];

	Clone (t, k, clonedAtoms);
	return true;
}

//tmppos in abc coordinates
void Clone (float tmppos[3], float k, std::vector<float>* clonedAtoms) 
{
bool iaz[3];
for (int q=0;q<3;q++)
	iaz[q]=isAlmostZero(tmppos[q]);

if (iaz[0] && iaz[1] &&iaz[2]){
		add(clonedAtoms, 0,0,1, k);
		add(clonedAtoms, 0,1,0, k);
		add(clonedAtoms, 0,1,1, k);
		add(clonedAtoms, 1,0,0, k);
		add(clonedAtoms, 1,0,1, k);
		add(clonedAtoms, 1,1,0, k);
		add(clonedAtoms, 1,1,1, k);
} else if (iaz[0] && iaz[1]) {
		add(clonedAtoms, 1,0,tmppos[2], k);
		add(clonedAtoms, 0,1,tmppos[2], k);
		add(clonedAtoms, 1,1,tmppos[2], k);
} else if (iaz[1] && iaz[2]) {
		add(clonedAtoms, tmppos[0],0,1, k);
		add(clonedAtoms, tmppos[0],1,0, k);
		add(clonedAtoms, tmppos[0],1,1, k);
} else if (iaz[0] && iaz[2]) {
		add(clonedAtoms, 0,tmppos[1],1, k);
		add(clonedAtoms, 1,tmppos[1],0, k);
		add(clonedAtoms, 1,tmppos[1],1, k);
} else if (iaz[0]) {
	add(clonedAtoms, 1,tmppos[1], tmppos[2], k);
} else if (iaz[1]) {
	add(clonedAtoms, tmppos[0],1,tmppos[2], k);
} else if (iaz[2]) {
	add(clonedAtoms, tmppos[0],tmppos[1], 1, k);
}

}
