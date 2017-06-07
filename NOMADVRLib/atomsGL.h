#ifndef __ATOMSGL_H
#define __ATOMSGL_H

#include "MyGL.h"

GLenum atomTexture(GLuint t);
GLenum SetupAtoms(GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint *BondIndices);
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer);
GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);
GLenum SetupMarker(GLuint *MarkerVAO, GLuint *MarkerVertBuffer);

bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLuint *MarkerP, 
								GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation,
								GLint *MarkerMatrixLocation);
bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation);
bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation);
bool PrepareUnitCellShader (GLuint *cellP, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation);
bool PrepareMarkerShader (GLuint *MP, GLint *MMatrixLocation);

void GetDisplacement(int p[3], float f[3]);

int getAtomTimesteps();
#endif //__ATOMSGL_H

