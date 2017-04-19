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
		//		"layout(location = 3) in vec2 uvIn;\n"
		"out vec4 color;\n"
		"out vec3 n;\n"
		"out highp vec4 pos;\n"
		//		"out vec2 uv;\n"
		"void main()\n"
		"{\n"
		"	color = vec4(colorIn.rgba);\n"
		"   n=normalize(normalsIn);\n"
		//		"   uv=uvIn;\n"
		"int i=gl_InstanceID / " GRIDSTR ";\n"
		"int j=gl_InstanceID % " GRIDSTR ";\n"
		"	pos = matrix * (position + vec4 (float(i)*0.15*101.0, 0, float(j)*0.15*101.0, 0));\n"
		"   gl_Position = pos;\n"
		//"	gl_Position = matrix * position;\n"
		"}\n",

		// Fragment Shader
#if defined(WIN32) || defined(CAVE)
		"#version 410 core\n"
#else
		"#version 300 es\n"
#endif
		"uniform sampler2D diffuse;\n" //now extra depth texture for peeling
		"in vec4 color;\n"
		"in vec3 n;\n"
		"in highp vec4 pos;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"vec4 mytex=texture(diffuse, vec2(pos.x/pos.w*0.5+0.5, pos.y/pos.w*0.5+0.5));\n"
		//http://www.gamedev.net/topic/556521-glsl-manual-shadow-map-biasscale/
		//"vec2 d=vec2(dFdx(pos.z), dFdy(pos.z));\n"
		//"highp float m=sqrt(d.x*d.x + d.y*d.y);\n"
		"if ((pos.z/pos.w+1)/2 <= mytex.r+0.0001 ) discard;\n"

		"lowp vec3 nn=normalize(n);"
		"lowp float a=max(0.0, dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
		"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
		"highp vec4 res=color;\n"
		//"outputColor = vec4(pos.x/pos.w*0.5+0.5, pos.y/pos.w*0.5+0.5, 0,1);\n"
		"	outputColor = vec4 ((res.rgb) * (0.2 + 0.2*a + 0.3*b), color.a);\n" 
		
		"}\n",
		nullptr
};