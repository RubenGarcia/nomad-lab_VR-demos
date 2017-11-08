//Data from https://gitlab.rzg.mpg.de/nomad-lab/encyclopedia-gui/blob/lauri_viz/viztools/structure/build/js/structureviewer.js
//rgb radius for easy transference to a texture
//jmol colours from https://gitlab.com/ase/ase/blob/master/ase/data/colors.py
//RGH FIXME, add the other colour scheme when requested.

#ifndef __ATOMS_H
#define __ATOMS_H

#include <stdio.h>
#include <vector>

extern float atomColours[][4];

const int atomsInPeriodicTable=118;

extern const char * TMPDIR;

int readAtomsXYZ(const char *const file, int **numatoms, int *timesteps, float ***pos);
int readAtomsCube(const char *const file, int **numatoms, int *timesteps, float ***pos);
int readAtomsJson (const char *const file, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms, const char *const token=0);
int readAtomsAnalyticsJson(const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],
	std::vector<float>** clonedAtoms);
int readAtomsJsonURL (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms, const char *const token=0);
#if defined(WIN32) || defined(CAVE)
int readAtomsJsonURLwget (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms, const char *const token=0);
#endif
const float MISSINGRADIUS=0.2f;
const float MISSINGR=1.f;
const float MISSINGG=1.f;
const float MISSINGB=1.f;

extern const char * readAtomsXYZErrors[];
extern const char * readAtomsCubeErrors[];
extern const char * readAtomsJsonErrors[];
extern const char * readAtomsAnalyticsJsonErrors[];

float atomRadius (int i);
int findAtom(const char *const s);

//internal functions
void discardline (FILE *F);
void Clone (float tmppos[3], float k, std::vector<float>* clonedAtoms);
bool CloneSpatialAtoms (float tmppos[3], float k, std::vector<float>* clonedAtoms);
void TransformAtoms(std::vector<float>* clonedAtoms, const float abc[3][3]);

extern bool inv_abc_init;
extern float inv_abc[3][3];

extern std::vector<const char*> extraAtomNames;
extern std::vector<float*> extraAtomData;

#endif //__ATOMS_H
