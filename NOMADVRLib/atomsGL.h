#ifndef __ATOMSGL_H
#define __ATOMSGL_H

#include "MyGL.h"

GLenum atomTexture(GLuint t);
GLenum SetupAtoms(GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint *BondIndices);
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer);
GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);

bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation);
bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation);
bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation);
bool PrepareUnitCellShader (GLuint *cellP, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation);

void GetDisplacement(int p[3], float f[3]);

int getAtomTimesteps();
#endif //__ATOMSGL_H

