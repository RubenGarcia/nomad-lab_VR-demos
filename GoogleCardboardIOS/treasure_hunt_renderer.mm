/*# Copyright 2016-2018 The NOMAD Developers Group*/
/* Copyright 2016 Google Inc. All rights reserved.
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

#include "treasure_hunt_renderer.h"

#include <assert.h>
#include <cmath>

#include "NOMADVRLib/MyGL.h"

#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/ConfigFile.h"
#include "NOMADVRLib/atomsGL.h"
#include "NOMADVRLib/UnitCellShaders.h"
#include "NOMADVRLib/TessShaders.h"
#include "NOMADVRLib/polyhedron.h"
#include "NOMADVRLib/IsosurfacesGL.h"
#include "NOMADVRLib/eprintf.h"

#include "GoogleCardboard/aux.h"

#define LOG_TAG "NOMADVRIOS"
#define LOGW(...) NSLog(@__VA_ARGS__)

void eprintf( const char *fmt, ... )
{
    va_list args;
    char buffer[ 2048 ];
    
    va_start( args, fmt );
    vsprintf( buffer, fmt, args );
    va_end( args );
    
    LOGW("Message in NOMADgvrT");
    if (*fmt=='\0')
        LOGW("Empty format");
    LOGW("<%s>", buffer);
}

// TODO(sanjayc): Refactor this app to reuse treasure_hunt_renderer from:
// vr/gvr/demos/treasurehunt. http://b/28880883

namespace {
//static const int kTextureFormat = GL_RGB;
//static const int kTextureType = GL_UNSIGNED_BYTE;

//static const float kZNear = 1.0f;
//static const float kZFar = 100.0f;
static const float kZNear = 0.01f;
static const float kZFar = 1000.0f;

static const uint64_t kPredictionTimeWithoutVsyncNanos = 50000000;

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
      result[i] += matrix.m[i][k]*vec[k];
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
        result.m[i][j] += matrix1.m[i][k]*matrix2.m[k][j];
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
  float width = static_cast<float>(texture_size.width);
  float height = static_cast<float>(texture_size.height);
  gvr::Rectf rect = ModulateRect(texture_rect, width, height);
  gvr::Recti result = {
      static_cast<int>(rect.left), static_cast<int>(rect.right),
      static_cast<int>(rect.bottom), static_cast<int>(rect.top)};
  return result;
}

static void CheckGLError(const char* label) {
  int gl_error = glGetError();
  if (gl_error != GL_NO_ERROR) {
    LOGW("GL error @ %s: %d", label, gl_error);
  }
  assert(glGetError() == GL_NO_ERROR);
}

}  // namespace

void TreasureHuntRenderer::loadConfigFile(void)
{
   //http://www.techotopia.com/index.php/Working_with_Directories_on_iPhone_OS#The_Application_Documents_Directory
    NSArray *dirPaths;
    NSString *docsDir;
    
    dirPaths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                                   NSUserDomainMask, YES);
    
    docsDir = [dirPaths objectAtIndex:0];
    NSFileManager *filemgr = [NSFileManager defaultManager];

    if ([filemgr changeCurrentDirectoryPath: docsDir] == NO)
        NSLog(@"Cannot change current directory");
    
    if ((error=::loadConfigFile("cytosine.ncfg"))<0) {
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
        LOGW("No atom glyph specified, using Icosahedron");
        solid=new Solid(Solid::Type::Icosahedron);
    }
    
    UserTranslation[0]=userpos[0]; //because of rotation in X to make Z vertical
    UserTranslation[1]=-userpos[2];
    UserTranslation[2]=userpos[1];

}


TreasureHuntRenderer::TreasureHuntRenderer(
    gvr_context* gvr_context)
    : gvr_api_(gvr::GvrApi::WrapNonOwned(gvr_context)),
      scratch_viewport_(gvr_api_->CreateBufferViewport())
{
    loadConfigFile();
}

TreasureHuntRenderer::~TreasureHuntRenderer() {}

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
    

  std::vector<gvr::BufferSpec> specs;
  specs.push_back(gvr_api_->CreateBufferSpec());
  render_size_ = specs[0].GetSize();
  swapchain_.reset(new gvr::SwapChain(gvr_api_->CreateSwapChain(specs)));

  viewport_list_.reset(new gvr::BufferViewportList(gvr_api_->CreateEmptyBufferViewportList()));

}

void TreasureHuntRenderer::DrawFrame() {
  viewport_list_->SetToRecommendedBufferViewports();
    
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
  gvr::Frame frame = swapchain_->AcquireFrame();

  // A client app does its rendering here.
  gvr::ClockTimePoint target_time = gvr::GvrApi::GetTimePointNow();
  target_time.monotonic_system_time_nanos += kPredictionTimeWithoutVsyncNanos;

  head_view_ = gvr_api_->GetHeadSpaceFromStartSpaceRotation(target_time);
  gvr::Mat4f left_eye_view_pose = MatrixMul(gvr_api_->GetEyeFromHeadMatrix(GVR_LEFT_EYE),
                                            head_view_);
  gvr::Mat4f right_eye_view_pose = MatrixMul(gvr_api_->GetEyeFromHeadMatrix(GVR_RIGHT_EYE),
                                             head_view_);
    
    if (animateMovement) {
        const float speed=0.01;
        gvr::Mat4f inv=invert(head_view_);
        
        std::array<float, 4> dir({0,0,1,0}); // {0,0,1,0}
        std::array<float, 4> dir2=MatrixVectorMul(inv, dir);
        
        for (int i=0;i<3;i++)
            UserTranslation[i]+=dir2[i]*speed*movementspeed;
        
    }
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_SCISSOR_TEST);
  viewport_list_->GetBufferViewport(0, &scratch_viewport_);
  DrawEye(GVR_LEFT_EYE, left_eye_view_pose, scratch_viewport_);
  viewport_list_->GetBufferViewport(1, &scratch_viewport_);
  DrawEye(GVR_RIGHT_EYE, right_eye_view_pose, scratch_viewport_);

  // Bind back to the default framebuffer.
  frame.Unbind();
  frame.Submit(*viewport_list_, head_view_);

  CheckGLError("onDrawFrame");
}

void TreasureHuntRenderer::OnTriggerEvent() {
    animateMovement=!animateMovement;
}

void TreasureHuntRenderer::OnPause() {
  gvr_api_->PauseTracking();
}

void TreasureHuntRenderer::OnResume() {
  gvr_api_->RefreshViewerProfile();
  gvr_api_->ResumeTracking();
}

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
  int* compileStatus = new int[1];
  glGetShaderiv(shader, GL_COMPILE_STATUS, compileStatus);

  // If the compilation failed, delete the shader.
  if (compileStatus[0] == 0) {
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
void TreasureHuntRenderer::DrawEye(gvr::Eye eye, const gvr::Mat4f& view_matrix,
                                   const gvr::BufferViewport& params) {
  const gvr::Recti pixel_rect =
      CalculatePixelSpaceRect(render_size_, params.GetSourceUv());

  glViewport(pixel_rect.left, pixel_rect.bottom,
             pixel_rect.right - pixel_rect.left,
             pixel_rect.top - pixel_rect.bottom);
  glScissor(pixel_rect.left, pixel_rect.bottom,
            pixel_rect.right - pixel_rect.left,
            pixel_rect.top - pixel_rect.bottom);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  CheckGLError("ColorParam");

  // Set the position of the light
  gvr::Mat4f perspective =
      PerspectiveMatrixFromView(params.GetSourceFov(), kZNear, kZFar);

    modelview_ = view_matrix;
    modelview_projection_cube_ = MatrixMul(perspective, modelview_);

    //rgh, draw scene here
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

