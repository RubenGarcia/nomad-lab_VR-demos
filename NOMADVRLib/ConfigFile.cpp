#include <algorithm>

#include "ConfigFile.h"
#include "atoms.hpp"
#include "eprintf.h"
#include "polyhedron.h"

const char * PATH;
const char * SCREENSHOT;
int ISOS;
int TIMESTEPS;
float **isocolours; // [ISOS][4];
const char **plyfiles;
float **translations;
float userpos[3];

float BACKGROUND[3];
int* numAtoms; //[timesteps]
float **atoms; //[timesteps][numAtoms[i]*4] //xyzu, u=atom number
float atomScaling;
std::vector<float> *clonedAtoms;
int numClonedAtoms;
int *basisvectorreps;

bool showTrajectories;
std::vector<int> atomtrajectories;
std::vector<std::vector<int>> atomtrajectoryrestarts;

float abc[3][3]; //basis vectors
bool has_abc = false;

int repetitions[3];
Solid *solid;

const char * loadConfigFileErrors[] =
{
	"All Ok",//0
	"file could not be opened", //-1
	"unrecognized parameter",//-2
	"values with no previous iso", //-3
	"colours with no previous iso",//-4
	"translations with no previous correct iso",//-5
	"Missing isos and xyzfile",//-6
	"missing values",//-7
	"timesteps from config file and from atom file do not match",//-8, unused, now minimum is used
	"isos parameter specified twice",//-9
	"Error reading unit cell parameters", //-10
	"Error reading repetitions",//-11
	"Non-periodic, but repetitions requested", //-12
	"No basis vectors, but repetitions requested", //-13
	"Error loading config file",// -14
	"Error reading atomglyph", //-15
	"Error loading xyz file, add 100 to see the error",//<-100
	"Error loading cube file, add 100 to see the error",//<-200
	"Error loading json file, add 200 to see the error",//<-300
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
	char s2[100];
	int r, c;
	r = fscanf(f, "%99s", s2);
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

int loadConfigFile(const char * f)
{
	//default values
	eprintf ("load config file start");
	bool nonperiodic=false;
	char base_url[1024]="http://enc-testing-nomad.esc.rzg.mpg.de/v1.0/materials/";
	char material[1024]="";
	BACKGROUND[0] = 0.95f;
	BACKGROUND[1] = 0.95f;
	BACKGROUND[2] = 0.95f;
	SCREENSHOT="C:\\temp\\frame";
	ISOS = 0;
	TIMESTEPS=0;
	PATH=strdup("");
	numAtoms=0;
	atomScaling=1;
	clonedAtoms=0;
	showTrajectories = false;
	basisvectorreps=0;
	numClonedAtoms=0;
	for (int i=0;i<3;i++)
		repetitions[i]=1;
	for (int i=0;i<3;i++)
		userpos[i] = 0;
	solid=0;
	//
	FILE *F = fopen(f, "r");
	if (F == 0)
	{
		eprintf( "Could not open config file %s\n", f);
		return -1;
	}
	char s[100];
	int r;
	while (!feof(F)) {
		r=fscanf(F, "%99s", s);
		if (r <= 0)
			continue;
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
		}
		else if (!strcmp(s, "cubefile")) {
			r=readString(F, s);
			if (r!=0)
				return -14;
			int timesteps;
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
		else if (!strcmp(s, "abc")) {
			for (int i=0;i<3;i++)
				for (int j=0;j<3;j++) {
					r = fscanf(F, "%f", &(abc[i][j]));
					if (r!=1)
						return -10;
				}
			has_abc = true;
		}
		else if (!strcmp(s, "json")) {
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
			eprintf ("load config file start, before readAtomsJsonURL");
			e = readAtomsJsonURL (url, &numAtoms, &timesteps, &atoms, abc, &clonedAtoms);
			eprintf ("load config file start, after readAtomsJsonURL");
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
			r=fscanf (F, "%s", s);
			if (r==0)
				return -15;
			if (!strcmp(s, "icosahedron"))
				solid=new Solid(Solid::Type::Icosahedron);
			else if(!strcmp(s, "octahedron"))
				solid=new Solid(Solid::Type::Octahedron);
			else if(!strcmp(s, "tetrahedron"))
				solid=new Solid(Solid::Type::Tetrahedron);
			else
				return -15;
		}
		else {
			eprintf( "Unrecognized parameter %s\n", s);
			fclose(F);
			return -2;
		}
	}

//verification and additional processing
	fclose(F);

	if (ISOS == 0 && numAtoms==0) {
		eprintf( "Missing isos and atomfile parameter\n");
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

	return 0;
}