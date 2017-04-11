#include <math.h>

#include "eprintf.h"
#include "TessShaders.h"
#include "UnitCellShaders.h"
#include "atomsGL.h"
#include "atoms.hpp"
#include "ConfigFile.h"
#include "CompileGLShader.h"
#include "polyhedron.h"

GLenum atomTexture(GLuint t)
{
	GLenum e;
	//rgh: scale atoms here
	//in google cardboard, this is called again if the program is running, so leave original or atoms get progresivelly smaller!
	float *a=new float[118*4];
	for (int i = 0; i < 118; i++) {
		a[i*4+0]=atomColours[i][0];
		a[i*4+1]=atomColours[i][1];
		a[i*4+2]=atomColours[i][2];
		a[i*4+3]=atomColours[i][3] * atomScaling;
	}
	glBindTexture(GL_TEXTURE_2D, t); //atom texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 118, 1, 0, GL_RGBA, GL_FLOAT, a);

	glBindTexture( GL_TEXTURE_2D, 0 );
	if ((e = glGetError()) != GL_NO_ERROR) {
		eprintf( "opengl error %d, atomTexture\n", e);
	}
	delete [] a;
	return e;
}

//WARNING: This should be called after SetupAtoms
//This means that numAtoms now has the cummulative distribution!
//This should be called after the atom texture is prepared, and therefore has the atomscaling pre-multiplied
GLenum SetupAtomsNoTess (GLuint **AtomVAO, GLuint **AtomVertBuffer, GLuint **AtomIndexBuffer)
{
	//eprintf ("SetupAtomsNoTess 1");
if (!numAtoms)
		return 0;

if (!solid) {
	eprintf ("SetupAtomsNoTess, error: no solid defined");
	return 0;
}

//eprintf ("SetupAtomsNoTess 2");
	//for now, render an icosahedron
	//http://prideout.net/blog/?p=48 //public domain code
	//xyz nxnynz u=atom type ; 7 floats
	int e;

	int totalatoms=numAtoms[TIMESTEPS-1];
	
//eprintf ("SetupAtomsNoTess 2");
	*AtomVAO = new GLuint[2]; //atoms, cloned atoms
	*AtomIndexBuffer= new GLuint[2];
	*AtomVertBuffer = new GLuint[2];

	glGenVertexArrays(2, *AtomVAO);
	glGenBuffers(2, *AtomIndexBuffer);
	glGenBuffers(2, *AtomVertBuffer);
//eprintf ("SetupAtomsNoTess 3");
	glBindVertexArray((*AtomVAO)[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*AtomIndexBuffer)[0]);
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[0]);
//eprintf ("SetupAtomsNoTess 4");
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	//eprintf ("SetupAtomsNoTess 5, totalatoms=%d, nVerts=%d", totalatoms, solid->nVerts);
	float *tmp = new float[solid->nVerts * 7 * totalatoms];
	//eprintf ("SetupAtomsNoTess 6");
#ifdef INDICESGL32		
	int *tmpi = new int[solid->nFaces*3 * totalatoms];
	//eprintf ("SetupAtomsNoTess 7");
	int *currenti=tmpi;
#else
	unsigned short *tmpi = new unsigned short[solid->nFaces*3 * totalatoms];
	//eprintf ("SetupAtomsNoTess 7B");
	unsigned short *currenti=tmpi;
#endif

	float *current=tmp;
	//eprintf ("Before For 1");
	for (int p=0;p<TIMESTEPS;p++) {
		for (int a = 0; a < numAtoms[p]-(p==0?0:numAtoms[p-1]); a++) {
			const int atomNumber = static_cast<int>(atoms[p][4 * a + 3]);
			const float radius = atomColours[atomNumber][3]*atomScaling;
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

	//glBindVertexArray(0);
	//glDisableVertexAttribArray(0);
	delete[] tmp;
	delete[] tmpi;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of SetupAtoms, l %d\n", e, __LINE__);

	//FIXME TODO: cloned atoms
	tmp = new float[solid->nVerts * 7 * numClonedAtoms];
	current=tmp;
	//eprintf ("SetupAtomsNoTess 6");
#ifdef INDICESGL32		
	tmpi = new int[solid->nFaces*3 * numClonedAtoms];
	//eprintf ("SetupAtomsNoTess 7");
	currenti=tmpi;
#else
	tmpi = new unsigned short[solid->nFaces*3 * numClonedAtoms];
	//eprintf ("SetupAtomsNoTess 7B");
	currenti=tmpi;
#endif
	//eprintf ("Before For 2");

	for (int a = 0; a < numClonedAtoms; a++) {
		const int atomNumber = static_cast<int>(clonedAtoms[0][4 * a + 3]);
		const float radius = atomColours[atomNumber][3]*atomScaling;
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
	
	//eprintf ("After For 2");


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

	return e;
} //SetupAtomsNoTess


GLenum SetupAtoms(GLuint **AtomVAO, GLuint **AtomVertBuffer)
{
	if (!numAtoms)
		return 0;
	//rgh FIXME: put this all in the same vao
	
	//http://prideout.net/blog/?p=48 //public domain code
	//xyz u=atom type ; 4 floats
	int e;

	int totalatoms=0;
	for (int i=0;i<TIMESTEPS;i++) {
		totalatoms += numAtoms[i];
	}
	eprintf("SetupAtoms: totalatoms=%d", totalatoms);

	*AtomVAO = new GLuint[2]; //atoms, cloned atoms
	*AtomVertBuffer = new GLuint[2];
	glGenVertexArrays(2, *AtomVAO);
	glGenBuffers(2, *AtomVertBuffer);

	glBindVertexArray((*AtomVAO)[0]);
	glBindBuffer(GL_ARRAY_BUFFER, (*AtomVertBuffer)[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	float *tmp = new float[4 * totalatoms];
	float *current=tmp;
	for (int p=0;p<TIMESTEPS;p++) {

		for (int a = 0; a < numAtoms[p]; a++) {
			for (int k = 0; k < 4; k++) {
				*current++ = atoms[p][4 * a + k];
			}
		} //a
		if (p!=0)
			numAtoms[p]+=numAtoms[p-1];
	} //p
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
			for (int p=1;p<TIMESTEPS;p++) {
				int a=atomtrajectories[t];
				if (fabs(atoms[p][a*4+0]-atoms[p-1][a*4+0])+
					fabs(atoms[p][a*4+1]-atoms[p-1][a*4+1])+
					fabs(atoms[p][a*4+2]-atoms[p-1][a*4+2])>max)
						atomtrajectoryrestarts[t].push_back(p);
			}
			atomtrajectoryrestarts[t].push_back(TIMESTEPS);
		}
	}
	delete[] tmp;

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

	return e;
}

GLenum SetupUnitCell(GLuint *UnitCellVAO, GLuint *UnitCellVertBuffer, GLuint *UnitCellIndexBuffer)
{
	if (!has_abc)
		return 0;
	GLenum e;
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

	float *tmp = new float[3*8];
	//0, a, b, c, a+b+c, b+c, a+c, a+b
	for (int i=0;i<3;i++) {
		tmp[0+i]=0;
		for (int j=0;j<3;j++)
			tmp[3*(j+1)+i]=abc[j][i];
		tmp[3*4+i]=abc[0][i]+abc[1][i]+abc[2][i];
		tmp[3*5+i]=			abc[1][i]+abc[2][i];
		tmp[3*6+i]=abc[0][i]+		abc[2][i];
		tmp[3*7+i]=abc[0][i]+abc[1][i];
	}

	int tmpi[12*2]={ //lines
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
		3,5
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3*8 , tmp,
			GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf( "opengl error %d, glBufferData, l %d\n", e, __LINE__);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tmpi), tmpi, GL_STATIC_DRAW);

	return e;
}


bool PrepareUnitCellAtomShader (GLuint *AtomP, GLuint *cellP, GLint *AtomMatrixLocation, GLint *UnitCellMatrixLocation,  GLint *UnitCellColourLocation){
	if (!PrepareAtomShader(AtomP, AtomMatrixLocation))
		return false;

	if (!PrepareUnitCellShader(cellP, UnitCellMatrixLocation, UnitCellColourLocation))
		return false;

	return true;
}

bool PrepareAtomShader (GLuint *AtomP, GLint *AtomMatrixLocation){
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
	return true;
}

bool PrepareAtomShaderNoTess (GLuint *AtomP, GLint *AtomMatrixLocation){
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
