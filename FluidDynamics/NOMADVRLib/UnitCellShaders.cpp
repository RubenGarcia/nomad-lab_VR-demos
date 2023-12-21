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

#include "UnitCellShaders.h"

const char * const UnitCellShaders [] = {"Unit Cell Renderer",
	//vertex
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"uniform mat4 matrix;\n"
	"layout(location = 0) in vec3 pos;\n"
	"void main()\n"
	"{\n"
	"gl_Position = matrix * vec4(pos, 1);\n"
	"}\n"
	,
	//fragment
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
	"#version 300 es\n"
#endif
	"uniform lowp vec4 color;\n"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"	outputColor = color;\n"
	"}\n"
	,
	//tess
	nullptr
};
