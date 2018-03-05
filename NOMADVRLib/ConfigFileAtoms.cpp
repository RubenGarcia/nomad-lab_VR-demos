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

float abc[3][3]; //basis vectors
bool has_abc = false;

int* numAtoms; //[timesteps]
float **atoms; //[timesteps][numAtoms[i]*4] //xyzu, u=atom number
bool fixedAtoms; 

float cubetrans[3];

float supercell[3];
int voxelSize[3];
