//Data from https://gitlab.rzg.mpg.de/nomad-lab/encyclopedia-gui/blob/lauri_viz/viztools/structure/build/js/structureviewer.js
//rgb radius for easy transference to a texture
//jmol colours from https://gitlab.com/ase/ase/blob/master/ase/data/colors.py
//RGH FIXME, add the other colour scheme when requested.
extern float atomColours[][4];

int readAtomsXYZ(const char *const file, int **numatoms, int *timesteps, float ***pos);
int readAtomsCube(const char *const file, int **numatoms, int *timesteps, float ***pos);
int readAtomsJson (const char *const file, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms);
int readAtomsJsonURL (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms);
int readAtomsJsonURLwget (const char *const f, int **numatoms, int *timesteps, float ***pos, float abc[3][3],  std::vector<float>** clonedAtoms);

const float MISSINGRADIUS=0.2;
const float MISSINGR=1;
const float MISSINGG=1;
const float MISSINGB=1;

extern char * readAtomsXYZErrors[];
extern char * readAtomsCubeErrors[];
extern char * readAtomsJsonErrors[];
extern char * readAtomsJsonErrors[];

//internal functions
void discardline (FILE *F);
void Clone (float tmppos[3], float k, std::vector<float>* clonedAtoms);
void TransformAtoms(std::vector<float>* clonedAtoms, const float abc[3][3]);
