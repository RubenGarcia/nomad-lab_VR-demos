float abc[3][3]; //basis vectors
bool has_abc = false;

int* numAtoms; //[timesteps]
float **atoms; //[timesteps][numAtoms[i]*4] //xyzu, u=atom number
bool fixedAtoms; 

float cubetrans[3];

float supercell[3];
int voxelSize[3];