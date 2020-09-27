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

#include "BondShaders.h"

const char * const BondShader [] = {"Bond Renderer",
	//vertex
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"uniform mat4 matrix;\n"
	"uniform float totalChains;\n"
	"layout(location = 0) in vec3 pos;\n"
	"layout(location = 1) in float atomIn;\n"
	"layout(location = 2) in float chain;\n"
	"out float c;\n"
	"void main()\n"
	"{\n"
	"gl_Position = matrix * vec4(pos, 1);\n"
	"if (totalChains<0.5)\n"
	"	c=atomIn;\n"
	"else\n"
	"	c=chain;\n"
	"}\n"
	,
	//fragment
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
	"#version 300 es\n"
#endif
	"in lowp vec4 color;\n"
	"out highp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"outputColor = color;\n"
	"}\n"
	, //tess
	nullptr
	, //geometry
#if defined(WIN32) || defined(CAVE)
	"#version 420 core\n"
#else
	"#version 300 es\n"
#endif
	"layout(binding=1) uniform sampler2D chainColours;\n"
	"layout(binding=0) uniform sampler2D atomData;\n"
	"uniform float totalChains;\n"
	"uniform float totalatoms;\n"
	"layout(lines) in;\n"
    "layout(line_strip, max_vertices = 4) out;\n"
	"in float c[];\n"
	"out vec4 color;\n"

	"vec4 GetColor (int v)\n"
	"{"
	"   float coord;"
	"   if (totalChains >0.5) {\n"
	"	   coord=c[v]/totalChains+0.5/totalChains;\n"
	"	   return texture(chainColours, vec2(coord, 0));\n"
	"	} else {\n"
	"	   coord=c[v]/totalatoms+0.5/totalatoms;\n"
	"		return texture(atomData, vec2(coord,0));\n"
	"   }\n"
	"}"

	"void main ()\n"
	"{\n"
	"float coord;\n"
	"if (abs (c[0]-c[1]) < 0.1) {\n"
	"	gl_Position = gl_in[0].gl_Position;\n"
	"   color=GetColor(0);"
	"	EmitVertex();\n"

	"	gl_Position = gl_in[1].gl_Position;\n"
	//"	color=...;\n" //no change
	"	EmitVertex();\n"
	"	EndPrimitive();\n"
	"} else {\n"
	"	gl_Position = gl_in[0].gl_Position;\n"
	"   color=GetColor(0);"
	"	EmitVertex();\n"

	"	gl_Position = (gl_in[0].gl_Position+gl_in[1].gl_Position)/2.0;\n"
	//"	color=...;\n" //no change
	"	EmitVertex();\n"

	//"	gl_Position = (gl_in[0].gl_Position+gl_in[1].gl_Position)/2.0;" //no change
	"   color=GetColor(1);"
	"	EmitVertex();\n"

	"	gl_Position = gl_in[1].gl_Position;\n"
	//"	color=texture(chainColours, vec2(coord, 0));\n" //no change
	"	EmitVertex();\n"
	"}\n"

    "EndPrimitive();\n"
	"}\n",
	//tess eval
	nullptr

};