/*
# Copyright 2016-2018 Ruben Jesus Garcia-Hernandez
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

#include "TessShaders.h"

/*rgh: for now default tesselation control, using 
glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL, {3,3,3,3});
glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL, {3,3});
*/

const char * const AtomShaders [] = {
	"Atom Renderer",

	// vertex shader
//Android 21 gives error: only supports up to '310 es'
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
"#version 300 es\n"
#endif
	"uniform sampler2D atomData;\n"
	"uniform float totalatoms=118.0;\n" //(float)atomsInPeriodicTable
	"uniform int selectedAtom;\n"
	"layout(location = 0) in vec3 center;\n"
	"layout(location = 1) in float atomIn;\n"
	"out vec4 vcolor;\n" //color , radius
	"out vec3 vcen;"
	"out float vselected;"
	"void main()\n"
	"{\n"
	"float coord=atomIn/totalatoms+0.5/totalatoms;\n"
	"vcolor=vec4(texture(atomData, vec2(coord, 0)));\n"
	"vselected=float(gl_VertexID==selectedAtom);\n"
	"vcen=center;\n"
	//"color.a=1;\n"
	"}\n",

	//fragment shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
"#version 300 es\n"
#endif
	"in lowp vec4 color;\n"
	//"in highp vec3 vertex;"
	"in highp vec3 normal;" //world space normal
	"in float selected;"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	//"vec3 U = dFdx(vertex);                 "
	//"vec3 V = dFdy(vertex);                 "
	"highp vec3 nn= normalize(normal);"
	//"vec3 nn = normalize(cross(U,V));"
	"lowp float a=abs(dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
	"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
	"highp vec4 res;\n"
	"bool s=abs(normal.x)<0.2;"
	"s=s||(abs(normal.y)<0.2);"
	"s=s||(abs(normal.z)<0.2);"
	//"res=selected*vec4(1.0, 0.5, 0.0, 1.0)+(1-selected)*color;\n"
	"if (selected>0.5 &&  s)"
	"	res=vec4(1.0, 0.5, 0.0, 1.0);"
	"else\n"
	"	res=color;\n"
//rgh FIXME: make this depend on the background colour. Otherwise looks almost black with white background
//version for white background:
	"	outputColor = vec4 ((res.rgb) * (0.4 + 0.3*a + 0.3*b), color.a);\n"
	"}\n",

	//tess eval
#if defined(WIN32) || defined(CAVE)
	"#version 400\n"
#else
"#version 320 es\n"
#endif
	//"layout(triangles, equal_spacing, cw) in;\n"
	"layout(quads, equal_spacing, cw) in;\n"
	"#define pi 3.1415926535897932384626433832795\n"
	"uniform mat4 matrix;\n"
	//"uniform mat4 mv;\n"
	"in vec4 vcolor[];\n" //color , radius
	"in vec3 vcen[];"
	"in float vselected[];"
	"out vec4 color;\n" //color 
	"out vec3 normal;\n"
	"out float selected;\n"
	"out vec3 n;\n"
//	"out vec3 vertex;"
//	"uniform mat4 Projection;\n"
//	"uniform mat4 Modelview;\n"

	"void main()\n"
	"{\n"
	"normal=vec3(sin(gl_TessCoord.x*2*pi)*cos((gl_TessCoord.y-0.5)*pi), "
	"cos(gl_TessCoord.x*2*pi)*cos((gl_TessCoord.y-0.5)*pi), "
	"sin((gl_TessCoord.y-0.5)*pi));"
	"vec3	vertex = normal * vcolor[0].w  + vcen[0];\n"
	"color=vec4(vcolor[0].xyz, 1);"
	"selected=vselected[0];\n"
	"gl_Position = matrix * vec4(vertex, 1);\n"
"}\n"

};

const char * const AtomShadersNoTess [] = {
	"Atom Renderer No Tess",
	//No Tess means smooth shading looks very strange. Better use per-face lighting
		// vertex shader
//Android 21 gives error: only supports up to '310 es'
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
"#version 300 es\n"
#endif
	"uniform sampler2D atomData;\n"
	"uniform mat4 matrix;\n"
	"uniform float totalatoms=118.0;\n" //(float)atomsInPeriodicTable
	"layout(location = 0) in vec3 pos;\n"
	"layout(location = 1) in vec3 normalIn;\n"
	"layout(location = 2) in float atomIn;\n"
	"out vec3 normal;\n"
	"out vec3 vertex;"
	"out vec4 color;\n" //color , radius
	"void main()\n"
	"{\n"
	"gl_Position = matrix * vec4(pos, 1);\n"
	"normal=normalIn;\n"
	"float coord=atomIn/totalatoms+0.5/totalatoms;\n"
	"color=vec4(texture(atomData, vec2(coord, 0)));\n"
	"color.a=1.0;\n"
	"vertex=pos;\n"
	"}\n",

	//fragment shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
"#version 300 es\n"
#endif
	"in lowp vec4 color;\n"
	"in highp vec3 vertex;"
	"in highp vec3 normal;\n"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"highp vec3 U = dFdx(vertex);                 "
	"highp vec3 V = dFdy(vertex);                 "
	//"highp vec3 nn = normalize(normal);\n"
	"highp vec3 nn = normalize(cross(U,V));"
	"lowp float a=abs(dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
	"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
	"highp vec4 res=color;\n"
	"	outputColor = vec4 ((res.rgb) * (0.4 + 0.3*a + 0.3*b), color.a);\n"
	"}\n",
	//tess eval
	nullptr
};
