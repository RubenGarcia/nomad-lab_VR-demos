﻿/*Uses code from rply which uses the MIT license*/
/*http://w3.impa.br/~diego/software/rply/*/
/*This license is compatible with Apache 2.0*/

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

#include <vector>
#include <math.h>
#include <stdint.h>

#include "MyGL.h"
#include "eprintf.h"
#include "ConfigFile.h" //for isocolours
#include "TessShaders.h" //for SHADERNAME, etc
#include "IsosurfacesGL.h"
#include "IsoShaders.h"
#include "CompileGLShader.h"

float *CubeVertices=0;
int *CubeIndices=0;
int CurrentVertex=-1;
int CurrentIndex=0;

int vertex_cb(p_ply_argument argument);
int face_cb(p_ply_argument argument);

inline void mult (float *o/*[4]*/, const float *m/*[16]*/, const float* v/*[4]*/)  //o=m*v
{
	
	for (int i=0;i<4;i++) {
		o[i]=0;
		for (int j=0;j<4;j++)
			o[i]+=m[j*4+i]*v[j];
	}
}

inline void normalize (float o[3])
{
	float mod=0;
	for (int i=0;i<3;i++)
		mod += o[i]*o[i];
	mod=sqrtf(mod);
	mod=1.0f/mod;
	for (int i=0;i<3;i++)
		o[i]*=mod;
}

bool AddModelToScene( const float *mat/*[16]*/, std::vector<float> &vertdata, 
#ifndef INDICESGL32
	std::vector<short> & vertindices, 
#else
	std::vector<GLuint> & vertindices,
#endif
	const char * model, bool useisotrans, bool colours, int set) 
{

	CurrentVertex=-1;
	CurrentIndex=0;
	 p_ply ply = ply_open(model, NULL, 0, NULL);
        if (!ply) {
			//rgh: files may not exist in the case of relative densities (0 density)
            //eprintf("ply returned null: file %s\n", model);
            return false;
        }
        if (!ply_read_header(ply)) {
            eprintf("ply: bad header in ply: file %s\n", model);
            return false;
        }

		int nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, nullptr, 0);
        ply_set_read_cb(ply, "vertex", "y", vertex_cb, nullptr, 1);
        ply_set_read_cb(ply, "vertex", "z", vertex_cb, nullptr, 2);

        ply_set_read_cb(ply, "vertex", "nx", vertex_cb, nullptr, 3);
        ply_set_read_cb(ply, "vertex", "ny", vertex_cb, nullptr, 4);
        ply_set_read_cb(ply, "vertex", "nz", vertex_cb, nullptr, 5);

		ply_set_read_cb(ply, "vertex", "red", vertex_cb, nullptr, 6);
        ply_set_read_cb(ply, "vertex", "green", vertex_cb, nullptr, 7);
        ply_set_read_cb(ply, "vertex", "blue", vertex_cb, nullptr, 8);
        ply_set_read_cb(ply, "vertex", "alpha", vertex_cb, nullptr, 9);

		//ply_set_read_cb(ply, "vertex", "texture_u", vertex_cb, this, 10);
		//ply_set_read_cb(ply, "vertex", "texture_v", vertex_cb, this, 11);

		CubeVertices = new float[nvertices*numComponents]; //xyz, nx, ny, nz, uv, rgba

        int ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);

        CubeIndices=new int[3*ntriangles];

        //dprintf("PLY %s: v=%ld, t=%ld\n", model, nvertices, ntriangles);
        if (!ply_read(ply)) {
            eprintf("Problem in ply_read");
            return false;
        }
        ply_close(ply);

	for (int i = 0; i < 3* ntriangles; i++) {
		vertindices.push_back(CubeIndices[i]);
	}

	
	for (int i = 0; i < nvertices; i++) {
		//pos
		float V1[4];
		for (int j=0;j<3;j++)
			V1[j] = CubeVertices[i * numComponents +j];
		V1[3]=1;
		float V[4];
		mult(V, mat, V1);
		for (int j=0;j<3;j++)
			vertdata.push_back(V[j]);
			
	//normals (FIXME should transform with inverse transform, but we think the matrix has uniform scaling and inv transpose = m)
	//rgh beware: normals are (nx, ny, nz, 0) to avoid being translated !!!
		for (int j=0;j<3;j++)
			V1[j] = CubeVertices[i * numComponents + 3+j];
		V1[3]=0;
		mult (V, mat, V1);
		normalize(V);
		for (int j=0;j<3;j++) {
			vertdata.push_back(V[j]);
			//eprintf ("normal %d %d %f", i, j, V[j]);
		}
		//colors (untransformed)
		if (!colours) {
			for (int j = 0; j < 4; j++)
				vertdata.push_back(isocolours[set][j]);
		} else {
			for (int j = 0; j < 3; j++)
				vertdata.push_back(CubeVertices[i * numComponents + 6 + j]);
			if (useisotrans)
				vertdata.push_back(isocolours[set][3]);
			else
				vertdata.push_back(CubeVertices[i * numComponents + 6 + 3]); //trans

		}
	}
	delete[] CubeVertices;
	delete[] CubeIndices;
	CubeVertices=0;
	CubeIndices=0;
	return true;
}

int vertex_cb(p_ply_argument argument) {
    long what;
	int ret=ply_get_argument_user_data(argument, nullptr, &what);
	if (!ret)
		return 0;
	if (what == 0)
		CurrentVertex++;
    if (what <=5 /* || what >=10*/ ){ //no uvs in these meshes.
		CubeVertices[CurrentVertex*numComponents+what]=(float)ply_get_argument_value(argument);
    } else {
        CubeVertices[CurrentVertex*numComponents+what]=(float)(ply_get_argument_value(argument)/255.0);
    }

    return 1;
}

int face_cb(p_ply_argument argument) {
    long length, value_index;
    ply_get_argument_property(argument, nullptr, &length, &value_index);
    if (value_index>=0 && value_index<=2)
        CubeIndices[CurrentIndex++]=(int)(ply_get_argument_value(argument));

    return 1;
}

GLenum PrepareGLiso (GLuint vao, GLuint vertbuffer, const std::vector<float> &vertdata, GLuint indbuffer,
 #ifndef INDICESGL32
	std::vector<short> & vertindices 
#else
	std::vector<GLuint> & vertindices
#endif
	)
{
	GLenum e;
	GLsizei stride = sizeof(float) * numComponents; 

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vertbuffer);
	if (vertdata.size() !=0)
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdata.size(), &vertdata[0], GL_STATIC_DRAW);
	else 
		eprintf("Warning: Iso has no data");
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);

	//now pos[3], normal[3], color[4]
	//pos
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)(0));

	if (glGetError() != GL_NO_ERROR)
		eprintf("opengl error attrib pointer 0\n");

	//normal
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)(3 * sizeof(float)));

	if ((e=glGetError()) != GL_NO_ERROR)
		eprintf("opengl error attrib pointer 1\n");

	//color
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(6 * sizeof(float)));

	if (glGetError() != GL_NO_ERROR)
		eprintf("opengl error attrib pointer 2\n");

		// populate the index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indbuffer);
	if (vertindices.size() !=0)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
	#ifndef INDICESGL32
			sizeof(uint16_t)
	#else
			sizeof(GLuint)
	#endif
			* vertindices.size(),
			&vertindices[0], GL_STATIC_DRAW);

	if ((e=glGetError()) != GL_NO_ERROR)
		eprintf("opengl error\n");

	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	return e;
}

GLenum PrepareISOShader (GLuint *p, GLint *mat) {
	*p= CompileGLShader(
		IsoShaders[SHADERNAME],
		IsoShaders[SHADERVERTEX],
		IsoShaders[SHADERFRAGMENT],
		IsoShaders[SHADERTESSEVAL]
		);
	*mat=glGetUniformLocation(*p, "matrix");
	if( *mat == -1 )
	{
		eprintf( "Unable to find matrix uniform in ISO shader\n" );
		
	}
return glGetError();
}

GLenum PrepareISOTransShader (GLuint *p, GLint *mat, GLuint *b) {
	*p= CompileGLShader(
		IsoTransparentShaders[SHADERNAME],
		IsoTransparentShaders[SHADERVERTEX],
		IsoTransparentShaders[SHADERFRAGMENT],
		IsoTransparentShaders[SHADERTESSEVAL]
		);
	*mat=glGetUniformLocation(*p, "matrix");
	if( *mat == -1 )
	{
		eprintf( "Unable to find matrix uniform in ISO shader (%s)\n", IsoTransparentShaders[SHADERNAME]);
		
	}
	*b=CompileGLShader(
		IsoBlendShaders[SHADERNAME],
		IsoBlendShaders[SHADERVERTEX],
		IsoBlendShaders[SHADERFRAGMENT],
		IsoBlendShaders[SHADERTESSEVAL]
		);
return glGetError();
}

//code not opengl es ready yet
//#if defined(WIN32) || defined (CAVE)
bool SetupDepthPeeling(int renderWidth, int renderHeight, int zlayers, GLuint *textures /*[zlayers+2 (2 depth, zlayers colour)]*/,
					   GLuint *peelingFramebuffer) 
{
	//https://www.opengl.org/wiki/Common_Mistakes
	//Until this is resolved in NVIDIA's drivers, it is advised to make sure that all textures have mipmap levels, and that all glTexParameteri​ 
	//values are properly set up for the format of the texture. For example, integral textures are not complete if the mag and min filters have any LINEAR fields.
	//eprintf ("SetupDepthPeeling, w %d, h %d\n", renderWidth, renderHeight);
	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, start SetupDepthPeeling\n", e);

	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, SetupDepthPeeling a\n", e);
#if defined(WIN32) || defined(CAVE)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, renderWidth, renderHeight, 0, 
			GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);
#else // 32 unsupported in GLES 3.0
#ifdef __APPLE__
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, renderWidth, renderHeight, 0,
                     GL_DEPTH_COMPONENT, GL_FLOAT, 0);
#else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, renderWidth, renderHeight, 0, 
			GL_DEPTH_COMPONENT, GL_FLOAT, 0);
#endif
#endif

		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, SetupDepthPeeling b\n", e);

		//cleaned at each frame
#if defined(WIN32) || defined(CAVE)
		CleanDepthTexture(textures[i]);
#else
		CleanDepthTexture(textures[i], renderWidth, renderHeight);
#endif
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, SetupDepthPeeling c\n", e);
	}

	for (int i = 0; i < zlayers; i++) {
		glBindTexture(GL_TEXTURE_2D, textures[2+i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderWidth, renderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, SetupDepthPeeling textures end\n", e);

	//now create framebuffer
	GLint dfb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dfb);
	glGenFramebuffers(1, peelingFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, *peelingFramebuffer);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e, __LINE__);

	glBindFramebuffer(GL_FRAMEBUFFER, dfb);



	return (e == GL_NO_ERROR);
}

//glClearTexImage available in OpenGL but not OpenGL ES
#if defined(WIN32) || defined(CAVE)
void CleanDepthTexture (GLuint t)
{
int e;
GLuint clearColor = 0;
//glBindTexture(GL_TEXTURE_2D, t);
glClearTexImage(t, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, &clearColor);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after glClearTexImage: %d\n", e);

}
#else
void CleanDepthTexture (GLuint t, int width, int height)
{
GLenum e;
const std::vector<float> z(width * height * 4, 0.0f);
glBindTexture(GL_TEXTURE_2D, t);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("CleanDepthTexture: Gl error after glBindTexture: %d\n", e);
glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, z.data());
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("CleanDepthTexture: Gl error after glTexSubImage2D: %d\n", e);

}
#endif

GLenum EnableDepthFB(unsigned int zl, const GLuint transP, 
	const GLuint peelingFramebuffer, const GLuint *texture /*[2+ZLAYERS]*/) 
{
GLenum e;
glUseProgram(transP);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB: Gl error after useprogram %d: %d\n", transP, e);
	return e;
}
glBindFramebuffer(GL_FRAMEBUFFER, peelingFramebuffer);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB: Gl error after glBindFramebuffer: %d\n", e);
	return e;
}
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
	texture[2 + zl], 0);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB: Gl error after glFramebufferTexture2D color: %d\n", e);
	return e;
}
glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
	texture[1-zl%2], 0);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB: Gl error after glFramebufferTexture2D depth: %d\n", e);
	return e;
}
glBindTexture(GL_TEXTURE_2D, texture[zl%2]);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB glBindTexture: Gl error %d\n", e);
	return e;
}
glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB glClearColor: Gl error %d\n", e);
	return e;
}
glDepthMask(GL_TRUE);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB glDepthMask: Gl error %d\n", e);
	return e;
}
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
if ((e = glGetError()) != GL_NO_ERROR) {
	eprintf("EnableDepthFB end: Gl error %d\n", e);
	return e;
}
return glGetError();
}

GLenum SetupBlending (GLuint *vao, GLuint *vertex, GLuint *indices)
{
	GLenum e;
	float z = 0.0f; 
	const float points[] = {
		-1, -1, z, 0, 0,
		-1, 1, z, 0, 1,
		1, 1, z, 1, 1,
		1, -1, z, 1, 0 };
	const GLuint iData[] = { 0, 1, 2,
		2, 3, 0 };
	glGenVertexArrays(1, vao);
	glBindVertexArray(*vao);
	glGenBuffers(1, vertex);
	glBindBuffer(GL_ARRAY_BUFFER, *vertex);
	glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(GLfloat), points, GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error after glBufferData: %d,  l %d\n", e, __LINE__);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e, __LINE__);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e,  __LINE__); 
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e,  __LINE__);

	glGenBuffers(1, indices);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e,  __LINE__);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *indices);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e,  __LINE__);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iData), iData, GL_STATIC_DRAW);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error: %d, l %d\n", e, __LINE__);
	glBindVertexArray(0);
	return e;
}

void DeleteBlendingBuffers(GLuint *vao, GLuint *vertex, GLuint *indices)
{
	glDeleteBuffers(1, vertex);
	glDeleteBuffers(1, indices);
	glDeleteVertexArrays(1, vao);
}

void BlendTextures(GLuint *textures, int zlayers)
{
GLenum e;
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error BlendTextures begin: %d\n", e);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int zl = zlayers - 1; zl >= 0; zl--) {
		glBindTexture(GL_TEXTURE_2D, textures[2 + zl]);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error BlendTextures glBindTexture: %d\n", e);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error BlendTextures glDrawElements: %d\n", e);
	}
}

