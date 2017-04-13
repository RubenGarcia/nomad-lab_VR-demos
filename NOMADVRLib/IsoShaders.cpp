#include "IsoShaders.h"

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
