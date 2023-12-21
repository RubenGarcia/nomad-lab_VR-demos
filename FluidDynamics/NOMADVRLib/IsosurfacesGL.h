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


#ifndef __ISOSURFACESGL_H
#define __ISOSURFACESGL_H

#include "../rply/rply.h"

//number of components in our vertices data xyz+nxnynz+rgba
#define numComponents 10

bool AddModelToScene( const float *mat/*[16]*/, std::vector<float> &vertdata, 
#ifndef INDICESGL32
	std::vector<short> & vertindices, 
#else
	std::vector<GLuint> & vertindices,
#endif
	const char * model, bool water, bool colours, int set);

GLenum PrepareGLiso (GLuint vao, GLuint vertbuffer, const std::vector<float> &vertdata, GLuint indbuffer,
 #ifndef INDICESGL32
	std::vector<short> & vertindices 
#else
	std::vector<GLuint> & vertindices
#endif
	);

GLenum SetupBlending (GLuint *vao, GLuint *vertex, GLuint *indices);
GLenum PrepareISOShader (GLuint *p, GLint *mat);
GLenum PrepareISOTransShader (GLuint *p, GLint *mat, GLuint *b);
//#if defined(WIN32) || defined (CAVE)
bool SetupDepthPeeling(int renderWidth, int renderHeight, int zlayers, GLuint *textures /*[zlayers+2 (2 depth, zlayers colour)]*/,
					   GLuint *peelingFramebuffer);
void CleanDepthTexture (GLuint t);


#if defined(WIN32) || defined(CAVE)
void CleanDepthTexture (GLuint t);
#else
void CleanDepthTexture (GLuint t, int width, int height);
#endif

GLenum EnableDepthFB(unsigned int zl, const GLuint transP, 
	const GLuint peelingFramebuffer, const GLuint *texture /*[2+ZLAYERS]*/);
void DeleteBlendingBuffers(GLuint *vao, GLuint *vertex, GLuint *indices);
void BlendTextures(GLuint *textures, int zlayers);
//#endif
#endif // __ISOSURFACESGL_H
