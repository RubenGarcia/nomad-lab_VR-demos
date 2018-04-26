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

#include "IsoShaders.h"

#define GRIDSTR "1"

const char * const IsoShaders [] = {"Iso Renderer",
	//vertex
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"uniform mat4 matrix;\n"
	"layout(location = 0) in vec3 pos;\n"
	"layout(location = 1) in vec3 normal;\n"
	"layout(location = 2) in vec4 color;\n"
	"out vec4 vcolor;\n"
	"out vec3 vnormal;\n"
	"void main()\n"
	"{\n"
	"gl_Position = matrix * vec4(pos, 1);\n"
	"vcolor=color;\n"
	"vnormal=normalize(normal);\n"
	"}\n"
	,
	//fragment
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"in lowp vec4 vcolor;\n"
	"in lowp vec3 vnormal;\n"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
		"lowp vec3 nn=normalize(vnormal);"
		"lowp float a=max(0.0, dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
		"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
		"highp vec4 res=vcolor;\n"
		"	outputColor = vec4 ((res.rgb) * (0.2 + 0.2*a + 0.3*b), vcolor.a);\n" 
	"}\n"
	,
	//tess
	nullptr
};

const char *const IsoTransparentShaders [] = {"Iso Transparent Renderer",
	//vertex
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"uniform mat4 matrix;\n"
	"layout(location = 0) in vec4 position;\n"
	"layout(location = 1) in vec3 normalsIn;\n"
	"layout(location = 2) in vec4 colorIn;\n"
	"out vec4 color;\n"
	"out vec3 n;\n"
	"out highp vec4 pos;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(colorIn.rgba);\n"
	"   n=normalize(normalsIn);\n"
	"	pos = matrix * position;\n"
	"   gl_Position = pos;\n"
	"}\n",

		// Fragment Shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
	"#version 300 es\n"
#endif
	"uniform sampler2D diffuse;\n" //now extra depth texture for peeling
	"in lowp vec4 color;\n"
	"in lowp vec3 n;\n"
	"in highp vec4 pos;\n"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"highp vec4 mytex=texture(diffuse, vec2(pos.x/pos.w*0.5+0.5, pos.y/pos.w*0.5+0.5));\n"

	"if ((pos.z/pos.w+1.0)/2.0 <= mytex.r+0.00001 ) discard;\n"

	"lowp vec3 nn=normalize(n);"
	"lowp float a=max(0.0, dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
	"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
	"highp vec4 res=color;\n"
	"	outputColor = vec4 ((res.rgb) * (0.2 + 0.2*a + 0.3*b), color.a);\n" 

	"}\n",
	nullptr
};

const char *const IsoBlendShaders [] = {"Iso Transparent Blend",
// vertex shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
	"#version 300 es\n"
#endif
	"layout(location = 0) in vec3 position;\n"
	"layout(location = 1) in vec2 v2TexCoordsIn;\n"
	"out vec2 v2TexCoord;\n"
	"void main()\n"
	"{\n"
	"	v2TexCoord = v2TexCoordsIn;\n"
	"	gl_Position = vec4(position, 1);\n"
	"}\n",
	//fragment shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
	"#version 300 es\n"
#endif
	"uniform sampler2D diffuse;\n"
	"in highp vec2 v2TexCoord;\n"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"   outputColor = texture( diffuse, v2TexCoord);\n"
	"}\n",
	nullptr
};
