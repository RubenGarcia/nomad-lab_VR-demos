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


#include <math.h>

#include "eprintf.h"
#include "TessShaders.h"
#include "UnitCellShaders.h"
#include "markerShaders.h"
#include "atomsGL.h"
#include "atoms.hpp"
#include "ConfigFile.h"
#include "CompileGLShader.h"
#include "polyhedron.h"
#include "Grid.h"

int getAtomTimesteps() 
{
	if (fixedAtoms)
		return 1;
	else
		return TIMESTEPS;
}

GLenum atomTexture(GLuint t)
{
	GLenum e;
	int finalatoms=getTotalAtomsInTexture();
	//rgh: scale atoms here
	//in google cardboard, this is called again if the program is running, so leave original or atoms get progresivelly smaller!
	float *a=new float[finalatoms*4];
	for (int i = 0; i < atomsInPeriodicTable; i++) {
		a[i*4+0]=atomColours[i][0];
		a[i*4+1]=atomColours[i][1];
		a[i*4+2]=atomColours[i][2];
		a[i*4+3]=atomColours[i][3] * atomScaling;
	}
	for (int i=0;i<extraAtomNames.size();i++) {
		a[(i+atomsInPeriodicTable)*4+0]=extraAtomData[i][0];
		a[(i+atomsInPeriodicTable)*4+1]=extraAtomData[i][1];
		a[(i+atomsInPeriodicTable)*4+2]=extraAtomData[i][2];
		a[(i+atomsInPeriodicTable)*4+3]=extraAtomData[i][3]*atomScaling;
	}
	glBindTexture(GL_TEXTURE_2D, t); //atom texture
	if ((e = glGetError()) != GL_NO_ERROR) {
		eprintf("opengl error %d, atomTexture bind\n", e);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if ((e = glGetError()) != GL_NO_ERROR) {
		eprintf("opengl error %d, atomTexture parameter\n", e);
	}
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, finalatoms, 1, 0, GL_RGBA, GL_FLOAT, a);
	if ((e = glGetError()) != GL_NO_ERROR) {
		eprintf("opengl error %d, atomTexture glTexImage2D\n", e);
	}
	glBindTexture( GL_TEXTURE_2D, 0 );
	if ((e = glGetError()) != GL_NO_ERROR) {
		eprintf("opengl error %d, atomTexture\n", e);
	}
	delete [] a;
	return e;
}

//WARNING: This should be called after SetupAtoms
//This means that numAtoms now has the cummulative distribution!
//This should be called after the atom texture is prepared, and therefore has the atomscaling pre-multiplied
GLenum SetupAtomsNoTess (GLuint **AtomVAO /*[4]*/, GLuint **AtomVertBuffer/*[3]*/, GLuint **AtomIndexBuffer/*[2]*/)
	//atoms, cloned atoms
	//rgh: FIXME: add AtomVAO[2] for atom trajectories
{
if (!numAtoms)
		return 0;

if (!solid) {
	eprintf ("SetupAtomsNoTess, error: no solid defined");
	return 0;
}

	//http://prideout.net/blog/?p=48 //public domain code
	//xyz nxnynz u=atom type ; 7 floats; u only used for colour
	int e;

	int totalatoms=numAtoms[getAtomTimesteps() -1];
	
	*AtomVAO = new GLuint[4]; //atoms, cloned atoms, unused (bonds use Tess atom positions), trajectories
	*AtomIndexBuffer= new GLuint[3];//atoms, cloned atoms, bonds
	*AtomVertBuffer = new GLuint[3];//atoms, cloned atoms, trajectories

	glGenVertexArrays(4, *AtomVAO);
	glGenBuffers(2, *AtomIndexBuffer);
	glGenBuffers(3, *AtomVertBuffer);
	glBindVertexArray((*AtomVAO)[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*AtomIndexBuffer)[0]);
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[0]);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	float *tmp = new float[solid->nVerts * 7 * totalatoms];
#ifdef INDICESGL32		
	int *tmpi = new int[solid->nFaces*3 * totalatoms];
	int *currenti=tmpi;
#else
	unsigned short *tmpi = new unsigned short[solid->nFaces*3 * totalatoms];
	unsigned short *currenti=tmpi;
#endif

	float *current=tmp;
	//eprintf ("Before For 1");
	for (int p=0;p<getAtomTimesteps() ;p++) {
		for (int a = 0; a < numAtoms[p]-(p==0?0:numAtoms[p-1]); a++) {
			const int atomNumber = static_cast<int>(atoms[p][4 * a + 3]);
			const float radius = atomRadius(atomNumber)*atomScaling;
			for (int i = 0; i < solid->nVerts; i++) { //verts
				for (int k = 0; k < 3; k++) {
					*current++ = solid->Verts[3 * i + k]* radius +atoms[p][4 * a + k]; //pos
				}
				for (int k = 0; k < 3; k++) {
					*current++ = solid->Verts[3 * i + k]; //normal
				}
				*current++ = static_cast<float>(atomNumber);
			} //i
			for (int i = 0; i < solid->nFaces * 3; i++)
				*currenti++ = solid->Faces[i] + (a+(p==0?0:numAtoms[p-1]))*solid->nVerts;
		} //a
	} //p
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) *totalatoms* 7 * solid->nVerts, tmp,
			GL_STATIC_DRAW);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
#ifdef INDICESGL32		
	sizeof(int)
#else
	sizeof(unsigned int)
#endif
		* totalatoms * 3 * solid->nFaces, tmpi, GL_STATIC_DRAW);


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));

	if (glGetError() != GL_NO_ERROR)
		eprintf("opengl error attrib pointer 0\n");

	delete[] tmp;
	delete[] tmpi;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of SetupAtoms, l %d\n", e, __LINE__);

	//FIXME TODO: cloned atoms
	tmp = new float[solid->nVerts * 7 * numClonedAtoms];
	current=tmp;
#ifdef INDICESGL32		
	tmpi = new int[solid->nFaces*3 * numClonedAtoms];
	currenti=tmpi;
#else
	tmpi = new unsigned short[solid->nFaces*3 * numClonedAtoms];
	currenti=tmpi;
#endif

	for (int a = 0; a < numClonedAtoms; a++) {
		const int atomNumber = static_cast<int>(clonedAtoms[0][4 * a + 3]);
		const float radius = atomRadius(atomNumber)*atomScaling;
		for (int i = 0; i < solid->nVerts; i++) { //verts
				for (int k = 0; k < 3; k++) {
					*current++ = solid->Verts[3 * i + k]* radius +clonedAtoms[0][4 * a + k]; //pos
				}
				for (int k = 0; k < 3; k++) {
					*current++ = solid->Verts[3 * i + k]; //normal
				}
				*current++ =  static_cast<float>(atomNumber);
		} //i
		for (int i = 0; i < solid->nFaces * 3; i++)
			*currenti++ = solid->Faces[i] + a*solid->nVerts;
	} //a
	
	glBindVertexArray((*AtomVAO)[1]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*AtomIndexBuffer)[1]);
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[1]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) *numClonedAtoms* 7 * solid->nVerts, tmp,
			GL_STATIC_DRAW);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);
	//eprintf ("After bufferdata, array buffer");
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
#ifdef INDICESGL32		
	sizeof(int)
#else
	sizeof(unsigned int)
#endif
		* numClonedAtoms * 3 * solid->nFaces, tmpi, GL_STATIC_DRAW);
	//eprintf ("After bufferdata, element array buffer");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glVertexAttribPointer, l %d\n", e, __LINE__);

	delete[] tmp;
	delete[] tmpi;

	glBindVertexArray(0);
	return e;
} //SetupAtomsNoTess


GLenum SetupAtoms(GLuint **AtomVAO /*[4]*/, GLuint **AtomVertBuffer /*[3]*/, GLuint *BondIndices)
{
	if (!numAtoms)
		return glGetError();
	//rgh FIXME: put this all in the same vao
	
	//http://prideout.net/blog/?p=48 //public domain code
	//xyz u=atom type ; 4 floats
	int e;

	int totalatoms=0;
	for (int i=0;i<getAtomTimesteps() ;i++) {
		totalatoms += numAtoms[i];
	}
	eprintf("SetupAtoms: totalatoms=%d", totalatoms);

	*AtomVAO = new GLuint[4]; //atoms, cloned atoms, bonds, trajectories
	*AtomVertBuffer = new GLuint[3]; //atoms, cloned atoms, trajectories

	glGenVertexArrays(4, *AtomVAO);
	glGenBuffers(3, *AtomVertBuffer);
	glGenBuffers(1, BondIndices);

	glBindVertexArray((*AtomVAO)[0]);
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	e=glGetError();
	if (e!=GL_NO_ERROR)
		eprintf ("gl error %d, %s %d", e, __FILE__, __LINE__);
	float *tmp = new float[4 * totalatoms];
	float *current=tmp;
	
	const int atomlimit=30;

	for (int p=0;p<getAtomTimesteps() ;p++) {
		for (int a = 0; a < numAtoms[p]; a++) {
			for (int k = 0; k < 4; k++) {
				*current++ = atoms[p][4 * a + k];
			}
		} //a
	}

	if (!displaybonds) {
		numBonds=nullptr;
		for (int p=1; p<getAtomTimesteps() ;p++) 
				numAtoms[p]+=numAtoms[p-1];
	} else {
		numBonds=new int[getAtomTimesteps() ];
		//can be slow, add loading screen here if Vive
		for (int p=0;p<getAtomTimesteps() ;p++) {
 
		if (numAtoms[p]<atomlimit) {
			//eprintf ("searching bonds basic");
			//bonds FIXME quadractic complexity	
					for (int a1=0; a1 < numAtoms[p]; a1++) {
						for (int a2=a1+1; a2 < numAtoms[p]; a2++){
							float d=0, r;
							for (int k=0;k<3;k++) {
								float dif=atoms[p][4 * a1 + k]-atoms[p][4 * a2 + k];
								d+=dif*dif;
							}
							r=atomRadius(static_cast<int>(atoms[p][4 * a1 + 3]))+
								atomRadius(static_cast<int>(atoms[p][4 * a2 + 3]));
							if (d*bondscaling<r*r) {// bond
								bonds.push_back(a1+(p==0?0:numAtoms[p-1]));
								bonds.push_back(a2+(p==0?0:numAtoms[p-1]));
							}
						}
					}
			} else { //more than 30 atoms, try grid optimization
			//eprintf ("searching bonds grid");

				float m[3];
				float M[3];
				for (int k=0; k<3;k++) {
					m[k]=M[k]=atoms[p][k];
				}
				for (int a = 1; a < numAtoms[p]; a++) {
					for (int k=0; k<3;k++) {
						if (m[k]>atoms[p][4*a+k])
							m[k]=atoms[p][4*a+k];
						if (M[k]<atoms[p][4*a+k])
							M[k]=atoms[p][4*a+k];
					}
				}
				grid g(m, M, pow(numAtoms[p], 1.0f/3.0f), bondscaling);
				for (int a = 1; a < numAtoms[p]; a++) 
					g.add(atoms[p]+4*a);
				for (int a = 0; a < numAtoms[p]; a++) {
					std::vector<float*> found=g.find(atoms[p]+4*a);
					for (int b=0;b<found.size();b++) {
						//if (found[b] < tmp+4*a) // already got this bound
						//	continue;
						bonds.push_back(a+(p==0?0:numAtoms[p-1]));
						bonds.push_back(((found[b]-atoms[p])/4)+(p==0?0:numAtoms[p-1]));
					}
				}
			}
			numBonds[p]=bonds.size();
			if (p!=0)
				numAtoms[p]+=numAtoms[p-1];
		} //p
	} // showbonds
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(0));
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(3 * sizeof(float)));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * totalatoms * 4 , tmp,
		GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glBufferData, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, end of SetupAtoms, l %d\n", e, __LINE__);
	
	if (showTrajectories) {
			//fill the restart buffer
		//use abc for measuring
		float max=0;
		if (has_abc) {
			for (int i=0;i<3;i++)
				for (int j=0;j<3;j++)
				max+=abc[i][j];
			max /=9*2;
		}

		for (unsigned int t=0;t<atomtrajectories.size();t++) {
			atomtrajectoryrestarts.push_back(std::vector<int>());
			atomtrajectoryrestarts[t].push_back(0);
			for (int p=1;p<getAtomTimesteps() ;p++) {
				int a=atomtrajectories[t];
				if (has_abc)
					if (fabs(atoms[p][a*4+0]-atoms[p-1][a*4+0])+
						fabs(atoms[p][a*4+1]-atoms[p-1][a*4+1])+
						fabs(atoms[p][a*4+2]-atoms[p-1][a*4+2])>max)
							atomtrajectoryrestarts[t].push_back(p);
			}
			atomtrajectoryrestarts[t].push_back(getAtomTimesteps() );
		}
	//need to setup a specific buffer because of GL_MAX_VERTEX_ATTRIB_STRIDE
	//only need xyz, not atom size
	//rgh FIXME: If we use index buffer instead, GPU storage is 1/3 of this
		float *traj = new float[atomtrajectories.size()*TIMESTEPS*3];
		for (unsigned int t = 0; t < atomtrajectories.size(); t++) {
			for (int i=0;i<TIMESTEPS;i++)
				for (int j = 0; j < 3; j++) {
					traj[t*TIMESTEPS * 3 + i * 3 + j] = tmp[i*numAtoms[0]*4+
																+atomtrajectories[t]*4
																+j];
				}
		}
		glBindVertexArray((*AtomVAO)[3]);
		glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) *atomtrajectories.size()*TIMESTEPS * 3, traj,
			GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
		glEnableVertexAttribArray(0);
		e = glGetError();
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, creating atom trajectories, l %d\n", e, __LINE__);

		delete[] traj;
	}
	delete[] tmp;
	//bonds
	if (displaybonds) {
		glBindVertexArray((*AtomVAO)[2]);
		glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *BondIndices);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*bonds.size(), bonds.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(0));
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	
		e=glGetError();
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, creating chemical bonds, l %d\n", e, __LINE__);
	}
	//now clones
	if (basisvectorreps ||!clonedAtoms) //do not replicate
		return e;


	glBindVertexArray((*AtomVAO)[1]); //rgh FIXME, only works for TIMESTEPS=1
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * clonedAtoms[0].size(), clonedAtoms[0].data(),
			GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(0));
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, end of Setup cloned Atoms, l %d\n", e, __LINE__);

	//rgh: we will need these again if we don't have tesselation
	//delete[] clonedAtoms;
	//clonedAtoms=0;
	glBindVertexArray(0);
	return e;
}

GLenum SetupInfoCube (GLuint *VAO, GLuint *VertBuffer, GLuint *IndexBuffer)
{
	glGenVertexArrays(1, VAO);
	glGenBuffers(1, VertBuffer);
	glGenBuffers(1, IndexBuffer);

	glBindVertexArray(*VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, *VertBuffer);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	//vec4 pos, vec3 normal, vec2 uv
	const int Nvert=9*26;
	const GLfloat vert[]={
		-1, +1, -1, 0,		0, 0, -1,	0, 1, //-z
		-1, -1, -1,	0,		0, 0, -1,	0, 0,
		+1, +1, -1,	0,		0, 0, -1,	1, 1,
		+1, -1, -1,	0,		0, 0, -1,	1, 0,
		-1, +1, +1, 0,		0, 0, 1,	0, 0,//+z
		-1, -1, +1,	0,		0, 0, 1,	0, 1,
		+1, +1, +1,	0,		0, 0, 1,	1, 0,
		+1, -1, +1,	0,		0, 0, 1,	1, 1,

		+1, -1, -1, 0,		+1, 0, 0,	0, 1,//+x
		+1, -1, +1, 0,		+1, 0, 0,	0, 0,//+x
		+1, +1, -1, 0,		+1, 0, 0,	1, 1,//+x
		+1, +1, +1, 0,		+1, 0, 0,	1, 0,//+x
		-1, -1, -1, 0,		-1, 0, 0,	0, 0,//-x
		-1, -1, +1, 0,		-1, 0, 0,	0, 1,//-x
		-1, +1, -1, 0,		-1, 0, 0,	1, 0,//-x
		-1, +1, +1, 0,		-1, 0, 0,	1, 1,//-x

		-1, 1, +1, 0,		0, -1, 0,	0, 1, //+y
		-1, 1, -1, 0,		0, -1, 0,	0, 0,
		+1, 1, +1, 0,		0, -1, 0,	1, 1,
		+1, 1, -1, 0,		0, -1, 0,	1, 0,
		-1, -1, +1, 0,		0, +1, 0,	0, 0,//-y
		-1, -1, -1, 0,		0, +1, 0,	0, 1,
		+1, -1, +1, 0,		0, +1, 0,	1, 0,
		+1, -1, -1, 0,		0, +1, 0,	1, 1,
		0, 0, 0, 1,			0,0,0,		0,0, //for the line between the cube and the atom
		0, 0, 1, 1,			0, 0, 0,	0, 0, //for the line between the cube and the atom

	};
	const short int ind[]={
		0, 1, 2, //z
		1, 3, 2,
		4, 5, 6,
		5, 7, 6,
		8, 9, 10,//x
		9, 11, 10,
		12, 13, 14,
		13, 15, 14,
		16, 17, 18,//y
		17, 19, 18,
		20, 21, 22,
		21, 23, 22,
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Nvert , vert,
			GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind), ind, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)(4*sizeof(float)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)(7*sizeof(float)));
	glBindVertexArray(0);

	return glGetError();
}

float getMarkerLobeScaling(int l, int k)
{
	if (l == k)
		return 2.0f;
	return 0.5f;
}

GLenum SetupMarkerNoTess(GLuint *MarkerVAO, GLuint *MarkerVertBuffer, GLuint *MarkerIndexBuffer)
{
	if (!markers)
		return glGetError();

	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, begin of SetupMarkerNoTess\n", e, __LINE__);

	glGenVertexArrays(1, MarkerVAO);
	glGenBuffers(1, MarkerVertBuffer);
	glGenBuffers(1, MarkerIndexBuffer);

	glBindVertexArray(*MarkerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, *MarkerVertBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *MarkerIndexBuffer);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (const void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (const void *)(3 * sizeof(float)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (const void *)(6 * sizeof(float)));

	float *tmp = new float[10 * MARKERSOLID::nVerts*3* TIMESTEPS]; //xyz, nxnynz, rgba; compatible with IsoShaders
	float *current = tmp;
	for (int i = 0; i < TIMESTEPS; i++) {
		for (int l = 0; l < 3; l++) {//3 ellipsoids
			for (int j = 0; j < MARKERSOLID::nVerts; j++) {
				for (int k = 0; k < 3; k++) { //pos
					float s = getMarkerLobeScaling(l, k);
					*current++ = s*MARKERSOLID::Verts[j * 3 + k] * markerscaling * atomScaling * atomRadius(0) +
						markers[i][k];
				}
				float length=0;
				for (int k = 0; k < 3; k++) { //normal; normalized in IsoShader
					float s = getMarkerLobeScaling(l, k);
					*current = MARKERSOLID::Verts[j * 3 + k]*s;
					length += (*current)*(*current);
					current++;
				}
				length=1.0f / sqrtf(length);
				for (int k=0;k<3;k++) {
					*(current-1-k)*=length;
				}
				for (int k = 0; k < 4; k++) { //colour
					*current++ = markercolours[i][k];
				}
			}
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 10 * MARKERSOLID::nVerts * 3 * TIMESTEPS, tmp,
		GL_STATIC_DRAW);

	delete[] tmp;
#ifdef INDICESGL32		
	int *tmpi=new int[TIMESTEPS * 3 * Icosahedron::nFaces * 3];
	int * currenti;
#else
	short *tmpi = new short[TIMESTEPS * 3 * Icosahedron::nFaces * 3];
	short *currenti;
#endif

	currenti = tmpi;
	for (int i = 0; i < TIMESTEPS; i++) {
		for (int l = 0; l < 3; l++) {//ellipsoids
			for (int j = 0; j < MARKERSOLID::nFaces; j++) {
				for (int k = 0; k < 3; k++) {
					*currenti++ = MARKERSOLID::Faces[j * 3 + k] + 
						MARKERSOLID::nVerts * (l+3*i);
				}
			}
		}
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
#ifdef INDICESGL32		
		sizeof(int)*MARKERSOLID::nFaces * 3 * TIMESTEPS * 3
#else
		sizeof(unsigned int)*MARKERSOLID::nFaces * 3 * TIMESTEPS * 3
#endif
		, tmpi, GL_STATIC_DRAW);
	delete[] tmpi;
	glBindVertexArray(0);
	return e;
}

GLenum SetupMarker(GLuint *MarkerVAO, GLuint *MarkerVertBuffer)
{//requires tesselation
	if (!markers)
		return glGetError();
	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, begin of SetupMarker\n", e, __LINE__);

	glGenVertexArrays(1, MarkerVAO);
	glGenBuffers(1, MarkerVertBuffer);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glGenBuffers, l %d\n", e, __LINE__);

	glBindVertexArray(*MarkerVAO);
	glBindBuffer(GL_ARRAY_BUFFER, *MarkerVertBuffer);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	const float size=atomRadius(0)*atomScaling*markerscaling;
	float *tmp = new float [8*TIMESTEPS];
	for (int i=0;i<TIMESTEPS;i++) {
		for (int j=0;j<3;j++) { //center [3]
			tmp[i*8+j]=markers[i][j];
		}
		tmp[i*8+3]=0.8*size; //size [1]
		for (int j=0;j<4;j++) {//colour[4]
			tmp[i*8+4+j]=markercolours[i][j];
		}
	}
	
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * TIMESTEPS*8 , tmp,
			GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)(0));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (const void *)(4*sizeof(float)));
	glBindVertexArray(0);
	return glGetError();
}

GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer)
{
	//add here both unit cell and supercell
	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, begin of SetupUnitCell\n", e, __LINE__);
	if (!has_abc)
		return e;
	glGenVertexArrays(1, UnitCellVAO);
	glGenBuffers(1, UnitCellVertBuffer);
	glGenBuffers(1, UnitCellIndexBuffer);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glGenBuffers, l %d\n", e, __LINE__);

	glBindVertexArray(*UnitCellVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *UnitCellIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, *UnitCellVertBuffer);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	float *tmp = new float[3*8*2];
	//0, a, b, c, a+b+c, b+c, a+c, a+b
	for (int i=0;i<3;i++) { //unit cell
		tmp[0+i]=0;
		for (int j=0;j<3;j++)
			tmp[3*(j+1)+i]=abc[j][i];
		tmp[3*4+i]=abc[0][i]+abc[1][i]+abc[2][i];
		tmp[3*5+i]=			abc[1][i]+abc[2][i];
		tmp[3*6+i]=abc[0][i]+		abc[2][i];
		tmp[3*7+i]=abc[0][i]+abc[1][i];
	}
	float displ[3]={0,0,0};
	if (translations && ISOS) 
		for (int j=0;j<3;j++)
			for (int i=0;i<3;i++)
				displ[i]+=-translations[0][i]*abc[j][i];
	for (int i=0;i<3;i++) { //rgh fixme, add displacement here as well
		tmp[3*8+i]=displ[i];
		for (int j=0;j<3;j++)
			tmp[3*(j+8+1)+i]=abc[j][i]*supercell[j]+displ[i];
		tmp[3*12+i]=abc[0][i]*supercell[0]+abc[1][i]*supercell[1]+abc[2][i]*supercell[2]+displ[i];
		tmp[3*13+i]=			abc[1][i]*supercell[1]+abc[2][i]*supercell[2]+displ[i];
		tmp[3*14+i]=abc[0][i]*supercell[0]+		abc[2][i]*supercell[2]+displ[i];
		tmp[3*15+i]=abc[0][i]*supercell[0]+abc[1][i]*supercell[1]+displ[i];
	}
	int tmpi[12*2*2]={ //lines, unit cell, 
		0,1, 
		1,6,
		6,3,
		3,0,
		2,7,
		7,4,
		4,5,
		5,2,
		0,2,
		1,7,
		6,4,
		3,5, // supercell
		0+8,1+8, 
		1+8,6+8,
		6+8,3+8,
		3+8,0+8,
		2+8,7+8,
		7+8,4+8,
		4+8,5+8,
		5+8,2+8,
		0+8,2+8,
		1+8,7+8,
		6+8,4+8,
		3+8,5+8
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3*8*2 , tmp,
			GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glBufferData vertex, l %d\n", e, __LINE__);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tmpi), tmpi, GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glBufferData index, l %d\n", e, __LINE__);
	glBindVertexArray(0);
	return e;
}


bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLuint *MarkerP, 
								GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation,
								GLint *MarkerMatrixLocation, GLint *totalatomsLocation, GLint *selectedAtomLocation){
	if (!PrepareAtomShader(AtomP, AtomMatrixLocation, totalatomsLocation, selectedAtomLocation))
		return false;

	if (!PrepareUnitCellShader(cellP, UnitCellMatrixLocation, UnitCellColourLocation))
		return false;

	if (!PrepareMarkerShader(MarkerP, MarkerMatrixLocation))
		return false;

	return true;
}

bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation, GLint *selectedAtomLocation){
		//https://www.gamedev.net/topic/591110-geometry-shader-point-sprites-to-spheres/
	//no rotation, only translations means we can do directional lighting in the shader.
	//FIXME
	//http://stackoverflow.com/questions/40101023/flat-shading-in-webgl
	*AtomP = CompileGLShader(
		AtomShaders[SHADERNAME],
		AtomShaders[SHADERVERTEX],
		AtomShaders[SHADERFRAGMENT],
		AtomShaders[SHADERTESSEVAL]
		);
	*AtomMatrixLocation=glGetUniformLocation(*AtomP, "matrix");
	if( *AtomMatrixLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in atom shader\n" );
		return false;
	}
	*totalatomsLocation=glGetUniformLocation(*AtomP, "totalatoms");
	if( *totalatomsLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in atom shader\n" );
		return false;
	}
	*selectedAtomLocation=glGetUniformLocation(*AtomP, "selectedAtom");
	if( *totalatomsLocation == -1 )
	{
		eprintf( "Unable to find selectedAtom uniform in atom shader\n" );
		return false;
	}
	return true;
}

bool PrepareMarkerShader (GLuint *MP, GLint *MMatrixLocation){
		//https://www.gamedev.net/topic/591110-geometry-shader-point-sprites-to-spheres/
	//no rotation, only translations means we can do directional lighting in the shader.
	//FIXME
	//http://stackoverflow.com/questions/40101023/flat-shading-in-webgl
	*MP = CompileGLShader(
		MarkerShaders[SHADERNAME],
		MarkerShaders[SHADERVERTEX],
		MarkerShaders[SHADERFRAGMENT],
		MarkerShaders[SHADERTESSEVAL]
		);
	*MMatrixLocation=glGetUniformLocation(*MP, "matrix");
	if( *MMatrixLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in atom shader\n" );
		return false;
	}
	return true;
}

bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation, GLint *totalatomsLocation){
		//https://www.gamedev.net/topic/591110-geometry-shader-point-sprites-to-spheres/
	//no rotation, only translations means we can do directional lighting in the shader.
	//FIXME
	//http://stackoverflow.com/questions/40101023/flat-shading-in-webgl
	*AtomP = CompileGLShader(
		AtomShadersNoTess[SHADERNAME],
		AtomShadersNoTess[SHADERVERTEX],
		AtomShadersNoTess[SHADERFRAGMENT],
		AtomShadersNoTess[SHADERTESSEVAL]
		);
	*AtomMatrixLocation=glGetUniformLocation(*AtomP, "matrix");
	if( *AtomMatrixLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in atom shader no tess\n" );
		return false;
	}
	*totalatomsLocation=glGetUniformLocation(*AtomP, "totalatoms");
	if( *totalatomsLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in atom shader\n" );
		return false;
	}
	return true;
}


bool PrepareUnitCellShader (GLuint *cellP, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation){
	*cellP= CompileGLShader(
		UnitCellShaders[SHADERNAME],
		UnitCellShaders[SHADERVERTEX],
		UnitCellShaders[SHADERFRAGMENT],
		UnitCellShaders[SHADERTESSEVAL]
		);
	*UnitCellMatrixLocation=glGetUniformLocation(*cellP, "matrix");
	if( *UnitCellMatrixLocation == -1 )
	{
		eprintf( "Unable to find matrix uniform in UnitCell shader\n" );
		return false;
	}
	*UnitCellColourLocation=glGetUniformLocation(*cellP, "color");
	if( *UnitCellColourLocation == -1 )
	{
		eprintf( "Unable to find color uniform in UnitCell shader\n" );
		return false;
	}
	return true;
}


/**p: input, f: output*/
void GetDisplacement(int p[3], float f[3])
{
float delta[3][3];
for (int ss=0;ss<3;ss++)
	for (int i=0;i<3;i++)
		delta[ss][i]=static_cast<float>(p[ss])*abc[ss][i];

for (int i=0;i<3;i++)
	f[i]=0;

for (int ss=0;ss<3;ss++)
	for (int i=0;i<3;i++)
		f[i]+=delta[ss][i];
}
