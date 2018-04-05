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

#ifndef TREASUREHUNT_APP_SRC_MAIN_JNI_TREASUREHUNTRENDERER_H_  // NOLINT
#define TREASUREHUNT_APP_SRC_MAIN_JNI_TREASUREHUNTRENDERER_H_  // NOLINT

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define ZLAYERS 2

class TreasureHuntRenderer {
 public:
  /**
   * Create a TreasureHuntRenderer using a given |gvr_context|.
   *
   * @param gvr_api The (non-owned) gvr_context.
   * @param gvr_audio_api The (owned) gvr::AudioApi context.
   */
  TreasureHuntRenderer(gvr_context* gvr_context);

  /**
   * Destructor.
   */
  ~TreasureHuntRenderer();

  /**
   * Initialize any GL-related objects. This should be called on the rendering
   * thread with a valid GL context.
   */
  void InitializeGl();

  /**
   * Draw the TreasureHunt scene. This should be called on the rendering thread.
   */
  void DrawFrame();

  /**
   * Hide the cube if it's being targeted.
   */
  void OnTriggerEvent();

  /**
   * Pause head tracking.
   */
  void OnPause();

  /**
   * Resume head tracking, refreshing viewer parameters if necessary.
   */
  void OnResume();

 private:
  int CreateTexture(int width, int height, int textureFormat, int textureType);

  /**
   * Converts a raw text file, saved as a resource, into an OpenGL ES shader.
   *
   * @param type  The type of shader we will be creating.
   * @param resId The resource ID of the raw text file.
   * @return The shader object handler.
   */
  int LoadGLShader(int type, const char** shadercode);

  /**
   * Draws a frame for an eye.
   *
   * @param eye The eye to render. Includes all required transformations.
   */
  void DrawEye(gvr::Eye eye, const gvr::Mat4f& view_matrix,
               const gvr::BufferViewport& params);

  /**
   * Draw the cube.
   *
   * We've set all of our transformation matrices. Now we simply pass them
   * into the shader.
   */
  void DrawCube();


  std::unique_ptr<gvr::GvrApi> gvr_api_;
  std::unique_ptr<gvr::BufferViewportList> viewport_list_;
  std::unique_ptr<gvr::SwapChain> swapchain_;
  gvr::BufferViewport scratch_viewport_;

  gvr::Mat4f head_view_;
  gvr::Mat4f model_cube_;
  gvr::Mat4f camera_;
  gvr::Mat4f view_;
  gvr::Mat4f modelview_projection_cube_;
  gvr::Mat4f modelview_;
  gvr::Sizei render_size_;
 
    void loadConfigFile(void);
    
    GLuint textures[2]; // white, atoms
    GLuint textDepthPeeling[ZLAYERS+2];
    GLuint peelingFramebuffer;
    //if no tesselation is available, we still need the tess atoms for the trajectories!
    GLuint *AtomTVAO=nullptr, *AtomTBuffer=nullptr, *AtomVAO=nullptr, *AtomBuffer=nullptr, *AtomIndices=nullptr,//[2], atoms, extraatoms
        BondIndices,
        UnitCellVAO, UnitCellBuffer, UnitCellIndexBuffer,
        MarkerVAO, MarkerVertBuffer, MarkerIndexBuffer;
    GLuint			AtomsP, UnitCellP; // framework does not provide support for tesselation and provides many things we don't need.
    GLint		AtomMatrixLoc, UnitCellMatrixLoc, UnitCellColourLoc, totalatomsLocation;
    GLuint	TransP=0, BlendP=0;
    GLint	TransMatrixLoc=-1;
    bool hasTess=true;
    
    GLuint *ISOVAO=nullptr/*[ISOS*TIMESTEPS]*/, *ISOBuffer=nullptr/*[ISOS*TIMESTEPS]*/,
    *ISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;
    GLuint ISOP;
    GLint ISOMatrixLoc;
    GLuint BlendVAO=0, BlendBuffer=0, BlendIndices=0;
    int *numISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;
    
    int currentSet=0;
    bool animateTimesteps=true;
    bool animateMovement=false;
    int currentIso;
    
    void RenderAtoms(const float *m);
    void RenderMarker(const float *m);
    void RenderUnitCell(const gvr::Mat4f eyeViewProjection);
    void RenderAtomTrajectoriesUnitCell();
    void RenderAtomTrajectories(const gvr::Mat4f eyeViewProjection);
    void RenderIsos(const gvr::Mat4f eyeViewProjection, int curDataPos);
    float UserTranslation[3]={10,0,0};
    
    int error=0;
};

#endif  // TREASUREHUNT_APP_SRC_MAIN_JNI_TREASUREHUNTRENDERER_H_  // NOLINT
