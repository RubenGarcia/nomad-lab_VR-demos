/*
# Copyright 2016-2018 The NOMAD Developers Group
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

#ifndef __CONFIGFILE_H
#define __CONFIGFILE_H
#include <vector>
#include "MyGL.h"
#include "polyhedron.h"

extern const char * PATH;
extern const char * SCREENSHOT;
extern int ISOS;
extern int TIMESTEPS;
extern float **isocolours; // [ISOS][4];
extern const char **plyfiles;
extern float **translations;
extern float userpos[3];
extern float scaling;

extern float BACKGROUND[3];
extern int* numAtoms; //[timesteps]
extern float **atoms; //[timesteps][numAtoms[i]*4] //xyzu, u=atom number
extern bool fixedAtoms; //all timesteps use the atoms for timestep 0. Used for excitons etc.
extern float atomScaling;
extern std::vector<float> *clonedAtoms;
extern std::vector<int> bonds;
extern int *numBonds;
extern bool displaybonds;
extern float bondscolours[4];
extern float atomtrajectorycolour[4];

extern int numClonedAtoms;
extern int *basisvectorreps;

extern bool showTrajectories;
extern std::vector<int> atomtrajectories;
extern std::vector<std::vector<int>> atomtrajectoryrestarts;

extern float abc[3][3]; //basis vectors
extern bool has_abc;
extern bool displayunitcell;
extern float supercell[3];
extern float markerscaling;
extern float unitcellcolour[4];
extern float supercellcolour[4];

extern float infolinecolour[4];

extern int repetitions[3];

extern Solid *solid;

extern bool saveStereo;
extern int screenshotdownscaling;

extern bool hapticFeedback;
extern bool showcontrollers;
extern bool gazenavigation;

extern int transparencyquality;
extern float nearclip, farclip;

extern float animationspeed; //how fast to change to next timestep
extern float movementspeed;  //how fast to move the user

//markers such as hole positions and electron positions
extern float ** markers;
extern float ** markercolours;
extern float cubetrans[3];
extern int voxelSize[3];

extern const char * loadConfigFileErrors[];

int loadConfigFile(const char * f);

struct information {
	float pos[3];
	float size;
	int atom; //-1=do not draw line
	const char* filename;
	GLuint tex;
};

extern std::vector<information> info;

#endif //__CONFIGFILE_H
