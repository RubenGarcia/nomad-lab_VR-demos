#ifndef __ISOSURFACESGL_H
#define __ISOSURFACESGL_H

#include "rply/rply.h"

//number of components in our vertices data xyz+nxnynz+rgba
#define numComponents 10

bool AddModelToScene( const float *mat/*[16]*/, std::vector<float> &vertdata, 
#ifndef INDICESGL32
	std::vector<short> & vertindices, 
#else
	std::vector<GLuint> & vertindices,
#endif
	const char * model, bool water, bool colours, int set);

GLenum PrepareGLiso (GLuint vao, GLuint vertbuffer, const std::vector<float> &vertdata, GLuint indbuffer,
 #ifndef INDICESGL32
	std::vector<short> & vertindices 
#else
	std::vector<GLuint> & vertindices
#endif
	);

GLenum PrepareISOShader (GLuint *p, GLint *mat);


#endif // __ISOSURFACESGL_H
