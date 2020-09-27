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


#include <stdio.h>
#include "MyGL.h"
#include "eprintf.h"
#include "CompileGLShader.h"

GLuint CompileGLShader( const char *pchShaderName, const char *pchVertexShader, const char *pchFragmentShader,
	const char *pchTessEvalShader /*= nullptr*/, const char *pchGeometryShader /*= nullptr*/, const char *pchTCS)
{
	GLuint unProgramID = glCreateProgram();

	GLuint nSceneVertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource( nSceneVertexShader, 1, &pchVertexShader, nullptr);
	glCompileShader( nSceneVertexShader );

	GLint vShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneVertexShader, GL_COMPILE_STATUS, &vShaderCompiled);
	if ( vShaderCompiled != GL_TRUE)
	{
		eprintf( "%s - Unable to compile vertex shader %d!\n", pchShaderName, nSceneVertexShader);

		GLchar mess[3000];
		GLsizei le;
		glGetShaderInfoLog(nSceneVertexShader, 3000, &le, mess);
		eprintf("error messages: %s", mess);

		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneVertexShader );
		return 0;
	}
	glAttachShader( unProgramID, nSceneVertexShader);
	glDeleteShader( nSceneVertexShader ); // the program hangs onto this once it's attached

	GLuint  nSceneFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource( nSceneFragmentShader, 1, &pchFragmentShader, nullptr);
	glCompileShader( nSceneFragmentShader );

	GLint fShaderCompiled = GL_FALSE;
	glGetShaderiv( nSceneFragmentShader, GL_COMPILE_STATUS, &fShaderCompiled);
	if (fShaderCompiled != GL_TRUE)
	{
		eprintf("%s - Unable to compile fragment shader %d!\n", pchShaderName, nSceneFragmentShader );
		GLchar mess[3000];
		GLsizei le;
		glGetShaderInfoLog(nSceneFragmentShader, 3000, &le, mess);
		eprintf("error messages: %s", mess);
		glDeleteProgram( unProgramID );
		glDeleteShader( nSceneFragmentShader );
		return 0;	
	}

	glAttachShader( unProgramID, nSceneFragmentShader );
	glDeleteShader( nSceneFragmentShader ); // the program hangs onto this once it's attached

	//tess eval
	if (pchTessEvalShader) {
#if defined (__APPLE__) && !defined (GL_TESS_EVALUATION_SHADER)
        eprintf ("Tess Eval Shaders unsupported on IOS");
        return 0;
#else
		GLuint  nSceneTessShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(nSceneTessShader, 1, &pchTessEvalShader, nullptr);
		glCompileShader(nSceneTessShader);

		GLint tShaderCompiled = GL_FALSE;
		glGetShaderiv(nSceneTessShader, GL_COMPILE_STATUS, &tShaderCompiled);
		if (tShaderCompiled != GL_TRUE)
		{
			eprintf("%s - Unable to compile tess eval shader %d!\n", pchShaderName, nSceneTessShader);
			GLchar mess[3000];
			GLsizei le;
			glGetShaderInfoLog(nSceneTessShader, 3000, &le, mess);
			eprintf("error messages: %s", mess);
			glDeleteProgram(unProgramID);
			glDeleteShader(nSceneTessShader);
			return 0;
		}
		glAttachShader(unProgramID, nSceneTessShader);
		glDeleteShader(nSceneTessShader); // the program hangs onto this once it's attached
#endif
	}
	if (pchGeometryShader) {
		GLuint  GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(GeometryShader, 1, &pchGeometryShader, nullptr);
		glCompileShader(GeometryShader);

		GLint tShaderCompiled = GL_FALSE;
		glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &tShaderCompiled);
		if (tShaderCompiled != GL_TRUE)
		{
			eprintf("%s - Unable to compile geometry shader %d!\n", pchShaderName, GeometryShader);
			GLchar mess[3000];
			GLsizei le;
			glGetShaderInfoLog(GeometryShader, 3000, &le, mess);
			eprintf("error messages: %s", mess);
			glDeleteProgram(unProgramID);
			glDeleteShader(GeometryShader);
			return 0;
		}
		glAttachShader(unProgramID, GeometryShader);
		glDeleteShader(GeometryShader); // the program hangs onto this once it's attached
	}

	if (pchTCS) {
		GLuint TCS = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(TCS, 1, &pchTCS, nullptr);
		glCompileShader(TCS);

		GLint tShaderCompiled = GL_FALSE;
		glGetShaderiv(TCS, GL_COMPILE_STATUS, &tShaderCompiled);
		if (tShaderCompiled != GL_TRUE)
		{
			eprintf("%s - Unable to compile tesselation control shader %d!\n", pchShaderName, TCS);
			GLchar mess[3000];
			GLsizei le;
			glGetShaderInfoLog(TCS, 3000, &le, mess);
			eprintf("error messages: %s", mess);
			glDeleteProgram(unProgramID);
			glDeleteShader(TCS);
			return 0;
		}
		glAttachShader(unProgramID, TCS);
		glDeleteShader(TCS); // the program hangs onto this once it's attached
	}

	glLinkProgram( unProgramID );

	GLint programSuccess = GL_TRUE;
	glGetProgramiv( unProgramID, GL_LINK_STATUS, &programSuccess);
	if ( programSuccess != GL_TRUE )
	{
		eprintf("%s - Error linking program %d!\n", pchShaderName, unProgramID);
		glDeleteProgram( unProgramID );
		return 0;
	}

	glUseProgram( unProgramID );
	glUseProgram( 0 );

	return unProgramID;
}
