#ifndef __MYGL_H
#define __MYGL_H

//FIXME, support more platforms in the future
#ifdef WIN32
#include <GL/glew.h>
#else // Samsung GearVR + Oculus Mobile
#include "Kernel/OVR_GlUtils.h"
#include "GLES2/gl2ext.h"
//ndk r10e does not provide opengles 3.2
#ifndef GL_TESS_EVALUATION_SHADER
#define GL_TESS_EVALUATION_SHADER GL_TESS_EVALUATION_SHADER_EXT
#endif
#endif

#endif //__MYGL_H