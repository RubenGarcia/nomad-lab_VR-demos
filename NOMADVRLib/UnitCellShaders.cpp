#include "UnitCellShaders.h"

const char * const UnitCellShaders [] = {"Unit Cell Renderer",
	//vertex
	#ifdef WIN32
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
	#ifdef WIN32
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
