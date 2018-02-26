/*
# Copyright 2016-2018 The NOMAD Developers Group
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
