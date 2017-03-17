//Data from https://gitlab.rzg.mpg.de/nomad-lab/encyclopedia-gui/blob/lauri_viz/viztools/structure/build/js/structureviewer.js
//rgb radius for easy transference to a texture
//jmol colours from https://gitlab.com/ase/ase/blob/master/ase/data/colors.py
//RGH FIXME, add the other colour scheme when requested.
extern float atomColours[][4];

void discardline (FILE *F);
int readAtomsXYZ(const char *const file, int **numatoms, int *timesteps, float ***pos);
int readAtomsCube(const char *const file, int **numatoms, int *timesteps, float ***pos);

const float MISSINGRADIUS=0.2;
const float MISSINGR=1;
const float MISSINGG=1;
const float MISSINGB=1;

extern char * readAtomsXYZErrors[];
extern char * readAtomsCubeErrors[];

