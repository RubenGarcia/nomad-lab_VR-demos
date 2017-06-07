#include "markerShaders.h"

const char * const MarkerShaders [] = {
	"Marker Renderer",

// vertex shader
//Android 21 gives error: only supports up to '310 es'
#if defined(WIN32) || defined(CAVE)
	"#version 410\n"
#else
"#version 300 es\n"
#endif
	"layout(location = 0) in vec4 centersize;\n"
	"layout(location = 1) in vec4 colour;\n"
	"out vec4 vcolor;\n" //color
	"out vec3 vcen;"
	"out vec3 vrad;" // ellipsoid
	"void main()\n"
	"{\n"
	//"gl_Position = matrix * vec4(position+center, 1);\n"
	"vcolor=colour;\n"
	"vcen=centersize.xyz;\n"
	"vec3 sc=vec3(0.5, 0.5, 0.5);\n"
	"if (gl_InstanceID==0)\n"
		"sc.x=2;\n"
	"else if (gl_InstanceID==1)\n"
		"sc.y=2;\n"
	"else if (gl_InstanceID==2)\n"
		"sc.z=2;\n"

	"vrad=centersize.aaa*sc;\n"
	"}\n",

	//fragment shader
#if defined(WIN32) || defined(CAVE)
	"#version 410 core\n"
#else
"#version 300 es\n"
#endif
	"in lowp vec4 color;\n"
	"in highp vec3 normal;"
	"out lowp vec4 outputColor;\n"
	"void main()\n"
	"{\n"
	"highp vec3 nn = normalize(normal);"
	"lowp float a=abs(dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
	"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
	"highp vec4 res=color;\n"
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
	"in vec4 vcolor[];\n" //color
	"in vec3 vcen[];"
	"in vec3 vrad[];" // ellipsoid
	"out vec4 color;\n" //color 
	"out vec3 normal;\n"
	"void main()\n"
	"{\n"
	"normal=vec3(sin(gl_TessCoord.x*2*pi)*cos((gl_TessCoord.y-0.5)*pi), "
	"cos(gl_TessCoord.x*2*pi)*cos((gl_TessCoord.y-0.5)*pi), "
	"sin((gl_TessCoord.y-0.5)*pi));"
	"vec3	vertex = normal * vrad[0]  + vcen[0];\n"
	"color=vcolor[0];"
	"gl_Position = matrix * vec4(vertex, 1);\n"
"}\n"

};