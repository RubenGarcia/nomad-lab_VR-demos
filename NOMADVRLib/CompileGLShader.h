#include "MyGL.h"

#define SHADERNAME 0
#define SHADERVERTEX 1
#define SHADERFRAGMENT 2
#define SHADERTESSEVAL 3

//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------

GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader,
	const char *pchTessEvalShader = nullptr);

