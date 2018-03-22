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

extern float bondscaling;

#endif //CONFIGFILEATOMS_H
