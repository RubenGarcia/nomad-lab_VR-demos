#ifndef CONFIGFILEATOMS_H
#define CONFIGFILEATOMS_H

extern float abc[3][3]; //basis vectors
extern bool has_abc;

extern int* numAtoms; //[timesteps]
extern float **atoms; //[timesteps][numAtoms[i]*4] //xyzu, u=atom number
extern bool fixedAtoms; //all timesteps use the atoms for timestep 0. Used for excitons etc.

extern float cubetrans[3];
extern int voxelSize[3];

extern float supercell[3];

#endif //CONFIGFILEATOMS_H
