LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

include ../../../../../cflags.mk

LOCAL_MODULE			:= ovrapp
LOCAL_SRC_FILES			:= ../../../Src/OvrApp.cpp ../../../Src/rply/rply.c \
	../../../Src/NOMADVRLib/ConfigFile.cpp ../../../Src/NOMADVRLib/atoms.cpp \
	../../../Src/happyhttp/happyhttp.cpp \
	../../../Src/NOMADVRLib/atomsGL.cpp \
	../../../Src/NOMADVRLib/CompileGLShader.cpp \
	../../../Src/NOMADVRLib/TessShaders.cpp \
	../../../Src/NOMADVRLib/UnitCellShaders.cpp \
	../../../Src/NOMADVRLib/polyhedron.cpp \
	../../../Src/NOMADVRLib/IsosurfacesGL.cpp \
	../../../Src/NOMADVRLib/IsoShaders.cpp
LOCAL_STATIC_LIBRARIES	:= vrsound vrmodel vrlocale vrgui vrappframework systemutils libovrkernel
LOCAL_SHARED_LIBRARIES	:= vrapi

LOCAL_CPP_FEATURES += exceptions
LOCAL_CFLAGS    := -DINDICESGL32 -DOCULUSMOBILE
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../Src/
include $(BUILD_SHARED_LIBRARY)

$(call import-module,LibOVRKernel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrApi/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/SystemUtils/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppFramework/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrGui/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrLocale/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrModel/Projects/AndroidPrebuilt/jni)
$(call import-module,VrAppSupport/VrSound/Projects/AndroidPrebuilt/jni)