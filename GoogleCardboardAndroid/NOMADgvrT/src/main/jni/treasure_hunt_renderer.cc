/*Uses code from Stackoverflow which uses the MIT license and the CC BY-SA 3.0*/
/*These licenses are compatible with Apache 2.0*/

/*
# Copyright 2016-2018 The NOMAD Developers Group
*/
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

#include <unistd.h>

#include "treasure_hunt_renderer.h"  // NOLINT

#include <android/log.h>
#include <assert.h>
#include <stdlib.h>
#include <cmath>
#include <random>

#include "NOMADVRLib/MyGL.h"

#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/ConfigFile.h"
#include "NOMADVRLib/atomsGL.h"
#include "NOMADVRLib/UnitCellShaders.h"
#include "NOMADVRLib/TessShaders.h"
#include "NOMADVRLib/polyhedron.h"
#include "NOMADVRLib/IsosurfacesGL.h"

#define LOG_TAG "NOMADgvrT"
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define CHECK(condition)                                                   \
  if (!(condition)) {                                                      \
    LOGE("*** CHECK FAILED at %s:%d: %s", __FILE__, __LINE__, #condition); \
    abort();                                                               \
  }

const char * configPath=0;
const float EPSILON = 0.00001f;

void eprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );
	va_end( args );

	LOGD("Message in NOMADgvrT");
	if (*fmt=='\0')
		LOGD("Empty format");
	LOGD("<%s>", buffer);
}


namespace {


gvr::Mat4f TranslationMatrix (float x, float y, float z) 
{
gvr::Mat4f mvs;
mvs.m[0][0]=1;mvs.m[0][1]=0;mvs.m[0][2]=0; mvs.m[0][3]=x;
mvs.m[1][0]=0;mvs.m[1][1]=1;mvs.m[1][2]=0;mvs.m[1][3]=y;
mvs.m[2][0]=0;mvs.m[2][1]=0;mvs.m[2][2]=1; mvs.m[2][3]=z;
mvs.m[3][0]=0;mvs.m[3][1]=0;mvs.m[3][2]=0; mvs.m[3][3]=1;
return mvs;
}

gvr::Mat4f TranslationMatrix (float v[3]) 
{
return TranslationMatrix (v[0], v[1], v[2]);
}

gvr::Mat4f ScalingMatrix (float x, float y, float z) 
{
gvr::Mat4f mvs;

mvs.m[0][0]=x;mvs.m[0][1]=0;mvs.m[0][2]=0; mvs.m[0][3]=0;
mvs.m[1][0]=0;mvs.m[1][1]=y;mvs.m[1][2]=0;mvs.m[1][3]=0;
mvs.m[2][0]=0;mvs.m[2][1]=0;mvs.m[2][2]=z; mvs.m[2][3]=0;
mvs.m[3][0]=0;mvs.m[3][1]=0;mvs.m[3][2]=0; mvs.m[3][3]=1;
return mvs;
}

gvr::Mat4f ScalingMatrix (float v[3]) 
{
return ScalingMatrix(v[0], v[1], v[2]);
}

gvr::Mat4f ScalingMatrix (float s)
{
return ScalingMatrix(s,s,s);
}

//static const float kZNear = 1.0f;
//static const float kZFar = 100.0f;
static const float kZNear = 0.01f;
static const float kZFar = 1000.0f;


static const int kCoordsPerVertex = 3;

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

float getCofactor(float m0, float m1, float m2,
                           float m3, float m4, float m5,
                           float m6, float m7, float m8)
{
    return m0 * (m4 * m8 - m5 * m7) -
           m1 * (m3 * m8 - m5 * m6) +
           m2 * (m3 * m7 - m4 * m6);
}

//adapted from
//openvr-0.9.19\samples\shared\matrices.cpp
static gvr::Mat4f invert (const gvr::Mat4f& m)
{
gvr::Mat4f r;
    // get cofactors of minor matrices
    float cofactor0 = getCofactor(m.m[1][1],m.m[1][2],m.m[1][3], m.m[2][1],m.m[2][2],m.m[2][3], m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor1 = getCofactor(m.m[1][0],m.m[1][2],m.m[1][3], m.m[2][0],m.m[2][2],m.m[2][3], m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor2 = getCofactor(m.m[1][0],m.m[1][1],m.m[1][3], m.m[2][0],m.m[2][1], m.m[2][3], m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor3 = getCofactor(m.m[1][0],m.m[1][1],m.m[1][2], m.m[2][0],m.m[2][1], m.m[2][2], m.m[3][0],m.m[3][1],m.m[3][2]);

    // get determinant
    float determinant = m.m[0][0] * cofactor0 - m.m[0][1] * cofactor1 + m.m[0][2] * cofactor2 - m.m[0][3] * cofactor3;

    if(fabs(determinant) <= EPSILON)
    {
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			r.m[i][j]=i==j;
        return r;
    }

    // get rest of cofactors for adj(M)
    float cofactor4 = getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[2][1],m.m[2][2],m.m[2][3], m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor5 = getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[2][0],m.m[2][2],m.m[2][3], m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor6 = getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[2][0],m.m[2][1], m.m[2][3], m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor7 = getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[2][0],m.m[2][1], m.m[2][2], m.m[3][0],m.m[3][1],m.m[3][2]);

    float cofactor8 = getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[1][1],m.m[1][2], m.m[1][3],  m.m[3][1],m.m[3][2],m.m[3][3]);
    float cofactor9 = getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[1][0],m.m[1][2], m.m[1][3],  m.m[3][0],m.m[3][2],m.m[3][3]);
    float cofactor10= getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[1][0],m.m[1][1], m.m[1][3],  m.m[3][0],m.m[3][1],m.m[3][3]);
    float cofactor11= getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[1][0],m.m[1][1], m.m[1][2],  m.m[3][0],m.m[3][1],m.m[3][2]);

    float cofactor12= getCofactor(m.m[0][1],m.m[0][2],m.m[0][3], m.m[1][1],m.m[1][2], m.m[1][3],  m.m[2][1], m.m[2][2],m.m[2][3]);
    float cofactor13= getCofactor(m.m[0][0],m.m[0][2],m.m[0][3], m.m[1][0],m.m[1][2], m.m[1][3],  m.m[2][0], m.m[2][2],m.m[2][3]);
    float cofactor14= getCofactor(m.m[0][0],m.m[0][1],m.m[0][3], m.m[1][0],m.m[1][1], m.m[1][3],  m.m[2][0], m.m[2][1], m.m[2][3]);
    float cofactor15= getCofactor(m.m[0][0],m.m[0][1],m.m[0][2], m.m[1][0],m.m[1][1], m.m[1][2],  m.m[2][0], m.m[2][1], m.m[2][2]);

    // build inverse matrix = adj(M) / det(M)
    // adjugate of M is the transpose of the cofactor matrix of M
    float invDeterminant = 1.0f / determinant;
    r.m[0][0] =  invDeterminant * cofactor0;
    r.m[0][1] = -invDeterminant * cofactor4;
    r.m[0][2] =  invDeterminant * cofactor8;
    r.m[0][3] = -invDeterminant * cofactor12;

    r.m[1][0] = -invDeterminant * cofactor1;
    r.m[1][1] =  invDeterminant * cofactor5;
    r.m[1][2] = -invDeterminant * cofactor9;
    r.m[1][3] =  invDeterminant * cofactor13;

    r.m[2][0] =  invDeterminant * cofactor2;
    r.m[2][1] = -invDeterminant * cofactor6;
    r.m[2][2]=  invDeterminant * cofactor10;
    r.m[2][3]= -invDeterminant * cofactor14;

    r.m[3][0]= -invDeterminant * cofactor3;
    r.m[3][1]=  invDeterminant * cofactor7;
    r.m[3][2]= -invDeterminant * cofactor11;
    r.m[3][3]=  invDeterminant * cofactor15;
return r;
}

static std::array<float, 16> MatrixToGLArray(const gvr::Mat4f& matrix) {
  // Note that this performs a *tranpose* to a column-major matrix array, as
  // expected by GL.
  std::array<float, 16> result;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result[j * 4 + i] = matrix.m[i][j];
    }
  }
  return result;
}

static std::array<float, 4> MatrixVectorMul(const gvr::Mat4f& matrix,
                                            const std::array<float, 4>& vec) {
  std::array<float, 4> result;
  for (int i = 0; i < 4; ++i) {
    result[i] = 0;
    for (int k = 0; k < 4; ++k) {
      result[i] += matrix.m[i][k] * vec[k];
    }
  }
  return result;
}

static gvr::Mat4f MatrixMul(const gvr::Mat4f& matrix1,
                            const gvr::Mat4f& matrix2) {
  gvr::Mat4f result;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.m[i][j] = 0.0f;
      for (int k = 0; k < 4; ++k) {
        result.m[i][j] += matrix1.m[i][k] * matrix2.m[k][j];
      }
    }
  }
  return result;
}

static gvr::Mat4f PerspectiveMatrixFromView(const gvr::Rectf& fov, float z_near,
                                            float z_far) {
  gvr::Mat4f result;
  const float x_left = -std::tan(fov.left * M_PI / 180.0f) * z_near;
  const float x_right = std::tan(fov.right * M_PI / 180.0f) * z_near;
  const float y_bottom = -std::tan(fov.bottom * M_PI / 180.0f) * z_near;
  const float y_top = std::tan(fov.top * M_PI / 180.0f) * z_near;
  const float zero = 0.0f;

  assert(x_left < x_right && y_bottom < y_top && z_near < z_far &&
         z_near > zero && z_far > zero);
  const float X = (2 * z_near) / (x_right - x_left);
  const float Y = (2 * z_near) / (y_top - y_bottom);
  const float A = (x_right + x_left) / (x_right - x_left);
  const float B = (y_top + y_bottom) / (y_top - y_bottom);
  const float C = (z_near + z_far) / (z_near - z_far);
  const float D = (2 * z_near * z_far) / (z_near - z_far);

  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result.m[i][j] = 0.0f;
    }
  }
  result.m[0][0] = X;
  result.m[0][2] = A;
  result.m[1][1] = Y;
  result.m[1][2] = B;
  result.m[2][2] = C;
  result.m[2][3] = D;
  result.m[3][2] = -1;

  return result;
}

static gvr::Rectf ModulateRect(const gvr::Rectf& rect, float width,
                               float height) {
  gvr::Rectf result = {rect.left * width, rect.right * width,
                       rect.bottom * height, rect.top * height};
  return result;
}

static gvr::Recti CalculatePixelSpaceRect(const gvr::Sizei& texture_size,
                                          const gvr::Rectf& texture_rect) {
  const float width = static_cast<float>(texture_size.width);
  const float height = static_cast<float>(texture_size.height);
  const gvr::Rectf rect = ModulateRect(texture_rect, width, height);
  const gvr::Recti result = {
      static_cast<int>(rect.left), static_cast<int>(rect.right),
      static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
  return result;
}

// Generate a random floating point number between 0 and 1.
static float RandomUniformFloat() {
  static std::random_device random_device;
  static std::mt19937 random_generator(random_device());
  static std::uniform_real_distribution<float> random_distribution(0, 1);
  return random_distribution(random_generator);
}

static void CheckGLError(const char* label) {
  int gl_error = glGetError();
  if (gl_error != GL_NO_ERROR) {
    LOGW("GL error @ %s: %d", label, gl_error);
    // Crash immediately to make OpenGL errors obvious.
    abort();
  }
}

static gvr::Sizei HalfPixelCount(const gvr::Sizei& in) {
  // Scale each dimension by sqrt(2)/2 ~= 7/10ths.
  gvr::Sizei out;
  out.width = (7 * in.width) / 10;
  out.height = (7 * in.height) / 10;
  return out;
}

static gvr::Mat4f ControllerQuatToMatrix(const gvr::ControllerQuat& quat) {
  gvr::Mat4f result;
  const float x = quat.qx;
  const float x2 = quat.qx * quat.qx;
  const float y = quat.qy;
  const float y2 = quat.qy * quat.qy;
  const float z = quat.qz;
  const float z2 = quat.qz * quat.qz;
  const float w = quat.qw;
  const float xy = quat.qx * quat.qy;
  const float xz = quat.qx * quat.qz;
  const float xw = quat.qx * quat.qw;
  const float yz = quat.qy * quat.qz;
  const float yw = quat.qy * quat.qw;
  const float zw = quat.qz * quat.qw;

  const float m11 = 1.0f - 2.0f * y2 - 2.0f * z2;
  const float m12 = 2.0f * (xy - zw);
  const float m13 = 2.0f * (xz + yw);
  const float m21 = 2.0f * (xy + zw);
  const float m22 = 1.0f - 2.0f * x2 - 2.0f * z2;
  const float m23 = 2.0f * (yz - xw);
  const float m31 = 2.0f * (xz - yw);
  const float m32 = 2.0f * (yz + xw);
  const float m33 = 1.0f - 2.0f * x2 - 2.0f * y2;

  return {{{m11, m12, m13, 0.0f},
           {m21, m22, m23, 0.0f},
           {m31, m32, m33, 0.0f},
           {0.0f, 0.0f, 0.0f, 1.0f}}};
}

static inline float VectorNorm(const std::array<float, 4>& vect) {
  return std::sqrt(vect[0] * vect[0] + vect[1] * vect[1] + vect[2] * vect[2]);
}

static float VectorInnerProduct(const std::array<float, 4>& vect1,
                                const std::array<float, 4>& vect2) {
  float product = 0;
  for (int i = 0; i < 3; i++) {
    product += vect1[i] * vect2[i];
  }
  return product;
}
}  // anonymous namespace

void TreasureHuntRenderer::keyPress (int k)
{ //X=99, Y=100, A=96, B=97
//UP=19, DOWN=20, LEFT=21, RIGHT=22, 
//https://developer.android.com/reference/android/view/KeyEvent.html
switch (k) {
	case 19:
		currentSet++;
		if (currentSet>TIMESTEPS-1)
	                currentSet=0;
		break;
	case 20:
		currentSet--;
		if (currentSet<0) 
			currentSet=TIMESTEPS-1;
		break;
	case 21:
		currentIso++;
		if (currentIso>ISOS)
			currentIso=0;
		break;
	case 22:
		currentIso--;
		if (currentIso<0)
			currentIso=ISOS;
		break;
	case 99: 
		animateTimesteps=!animateTimesteps;
		break;
	case 96:
		animateMovement=!animateMovement;
		
}
}

void TreasureHuntRenderer::loadConfigFile(void)
{
	if (configPath!=nullptr)
		eprintf ("configPath=<%s>", configPath);
	else
		eprintf ("configPath is null");

	if (TMPDIR!=nullptr)
		eprintf ("TMPDIR=<%s>", TMPDIR);
	else
		eprintf ("TMPDIR is null");
	
	std::string s(configPath);
	chdir(s.substr(0, s.find_last_of("\\/")).c_str());
	

	if ((error=::loadConfigFile(configPath))<0) {
		if (-100<error) {
			eprintf(loadConfigFileErrors[-error]);
			eprintf("Config file reading error");
		} else if (-200<error){
			eprintf(readAtomsXYZErrors[-error-100]);
			eprintf("XYZ file reading error");
		} else if (-300<error) {
			eprintf(readAtomsCubeErrors[-error-200]);
			eprintf("Cube file reading error");
		} else {
			eprintf(readAtomsJsonErrors[-error-300]);
			eprintf("Json reading error");
		}	
	}
	if (!solid) {
		LOGD("No atom glyph specified, using Icosahedron");
		solid=new Solid(Solid::Type::Icosahedron);
	}

	UserTranslation[0]=userpos[0]; //because of rotation in X to make Z vertical
	UserTranslation[1]=-userpos[2];
	UserTranslation[2]=userpos[1];
//	for (int i=0;i<3;i++)
// 		UserTranslation[i]=userpos[i];

//	LOGD("UT=%f, %f, %f\n", UserTranslation[0], UserTranslation[1], UserTranslation[2]);
}

TreasureHuntRenderer::TreasureHuntRenderer(
    gvr_context* gvr_context, std::unique_ptr<gvr::AudioApi> gvr_audio_api)
    : gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
      scratch_viewport_(gvr_api_->CreateBufferViewport()),
      gvr_controller_api_(nullptr),
      gvr_viewer_type_(gvr_api_->GetViewerType()) {

	loadConfigFile();
	//eprintf("after config load, timesteps=%d", TIMESTEPS);
	


  ResumeControllerApiAsNeeded();
  if (gvr_viewer_type_ == GVR_VIEWER_TYPE_CARDBOARD) {
    LOGD("Viewer type: CARDBOARD");
  } else if (gvr_viewer_type_ == GVR_VIEWER_TYPE_DAYDREAM) {
    LOGD("Viewer type: DAYDREAM");
  } else {
    LOGE("Unexpected viewer type.");
  }
}

TreasureHuntRenderer::~TreasureHuntRenderer() {
	glDeleteProgram(AtomsP);
	glDeleteProgram(UnitCellP);

	glDeleteTextures(2, textures);
}

const char * ErrorsGL[] = {
	"Failure compiling Unit Cell Shader", //-401
	"Failure compiling Atom Shader NoTess", //-402;

};

void TreasureHuntRenderer::InitializeGl() {
  gvr_api_->InitializeGl();

glGenTextures(2, textures);
glGenTextures(2+ZLAYERS, textDepthPeeling);
    //white
    unsigned char data2[4]={255,255,255,255}; //white texture for non-textured meshes
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);

    // Create the programs
	//Leave atoms until we check if android 7 has gles 3.2 or if we use the old icosahedron method with no tesselation
	if (!PrepareUnitCellShader (&UnitCellP, &UnitCellMatrixLoc, &UnitCellColourLoc)) {
		eprintf("OneTimeInit, failure compiling Unit Cell Shader");
		error=-401;
		return ;
	}
	
	//rgh: for now, we don't have any tess-ready phones
	//if (!PrepareAtomShader(&AtomsP, &AtomMatrixLoc)) {
		hasTess=false;
		if (!PrepareAtomShaderNoTess(&AtomsP, &AtomMatrixLoc, &totalatomsLocation)) {
			error=-402;
			eprintf ("PrepareAtomShaderNoTess failed");
		}
	//};

	//atom texture
	int e;
	
	e=atomTexture(textures[1]);
	if (e!=GL_NO_ERROR) {
		eprintf ("atomTexture error %d", e);
		error=-403;
	}

	e=SetupAtoms(&AtomTVAO, &AtomTBuffer, &BondIndices);
	if (e!=GL_NO_ERROR) {
		eprintf ("SetupAtoms error %d", e);
		error=-404;
	}

	if (!hasTess)
		e=SetupAtomsNoTess(&AtomVAO, &AtomBuffer, &AtomIndices);

	if (e!=GL_NO_ERROR) {
		eprintf ("SetupAtomsNoTess error %d, tess=%d", e, hasTess);
		error=-405;
	}
	e=SetupUnitCell(&UnitCellVAO, &UnitCellBuffer, &UnitCellIndexBuffer);
	if (e!=GL_NO_ERROR) {
		eprintf ("SetupUnitCell error %d", e);
		error=-406;
	}
	e=SetupMarkerNoTess(&MarkerVAO, &MarkerVertBuffer, &MarkerIndexBuffer);
	if (e!=GL_NO_ERROR) {
		eprintf ("SetupMarkerNoTess error %d", e);
		error=-411;
	}

  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality.
  render_size_ =
      HalfPixelCount(gvr_api_->GetMaximumEffectiveRenderTargetSize());
  std::vector<gvr::BufferSpec> specs;

  specs.push_back(gvr_api_->CreateBufferSpec());
  specs[0].SetColorFormat(GVR_COLOR_FORMAT_RGBA_8888);
  specs[0].SetDepthStencilFormat(GVR_DEPTH_STENCIL_FORMAT_DEPTH_16);
  specs[0].SetSize(render_size_);
  specs[0].SetSamples(2);

  swapchain_.reset(new gvr::SwapChain(gvr_api_->CreateSwapChain(specs)));

  viewport_list_.reset(
      new gvr::BufferViewportList(gvr_api_->CreateEmptyBufferViewportList()));

   if (ISOS || markers) {
	PrepareISOShader(&ISOP, &ISOMatrixLoc);
   }
//isosurfaces
	if (ISOS) {
		currentIso=ISOS;
		//rgh: FIXME Additive blending for android; do not need most of these textures
		//additive blending means white background makes everything white
		float m=BACKGROUND[0];
		m=std::max(m, BACKGROUND[1]);
		m=std::max(m, BACKGROUND[2]);
		if (m>0.2f) { //make background darker, preserving colour, so additive blending works
			for (int i=0;i<3;i++)
				BACKGROUND[i]*=0.2f / m;
		}

		e=SetupDepthPeeling(render_size_.width, render_size_.height, ZLAYERS, 
			textDepthPeeling, &peelingFramebuffer);
		if (!e) {
			eprintf("Error setting up DepthPeeling\n");
			error=-407;
		}


		if (GL_NO_ERROR!=(e=PrepareISOTransShader (&TransP, 
			&TransMatrixLoc, &BlendP)))
		{
			eprintf( "Error Preparing Transparency shader, %d\n", e );
			error=-408;
		}
		SetupBlending(&BlendVAO, &BlendBuffer, &BlendIndices);

		std::vector<float> vertices;
#ifndef INDICESGL32
		std::vector<short> indices;
#else
		std::vector<GLuint> indices;
#endif
		numISOIndices=new int[TIMESTEPS*ISOS];
		ISOVAO=new GLuint[TIMESTEPS*ISOS];
		ISOBuffer=new GLuint[TIMESTEPS*ISOS];
		ISOIndices=new GLuint[TIMESTEPS*ISOS];

		glGenBuffers(TIMESTEPS*ISOS, ISOBuffer);
		glGenVertexArrays(TIMESTEPS*ISOS, ISOVAO);
		glGenBuffers(TIMESTEPS*ISOS, ISOIndices);

		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("opengl error %d, glGenBuffers\n", e);

		char tmpname[250];
		int timestep=1;
		for (int p = 0; p < TIMESTEPS*ISOS; p++) {
			sprintf(tmpname, "%s%d-%s.ply", PATH, timestep, 
				plyfiles[p % ISOS]);
			gvr::Mat4f trans;
			gvr::Mat4f matFinal, matcubetrans, mvs, sc, sctrans;

		if (voxelSize[0]!=-1) {
			mvs=ScalingMatrix(scaling/(float)voxelSize[0], scaling/(float)voxelSize[1],
				scaling/(float)voxelSize[2]);
			matcubetrans=TranslationMatrix(cubetrans);
			gvr::Mat4f abcm {abc[0][0], abc[0][1], abc[0][2], 0,
					abc[1][0], abc[1][1], abc[1][2], 0,
					abc[2][0], abc[2][1], abc[2][2], 0,
					0, 0, 0, 1};
			sc=ScalingMatrix(supercell);
			sctrans=TranslationMatrix(-translations[p%ISOS][2], 
					-translations[p%ISOS][1], -translations[p%ISOS][0]);
			matFinal = MatrixMul(abcm,sctrans);
			matFinal=MatrixMul(matFinal,sc);
			matFinal=MatrixMul(matFinal,mvs);
		} else {
			for (int i=0;i<4;i++)
				for(int j=0;j<4;j++)
					matFinal.m[i][j]=(i==j);
			for (int i=0;i<3;i++)
				matFinal.m[i][3]=translations[p%ISOS][i];

			trans.m[0][0]=scaling;trans.m[0][1]=0;trans.m[0][2]=0; trans.m[0][3]=0;
			trans.m[1][0]=0;trans.m[1][1]=scaling;trans.m[1][2]=0;trans.m[1][3]=0; 
			trans.m[2][0]=0;trans.m[2][1]=0;trans.m[2][2]=scaling; trans.m[2][3]=0;
			trans.m[3][0]=0;trans.m[3][1]=0;trans.m[3][2]=0; trans.m[3][3]=1;
			matFinal=MatrixMul(trans, matFinal);
		}
			float mat[16];
			for (int i=0;i<4;i++)
				for (int j=0;j<4;j++)
					mat[j*4+i]=matFinal.m[i][j];
			if (!AddModelToScene(mat, vertices, indices,
	                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              			tmpname, false, isocolours[p%ISOS][0]<0, p%ISOS))
			{
				eprintf("Error loading ply file %s\n", tmpname);
				//return; 
			}
#ifndef INDICESGL32
			if (vertices.size() > 65535 * numComponents)
			{
				eprintf("Mesh has more than 64k vertices (%d), unsupported\n", vertdataarray[currentlod][p].size() / numComponents);
				return;
			}
#endif
			numISOIndices[p] = indices.size();
			if (GL_NO_ERROR!=PrepareGLiso(ISOVAO[p], ISOBuffer[p], 
				vertices, ISOIndices[p], indices))
				eprintf ("PrepareGLiso, GL error");
			
			vertices.clear();
			indices.clear();

			if (p % ISOS == ISOS - 1) {
				//eprintf ("timestep %d", timestep);
				timestep++;
			}
		}
	}



}

void TreasureHuntRenderer::ResumeControllerApiAsNeeded() {
  switch (gvr_viewer_type_) {
    case GVR_VIEWER_TYPE_CARDBOARD:
      gvr_controller_api_.reset();
      break;
    case GVR_VIEWER_TYPE_DAYDREAM:
      if (!gvr_controller_api_) {
        // Initialized controller api.
        gvr_controller_api_.reset(new gvr::ControllerApi);
        CHECK(gvr_controller_api_);
        CHECK(gvr_controller_api_->Init(gvr::ControllerApi::DefaultOptions(),
                                        gvr_api_->cobj()));
      }
      gvr_controller_api_->Resume();
      break;
    default:
      LOGE("unexpected viewer type.");
      break;
  }
}

void TreasureHuntRenderer::ProcessControllerInput() {
  const int old_status = gvr_controller_state_.GetApiStatus();
  const int old_connection_state = gvr_controller_state_.GetConnectionState();

  // Read current controller state.
  gvr_controller_state_.Update(*gvr_controller_api_);

  // Print new API status and connection state, if they changed.
  if (gvr_controller_state_.GetApiStatus() != old_status ||
      gvr_controller_state_.GetConnectionState() != old_connection_state) {
    LOGD("TreasureHuntApp: controller API status: %s, connection state: %s",
         gvr_controller_api_status_to_string(
             gvr_controller_state_.GetApiStatus()),
         gvr_controller_connection_state_to_string(
             gvr_controller_state_.GetConnectionState()));
  }

  // Trigger click event if app/click button is clicked.
  if (gvr_controller_state_.GetButtonDown(GVR_CONTROLLER_BUTTON_APP) ||
      gvr_controller_state_.GetButtonDown(GVR_CONTROLLER_BUTTON_CLICK)) {
//    OnTriggerEvent();
  }
}

void TreasureHuntRenderer::DrawFrame() {
  if (gvr_viewer_type_ == GVR_VIEWER_TYPE_DAYDREAM) {
    ProcessControllerInput();
  }

if (animateTimesteps) {
	if (animationspeed>1)
		currentSet+=animationspeed;
	else {
		static float current=0;
		current+=animationspeed;
		if (current>1) {
			currentSet++;
			current=0;
		}
	}
            if (currentSet>TIMESTEPS-1)
                currentSet=0;
	}

  PrepareFramebuffer();
  gvr::Frame frame = swapchain_->AcquireFrame();

  // A client app does its rendering here.
  gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
  target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

  head_view_ = gvr_api_->GetHeadSpaceFromStartSpaceRotation(target_time);

if (animateMovement) {
	const float speed=0.01;
	gvr::Mat4f inv=invert(head_view_);

	std::array<float, 4> dir({0,0,1,0}); // {0,0,1,0}
	std::array<float, 4> dir2=MatrixVectorMul(inv, dir);
	
	for (int i=0;i<3;i++)
		UserTranslation[i]+=dir2[i]*speed*movementspeed;

}


	LOGD("UT=%f, %f, %f\n", UserTranslation[0], UserTranslation[1], UserTranslation[2]);
	//LOGD("MovementSpeed=%f\n", movementspeed);

  gvr::Mat4f left_eye_matrix = gvr_api_->GetEyeFromHeadMatrix(GVR_LEFT_EYE);
  gvr::Mat4f right_eye_matrix = gvr_api_->GetEyeFromHeadMatrix(GVR_RIGHT_EYE);
  gvr::Mat4f left_eye_view = MatrixMul(left_eye_matrix, head_view_);
  gvr::Mat4f right_eye_view = MatrixMul(right_eye_matrix, head_view_);

  viewport_list_->SetToRecommendedBufferViewports();
  gvr::BufferViewport reticle_viewport = gvr_api_->CreateBufferViewport();

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_BLEND);

  // Draw the world.
  frame.BindBuffer(0);
  if (error<0) {
	//rgh: FIXME, add textures with messages here
		if (-100<error) { //error with ncfg file
			glClearColor(1.f, 0.f, 0.f, 1.f); 
		} else if (-200<error){//error loading xyz
			glClearColor(0.f, 1.f, 0.f, 1.f); 
		} else if (-300<error) {//error loading gaussian
			glClearColor(0.f, 0.f, 1.f, 1.f); 
		} else if (-400<error) {//error loading encyclopedia json
			glClearColor(0.f, 1.f, 1.f, 1.f); 
		} else if (-500<error) {//error loading analytics json
			glClearColor(1.f, 1.f, 0.f, 1.f); 
		} else {
			glClearColor(1.f, 0.f, 1.f, 1.f); 
		}
  } else {
	glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1.f);
  }

    // Dark background so text shows up.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  viewport_list_->GetBufferViewport(0, &scratch_viewport_);
  DrawWorld(left_eye_view, scratch_viewport_);
  viewport_list_->GetBufferViewport(1, &scratch_viewport_);
  DrawWorld(right_eye_view, scratch_viewport_);
  frame.Unbind();

  // Submit frame.
  frame.Submit(*viewport_list_, head_view_);

  CheckGLError("onDrawFrame");
}

void TreasureHuntRenderer::PrepareFramebuffer() {
  // Because we are using 2X MSAA, we can render to half as many pixels and
  // achieve similar quality.
  const gvr::Sizei recommended_size =
      HalfPixelCount(gvr_api_->GetMaximumEffectiveRenderTargetSize());
  if (render_size_.width != recommended_size.width ||
      render_size_.height != recommended_size.height) {
    // We need to resize the framebuffer.
    swapchain_->ResizeBuffer(0, recommended_size);
    render_size_ = recommended_size;
  }
}

void TreasureHuntRenderer::OnPause() {
  gvr_api_->PauseTracking();
  if (gvr_controller_api_) gvr_controller_api_->Pause();
}

void TreasureHuntRenderer::OnResume() {
  gvr_api_->ResumeTracking();
  gvr_viewer_type_ = gvr_api_->GetViewerType();
  ResumeControllerApiAsNeeded();
}

/**
 * Converts a raw text file, saved as a resource, into an OpenGL ES shader.
 *
 * @param type  The type of shader we will be creating.
 * @param resId The resource ID of the raw text file.
 * @return The shader object handler.
 */
int TreasureHuntRenderer::LoadGLShader(int type, const char** shadercode) {
  int shader = glCreateShader(type);
  glShaderSource(shader, 1, shadercode, nullptr);
  glCompileShader(shader);

  // Get the compilation status.
  int compileStatus;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);

  // If the compilation failed, delete the shader.
  if (compileStatus == 0) {
    glDeleteShader(shader);
    shader = 0;
  }

  return shader;
}

/**
 * Draws a frame for an eye.
 *
 * @param eye The eye to render. Includes all required transformations.
 */
void TreasureHuntRenderer::DrawWorld(const gvr::Mat4f& view_matrix,
                                     const gvr::BufferViewport& viewport) {

  const gvr::Recti pixel_rect =
      CalculatePixelSpaceRect(render_size_, viewport.GetSourceUv());

  glViewport(pixel_rect.left, pixel_rect.bottom,
             pixel_rect.right - pixel_rect.left,
             pixel_rect.top - pixel_rect.bottom);

  CheckGLError("World drawing setup");


  const gvr::Mat4f perspective =
      PerspectiveMatrixFromView(viewport.GetSourceFov(), kZNear, kZFar);
  modelview_ = MatrixMul(view_matrix, model_cube_);
  modelview_projection_cube_ = MatrixMul(perspective, modelview_);

  if (gvr_viewer_type_ == GVR_VIEWER_TYPE_DAYDREAM) {
    gvr::Mat4f controller_matrix =
        ControllerQuatToMatrix(gvr_controller_state_.GetOrientation());
//    DrawCursor();
  }

//glClearColor(0,0.5,0.2,1);
//glClear(GL_COLOR_BUFFER_BIT);

	//NOMAD rendering
glDisable(GL_CULL_FACE);
modelview_=view_matrix;
modelview_projection_cube_ = MatrixMul(perspective, modelview_);

//make sure we have the same coordinate system in HTC Vive and android

const gvr::Mat4f rot={.m={1,0,0,0, 	0,0,1,0,	0,-1,0,0,	0,0,0,1}};

gvr::Mat4f trans=MatrixMul (TranslationMatrix (UserTranslation), rot);
gvr::Mat4f finalMatrix=MatrixMul(modelview_projection_cube_, trans);


if(error)
	return;
if (has_abc) {
	RenderUnitCell(modelview_projection_cube_);
} else {
	RenderAtomTrajectories(modelview_projection_cube_);
}

if (ISOS)
	RenderIsos(modelview_projection_cube_, currentIso);
}

void TreasureHuntRenderer::RenderIsos(const gvr::Mat4f eyeViewProjection, int curDataPos)
{
GLenum e;

gvr::Mat4f trans=TranslationMatrix (UserTranslation[0], UserTranslation[1], UserTranslation[2]);
const gvr::Mat4f rot={.m={1,0,0,0, 	0,0,1,0,	0,-1,0,0,	0,0,0,1}};					
gvr::Mat4f transform = MatrixMul(eyeViewProjection, MatrixMul(trans,rot));

float t[16];
for (int i=0;i<4;i++)
	for (int j=0;j<4;j++)
		t[j*4+i]=transform.m[i][j];

glUseProgram(ISOP);
if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
glUniformMatrix4fv(ISOMatrixLoc, 1, GL_FALSE, t);
if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("2 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);

if (curDataPos!=ISOS) {
	glBindVertexArray(ISOVAO[currentSet*ISOS+curDataPos]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("3 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
	//eprintf ("Drawing %d vertices, isos", numISOIndices[currentSet*ISOS+curDataPos]);
	glDrawElements(GL_TRIANGLES,numISOIndices[currentSet*ISOS+curDataPos] , GL_UNSIGNED_INT, 0);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("4 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
} else {
/*transparency*/
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glDepthMask(GL_FALSE);
	for (int i=0;i<ISOS;i++) {
		glBindVertexArray(ISOVAO[currentSet*ISOS+i]);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("5 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
		//eprintf ("Drawing %d vertices, isos", numISOIndices[currentSet*ISOS+i]);
		glDrawElements(GL_TRIANGLES,numISOIndices[currentSet*ISOS+i] , GL_UNSIGNED_INT, 0); 
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("6 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
	}
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
} //if (curDataPos!=ISOS) 
//eprintf ("end of RenderIsos");
glBindVertexArray(0);
}

void TreasureHuntRenderer::RenderMarker(const float *m) //m[16]
{
int e;
if (!markers)
	return;

glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
glBindVertexArray(MarkerVAO);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("b %d", e);
glUseProgram(ISOP);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("c %d", e);
glUniformMatrix4fv(ISOMatrixLoc, 1, GL_FALSE, m);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("d %d, matrixloc=%d, program=%d", e, ISOMatrixLoc, ISOP); 
glDrawElements(GL_TRIANGLES, 3*3*MARKERSOLID::nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT, (void*)(currentSet*sizeof(unsigned short)*3*3*MARKERSOLID::nFaces
)
#else
				GL_UNSIGNED_INT, (void*)(currentSet*sizeof(unsigned int)*3*3*MARKERSOLID::nFaces
)
#endif
	);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("e %d", e);
glBindVertexArray(0);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("f %d", e);

glDisable(GL_BLEND);
}


void TreasureHuntRenderer::RenderAtoms(const float *m) //m[16]
{
	//eprintf ("RenderAtoms start numatoms %d, timestep %d", numAtoms[currentSet], currentSet);
	//eprintf ("solid nfaces %d", solid->nFaces);
	int e;
	if (numAtoms==0)
		return;
	
	if (hasTess) {
	//FIXME, unimplemented
		LOGW("FIXME, No Tess code for atoms yet!");
		return;
	} else { //no tess
		glBindVertexArray(AtomVAO[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AtomIndices[0]);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("1 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);
		glBindBuffer(GL_ARRAY_BUFFER, AtomBuffer[0]);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("2 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("3 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("4 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("5 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glBindTexture(GL_TEXTURE_2D, textures[1]);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("6 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glUseProgram(AtomsP);
		glUniform1f(totalatomsLocation, (float)getTotalAtomsInTexture());
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("7 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		glUniformMatrix4fv(AtomMatrixLoc, 1, GL_FALSE, m);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("8 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		if (currentSet==0) {
			glDrawElements(GL_TRIANGLES, numAtoms[currentSet]* 3 * solid->nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT,
#else
				GL_UNSIGNED_INT,
#endif	
				0);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("9 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		} else {
			glDrawElements(GL_TRIANGLES, (numAtoms[currentSet]-numAtoms[currentSet-1]) * 3 * solid->nFaces,
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT, (void*)(numAtoms[currentSet-1]*sizeof(unsigned short)*3*solid->nFaces)
#else
				GL_UNSIGNED_INT, (void*)(numAtoms[currentSet-1]*sizeof(unsigned int)*3*solid->nFaces)
#endif
				);
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("10 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);

		}
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render  Atom timestep =%d: %d\n", currentSet, e);
		//now cloned atoms
		if (numClonedAtoms!=0 && currentSet==0) {
			glBindVertexArray(AtomVAO[1]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AtomIndices[1]);
			glBindBuffer(GL_ARRAY_BUFFER, AtomBuffer[1]);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));

			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("5 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);
			glDrawElements(GL_TRIANGLES, numClonedAtoms* 3 * solid->nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT,
#else
				GL_UNSIGNED_INT,
#endif	
				0);			
			
			
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("Gl error after Render cloned Atom timestep =%d: %d\n", currentSet, e);
		} // painting cloned atoms
		//now bonds
		if (numBonds && displaybonds /*&& showAtoms*/) {
			glLineWidth(bondThickness);
			glBindVertexArray(AtomTVAO[2]);
			glUseProgram(UnitCellP);
			glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, m);
			glUniform4fv(UnitCellColourLoc, 1, bondscolours);
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("Gl error after Render Atom bonds uniform timestep =%d: %d\n", currentSet, e);
			if (currentSet==0||fixedAtoms)
				glDrawElements(GL_LINES, numBonds[0],  GL_UNSIGNED_INT, (void*)0);
			else
				glDrawElements(GL_LINES, numBonds[currentSet]-numBonds[currentSet-1], GL_UNSIGNED_INT, 
					(void*)(sizeof(int)*numBonds[currentSet-1]) );
			glLineWidth(1.0f);
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("Gl error after Render Atom bonds timestep =%d: %d\n", currentSet, e);
			glBindVertexArray(0);
		}
	} // no tess
}



void TreasureHuntRenderer::RenderAtomTrajectories(const gvr::Mat4f eyeViewProjection)
{
int e;
if (!numAtoms)
	return;
gvr::Mat4f sc=ScalingMatrix(scaling);
gvr::Mat4f trans=TranslationMatrix (UserTranslation[0]/scaling, UserTranslation[1]/scaling, UserTranslation[2]/scaling);
const gvr::Mat4f rot={.m={1,0,0,0, 	0,0,1,0,	0,-1,0,0,	0,0,0,1}};					
//trans.translate(iPos).rotateX(-90).translate(UserPosition);
gvr::Mat4f transform = MatrixMul(eyeViewProjection, MatrixMul(sc, MatrixMul(trans,rot)));
//gvr::Mat4f transform=eyeViewProjection;					
float t[16];
for (int i=0;i<4;i++)
	for (int j=0;j<4;j++)
		t[j*4+i]=transform.m[i][j];
glUseProgram(UnitCellP);
glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after glUniform4fv 1 RenderAtomTrajectories: %d\n", e);
RenderAtomTrajectoriesUnitCell();
RenderAtoms(t);
RenderMarker(t);
}

void TreasureHuntRenderer::RenderAtomTrajectoriesUnitCell()
{
	//now trajectories
if (!showTrajectories)
	return;

int e;
if (!AtomTVAO) {
	LOGW("RenderAtomTrajectoriesUnitCell, no atoms");
	return;
}
glBindVertexArray(AtomTVAO[0]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderAtomTrajectoriesUnitCell: %d\n", e);
//glUseProgram(UnitCellP);
//glUniformMatrix4fv(m_nUnitCellMatrixLocation, 1, GL_FALSE, matrix);
glUniform4fv(UnitCellColourLoc, 1, atomtrajectorycolour);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after glUniform4fv 2 RenderAtomTrajectoriesUnitCell: %d\n", e);
//glEnableVertexAttribArray(0);
//glDisableVertexAttribArray(1);

//LOG("atomtrajectories.size()=%d", atomtrajectories.size());
//rgh FIXME, old code which does not work with large atom sets!

glBindBuffer(GL_ARRAY_BUFFER, AtomTBuffer[0]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("3 Gl error RenderAtomTrajectoriesUnitCell: %d\n", e);

for (unsigned int i=0;i<atomtrajectories.size();i++) {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float)*numAtoms[0], 
		(const void *)(0+4*sizeof(float)*atomtrajectories[i]));
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("4 Gl error RenderAtomTrajectoriesUnitCell: %d\n", e);

		//LOG("atomtrajectoryrestarts[%d].size()=%d", i, atomtrajectoryrestarts[i].size());
	for (unsigned int j=1;j<atomtrajectoryrestarts[i].size();j++) {
		int orig=atomtrajectoryrestarts[i][j-1];
		int count=atomtrajectoryrestarts[i][j]-atomtrajectoryrestarts[i][j-1];
		glDrawArrays(GL_LINE_STRIP, orig, count);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("5 Gl error RenderAtomTrajectoriesUnitCell: %d, orig=%d, count=%d\n", e, orig, count);

	} //j
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error after Render Atom trajectories: %d\n", e);
} //i

} //OvrApp::RenderAtomTrajectoriesUnitCell()


void TreasureHuntRenderer::RenderUnitCell(const gvr::Mat4f eyeViewProjection)
{
	//eprintf ("eyeViewProjection");
	//for (int i=0;i<4;i++)
	//	for (int j=0;j<4;j++)
	//		eprintf ("%d %d = %f", i, j, eyeViewProjection.m[i][j]);
	//eprintf ("RenderUnitCell, has_abc=%d", has_abc);
	if (!has_abc)
		return;
	if (UnitCellVAO==0)
		eprintf ("Error, Unit Cell VAO not loaded");
	int e;
	
	int p[3];
	gvr::Mat4f sc=ScalingMatrix(scaling);
	for (p[0]=0;p[0]<repetitions[0];(p[0])++)
		for (p[1]=0;p[1]<repetitions[1];(p[1])++)
			for (p[2]=0;p[2]<repetitions[2];(p[2])++)
				{
					float delta[3];
					GetDisplacement(p, delta);
					/*gvr::Mat4f trans={1,0,0,delta[0]+UserTranslation[0],
						0,1,0,delta[1]+UserTranslation[1],
						0,0,1,delta[2]+UserTranslation[2],
						0,0,0,1};*/
gvr::Mat4f trans=TranslationMatrix (delta[0]+UserTranslation[0]/scaling, delta[2]+UserTranslation[1]/scaling, 
	-delta[1]+UserTranslation[2]/scaling);
const gvr::Mat4f rot={.m={1,0,0,0, 	0,0,1,0,	0,-1,0,0,	0,0,0,1}};					
//trans.translate(iPos).rotateX(-90).translate(UserPosition);
gvr::Mat4f transform = MatrixMul(eyeViewProjection, MatrixMul(sc, MatrixMul(trans,rot)));					



					//gvr::Mat4f transform = MatrixMul(eyeViewProjection,trans);
					//gvr::Mat4f transform=eyeViewProjection;					
					float t[16];
					for (int i=0;i<4;i++)
						for (int j=0;j<4;j++)
							t[j*4+i]=transform.m[i][j];
					glUseProgram(UnitCellP);
					glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
					if ((e = glGetError()) != GL_NO_ERROR)
						eprintf("Gl error after glUniform4fv 1 RenderUnitCell: %d\n", e);
					if (displayunitcell) {
						glUniform4fv(UnitCellColourLoc, 1, unitcellcolour);
						if ((e = glGetError()) != GL_NO_ERROR)
							eprintf("Gl error after glUniform4fv 2 RenderUnitCell: %d\n", e);
						glBindVertexArray(UnitCellVAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, UnitCellIndexBuffer);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("1 Gl error RenderAtom timestep =%d: %d\n", currentSet, e);
		glBindBuffer(GL_ARRAY_BUFFER, UnitCellBuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
		glEnableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
		glDisableVertexAttribArray(3);
						if ((e = glGetError()) != GL_NO_ERROR)
							eprintf("Gl error after glBindVertexArray RenderUnitCell: %d\n", e);
						glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
						//glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);
						if ((e = glGetError()) != GL_NO_ERROR)
							eprintf("Gl error after RenderUnitCell: %d\n", e);
					}
					//atom trajectories
					RenderAtomTrajectoriesUnitCell();
					RenderAtoms(t);
					RenderMarker(t);
				}
}

void TreasureHuntRenderer::OnTriggerEvent() {
	animateMovement=!animateMovement;
}

