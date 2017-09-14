/* Copyright 2017 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <android/log.h>
#include <jni.h>

#include <memory>

#include "treasure_hunt_renderer.h"  // NOLINT
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "NOMADVRLib/atoms.hpp" //for TMPDIR

#include "treasure_hunt_jni.h"

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL              \
      Java_com_google_vr_ndk_samples_treasurehunt_MainActivity_##method_name

//http://stackoverflow.com/questions/12900695/how-to-obtain-jni-interface-pointer-jnienv-for-asynchronous-calls
    JavaVM* javaVM = nullptr;
    jobject jo;

namespace {

inline jlong jptr(TreasureHuntRenderer *native_treasure_hunt) {
  return reinterpret_cast<intptr_t>(native_treasure_hunt);
}

inline TreasureHuntRenderer *native(jlong ptr) {
  return reinterpret_cast<TreasureHuntRenderer *>(ptr);
}
}  // anonymous namespace

//http://stackoverflow.com/questions/28622036/android-ndk-calling-java-functions-from-c
/*void GetUrl (const char *url, const char *path) 
{
    jstring jurl = genv->NewStringUTF(url);
    jstring jpath = genv->NewStringUTF(path);

    jclass jc = genv->FindClass("NOMAD");
    jmethodID mid = genv->GetStaticMethodID(jc, "GetUrl", "(Ljava/lang/String;Ljava/lang/String;)V");

    genv->CallStaticVoidMethod(jc, mid, jurl, jpath);	
}*/

void DisplayMessage (const char *s)
{
	JNIEnv *env;
    	javaVM->AttachCurrentThread(&env, nullptr);
	jstring js = env->NewStringUTF(s);
	jclass cls = env->GetObjectClass(jo);
	jmethodID mid = env->GetMethodID(cls, "DisplayMessage", "(Ljava/lang/String;)V");
	env->CallVoidMethod(jo, mid, js);
} 

extern "C" {

JNI_METHOD(jlong, nativeCreateRenderer)
(JNIEnv *env, jclass clazz, jobject class_loader, jobject android_context,
 jlong native_gvr_api) {
  std::unique_ptr<gvr::AudioApi> audio_context(new gvr::AudioApi);
  audio_context->Init(env, android_context, class_loader,
                      GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);

  return jptr(
      new TreasureHuntRenderer(reinterpret_cast<gvr_context *>(native_gvr_api),
                               std::move(audio_context)));
}

JNI_METHOD(void, nativeDestroyRenderer)
(JNIEnv *env, jclass clazz, jlong native_treasure_hunt) {
  delete native(native_treasure_hunt);
}

JNI_METHOD(void, nativeInitializeGl)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->InitializeGl();
}

JNI_METHOD(void, nativeDrawFrame)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->DrawFrame();
}

JNI_METHOD(void, nativeOnTriggerEvent)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnTriggerEvent();
}

JNI_METHOD(void, nativeOnPause)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnPause();
}

JNI_METHOD(void, nativeOnResume)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->OnResume();
}

//https://library.vuforia.com/articles/Solution/How-To-Communicate-Between-Java-and-C-using-the-JNI
JNI_METHOD(void, nativeSetConfigFile)
(JNIEnv *env, jobject obj, jstring s, jstring e) {
	env->GetJavaVM(&javaVM);
	jo=env->NewGlobalRef(obj);
	configPath = env->GetStringUTFChars(s , NULL ) ;
	TMPDIR=env->GetStringUTFChars(e , NULL ) ;

}

JNI_METHOD(void, nativeLoadConfigFile)
(JNIEnv *env, jobject obj, jlong native_treasure_hunt) {
  native(native_treasure_hunt)->loadConfigFile();
}


}  // extern "C"
