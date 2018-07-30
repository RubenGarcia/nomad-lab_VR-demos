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


#include <algorithm>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "ConfigFile.h"
#include "atoms.hpp"
#include "eprintf.h"
#include "polyhedron.h"

char * PATH;
char * SCREENSHOT;
int ISOS;
float **isocolours; // [ISOS][4];
const char **plyfiles;
float **translations;
float userpos[3];
float scaling;
float markerscaling;

float BACKGROUND[3];

float atomScaling;
std::vector<float> *clonedAtoms;
std::vector<int> bonds;
int *numBonds;
bool displaybonds;
float bondscolours[4];
float atomtrajectorycolour[4];

int numClonedAtoms;
int *basisvectorreps;

bool showTrajectories;
std::vector<int> atomtrajectories;
std::vector<std::vector<int>> atomtrajectoryrestarts;

bool displayunitcell;

float unitcellcolour[4];
float supercellcolour[4];

float infolinecolour[4];

int repetitions[3];
Solid *solid;

bool saveStereo;
int screenshotdownscaling;
bool hapticFeedback;
bool showcontrollers;
bool gazenavigation;
int transparencyquality;
float nearclip, farclip;

//markers such as hole positions and electron positions
float ** markers;
float ** markercolours;

float animationspeed;
float movementspeed;
int sidebuttontimestep; 

menubutton_t menubutton;

std::vector<information> info;

int secret;
const char * server;
int port;

const char * loadConfigFileErrors[] =
{
	"All Ok",//0
	"File could not be opened", //-1
	"Unrecognized parameter",//-2
	"Values with no previous iso", //-3
	"Colours with no previous iso",//-4
	"Translations with no previous correct iso",//-5
	"Isos present but no timestep value given",//-6
	"Missing values",//-7
	"Timesteps from config file and from atom file do not match",//-8, unused, now minimum is used
	"Isos parameter specified twice",//-9
	"Error reading unit cell parameters", //-10
	"Error reading repetitions",//-11
	"Non-periodic, but repetitions requested", //-12
	"No basis vectors, but repetitions requested", //-13
	"Error loading config file",// -14
	"Error reading atomglyph", //-15
	"Error reading token", //-16
	"Markers with no previous correct timesteps parameter", //-17
	"Markercolours with no previous correct timesteps parameter", //-18
	"Error reading atomcolour", // -19
	"Error reading newatom", //-20
	"Error loading xyz file, add 100 to see the error",//<-100
	"Error loading cube file, add 200 to see the error",//<-200
	"Error loading encyclopedia json file, add 300 to see the error",//<-300
	"Error loading analytics json file, add 400 to see the error",//<-400
};

void updateTIMESTEPS (int timesteps)
{ 
if (TIMESTEPS==0)
	TIMESTEPS=timesteps;
else
	TIMESTEPS=std::min(TIMESTEPS, timesteps);
}

int readString(FILE *f, char *s)
{
	char s2[2048];
	int r, c;
	r = fscanf(f, "%2047s", s2);
	if (r!=1)
		return -1;
	if (s2[0]!='"') {
		strcpy(s, s2);
		return 0;
	} else {
		strcpy(s, s2+1); //skip "
		size_t l = strlen(s);
		if (s[l-1] == '"') {
			s[l-1] = '\0';
			return 0;
		}
		else {
			char *p = s2;
			do {
				c = fgetc(f);
				*p++ = c;
			} while (c != EOF && c != '"');
			*(p-1) = '\0';
			strcat(s, s2);
		}
	}
	return 0;
}

void fixFile(char * file)
{
#ifdef WIN32
	const char c='\\';
#else
	const char c='/';
#endif
while (*file!='\0') {
	if (*file=='/' || *file=='\\')
		*file=c;
	file++;
}
}

void cleanConfig()
{
	for (int i = 0; i < ISOS; i++) {
		delete[] isocolours[i];
		delete[] translations[i];
	}
	delete[] isocolours;
	isocolours=nullptr;
	delete[] translations;
	translations=nullptr;
	if (plyfiles) {
		for (int i=0;i<ISOS;i++)
			free ((void*)(plyfiles[i])); //strdup
		delete[] plyfiles;
	}
	plyfiles=nullptr;
	free(PATH);
	PATH=nullptr;
	atomtrajectoryrestarts.clear();
	free(SCREENSHOT);
	SCREENSHOT=nullptr;

	if (markers) {
		for (int i=0;i<TIMESTEPS;i++) {
			delete[] markers[i];
			delete[] markercolours[i];
		}
		delete[] markers;
		delete[] markercolours;
		markers=nullptr;
		markercolours=nullptr;
	}
	for (int i=0;i<info.size();i++) {
		free(info[i].filename);
	}
	info.clear();

	if (numAtoms) {
		for (int i=0;i<getAtomTimesteps();i++) {
			delete[] atoms[i];
		}
		delete[] atoms;
		delete[] numAtoms;
		numAtoms=nullptr;
		atoms=nullptr;
	}

	delete[] server;
	server=nullptr;
}

void initState()
{
	BACKGROUND[0] = 0.95f;
	BACKGROUND[1] = 0.95f;
	BACKGROUND[2] = 0.95f;
	SCREENSHOT=strdup("C:\\temp\\frame");
	ISOS = 0;
	TIMESTEPS=0;
	PATH=strdup("");
	numAtoms=nullptr;
	atomScaling=1;
	clonedAtoms=0;
	fixedAtoms=false;
	bonds.clear();
	displaybonds=false;
	showTrajectories = false;
	basisvectorreps=0;
	numClonedAtoms=0;
	has_abc=false;
	for (int i=0;i<3;i++)
		repetitions[i]=1;
	for (int i=0;i<3;i++)
		userpos[i] = 0;
	for (int i=0;i<3;i++)
		supercell[i] = 1;
	solid=0;

	markers=nullptr;
	markercolours=nullptr;
	displayunitcell=false;
	scaling =1;
	markerscaling=0.8;
	for (int i=0;i<3;i++)
		cubetrans[i]=0;
	translations=nullptr;
	for (int i=0;i<3;i++)
		voxelSize[i]=-1;
	saveStereo=false;
	screenshotdownscaling=1;
	hapticFeedback=false;
	showcontrollers=false;
	gazenavigation=false;
	inv_abc_init=false;

	transparencyquality=12;
	nearclip=0.2f;
	farclip=200.f;

	for (int i=0;i<4;i++)
		unitcellcolour[i]=1.0f;
	
	supercellcolour[0]=0.0f;
	supercellcolour[1]=1.0f;
	supercellcolour[2]=1.0f;
	supercellcolour[3]=1.0f;

	bondscolours[0]=0.5f;
	bondscolours[1]=0.5f;
	bondscolours[2]=1.0f;
	bondscolours[3]=1.0f;

	atomtrajectorycolour[0]=1.0f;
	atomtrajectorycolour[1]=0.0f;
	atomtrajectorycolour[2]=0.0f;
	atomtrajectorycolour[3]=1.0f;

	animationspeed=1.0f;
	movementspeed=1.0f;
	sidebuttontimestep=-1;

	infolinecolour[0] = 1.0f;
	infolinecolour[1] = 1.0f;
	infolinecolour[2] = 0.0f;
	infolinecolour[3] = 1.0f;

	menubutton = Record;

	bondscaling = 0.7f;
	bondThickness = 1.0f;

	secret=0;
	server=nullptr;
	port=-1;
}

int loadConfigFile(const char * f)
{
	//default values
	//eprintf ("load config file start");
	bool nonperiodic=false;
	char base_url[1024]="http://enc-testing-nomad.esc.rzg.mpg.de/v1.0/materials/";
	char material[1024]="";
	initState();
	char *token=0;

	FILE *F = fopen(f, "r");
	if (F == 0)
	{
		eprintf( "Could not open config file %s\n", f);
		eprintf("Error: %d", errno);
		return -1;
	}
	char s[100];
	int r;
	while (!feof(F)) {
		r=fscanf(F, "%99s", s);
		if (r <= 0)
			continue;
		if ((unsigned char)s[0]==0xef && (unsigned char)s[1]==0xbb && (unsigned char)s[2]==0xbf) //utf bom
			strcpy (s, s+3);
		if (s[0] == '#') {//comment
			discardline(F);
			continue;
		}
		if (!strcmp(s, "timesteps")) {
			int timesteps;
			r = fscanf(F, "%d", &timesteps);
			if (TIMESTEPS==0)
				TIMESTEPS=timesteps;
			else
				TIMESTEPS=std::min(TIMESTEPS, timesteps);
		}
		else if (!strcmp(s, "isos")) {
			if (ISOS!=0) 
				return -9;
			r = fscanf(F, "%d", &ISOS);
			plyfiles = new const char*[ISOS];
			isocolours = new float*[ISOS];
			translations = new float*[ISOS];
			for (int i = 0; i < ISOS; i++) {
				plyfiles[i] = strdup("");
				isocolours[i] = new float[4];
				translations[i] = new float[3];
				//default values
				for (int j = 0; j < 4; j++)
					isocolours[i][j] = 1.f;
				for (int j = 0; j < 3; j++)
					translations[i][j] = 0.f;
			}
		}
		else if (!strcmp(s, "values")) {
			if (ISOS == 0) {
				eprintf( "values with no previous correct isos parameter\n");
				fclose(F);
				return -3;
			}
			for (int i = 0; i < ISOS; i++) {
				r=readString(F, s);
				if (r!=0)
					return -14;
				free ((void*)(plyfiles[i]));
				plyfiles[i] = strdup(s);
			}

		}
		else if (!strcmp(s, "colours")) {
			if (ISOS == 0) {
				eprintf( "colours with no previous correct isos parameter\n");
				return -4;
			}
			for (int i = 0; i < ISOS; i++) {
				for (int j = 0; j < 4; j++)
					r = fscanf(F, "%f", &(isocolours[i][j]));
				//if (r==0)
			}
		}
		else if (!strcmp(s, "path")) {
			r=readString(F, s);
			if (r!=0)
				return -14;			
			free((void*)PATH);
			PATH = strdup(s);
		}
		else if (!strcmp(s, "background")) {
			for (int i = 0; i < 3; i++)
				r = fscanf(F, "%f", BACKGROUND + i);
		}
		else if (!strcmp(s, "translations")) {
			if (ISOS == 0) {
				eprintf( "translations with no previous correct isos parameter\n");
				fclose(F);
				return -5;
			}
			for (int i=0;i<ISOS;i++)
				for (int j = 0; j < 3; j++) {
					r = fscanf(F, "%f", &(translations[i][j]));
				}
		}
		else if (!strcmp(s, "userpos")) {
			for (int j = 0; j < 3; j++) {
				r = fscanf(F, "%f", &(userpos[j]));
			}
		}
		else if (!strcmp(s, "screenshot")) {
			r=readString(F, s);
			if (r!=0)
				return -14;
			free (SCREENSHOT);
			SCREENSHOT = strdup(s);
		}
		else if (!strcmp(s, "xyzfile")||!strcmp(s, "atomfile")) {
			r=readString(F, s);
			if (r!=0)
				return -14;
			int timesteps;
			char file[256];
			sprintf (file, "%s%s", PATH, s);
			fixFile(file);
			int e;
			e=readAtomsXYZ(file, &numAtoms, &timesteps, &atoms);
			if (e<0)
				return e-100;
			updateTIMESTEPS (timesteps);
			//eprintf ("After read of xyzfile, numatoms 0 =%d", numAtoms[0]);
		}
		else if (!strcmp(s, "cubefile")) {
			r=readString(F, s);
			if (r!=0)
				return -14;
			int timesteps=TIMESTEPS;
			char file[256];
			sprintf(file, "%s%s", PATH, s);
			fixFile(file);
			int e;
			e = readAtomsCube(file, &numAtoms, &timesteps, &atoms);
			if (e<0)
				return e - 200;
			updateTIMESTEPS (timesteps);
			//	return -8;
		}
		else if (!strcmp(s, "atomscaling")) {
			r = fscanf(F, "%f", &atomScaling);
		}
		else if (!strcmp(s, "scaling")) {
			r = fscanf(F, "%f", &scaling);
		}
		else if (!strcmp(s, "abc") || !strcmp (s, "unitcell") ) {
			for (int i=0;i<3;i++)
				for (int j=0;j<3;j++) {
					r = fscanf(F, "%f", &(abc[i][j]));
					if (r!=1)
						return -10;
				}
			has_abc = true;
		}
		else if (!strcmp(s, "json") || !strcmp (s, "encyclopediajson")) {
			r=readString(F, s);
			if (r!=0)
				return -14;
			char file[256];
			sprintf(file, "%s%s", PATH, s);
			fixFile(file);
			int e;
			int timesteps;
			//rgh fixme, we know only one
			e = readAtomsJson (file, &numAtoms, &timesteps, &atoms, abc, &clonedAtoms);
			if (e<0)
				return e-300;
			numClonedAtoms=clonedAtoms[0].size()/4;
			has_abc=true;
			updateTIMESTEPS (timesteps);
		}
		else if (!strcmp(s, "analyticsjson")) {
			r = readString(F, s);
			if (r != 0)
				return -14;
			char file[256];
			sprintf(file, "%s%s", PATH, s);
			fixFile(file);
			int e;
			int timesteps;
			//rgh fixme, we know only one
			e = readAtomsAnalyticsJson(file, &numAtoms, &timesteps, &atoms, abc, &clonedAtoms);
			if (e<0)
				return e - 400;

			if (has_abc)
				numClonedAtoms = clonedAtoms[0].size() / 4;
			else
				numClonedAtoms=0;

			updateTIMESTEPS(timesteps);
		}
		else if (!strcmp(s, "baseurl")) {
			r=readString (F, base_url);
			if (r!=0)
				return -14;
		} 
		else if (!strcmp(s, "jsonurl")) {
			int e;
			int timesteps;
			r=readString (F, material);
			if (r!=0)
				return -14;
			char url[2048];
			sprintf (url, "%s%s", base_url, material);
			//rgh fixme, we know only one
			//eprintf ("load config file start, before readAtomsJsonURL");
			e = readAtomsJsonURL (url, &numAtoms, &timesteps, &atoms, abc, &clonedAtoms, token);
			//eprintf ("load config file start, after readAtomsJsonURL");
			if (e<0)
				return e-300;
			numClonedAtoms=clonedAtoms[0].size()/4;
			has_abc=true;
			updateTIMESTEPS (timesteps);
		}
		else if (!strcmp(s, "showtrajectory")) {
			showTrajectories = true;
			int atom;
			while (1 == (r = fscanf(F, "%d", &atom))) {
				atomtrajectories.push_back(atom - 1);
			}
			if (atomtrajectories.size() == 0) //empty showtrajectory
				showTrajectories = false;
		} else if (!strcmp(s, "nonperiodic")) {
			nonperiodic=true;
		} else if (!strcmp(s, "repetitions")) {
			for (int j=0;j<3;j++) {
					r = fscanf(F, "%d", repetitions+j);
					if (r!=1)
						return -11;
			}
		} else if (!strcmp(s, "atomglyph")) {
			r=fscanf (F, "%99s", s);
			if (r==0)
				return -15;
			if (!strcmp(s, "icosahedron"))
				solid=new Solid(Solid::Type::Icosahedron);
			else if(!strcmp(s, "octahedron"))
				solid=new Solid(Solid::Type::Octahedron);
			else if(!strcmp(s, "tetrahedron"))
				solid=new Solid(Solid::Type::Tetrahedron);
			else if(!strcmp(s, "sphere")) {
				int subd;
				r=fscanf (F, "%d", &subd);
				solid=new Solid(Solid::Type::Sphere, subd);
			}
			else
				return -15;
		} else if (!strcmp (s, "token")) {
			if (token)
				delete (token);
			token=new char [2048];
			r=readString (F, token);
			if (r!=0)
				return -16;
		} else if (!strcmp (s, "fixedatoms")) {
			fixedAtoms=true;
		} else if (!strcmp (s, "markers")) {
			if (TIMESTEPS == 0) {
				eprintf( "markers with no previous correct timesteps parameter\n");
				fclose(F);
				return -17;
			}
			markers=new float* [TIMESTEPS];
			for (int i=0;i<TIMESTEPS;i++) {
				markers[i]=new float[3];
				//in abc coordinates if they exist
				float tmp[3];
				for (int j = 0; j < 3; j++) {
					r = fscanf(F, "%f", &(tmp[j]));
				}
				if (has_abc) {
					//for (int s=0;s<3;s++) 
					//	tmp[s]-=translations[0][s]; //using translation of iso 0
					//check if coordinates are outside of cell
					if (translations) {
						for (int s=0;s<3;s++) {
							while (tmp[s]<-translations[0][s])
								tmp[s]+=supercell[2-s];
						}
					}
					for (int s=0;s<3;s++) {
						markers[i][s]=tmp[2]*abc[0][s]+tmp[1]*abc[1][s]+tmp[0]*abc[2][s]; //hole positions seem to be in zyx
					}
				}
				else
					for (int s=0;s<3;s++)
						markers[i][s]=tmp[s];

			}
		} else if (!strcmp (s, "markercolours")) {
		if (TIMESTEPS == 0) {
			eprintf( "markecolours with no previous correct timesteps parameter\n");
			fclose(F);
			return -18;
		}
		markercolours=new float* [TIMESTEPS];
		for (int i=0;i<TIMESTEPS;i++) {
			markercolours[i]=new float[4];
			for (int j = 0; j < 4; j++) {
				r = fscanf(F, "%f", &(markercolours[i][j]));
			}
		}
		} else if (!strcmp (s, "displaybonds")) {
			displaybonds=true;
		} else if (!strcmp (s, "atomcolour")) {
			char atom [100];
			float rgb[3];
			r = fscanf(F, "%99s %f %f %f", atom, rgb, rgb + 1, rgb + 2);
			if (r!=4) {
				eprintf ("Error loading atom colour");
				return -19;
			}
			int a=findAtom(atom);
			if (a==-1) {
				eprintf ("atomcolour, unknown atom type %s", atom);
				return -19;
			}
			for (int i=0;i<3;i++)
				atomColours[a][i]=rgb[i];
		} else if (!strcmp (s, "newatom")) {
			char atom [100];
			float rgb[3];
			float size;
			r = fscanf(F, "%99s %f %f %f %f", atom, rgb, rgb + 1, rgb + 2, &size);
			if (r!=5) {
				eprintf ("Error loading newatom");
				return -20;
			}
			int a=findAtom(atom);
			if (a!=-1) {
				if (a<atomsInPeriodicTable) {
					for (int i=0;i<3;i++)
						atomColours[a][i]=rgb[i];
					atomColours[a][3]=size;
				} else {
					for (int i=0;i<3;i++)
						extraAtomData[a-atomsInPeriodicTable][i]=rgb[i];
					extraAtomData[a-atomsInPeriodicTable][3]=size;
				}
			} else { //new
				extraAtomNames.push_back(strdup(atom));
				extraAtomData.push_back(new float[4]);
				float *e=extraAtomData.back();
				for (int i=0;i<3;i++)
					e[i]=rgb[i];
				e[3]=size;
			}
		} else if (!strcmp (s, "markerscaling")) {
			r = fscanf(F, "%f", &markerscaling);
		} else if (!strcmp (s, "displayunitcell")) {
			displayunitcell=true;
		} else if (!strcmp (s, "stereoscreenshot")) {
			saveStereo=true;
		} else if (!strcmp (s, "screenshotdownscaling")) {
			r= fscanf(F, "%d", &screenshotdownscaling);
			if (r<1)
				eprintf ("Error reading screenshotdownscaling value");
		} else if (!strcmp (s, "hapticfeedback")) {
			hapticFeedback=true;
		} else if (!strcmp (s, "supercell")) {
			r=fscanf (F, "%f %f %f", supercell, supercell+1, supercell+2);
			if (r<3)
				eprintf ("Error reading supercell value");
		} else if (!strcmp (s, "showcontrollers")) { 
			showcontrollers=true;
		} else if (!strcmp (s, "gazenavigation")) { 
			gazenavigation=true;
		} else if (!strcmp (s, "transparencyquality")) {
			r=fscanf (F, "%d", &transparencyquality);
			if (r<1)
				eprintf ("Error reading transparencyquality value");
		} else if (!strcmp (s, "clippingplanes")) {
			r=fscanf (F, "%f %f", &nearclip, &farclip);
			if (r<2)
				eprintf ("Error reading clippingplanes values");
		}else if (!strcmp (s, "bondscolour")) {
			r=fscanf (F, "%f %f %f", bondscolours, bondscolours+1, bondscolours+2);
			if (r<3)
				eprintf ("Error reading bondscolour value");
		}else if (!strcmp (s, "unitcellcolour")) {
			r=fscanf (F, "%f %f %f", unitcellcolour, unitcellcolour+1, unitcellcolour+2);
			if (r<3)
				eprintf ("Error reading unitcellcolour value");
		}else if (!strcmp (s, "supercellcolour")) {
			r=fscanf (F, "%f %f %f", supercellcolour, supercellcolour+1, supercellcolour+2);
			if (r<3)
				eprintf ("Error reading supercellcolour value");
		}else if (!strcmp (s, "atomtrajectorycolour")) {
			r=fscanf (F, "%f %f %f", atomtrajectorycolour, atomtrajectorycolour+1, atomtrajectorycolour+2);
			if (r<3)
				eprintf ("Error reading atomtrajectorycolour value");
		}else if (!strcmp (s, "infolinecolour")) {
			r=fscanf (F, "%f %f %f", infolinecolour, infolinecolour+1, infolinecolour+2);
			if (r<3)
				eprintf ("Error reading atomtrajectorycolour value");
		}
		else if (!strcmp(s, "animationspeed")) {
			r = fscanf(F, "%f", &animationspeed);
			if (r < 1)
				eprintf("Error reading animationspeed");
		}
		else if (!strcmp(s, "bondscaling")) {
			r = fscanf(F, "%f", &bondscaling);
			if (r<1)
				eprintf("Error reading bondscaling");
			bondscaling = sqrt(bondscaling);
		} else if (!strcmp(s, "bondthickness")) {
			r = fscanf(F, "%f", &bondThickness);
			if (r<1)
				eprintf("Error reading bondthickness");
		}
		else if (!strcmp(s, "menubutton")) {
			r = fscanf(F, "%99s", s);
			if (!strcmp(s, "Record"))
				menubutton = Record;
			else if (!strcmp(s, "Infobox"))
				menubutton = Infobox;
			else if (!strcmp (s, "Nothing"))
				menubutton = Nothing;
			else eprintf ("Unknown menubutton parameter %s\n", s);
		} else if (!strcmp (s, "movementspeed")) {
			r=fscanf (F, "%f", &movementspeed);
			if (r<1)
				eprintf ("Error reading movementspeed");
		} else if (!strcmp (s, "sidebuttontimestep")) {
			r=fscanf (F, "%d", &sidebuttontimestep);
			if (r<1)
				eprintf ("Error reading sidebuttontimestep");		
#ifdef WIN32
		} else if (!strcmp (s, "info")) {
			information i;
			r=fscanf (F, "%f %f %f %f %d", i.pos, i.pos+1, i.pos+2, &(i.size), &(i.atom));
			if (r<5)
				eprintf ("Error reading info");
			r=readString(F, s);
			if (r!=0)
				eprintf ("Error reading info");
			char file[256];
			sprintf (file, "%s%s", PATH, s);
			i.filename=strdup(file);
			//i.tex=LoadPNG(i.filename); //opengl not initialized yet
			info.push_back(i);
#endif
		} else if (!strcmp (s, "\x0d")) { //discard windows newline (problem in Sebastian Kokott's phone (?!)
			continue;
		} else if (!strcmp (s, "server")) { //multiuser support
			int r;
			if (server)
				delete(server);
			server=new char[100];
			r=fscanf (F, "%99s %d %d", server, &port, &secret);
			if (r<3) {
				eprintf ("Error reading server paramters");
			}
		} else {
			eprintf( "Unrecognized parameter %s\n", s);
			for (int i=0;i<strlen(s);i++)
				eprintf ("<%d>", s[i]);
			fclose(F);
			return -2;
		}
	}

//verification and additional processing
	fclose(F);

	if (ISOS != 0 && TIMESTEPS == 0) {
		eprintf("Isos requested, but no timesteps indicated\n");
		return -6;
	}
	if (ISOS !=0 && plyfiles[0] == 0) {
		eprintf( "Missing values parameter\n");
		fclose(F);
		return -7;
	}
	if (nonperiodic) {
		numClonedAtoms=0;
		if (repetitions[0]!=1 ||repetitions[1]!=1 ||repetitions[2]!=1)
			return -12;
	}
	if (repetitions[0]!=1 ||repetitions[1]!=1 ||repetitions[2]!=1) {
		if (!has_abc)
			return (-13);
		numClonedAtoms=0;
	}

	if (showTrajectories && atomtrajectories[0]<0) {
		atomtrajectories.clear();
		for (int i=0;i<*numAtoms;i++)
			atomtrajectories.push_back(i);
	}

	if (markers && !markercolours) {
		markercolours=new float* [TIMESTEPS];
		float mc [4];
		if (fabs (BACKGROUND[0]-0.5) < 0.1 && fabs (BACKGROUND[1]-0.5) < 0.1 && fabs (BACKGROUND[2]-0.5) < 0.1) //almost grey
			for (int j = 0; j < 3; j++) {
				mc[j]=0.0f;
			}
		else
			for (int j = 0; j < 3; j++) {
				mc[j]=1-BACKGROUND[j];
			}
		mc[3]=0.5;
		for (int i=0;i<TIMESTEPS;i++) {
			markercolours[i]=new float[4];
			for (int j = 0; j < 4; j++) {
				markercolours [i][j]=mc[j];
			}
		}
	}

	//chemical bonds
	//if (numAtoms) {
	//}

	//eprintf ("Before returning, numatoms 0 =%d", numAtoms[0]);
	return 0;
}
