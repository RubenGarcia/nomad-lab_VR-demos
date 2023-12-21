/*
# Copyright 2016-2018 Ruben Jesus Garcia-Hernandez, Matthias Chrisopher Albert
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

#ifndef __DRAWVECTORS_H 
#define __DRAWVECTORS_H

#include "NOMADVRLib/MyGL.h"

class loadAtomVectorsGL{
	GLuint *AtomVAO;
	GLuint *AtomVertBuffer;
	GLuint *Indices;

	// textures
	GLuint *textureVAO;
	GLuint *textureBuffer;
	GLuint *textureIndices;
	GLuint *m_iTexture;

	float* textureVertices;
	int* textureIndexBuffer;

	//digits:
	GLuint *digitVAO;
	GLuint *digitBuffer;
	GLuint *digitIndices;
	GLuint *digit_m_iTexture;

	//loading screen:
	GLuint *loadingTextureVAO;
	GLuint *loadingTextureBuffer;
	GLuint *loadingTextureIndices;
	GLuint *loading_m_iTexture;
	// textures end

	int e;
	int totalatoms;
	float *velocityVectors; //velocity
	float *accelerationVectors; //acceleration
	float *veloCrossAccel; // velocity x acceleration
	int* indexBuffer;
public:
	GLuint SetupAtomVelocity();
	GLuint GetVelocityVAO() 
	{
		return AtomVAO[0];
	}
	GLuint GetAccelerationVAO() 
	{
		return AtomVAO[1];
	}
	GLuint GetVeloCrossAccelVAO() 
	{
		return AtomVAO[2];
	}

	//textures
	GLuint SetupTextures();
	GLuint GetTextureVAO()
	{
		return textureVAO[0];
	}
	GLuint GetTexture()
	{
		return m_iTexture[0];
	}

	GLuint SetupSimpleTextures(); //uses GetTextureVao & GetTexture
	// Use either SetupTexture OR SetupSimpleTextures. They overwrite each other partially.

	//digits to display userposition
	GLuint SetupDigitTextures();
	GLuint GetDigitVAO()
	{
		return digitVAO[0];
	}
	GLuint GetDigitTexture()
	{
		return digit_m_iTexture[0];
	}
	//loading texture for thread
	GLuint SetupLoadingIsosTexture();
	GLuint GetLoadingTextureVAO()
	{
		return loadingTextureVAO[0];
	}
	GLuint GetloadingTexture()
	{
		return loading_m_iTexture[0];
	}
};

	float* getVelocities(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms);
	float* getAccelerations(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms);
	float* getVeloCrossAccel(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms);

#endif