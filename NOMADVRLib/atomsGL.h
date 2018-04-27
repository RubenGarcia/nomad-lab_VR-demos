/*This code is based on openvr, which uses the 3-clause BSD license*/
/*https://github.com/ValveSoftware/openvr/blob/master/LICENSE*/
//========= Copyright Valve Corporaion ============//
/*This license is compatible with Apache 2.0*/

/*This code is therefore licensed under Apache 2.0*/
/*
# Copyright 2016-2018 Ruben Jesus Garcia Hernandez
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
void CleanAtoms (GLuint **AtomVAO /*[4]*/, GLuint **AtomVertBuffer /*[3]*/, GLuint *BondIndices);
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer);
void CleanUnitCell (GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);
GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);
void CleanMarker (GLuint *MarkerVAO, GLuint *MarkerVertBuffer);
GLenum SetupMarker(GLuint *MarkerVAO, GLuint *MarkerVertBuffer);
GLenum SetupMarkerNoTess(GLuint *MarkerVAO, GLuint *MarkerVertBuffer, GLuint *MarkerIndexBuffer);
void CleanInfoCube  (GLuint *VAO, GLuint *VertBuffer, GLuint *IndexBuffer);
GLenum SetupInfoCube (GLuint *VAO, GLuint *VertBuffer, GLuint *IndexBuffer);

bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLuint *MarkerP, 
								GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation,
								GLint *MarkerMatrixLocation, GLint *totalatomsLocation, GLint *selectedAtomLocation);
bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation, GLint *selectedAtomLocation);
bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation);
bool PrepareUnitCellShader (GLuint *cellP, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation);
bool PrepareMarkerShader (GLuint *MP, GLint *MMatrixLocation);

void GetDisplacement(int p[3], float f[3]);

inline int getTotalAtomsInTexture()
{
	return atomsInPeriodicTable+extraAtomNames.size();
}
#endif //__ATOMSGL_H

