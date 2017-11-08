#ifndef __ATOMSGL_H
#define __ATOMSGL_H

#include "MyGL.h"
#include "atoms.hpp"

GLenum atomTexture(GLuint t);
GLenum SetupAtoms(GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint *BondIndices);
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer);
GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer);
GLenum SetupMarker(GLuint *MarkerVAO, GLuint *MarkerVertBuffer);

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

