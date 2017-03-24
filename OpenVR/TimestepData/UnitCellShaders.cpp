#include "UnitCellShaders.h"

const char * const UnitCellShaders [] = {"Unit Cell Renderer",
	//vertex
	"#version 410\n"
	"uniform mat4 matrix;\n"
	"layout(location = 0) in vec3 pos;\n"
	"void main()\n"
	"{\n"
	"gl_Position = matrix * vec4(pos, 1);\n"
	"}\n"
	,
	//fragment
	"#version 410 core\n"
	"uniform vec4 color;\n"
	"out vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"	outputColor = color;\n"
	"}\n"
	,
	//tess
	nullptr
};
