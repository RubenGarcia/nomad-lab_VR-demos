#ifndef __MYGL_H
#define __MYGL_H

//FIXME, support more platforms in the future
#if defined(WIN32) || defined(CAVE)
	#include <GL/glew.h>
#else // Samsung GearVR + Oculus Mobile
	#ifdef OCULUSMOBILE
		#include "Kernel/OVR_GlUtils.h"
	#else
		#include <GLES3/gl31.h>
	//google cardboard, gvr
	#endif //OCULUSMOBILE
	#include "GLES2/gl2ext.h"
//ndk r10e does not provide opengles 3.2
	#ifndef GL_TESS_EVALUATION_SHADER
		#define GL_TESS_EVALUATION_SHADER GL_TESS_EVALUATION_SHADER_EXT
	#endif //GL_TESS_EVALUATION_SHADER
	#ifndef GL_DEPTH_COMPONENT32
		#define GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
	#endif
#endif //WIN32

#endif //__MYGL_H
