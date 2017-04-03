#include "MyGL.h"

//-----------------------------------------------------------------------------
// Purpose: Compiles a GL shader program and returns the handle. Returns 0 if
//			the shader couldn't be compiled for some reason.
//-----------------------------------------------------------------------------

GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader,
	const char *pchTessEvalShader = nullptr);

