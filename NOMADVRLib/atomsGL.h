/*This code is based on openvr, which uses the 3-clause BSD license*/
/*https://github.com/ValveSoftware/openvr/blob/master/LICENSE*/
//========= Copyright Valve Corporaion ============//
/*This license is compatible with Apache 2.0*/

/*This code is therefore licensed under Apache 2.0*/
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

#ifndef __ATOMSGL_H
#define __ATOMSGL_H

#include "MyGL.h"
#include "atoms.hpp"

GLenum atomTexture(GLuint t);
GLenum SetupAtoms(GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint *BondIndices);
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer);
GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);
GLenum SetupMarker(GLuint *MarkerVAO, GLuint *MarkerVertBuffer);
GLenum SetupInfoCube (GLuint *VAO, GLuint *VertBuffer, GLuint *IndexBuffer);

bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLuint *MarkerP, 
								GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation,
								GLint *MarkerMatrixLocation, GLint *totalatomsLocation);
bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation);
bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation);
bool PrepareUnitCellShader (GLuint *cellP, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation);
bool PrepareMarkerShader (GLuint *MP, GLint *MMatrixLocation);

void GetDisplacement(int p[3], float f[3]);

int getAtomTimesteps();
inline int getTotalAtomsInTexture()
{
	return atomsInPeriodicTable+extraAtomNames.size();
}
#endif //__ATOMSGL_H

