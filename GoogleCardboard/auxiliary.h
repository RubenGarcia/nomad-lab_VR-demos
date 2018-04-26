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

#include "vr/gvr/capi/include/gvr.h"

const float EPSILON = 0.00001f;

gvr::Mat4f TranslationMatrix (float x, float y, float z);
gvr::Mat4f TranslationMatrix (float v[3]);
gvr::Mat4f ScalingMatrix (float x, float y, float z);
gvr::Mat4f ScalingMatrix (float v[3]);
gvr::Mat4f ScalingMatrix (float s);

float getCofactor(float m0, float m1, float m2,
                  float m3, float m4, float m5,
                  float m6, float m7, float m8);

gvr::Mat4f invert (const gvr::Mat4f& m);

