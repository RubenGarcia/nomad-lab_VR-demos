

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

#ifndef TREASUREHUNT_APP_SRC_MAIN_JNI_TREASUREHUNTRENDERER_H_  // NOLINT
#define TREASUREHUNT_APP_SRC_MAIN_JNI_TREASUREHUNTRENDERER_H_  // NOLINT

#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <jni.h>

#include <memory>
#include <string>
#include <thread>  // NOLINT
#include <vector>

#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_controller.h"
#include "vr/gvr/capi/include/gvr_types.h"

#define ZLAYERS 2

extern const char * configPath;

class TreasureHuntRenderer {
 public:
  /**
   * Create a TreasureHuntRenderer using a given |gvr_context|.
   *
   * @param gvr_api The (non-owned) gvr_context.
   * @param gvr_audio_api The (owned) gvr::AudioApi context.
   */
  TreasureHuntRenderer(gvr_context* gvr_context,
                       std::unique_ptr<gvr::AudioApi> gvr_audio_api);


	void keyPress (int k);
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
   * Pause head tracking.
   */
  void OnPause();

  /**
   * Resume head tracking, refreshing viewer parameters if necessary.
   */
  void OnResume();
void OnTriggerEvent();
void loadConfigFile(void);
 private:
  int CreateTexture(int width, int height, int textureFormat, int textureType);

  /*
   * Prepares the GvrApi framebuffer for rendering, resizing if needed.
   */
  void PrepareFramebuffer();

  /**
   * Converts a raw text file, saved as a resource, into an OpenGL ES shader.
   *
   * @param type  The type of shader we will be creating.
   * @param resId The resource ID of the raw text file.
   * @return The shader object handler.
   */
  int LoadGLShader(int type, const char** shadercode);

  /**
   * Draws all world-space objects for one eye.
   *
   * @param view_matrix View transformation for the current eye.
   * @param viewport The buffer viewport for which we are rendering.
   */
  void DrawWorld(const gvr::Mat4f& view_matrix,
                 const gvr::BufferViewport& viewport);

   /**
   * Process the controller input.
   *
   * The controller state is updated with the latest touch pad, button clicking
   * information, connection state and status of the controller. A log message
   * is reported if the controller status or connection state changes. A click
   * event is triggered if a click on app/click button is detected.
   */
  void ProcessControllerInput();

  /*
   * Resume the controller api if needed.
   *
   * If the viewer type is cardboard, set the controller api pointer to null.
   * If the viewer type is daydream, initialize the controller api as needed and
   * resume.
   */
  void ResumeControllerApiAsNeeded();

  std::unique_ptr<gvr::GvrApi> gvr_api_;
  std::unique_ptr<gvr::BufferViewportList> viewport_list_;
  std::unique_ptr<gvr::SwapChain> swapchain_;
  gvr::BufferViewport scratch_viewport_;

  std::vector<float> lightpos_;

//gles 2 does not have vao, porting demo to gles3

  gvr::Mat4f head_view_;
  gvr::Mat4f model_cube_;
  gvr::Mat4f camera_;
  gvr::Mat4f view_;
  gvr::Mat4f modelview_projection_cube_;
  gvr::Mat4f modelview_;
  gvr::Sizei render_size_;

  int score_;

  gvr::AudioSourceId audio_source_id_;

  gvr::AudioSourceId success_source_id_;

  std::thread audio_initialization_thread_;

  // Controller API entry point.
  std::unique_ptr<gvr::ControllerApi> gvr_controller_api_;

  // The latest controller state (updated once per frame).
  gvr::ControllerState gvr_controller_state_;

  gvr::ViewerType gvr_viewer_type_;

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
