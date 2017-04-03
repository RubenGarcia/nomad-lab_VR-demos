﻿//========= Copyright Valve Corporation ============//
#define NOMINMAX

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DEBUG_NEW

#include <vector>

#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
//#include <SDL_ttf.h>
#include <gl/glu.h>
#include <stdio.h>
#include <string>
#include <cstdlib>


#include <winsock2.h>

#include <openvr.h>

#include "shared/lodepng.h"
#include "shared/Matrices.h"
#include "shared/pathtools.h"

#include "rply/rply.h"

#include "NOMADVRLib/ConfigFile.h"
#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/atomsGL.h"

#include "NOMADVRLib/TessShaders.h"
#include "NOMADVRLib/UnitCellShaders.h"

#include "NOMADVRLib/CompileGLShader.h"

#include "NOMADVRLib/polyhedron.h"

static int vertex_cb(p_ply_argument argument);
static int face_cb(p_ply_argument argument);

static float *CubeVertices;
static int *CubeIndices;
static int CurrentVertex;
static int CurrentIndex;

//#define SOLID Tetrahedron //not nicely uniform distribution
//#define SOLID Icosahedron
#define TESSSUB 16

//TTF_Font* font;

//#define LOCALTEST

/*#ifdef LOCALTEST
#define PATH "Y:\\v2t\\datasets\\mpcdf_CO2_CaO\\cubes-bigger\\IsosurfacesFixed\\"
#else
#define PATH "data\\CO2-CaO-B\\"
//#define PATH "C:\\Users\\mobile\\Desktop\\openvrDemos\\win64\\data\\CO2-CaO-B\\"
#endif
*/
#define ZLAYERS 12

//number of components in our vertices data xyz+nxnynz+rgba
#define numComponents 10

#define MAXGPU 300
//use 32 or 16 bits for index (in case more than 64k vertices / mesh
#define INDICESGL32 

//shown on https://www.vbw-bayern.de/vbw/Aktionsfelder/Innovation-F-T/Forschung-und-Technologie/Zukunft-digital-Big-Data.jsp

#define GRID 1
#define GRIDSTR "1"
#define NUMLODS 1


#define NUMPLY (TIMESTEPS * ISOS)

class CGLRenderModel
{
public:
	CGLRenderModel( const std::string & sRenderModelName );
	~CGLRenderModel();

	bool BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture );
	void Cleanup();
	void Draw();
	const std::string & GetName() const { return m_sModelName; }

private:
	GLuint m_glVertBuffer;
	GLuint m_glIndexBuffer;
	GLuint m_glVertArray;
	GLuint m_glTexture;
	GLsizei m_unVertexCount;
	std::string m_sModelName;
};

static bool g_bPrintf = true;

//-----------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
class CMainApplication
{
public:
	CMainApplication(int argc, char *argv[]);
	virtual ~CMainApplication();

	bool BInit();
	bool BInitGL();
	bool BInitCompositor();

	void SetupRenderModels();

	void Shutdown();

	void RunMainLoop();
	bool HandleInput();
	void ProcessVREvent(const vr::VREvent_t & event);
	void RenderFrame();

	bool SetupTexturemaps();
	bool SetupDepthPeeling();

	void SetupScene();
	void SetupIsosurfaces();
	void SetupAtoms();
	void SetupUnitCell();

	bool AddModelToScene(Matrix4 mat, std::vector<float> &vertdata, 
#ifndef INDICESGL32
		std::vector<short> & vertindices, 
#else
		std::vector<GLuint> & vertindices,
#endif
		const char *const data, bool water, bool colours, int set);

	void DrawControllers();

	bool SetupStereoRenderTargets();
	void SetupDistortion();
	void SetupCameras();

	void RenderStereoTargets();
	void RenderDistortion();
	void RenderScene(vr::Hmd_Eye nEye);
	void RenderAtoms(const vr::Hmd_Eye &nEye);
	void RenderAtomsUnitCell(const vr::Hmd_Eye &nEye, int p[3]);

	void RenderUnitCell(const vr::Hmd_Eye &nEye);
	//Vector3 GetDisplacement(int p[3]);

	Matrix4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
	Matrix4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye);
	Matrix4 GetCurrentViewMatrix( vr::Hmd_Eye nEye );
	void UpdateHMDMatrixPose();

	Matrix4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t &matPose);

	bool CreateAllShaders();

	void SetupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t unTrackedDeviceIndex);
	CGLRenderModel *FindOrLoadRenderModel(const char *pchRenderModelName);
	void RenderAllTrackedRenderModels(vr::Hmd_Eye nEye);
private:
	bool m_bDebugOpenGL;
	bool m_bVerbose;
	bool m_bPerf;
	bool m_bVblank;
	bool m_bGlFinishHack;

	vr::IVRSystem *m_pHMD;
	vr::IVRRenderModels *m_pRenderModels;
	std::string m_strDriver;
	std::string m_strDisplay;
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	Matrix4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
	bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];

	std::vector<float> **vertdataarray; //[LOD][PLY]
#ifndef INDICESGL32
	std::vector<short> **vertindicesarray;//[LOD][PLY]
#else
	std::vector<GLuint> **vertindicesarray;//[LOD][PLY]
#endif
private: // SDL bookkeeping
	SDL_Window *m_pWindow;
	uint32_t m_nWindowWidth;
	uint32_t m_nWindowHeight;

	SDL_GLContext m_pContext;

	void PaintGrid(const vr::Hmd_Eye &nEye, const int iso);
	bool PaintBox();


private: // OpenGL bookkeeping
	int m_iTrackedControllerCount;
	int m_iTrackedControllerCount_Last;
	int m_iValidPoseCount;
	int m_iValidPoseCount_Last;
	bool m_bShowCubes;
	bool buttonPressed[2][vr::k_unMaxTrackedDeviceCount]; //grip, application menu
	std::string m_strPoseClasses;                            // what classes we saw poses for this frame
	char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class

	int m_iSceneVolumeWidth;
	int m_iSceneVolumeHeight;
	int m_iSceneVolumeDepth;
	float m_fScaleSpacing;
	float m_fScale;

	int m_iSceneVolumeInit;                                  // if you want something other than the default 20x20x20

	float m_fNearClip;
	float m_fFarClip;

	GLuint *m_iTexture; //[3+ZLAYERS+1] // white, depth1, depth2, color[ZLAYERS], atomtexture
	SDL_Texture **axisTextures; //[6]
	GLuint peelingFramebuffer;
	unsigned int **m_uiVertcount;// [LODS][NUMPLY];

	//for isos
	GLuint **m_glSceneVertBuffer; // [LODS][NUMPLY];
	GLuint **m_unSceneVAO; //[LODS][NUMPLY];
	GLuint **m_unSceneVAOIndices; //[LODS][NUMPLY];

	//for atoms
	GLuint *m_glAtomVertBuffer; // [TIMESTEPS];
	GLuint *m_unAtomVAO; //[TIMESTEPS];

	//for unit cells
	GLuint m_glUnitCellVertBuffer; // primitive, possibly non-primitive in further. Deformed cube
	GLuint m_glUnitCellIndexBuffer; // 
	GLuint m_unUnitCellVAO; //

	int currentset;
	float elapsedtime;
	static const float videospeed;
	int currentiso;
	int firstdevice;
	GLuint m_unLensVAO;
	GLuint m_glIDVertBuffer;
	GLuint m_glIDIndexBuffer;
	unsigned int m_uiIndexSize;

	GLuint m_glControllerVertBuffer;
	GLuint m_unControllerVAO;
	unsigned int m_uiControllerVertcount;

	Matrix4 m_mat4HMDPose;
	Matrix4 m_mat4eyePosLeft;
	Matrix4 m_mat4eyePosRight;

	Matrix4 m_mat4ProjectionCenter;
	Matrix4 m_mat4ProjectionLeft;
	Matrix4 m_mat4ProjectionRight;

	Vector3 UserPosition;

	struct VertexDataScene
	{
		Vector3 position;
		Vector2 texCoord;
	};

	struct VertexDataLens
	{
		Vector2 position;
		Vector2 texCoordRed;
		Vector2 texCoordGreen;
		Vector2 texCoordBlue;
	};

	GLuint m_unSceneProgramID;
	GLuint m_unLensProgramID;
	GLuint m_unControllerTransformProgramID;
	GLuint m_unRenderModelProgramID;
	GLuint m_unAtomsProgramID;
	GLuint m_unUnitCellProgramID;

	GLint m_nSceneMatrixLocation;
	GLint m_nBlendingIntLocation;
	GLint m_nControllerMatrixLocation;
	GLint m_nRenderModelMatrixLocation;
	GLint m_nAtomMatrixLocation;
	GLint m_nAtomMVLocation;
	GLint m_nUnitCellMatrixLocation, m_nUnitCellColourLocation;

	struct FramebufferDesc
	{
		GLuint m_nDepthBufferId;
		GLuint m_nRenderTextureId;
		GLuint m_nRenderFramebufferId;
		GLuint m_nResolveTextureId;
		GLuint m_nResolveFramebufferId;
	};
	FramebufferDesc leftEyeDesc;
	FramebufferDesc rightEyeDesc;

	bool CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc );
	void CleanDepthTexture();
	uint32_t m_nRenderWidth;
	uint32_t m_nRenderHeight;

	std::vector< CGLRenderModel * > m_vecRenderModels;
	CGLRenderModel *m_rTrackedDeviceToRenderModel[ vr::k_unMaxTrackedDeviceCount ];

	char * pixels; //for saving screenshots to disk
	int framecounter;
	bool savetodisk;
};

const float CMainApplication::videospeed = 0.01f;


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void dprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	if ( g_bPrintf )
		printf( "%s", buffer );

	OutputDebugStringA( buffer );
	if (strncmp(buffer, "PoseCount", 9) && strncmp (buffer, "Device", 6))
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Warning", buffer, 0);
}


//pure windows, no sdl
void eprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf_s( buffer, fmt, args );
	va_end( args );

	if ( g_bPrintf )
		printf( "%s", buffer );

	MessageBoxA(0, buffer, "Warning", 0);
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMainApplication::CMainApplication(int argc, char *argv[])
	: m_pWindow(NULL)
	, m_pContext(NULL)
	, m_nWindowWidth(1920)
	, m_nWindowHeight(1080)
	, m_unSceneProgramID(0)
	, m_unLensProgramID(0)
	, m_unControllerTransformProgramID(0)
	, m_unRenderModelProgramID(0)
	, m_unAtomsProgramID(0)
	, m_unUnitCellProgramID(0)
	, m_pHMD(NULL)
	, m_pRenderModels(NULL)
	, m_bDebugOpenGL(false)
	, m_bVerbose(false)
	, m_bPerf(false)
	, m_bVblank(false)
	, m_bGlFinishHack(true)
	, m_glControllerVertBuffer(0)
	, m_unControllerVAO(0)
	, m_unLensVAO(0)
	, m_unSceneVAO(0)
	, m_unSceneVAOIndices(0)
	, m_unAtomVAO(0)
	, m_glAtomVertBuffer(0)
	, m_glUnitCellVertBuffer(-1)
	, m_glUnitCellIndexBuffer(-1)
	, m_unUnitCellVAO(0)
	, m_nSceneMatrixLocation(-1)
	, m_nBlendingIntLocation(-1)
	, m_nControllerMatrixLocation(-1)
	, m_nRenderModelMatrixLocation(-1)
	, m_nAtomMatrixLocation(-1)
	, m_nAtomMVLocation(-1)
	, m_nUnitCellMatrixLocation(-1)
	, m_nUnitCellColourLocation(-1)
	, m_iTrackedControllerCount(0)
	, m_iTrackedControllerCount_Last(-1)
	, m_iValidPoseCount(0)
	, m_iValidPoseCount_Last(-1)
	, m_iSceneVolumeInit(20)
	, m_strPoseClasses("")
	, m_bShowCubes(true)
	, currentset(0)
	, elapsedtime(videospeed*float(SDL_GetTicks()))
	, currentiso(ISOS)
	, firstdevice(-1)
	, m_iTexture(0)
	, axisTextures(0)
	, peelingFramebuffer(0)
	, m_uiVertcount(0)
	, m_glSceneVertBuffer(0)
	//, UserPosition(Vector3(-101.0f * 0.15f*0.5f*GRID + 12.5f, -15.0f, -101.0f * 0.15f*0.5f*GRID + 101.0f * 0.15f*0.25f))
	, UserPosition(Vector3(-userpos[0] * 0.04f, -userpos[1] * 0.04f, -userpos[2] * 0.04f))
	, vertdataarray(0)
	, vertindicesarray(0)
	, pixels(0)
	, framecounter(0)
	, savetodisk(false)
{
	for (int j=0;j<2;j++)
		for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
			buttonPressed[j][i] = false;
		}

	for( int i = 1; i < argc; i++ )
	{
		if( !stricmp( argv[i], "-gldebug" ) )
		{
			m_bDebugOpenGL = true;
		}
		else if( !stricmp( argv[i], "-verbose" ) )
		{
			m_bVerbose = true;
		}
		else if( !stricmp( argv[i], "-novblank" ) )
		{
			m_bVblank = false;
		}
		else if( !stricmp( argv[i], "-noglfinishhack" ) )
		{
			m_bGlFinishHack = false;
		}
		else if( !stricmp( argv[i], "-noprintf" ) )
		{
			g_bPrintf = false;
		}
		else if ( !stricmp( argv[i], "-cubevolume" ) && ( argc > i + 1 ) && ( *argv[ i + 1 ] != '-' ) )
		{
			m_iSceneVolumeInit = atoi( argv[ i + 1 ] );
			i++;
		}
	}
	// other initialization tasks are done in BInit
	memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));
};


//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMainApplication::~CMainApplication()
{
	// work is done in Shutdown
	dprintf( "Shutdown" );
}


//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
std::string GetTrackedDeviceString( vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL )
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, NULL, 0, peError );
	if( unRequiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[ unRequiredBufferLen ];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty( unDevice, prop, pchBuffer, unRequiredBufferLen, peError );
	std::string sResult = pchBuffer;
	delete [] pchBuffer;
	return sResult;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInit()
{


	if ( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_TIMER ) < 0 )
	{
		printf("%s - SDL could not initialize! SDL Error: %s\n", __FUNCTION__, SDL_GetError());
		return false;
	}

	// Loading the SteamVR Runtime
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Scene );

	if ( eError != vr::VRInitError_None )
	{
		m_pHMD = NULL;
		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to init VR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}


	m_pRenderModels = (vr::IVRRenderModels *)vr::VR_GetGenericInterface( vr::IVRRenderModels_Version, &eError );
	if( !m_pRenderModels )
	{
		m_pHMD = NULL;
		vr::VR_Shutdown();

		char buf[1024];
		sprintf_s( buf, sizeof( buf ), "Unable to get render model interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription( eError ) );
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "VR_Init Failed", buf, NULL );
		return false;
	}

	int nWindowPosX = 0;
	int nWindowPosY = 0;
	m_nWindowWidth = 1920;
	m_nWindowHeight = 1080;
	Uint32 unWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN_DESKTOP*/; //rgh: interferes with mode change to 100Hz refresh

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 4 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 0 );
	SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, 0 );
	if( m_bDebugOpenGL )
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG );

	m_pWindow = SDL_CreateWindow( "Geophysics OpenVR SDL", nWindowPosX, nWindowPosY, m_nWindowWidth, m_nWindowHeight, unWindowFlags );
	if (m_pWindow == NULL)
	{
		printf( "%s - Window could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	m_pContext = SDL_GL_CreateContext(m_pWindow);
	if (m_pContext == NULL)
	{
		printf( "%s - OpenGL context could not be created! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}

	glewExperimental = GL_TRUE;
	GLenum nGlewError = glewInit();
	if (nGlewError != GLEW_OK)
	{
		printf( "%s - Error initializing GLEW! %s\n", __FUNCTION__, glewGetErrorString( nGlewError ) );
		return false;
	}
	glGetError(); // to clear the error caused deep in GLEW

	if ( SDL_GL_SetSwapInterval( m_bVblank ? 1 : 0 ) < 0 )
	{
		printf( "%s - Warning: Unable to set VSync! SDL Error: %s\n", __FUNCTION__, SDL_GetError() );
		return false;
	}


	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	m_strDriver = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String );
	m_strDisplay = GetTrackedDeviceString( m_pHMD, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String );

	std::string strWindowTitle = "Geophysics OpenVR - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle( m_pWindow, strWindowTitle.c_str() );
	
	// cube array
 	m_iSceneVolumeWidth = m_iSceneVolumeInit;
 	m_iSceneVolumeHeight = m_iSceneVolumeInit;
 	m_iSceneVolumeDepth = m_iSceneVolumeInit;
 		
 	m_fScale = 0.04f; //0.15f; //rgh: original too big for room
 	m_fScaleSpacing = 4.0f;
 
 	m_fNearClip = 0.2f;
 	m_fFarClip = 50.0f;//rgh: original 30 too small for our skymap
 

 
// 		m_MillisecondsTimer.start(1, this);
// 		m_SecondsTimer.start(1000, this);
	
	if (!BInitGL())
	{
		printf("%s - Unable to initialize OpenGL!\n", __FUNCTION__);
		return false;
	}

	if (!BInitCompositor())
	{
		printf("%s - Failed to initialize VR Compositor!\n", __FUNCTION__);
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void APIENTRY DebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char* message, const void* userParam)
{
	dprintf( "GL Error: %s\n", message );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInitGL()
{
	if( m_bDebugOpenGL )
	{
		glDebugMessageCallback(DebugCallback, nullptr);
		glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE );
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	}

	if( !CreateAllShaders() )
		return false;

	SetupTexturemaps();
	SetupScene();
	SetupCameras();
	SetupStereoRenderTargets();
	SetupDistortion();
	SetupDepthPeeling();

	SetupRenderModels();

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::BInitCompositor()
{
	vr::EVRInitError peError = vr::VRInitError_None;

	if ( !vr::VRCompositor() )
	{
		printf( "Compositor initialization failed. See log file for details\n" );
		return false;
	}

	return true;
}


void deleteVaos (GLuint *** vaos, int numlods, int numply)
{
if( *vaos != 0 )
{
	for (int i = 0; i < numlods; i++) {
		glDeleteVertexArrays(numply, (*vaos)[i]);
		delete[] (*vaos)[i];
	}
	delete[] (*vaos);
	*vaos = 0;
}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::Shutdown()
{
	if( m_pHMD )
	{
		vr::VR_Shutdown();
		m_pHMD = NULL;
	}

	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		delete (*i);
	}
	m_vecRenderModels.clear();
	
	if (vertdataarray) {
		for (int i = 0; i < NUMLODS; i++)
			if (vertdataarray[i])
				delete[] vertdataarray[i];
		delete[]vertdataarray;
		vertdataarray = 0;
	}
	if (vertindicesarray){
		for (int i = 0; i < NUMLODS; i++)
			if (vertindicesarray[i])
				delete[] vertindicesarray[i];
		delete[] vertindicesarray;
		vertindicesarray = 0;
	}
	if (m_pContext)
	{
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_FALSE);
		glDebugMessageCallback(nullptr, nullptr);
		if (m_glSceneVertBuffer) {
			for (int i = 0; i < NUMLODS; i++) {
				glDeleteBuffers(NUMPLY, m_glSceneVertBuffer[i]);
				delete[] (m_glSceneVertBuffer[i]);
			}
			delete[] m_glSceneVertBuffer;
			m_glSceneVertBuffer = nullptr;
		}
		glDeleteBuffers(1, &m_glIDVertBuffer);
		glDeleteBuffers(1, &m_glIDIndexBuffer);

		if ( m_unSceneProgramID )
		{
			glDeleteProgram( m_unSceneProgramID );
		}
		if ( m_unControllerTransformProgramID )
		{
			glDeleteProgram( m_unControllerTransformProgramID );
		}
		if ( m_unRenderModelProgramID )
		{
			glDeleteProgram( m_unRenderModelProgramID );
		}
		if ( m_unLensProgramID )
		{
			glDeleteProgram( m_unLensProgramID );
		}
		if ( m_unAtomsProgramID )
		{
			glDeleteProgram( m_unAtomsProgramID );
		}
		if (m_unUnitCellProgramID)
			glDeleteProgram(m_unUnitCellProgramID);

		glDeleteRenderbuffers( 1, &leftEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &leftEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &leftEyeDesc.m_nResolveFramebufferId );

		glDeleteRenderbuffers( 1, &rightEyeDesc.m_nDepthBufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nRenderTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nRenderFramebufferId );
		glDeleteTextures( 1, &rightEyeDesc.m_nResolveTextureId );
		glDeleteFramebuffers( 1, &rightEyeDesc.m_nResolveFramebufferId );

		if( m_unLensVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unLensVAO );
		}
		deleteVaos(&m_unSceneVAO, NUMLODS, NUMPLY);
		if( m_unAtomVAO != 0 )
		{
			glDeleteVertexArrays(1, m_unAtomVAO);
			delete[] m_unAtomVAO;
			m_unAtomVAO = 0;
		}		
		if (m_glAtomVertBuffer!=0)
		{
			glDeleteBuffers(1, m_glAtomVertBuffer);
			delete[] m_glAtomVertBuffer;
			m_glAtomVertBuffer=0;
		}

		if (m_unUnitCellVAO!=0) {
			glDeleteVertexArrays(1, &m_unUnitCellVAO);
		}
		if (m_glUnitCellVertBuffer!=-1) {
			glDeleteBuffers(1, &m_glUnitCellVertBuffer);
			m_glUnitCellVertBuffer=-1;
		}

		if (m_glUnitCellIndexBuffer!=-1) {
			glDeleteBuffers(1, &m_glUnitCellIndexBuffer);
			m_glUnitCellIndexBuffer=-1;
		}

		if( m_unControllerVAO != 0 )
		{
			glDeleteVertexArrays( 1, &m_unControllerVAO );
		}
		if (m_unSceneVAOIndices!=0)
		{
			for (int i=0;i<NUMLODS;i++) {
				glDeleteBuffers(NUMPLY, m_unSceneVAOIndices[i]);
				delete[] m_unSceneVAOIndices[i];
			}
			delete[] m_unSceneVAOIndices;
		}
		if (m_iTexture != 0) {
			glDeleteTextures(3+ZLAYERS+1, m_iTexture);
			delete[] m_iTexture;
			m_iTexture = 0;

		}
		if (axisTextures !=0) {
			delete[] axisTextures;
		}
		if (peelingFramebuffer!=0)
			glDeleteFramebuffers(1, &peelingFramebuffer);
	}

	if( m_pWindow )
	{
		SDL_DestroyWindow(m_pWindow);
		m_pWindow = NULL;
	}

	if (m_uiVertcount!=0) {
		for (int i=0;i<NUMLODS;i++) {
			delete[] m_uiVertcount[i];
		}
		delete[] m_uiVertcount;
	}

	if (numAtoms!=0) {
		for (int i=0;i<TIMESTEPS;i++) {
			delete[] atoms[i];
		}
		delete[] atoms;
		delete[] numAtoms;
		numAtoms=0;
		atoms=0;
	}
	if (pixels !=0)
		delete [] pixels;
	SDL_Quit();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::HandleInput()
{
	SDL_Event sdlEvent;
	bool bRet = false;

	float speed = 0.02f;

	while (SDL_PollEvent(&sdlEvent) != 0)
	{
		if (sdlEvent.type == SDL_QUIT)
		{
			bRet = true;
		}
		else if (sdlEvent.type == SDL_KEYDOWN)
		{// q is too near 1, disable
			if (sdlEvent.key.keysym.sym == SDLK_ESCAPE
				/*|| sdlEvent.key.keysym.sym == SDLK_q*/)
			{
				bRet = true;
			}
			if (sdlEvent.key.keysym.sym == SDLK_c)
			{
				m_bShowCubes = !m_bShowCubes;
			} else if (sdlEvent.key.keysym.sym == SDLK_s)
			{
				savetodisk = !savetodisk;
			}
			//rgh: add keyboard navigation here
			if (sdlEvent.key.keysym.sym == SDLK_1) {
				currentset++;
				if (currentset >= TIMESTEPS)
					currentset = 0;
			}
			if (sdlEvent.key.keysym.sym == SDLK_2) {
				currentset--;
				if (currentset < 0)
					currentset = TIMESTEPS -1;
			}
			if (sdlEvent.key.keysym.sym == SDLK_0) {
				currentiso++;
				if (currentiso > ISOS)
					currentiso = 0;
			}
			if (sdlEvent.key.keysym.sym == SDLK_a) {
				Matrix4 tmp = m_mat4HMDPose;
				UserPosition += tmp.invert()*Vector3(0, 0, speed);
				//UserPosition.x += speed;
			}
			if (sdlEvent.key.keysym.sym == SDLK_y) {
				//UserPosition.x -= speed;
				Matrix4 tmp = m_mat4HMDPose;
				UserPosition -= tmp.invert()*Vector3(0, 0, speed);
			}
			if (sdlEvent.key.keysym.sym == SDLK_o) {
				UserPosition[2] -= 0.1f;
				dprintf("%f %f\n", UserPosition[0], UserPosition[2]);
			}
			if (sdlEvent.key.keysym.sym == SDLK_p) {
				UserPosition[2] += 0.1f;
				dprintf("%f %f\n", UserPosition[0], UserPosition[2]);
			}
		}
	}

	// Process SteamVR events
	vr::VREvent_t event;
	while (m_pHMD->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}

	// Process SteamVR controller state
	for (vr::TrackedDeviceIndex_t unDevice = 0; unDevice < vr::k_unMaxTrackedDeviceCount; unDevice++)
	{
		vr::VRControllerState_t state;
		if (m_pHMD->GetControllerState(unDevice, &state))
		{
			m_rbShowTrackedDevice[unDevice] = state.ulButtonPressed == 0;

			if (!buttonPressed[1][unDevice] && state.ulButtonTouched&vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu)) {
				buttonPressed[1][unDevice] = true;
				if (firstdevice == -1)
					firstdevice = unDevice;
				savetodisk = !savetodisk;
			}
			else if (buttonPressed[1][unDevice] && 0 == (state.ulButtonTouched&vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu)))
			{
				buttonPressed[1][unDevice] = false;
			}

			if (!buttonPressed[0][unDevice] && state.ulButtonTouched&vr::ButtonMaskFromId(vr::k_EButton_Grip))
			{
				buttonPressed[0][unDevice] = true;
				if (firstdevice == -1)
					firstdevice = unDevice;
				if (unDevice == firstdevice) {
					currentset--;
					if (currentset < 0)
						currentset = TIMESTEPS - 1;
				} else {
					currentiso++;
					if (currentiso > ISOS)
						currentiso = 0;
				}
			} else if (buttonPressed[0][unDevice] && (state.ulButtonTouched&vr::ButtonMaskFromId(vr::k_EButton_Grip)) == 0)
				buttonPressed[0][unDevice] = false;

			if (state.rAxis[1].x >0.1)
			{
				if (firstdevice == -1)
					firstdevice = unDevice;
				if (unDevice == firstdevice) {
					Matrix4 tmp = m_mat4HMDPose;
					UserPosition += tmp.invert()*Vector3(0, 0, speed);
				}
				else {
					float newtime = videospeed*float(SDL_GetTicks());;
					if (newtime-elapsedtime > 1) {
						elapsedtime = newtime;
						currentset++;
						if (currentset >= TIMESTEPS)
							currentset = 0;
					}
				}
			}
		}
	}

	return bRet;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RunMainLoop()
{
	bool bQuit = false;

	SDL_StartTextInput();
	SDL_ShowCursor( SDL_DISABLE );

	while ( !bQuit )
	{
		bQuit = HandleInput();
		RenderFrame();
	}

	SDL_StopTextInput();
}


//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void CMainApplication::ProcessVREvent( const vr::VREvent_t & event )
{
	switch (event.eventType)
	{
		case vr::VREvent_TrackedDeviceActivated:
			{
				SetupRenderModelForTrackedDevice(event.trackedDeviceIndex);
				dprintf("Device %u attached. Setting up render model.\n", event.trackedDeviceIndex);
			}
		break;
		case vr::VREvent_TrackedDeviceDeactivated:
			{
				dprintf("Device %u detached.\n", event.trackedDeviceIndex);
			}
		break;
		case vr::VREvent_TrackedDeviceUpdated:
			{
				dprintf("Device %u updated.\n", event.trackedDeviceIndex);
			}
		break;
		/*
		case vr::VREvent_ButtonTouch:
			{
				buttonPressed = true;
			}
		break;
		case vr::VREvent_ButtonUntouch:
		{
			buttonPressed = false;
		}
		break;*/
		/*case vr::VREvent_ButtonPress:
			{
				m_bShowCubes = ~m_bShowCubes;
			}*/
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderFrame()
{
	// for now as fast as possible
	if ( m_pHMD )
	{
		DrawControllers();
		RenderStereoTargets();
		RenderDistortion();

		vr::Texture_t leftEyeTexture = {(void*)leftEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {(void*)rightEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
	}

	if ( m_bVblank && m_bGlFinishHack )
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow( m_pWindow );
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	// Flush and wait for swap.
	if ( m_bVblank )
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
		
		dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}

	UpdateHMDMatrixPose();
}





//-----------------------------------------------------------------------------
// Purpose: Creates all the shaders used by HelloVR SDL
//-----------------------------------------------------------------------------
bool CMainApplication::CreateAllShaders()
{
	m_unSceneProgramID = CompileGLShader(
		"Scene",

		//rgh FIXME, multiply normals by modelview!!
		// Vertex Shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 normalsIn;\n"
		"layout(location = 2) in vec4 colorIn;\n"
		//		"layout(location = 3) in vec2 uvIn;\n"
		"out vec4 color;\n"
		"out vec3 n;\n"
		"out highp vec4 pos;\n"
		//		"out vec2 uv;\n"
		"void main()\n"
		"{\n"
		"	color = vec4(colorIn.rgba);\n"
		"   n=normalize(normalsIn);\n"
		//		"   uv=uvIn;\n"
		"int i=gl_InstanceID / " GRIDSTR ";\n"
		"int j=gl_InstanceID % " GRIDSTR ";\n"
		"	pos = matrix * (position + vec4 (float(i)*0.15*101.0, 0, float(j)*0.15*101.0, 0));\n"
		"   gl_Position = pos;\n"
		//"	gl_Position = matrix * position;\n"
		"}\n",

		// Fragment Shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n" //now extra depth texture for peeling
		"in vec4 color;\n"
		"in vec3 n;\n"
		"in highp vec4 pos;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"vec4 mytex=texture(diffuse, vec2(pos.x/pos.w*0.5+0.5, pos.y/pos.w*0.5+0.5));\n"
		//http://www.gamedev.net/topic/556521-glsl-manual-shadow-map-biasscale/
		//"vec2 d=vec2(dFdx(pos.z), dFdy(pos.z));\n"
		//"highp float m=sqrt(d.x*d.x + d.y*d.y);\n"
		"if ((pos.z/pos.w+1)/2 <= mytex.r+0.0001 ) discard;\n"

		"lowp vec3 nn=normalize(n);"
		"lowp float a=max(0.0, dot(nn, vec3(0,sqrt(2.0)/2.0,sqrt(2.0)/2.0)));\n"
		"lowp float b=max(0.0, dot(nn, vec3(0,0,1)));\n"
		"highp vec4 res=color;\n"
		//"outputColor = vec4(pos.x/pos.w*0.5+0.5, pos.y/pos.w*0.5+0.5, 0,1);\n"
		"	outputColor = vec4 ((res.rgb) * (0.2 + 0.2*a + 0.3*b), color.a);\n" 
		
		"}\n"
		);
	m_nSceneMatrixLocation = glGetUniformLocation( m_unSceneProgramID, "matrix" );
	if( m_nSceneMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in scene shader\n" );
		return false;
	}
	/*m_nBlendingIntLocation = glGetUniformLocation(m_unSceneProgramID, "blending");
	if (m_nBlendingIntLocation == -1)
	{
		dprintf("Unable to find blending uniform in scene shader\n");
		return false;
	}*/ 
	m_unControllerTransformProgramID = CompileGLShader(
		"Controller",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3ColorIn;\n"
		"out vec4 v4Color;\n"
		"void main()\n"
		"{\n"
		"	v4Color.xyz = v3ColorIn; v4Color.a = 1.0;\n"
		"	gl_Position = matrix * position;\n"
		"}\n",

		// fragment shader
		"#version 410\n"
		"in vec4 v4Color;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = v4Color;\n"
		"}\n"
		);
	m_nControllerMatrixLocation = glGetUniformLocation( m_unControllerTransformProgramID, "matrix" );
	if( m_nControllerMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in controller shader\n" );
		return false;
	}

	m_unRenderModelProgramID = CompileGLShader( 
		"render model",

		// vertex shader
		"#version 410\n"
		"uniform mat4 matrix;\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec3 v3NormalIn;\n"
		"layout(location = 2) in vec2 v2TexCoordsIn;\n"
		"out vec2 v2TexCoord;\n"
		"void main()\n"
		"{\n"
		"	v2TexCoord = v2TexCoordsIn;\n"
		"	gl_Position = matrix * vec4(position.xyz, 1);\n"
		"}\n",

		//fragment shader
		"#version 410 core\n"
		"uniform sampler2D diffuse;\n"
		"in vec2 v2TexCoord;\n"
		"out vec4 outputColor;\n"
		"void main()\n"
		"{\n"
		"   outputColor = texture( diffuse, v2TexCoord);\n"
		"}\n"

		);
	m_nRenderModelMatrixLocation = glGetUniformLocation( m_unRenderModelProgramID, "matrix" );
	if( m_nRenderModelMatrixLocation == -1 )
	{
		dprintf( "Unable to find matrix uniform in render model shader\n" );
		return false;
	}

	if (!PrepareUnitCellAtomShader (&m_unAtomsProgramID, &m_unUnitCellProgramID, &m_nAtomMatrixLocation, 
			&m_nUnitCellMatrixLocation,  &m_nUnitCellColourLocation))
		return false;


	m_unLensProgramID = CompileGLShader(
		"Distortion",

		// vertex shader
		"#version 410 core\n"
		"layout(location = 0) in vec4 position;\n"
		"layout(location = 1) in vec2 v2UVredIn;\n"
		"layout(location = 2) in vec2 v2UVGreenIn;\n"
		"layout(location = 3) in vec2 v2UVblueIn;\n"
		"noperspective  out vec2 v2UVred;\n"
		"noperspective  out vec2 v2UVgreen;\n"
		"noperspective  out vec2 v2UVblue;\n"
		"void main()\n"
		"{\n"
		"	v2UVred = v2UVredIn;\n"
		"	v2UVgreen = v2UVGreenIn;\n"
		"	v2UVblue = v2UVblueIn;\n"
		"	gl_Position = position;\n"
		"}\n",

		// fragment shader
		"#version 410 core\n"
		"uniform sampler2D mytexture;\n"

		"noperspective  in vec2 v2UVred;\n"
		"noperspective  in vec2 v2UVgreen;\n"
		"noperspective  in vec2 v2UVblue;\n"

		"out vec4 outputColor;\n"

		"void main()\n"
		"{\n"
		"	float fBoundsCheck = ( (dot( vec2( lessThan( v2UVgreen.xy, vec2(0.05, 0.05)) ), vec2(1.0, 1.0))+dot( vec2( greaterThan( v2UVgreen.xy, vec2( 0.95, 0.95)) ), vec2(1.0, 1.0))) );\n"
		"	if( fBoundsCheck > 1.0 )\n"
		"	{ outputColor = vec4( 0, 0, 0, 1.0 ); }\n"
		"	else\n"
		"	{\n"
		"		float red = texture(mytexture, v2UVred).x;\n"
		"		float green = texture(mytexture, v2UVgreen).y;\n"
		"		float blue = texture(mytexture, v2UVblue).z;\n"
		"		outputColor = vec4( red, green, blue, 1.0  ); }\n"
		"}\n"
		);


	return m_unSceneProgramID != 0 
		&& m_unControllerTransformProgramID != 0
		&& m_unRenderModelProgramID != 0
		&& m_unLensProgramID != 0;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupTexturemaps()
{

	//rgh: textures:	[0] White texture for textureless meshes
	//					[1,2] Depth texture for depth peeling ping pong (needs to be initialized after SetupStereoRenderTargets)
	//					[3..ZLAYERS+2] Colour textures for transparency
	//sdl_textures: 6	[0..5] a b c alpha beta gamma text for axis labels
	//std::string sExecutableDirectory = Path_StripFilename(Path_GetExecutablePath());
	//std::string strFullPath = Path_MakeAbsolute("../cube_texture.png", sExecutableDirectory);

	m_iTexture = new GLuint[3+ZLAYERS+1]; // white, depth1, depth2, color[ZLAYERS], atomtexture
	glGenTextures(3+ZLAYERS+1, m_iTexture);

	//white
	unsigned char data2[4] = { 255, 255, 255, 255 }; //white texture for non-textured meshes
	glBindTexture(GL_TEXTURE_2D, m_iTexture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);

	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("opengl error %d, SetupTextureMaps 1\n", e);

	e=atomTexture( m_iTexture[3+ZLAYERS]);

	glBindTexture( GL_TEXTURE_2D, 0 );

	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("opengl error %d, SetupTextureMaps 2\n", e);

	//sdl text http://stackoverflow.com/questions/5289447/using-sdl-ttf-with-opengl/5289823#5289823
	//http://stackoverflow.com/questions/28880562/rendering-text-with-sdl2-and-opengl
	//FIXME TODO
	/*int size=10;
	=TTF_OpenFont("", size);
	axisTextures=new SDL_Texture*[6];

	const wchar_t axis[6]={'a', 'b', 'c', '\u03B1', '\u03B2', '\u03B3'};
	for (int i=0;i<6;i++) {
		axisTextures[i]=TTF_RenderUTF8_Blended();
	}
	*/

	return ( m_iTexture != 0 && e==GL_NO_ERROR);
}

bool CMainApplication::SetupDepthPeeling()
{
	//https://www.opengl.org/wiki/Common_Mistakes
	//Until this is resolved in NVIDIA's drivers, it is advised to make sure that all textures have mipmap levels, and that all glTexParameteri​ 
	//values are properly set up for the format of the texture. For example, integral textures are not complete if the mag and min filters have any LINEAR fields.

	GLenum e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("opengl error %d, start SetupDepthPeeling\n", e);
	GLuint clearColor = 0;
	for (int i = 0; i < 2; i++) {
		glBindTexture(GL_TEXTURE_2D, m_iTexture[1 + i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("opengl error %d, %s SetupDepthPeeling a\n", e, gluErrorString(e));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_nRenderWidth, m_nRenderHeight, 0, 
			GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("opengl error %d, %s SetupDepthPeeling b\n", e, gluErrorString(e));

		glClearTexImage(m_iTexture[1], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, &clearColor);

		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("opengl error %d, SetupDepthPeeling c\n", e);
	}

	for (int i = 0; i < ZLAYERS; i++) {
		glBindTexture(GL_TEXTURE_2D, m_iTexture[3+i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nRenderWidth, m_nRenderHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	}

	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("opengl error %d, %s SetupDepthPeeling textures end\n", e, gluErrorString(e));

	//now create framebuffer
	GLint dfb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dfb);
	glGenFramebuffers(1, &peelingFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, peelingFramebuffer);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error: %d, %s l %d\n", e, gluErrorString(e), __LINE__);

	glBindFramebuffer(GL_FRAMEBUFFER, dfb);



	return (e == GL_NO_ERROR);
}

//-----------------------------------------------------------------------------
// Purpose: Load the scene into OpenGL
//-----------------------------------------------------------------------------
void CMainApplication::SetupScene()
{
	SetupIsosurfaces();
	SetupAtoms();
	delete[] clonedAtoms;
	clonedAtoms=0;
	SetupUnitCell();
}

//-----------------------------------------------------------------------------
// Purpose: Load the atoms into OpenGL
//-----------------------------------------------------------------------------
void CMainApplication::SetupUnitCell()
{
	GLenum e;
	e=::SetupUnitCell(&m_unUnitCellVAO, &m_glUnitCellVertBuffer, &m_glUnitCellIndexBuffer);
	if (e!=GL_NO_ERROR)
		dprintf("opengl error %d, SetupUnitCell, l %d\n", e, __LINE__);
}

void CMainApplication::SetupAtoms()
{
	GLenum e;
	e=::SetupAtoms(&m_unAtomVAO, &m_glAtomVertBuffer);
//	GLuint *vao, *buffer, *index;
//	e=::SetupAtomsNoTess(&vao, &buffer, &index);
	if (e!=GL_NO_ERROR)
		dprintf("opengl error %d, SetupAtoms, l %d\n", e, __LINE__);
}



//-----------------------------------------------------------------------------
// Purpose: Load the isosurfaces into OpenGL
//-----------------------------------------------------------------------------
void CMainApplication::SetupIsosurfaces()
{
	if (!m_pHMD)
		return;
	//rgh: add scene loading here
	vertdataarray = new std::vector<float>*[NUMLODS];
#ifndef INDICESGL32
	vertindicesarray = new std::vector<short>*[NUMLODS];
#else
	vertindicesarray = new std::vector<GLuint>*[NUMLODS];
#endif
	for (int i = 0; i < NUMLODS; i++) {//LODS 100%, 25%, 6%, 2% 
		vertdataarray[i] = new std::vector<float>[NUMPLY];
#ifndef INDICESGL32
		vertindicesarray[i] = new std::vector<short>[NUMPLY];
#else
		vertindicesarray[i] = new std::vector<GLuint>[NUMPLY];
#endif
	}

	Matrix4 matScale;
	matScale.scale(m_fScale, m_fScale, m_fScale);
	Matrix4 matTransform;
	/*matTransform.translate(
		-((float)m_iSceneVolumeWidth * m_fScaleSpacing) / 1.f,
		-((float)m_iSceneVolumeHeight * m_fScaleSpacing) / 1.f,
		-((float)m_iSceneVolumeDepth * m_fScaleSpacing) / 1.f);*/
	matTransform.rotateX(-90);
	Matrix4 mat = matScale * matTransform;

	const char *lods[] = {""};

	m_glSceneVertBuffer = new GLuint*[NUMLODS];
	m_uiVertcount = new unsigned int*[NUMLODS];
	m_unSceneVAO = new unsigned int*[NUMLODS];
	m_unSceneVAOIndices = new unsigned int*[NUMLODS];

	GLenum e;


	glClearColor(0,0,0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(m_unControllerTransformProgramID);
	float loadingmat[] = { 2, 0, 0, 0,
		0, 2, 0, 0,
		0, 0, 1, 0,
		-1, -1, 0, 1 };

	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s\n", e, gluErrorString(e));
	//glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, m_iTexture[1]);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s\n", e, gluErrorString(e));

	glDisable(GL_CULL_FACE);
	float z = 0.0f; 
	float points[] = {//pos [4], color [3]
		0, 0, z, 1, 1,1,1,
		0, 1, z, 1, 1, 1, 1,
		1, 1, z, 1, 1, 1, 1,
		1, 0, z, 1, 1, 1, 1, };

	//http://stackoverflow.com/questions/21767467/glvertexattribpointer-raising-impossible-gl-invalid-operation
	GLuint myvao;
	glGenVertexArrays(1, &myvao);
	glBindVertexArray(myvao);
	GLuint testBuf;
	glGenBuffers(1, &testBuf);
	glBindBuffer(GL_ARRAY_BUFFER, testBuf);
	glBufferData(GL_ARRAY_BUFFER, 4 * 7 * sizeof(GLfloat), points, GL_STATIC_DRAW);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	

	GLuint iData[] = { 0, 1, 2,
		2, 3, 0 };

	GLuint indexBufferID;
	glGenBuffers(1, &indexBufferID);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iData), iData, GL_STATIC_DRAW);


	for (int i = 0; i < NUMLODS; i++) {
		m_glSceneVertBuffer[i] = new GLuint[NUMPLY];
		m_uiVertcount[i] = new unsigned int[NUMPLY];
		m_unSceneVAO[i] = new unsigned int[NUMPLY];
		m_unSceneVAOIndices[i] = new unsigned int[NUMPLY];


		glGenBuffers(NUMPLY, m_glSceneVertBuffer[i]);
		glGenVertexArrays(NUMPLY, m_unSceneVAO[i]);
	
		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("opengl error %d, glGenVertexArrays\n", e);
		// Create and populate the index buffer
		glGenBuffers(NUMPLY, m_unSceneVAOIndices[i]);
		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("opengl error %d, glGenBuffers\n", e);

	}
	GLsizei stride = sizeof(float) * numComponents; //sizeof(VertexDataScene);

	char tmpname[250];
	for (int currentlod = 0; currentlod < NUMLODS; currentlod++) {
		int time = 1;
		for (int p = 0; p < NUMPLY; p++) {
			glBindVertexArray(myvao);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("opengl error %d, l %d\n", e, __LINE__);
			//loadingmat[0]=(float)currentlod/NUMLODS; //xscaling
			loadingmat[0]=2*(float)p/NUMPLY; //yscaling
			glUniformMatrix4fv(m_nControllerMatrixLocation, 1, GL_FALSE, loadingmat);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("opengl error %d, l %d\n", e, __LINE__);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("opengl error %d, l %d\n", e, __LINE__);

			SDL_GL_SwapWindow(m_pWindow);

			//http://stackoverflow.com/questions/9052224/error4error-c3861-snprintf-identifier-not-found
			sprintf(tmpname, "%s%s%d-%s.ply", PATH, lods[currentlod], time, plyfiles[p % ISOS]);

			vertdataarray[currentlod][p].clear();
			vertindicesarray[currentlod][p].clear();

			Matrix4 matFinal;
			matFinal.translate(translations[p%ISOS][0], translations[p%ISOS][1], translations[p%ISOS][2]);
			matFinal = mat*matFinal;
			if (!AddModelToScene(matFinal, vertdataarray[currentlod][p], vertindicesarray[currentlod][p],
				tmpname, false, isocolours[p%ISOS][0]<0, p%ISOS))
			{
				dprintf("Error loading ply file %s\n", tmpname);
				//m_bShowCubes = false;
				//return; 
			}
#ifndef INDICESGL32
			if (vertdataarray[currentlod][p].size() > 65535 * numComponents)
			{
				dprintf("Mesh has more than 64k vertices (%d), unsupported\n", vertdataarray[currentlod][p].size() / numComponents);
				m_bShowCubes = false;
				return;
			}
#endif
			m_uiVertcount[currentlod][p] = vertindicesarray[currentlod][p].size();  //rgh: now pos, normal, color

			glBindVertexArray(m_unSceneVAO[currentlod][p]);

			glBindBuffer(GL_ARRAY_BUFFER, m_glSceneVertBuffer[currentlod][p]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray[currentlod][p].size(), &vertdataarray[currentlod][p][0],
				GL_STATIC_DRAW);

			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);
			int offset = 0;

			//now pos[3], normal[3], color[4]
			//pos
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			if (glGetError() != GL_NO_ERROR)
				dprintf("opengl error attrib pointer 0\n");

			//normal
			offset += 3 * sizeof(GLfloat); //3 floats
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			if (glGetError() != GL_NO_ERROR)
				dprintf("opengl error attrib pointer 1\n");

			//color
			offset += 3 * sizeof(GLfloat); //6 floats
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			if (glGetError() != GL_NO_ERROR)
				dprintf("opengl error attrib pointer 2\n");

			//uv
			/*offset += 4 * sizeof(GLfloat); //10 floats
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

			if (glGetError() != GL_NO_ERROR)
			dprintf("opengl error attrib pointer 3\n");*/

			// populate the index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_unSceneVAOIndices[currentlod][p]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
#ifndef INDICESGL32
				sizeof(uint16_t)
#else
				sizeof(GLuint)
#endif
				* vertindicesarray[currentlod][p].size(),
				&vertindicesarray[currentlod][p][0], GL_STATIC_DRAW);



			glBindVertexArray(0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
			//glDisableVertexAttribArray(3);

			if (glGetError() != GL_NO_ERROR)
				dprintf("opengl error\n");

			//FIXME: after we go to 64 bits, keep the data in ram
			vertdataarray[currentlod][p].clear();
			vertindicesarray[currentlod][p].clear();
			vertdataarray[currentlod][p].resize(0);
			vertindicesarray[currentlod][p].resize(0);


			if (p % ISOS == ISOS - 1)
				time++;

		}
		delete[] vertdataarray[currentlod];
		delete[] vertindicesarray[currentlod];
		vertdataarray[currentlod] = nullptr;
		vertindicesarray[currentlod] = nullptr;

	} // for each lod
	//glEnable(GL_DEPTH_TEST);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glDeleteBuffers(1, &testBuf);
	glDeleteBuffers(1, &indexBufferID);
	glDeleteVertexArrays(1, &myvao);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
	glUseProgram(0);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}


//-----------------------------------------------------------------------------
// Purpose: Load a ply file
//-----------------------------------------------------------------------------


bool CMainApplication::AddModelToScene( Matrix4 mat, std::vector<float> &vertdata, 
#ifndef INDICESGL32
	std::vector<short> & vertindices, 
#else
	std::vector<GLuint> & vertindices,
#endif
	const char * model, bool water, bool colours, int set) 
{

	CurrentVertex=-1;
	CurrentIndex=0;
	 p_ply ply = ply_open(model, NULL, 0, NULL);
	 //p_ply ply = ply_open("Y:\\v2t\\media\\visual_designs\\logos_3d\\v2c\\v2cSchrift.ply", NULL, 0, NULL);
        if (!ply) {
            dprintf("ply returned null: file %s\n", model);
            return false;
        }
        if (!ply_read_header(ply)) {
            dprintf("ply: bad header in ply: file %s\n", model);
            return false;
        }

        int nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, this, 0);
        ply_set_read_cb(ply, "vertex", "y", vertex_cb, this, 1);
        ply_set_read_cb(ply, "vertex", "z", vertex_cb, this, 2);

        ply_set_read_cb(ply, "vertex", "nx", vertex_cb, this, 3);
        ply_set_read_cb(ply, "vertex", "ny", vertex_cb, this, 4);
        ply_set_read_cb(ply, "vertex", "nz", vertex_cb, this, 5);

		ply_set_read_cb(ply, "vertex", "red", vertex_cb, this, 6);
        ply_set_read_cb(ply, "vertex", "green", vertex_cb, this, 7);
        ply_set_read_cb(ply, "vertex", "blue", vertex_cb, this, 8);
        ply_set_read_cb(ply, "vertex", "alpha", vertex_cb, this, 9);

		//ply_set_read_cb(ply, "vertex", "texture_u", vertex_cb, this, 10);
		//ply_set_read_cb(ply, "vertex", "texture_v", vertex_cb, this, 11);

		CubeVertices = new float[nvertices*numComponents]; //xyz, nx, ny, nz, uv, rgba

        int ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);

        CubeIndices=new int[3*ntriangles];

        //dprintf("PLY %s: v=%ld, t=%ld\n", model, nvertices, ntriangles);
        if (!ply_read(ply)) {
            dprintf("Problem in ply_read");
            return false;
        }
        ply_close(ply);

		for (int i = 0; i < 3* ntriangles; i++) {
			vertindices.push_back(CubeIndices[i]);
		}

	
		for (int i = 0; i < nvertices; i++) {
			//pos
			Vector4 V1 = Vector4(CubeVertices[i * numComponents + 0],
				CubeVertices[i * numComponents + 1],
				CubeVertices[i * numComponents + 2], 1);
			Vector4 V = mat * V1;
			vertdata.push_back(V.x);
			vertdata.push_back(V.y);
			vertdata.push_back(V.z);
			//normals (FIXME should transform with inverse transform, but we think the matrix has uniform scaling and inv transpose = m)
			//rgh beware: normals are (nx, ny, nz, 0) to avoid being translated !!!
			V1 = Vector4(CubeVertices[i * numComponents + 3],
				CubeVertices[i * numComponents + 4],
				CubeVertices[i * numComponents + 5], 0);
			V = (mat * V1).normalize();
			vertdata.push_back(V.x);
			vertdata.push_back(V.y);
			vertdata.push_back(V.z);
			//colors (untransformed)
			if (!colours) {
				for (int j = 0; j < 4; j++)
					vertdata.push_back(isocolours[set][j]);
			} else {
				for (int j = 0; j < 4; j++)
					vertdata.push_back(CubeVertices[i * numComponents + 6 + j]);
			}
		}
		delete[] CubeVertices;
		delete[] CubeIndices;
		return true;
}



//-----------------------------------------------------------------------------
// Purpose: Draw all of the controllers as X/Y/Z lines
//-----------------------------------------------------------------------------
void CMainApplication::DrawControllers()
{
	//rgh: the format for the vertarray has changed, now xyz, nxnynz, rgba, uv. Cancel this function until needed. Needs to fix fill of points down
	return;

	// don't draw controllers if somebody else has input focus
	if( m_pHMD->IsInputFocusCapturedByAnotherProcess() )
		return;

	std::vector<float> vertdataarray;

	m_uiControllerVertcount = 0;
	m_iTrackedControllerCount = 0;

	for ( vr::TrackedDeviceIndex_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; ++unTrackedDevice )
	{
		if ( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		if( m_pHMD->GetTrackedDeviceClass( unTrackedDevice ) != vr::TrackedDeviceClass_Controller )
			continue;

		m_iTrackedControllerCount += 1;

		if( !m_rTrackedDevicePose[ unTrackedDevice ].bPoseIsValid )
			continue;

		const Matrix4 & mat = m_rmat4DevicePose[unTrackedDevice];

		Vector4 center = mat * Vector4( 0, 0, 0, 1 );

		for ( int i = 0; i < 3; ++i )
		{
			Vector3 color( 0, 0, 0 );
			Vector4 point( 0, 0, 0, 1 );
			point[i] += 0.05f;  // offset in X, Y, Z
			color[i] = 1.0;  // R, G, B
			point = mat * point;
			vertdataarray.push_back( center.x );
			vertdataarray.push_back( center.y );
			vertdataarray.push_back( center.z );

			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			vertdataarray.push_back( point.x );
			vertdataarray.push_back( point.y );
			vertdataarray.push_back( point.z );
		
			vertdataarray.push_back( color.x );
			vertdataarray.push_back( color.y );
			vertdataarray.push_back( color.z );
		
			m_uiControllerVertcount += 2;
		}

		Vector4 start = mat * Vector4( 0, 0, -0.02f, 1 );
		Vector4 end = mat * Vector4( 0, 0, -39.f, 1 );
		Vector3 color( .92f, .92f, .71f );

		vertdataarray.push_back( start.x );vertdataarray.push_back( start.y );vertdataarray.push_back( start.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );

		vertdataarray.push_back( end.x );vertdataarray.push_back( end.y );vertdataarray.push_back( end.z );
		vertdataarray.push_back( color.x );vertdataarray.push_back( color.y );vertdataarray.push_back( color.z );
		m_uiControllerVertcount += 2;
	}

	// Setup the VAO the first time through.
	if ( m_unControllerVAO == 0 )
	{
		glGenVertexArrays( 1, &m_unControllerVAO );
		glBindVertexArray( m_unControllerVAO );

		glGenBuffers( 1, &m_glControllerVertBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

		GLuint stride = 2 * 3 * sizeof( float );
		GLuint offset = 0;

		glEnableVertexAttribArray( 0 );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		offset += sizeof( Vector3 );
		glEnableVertexAttribArray( 1 );
		glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, stride, (const void *)offset);

		glBindVertexArray( 0 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, m_glControllerVertBuffer );

	// set vertex data if we have some
	if( vertdataarray.size() > 0 )
	{
		//$ TODO: Use glBufferSubData for this...
		glBufferData( GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STREAM_DRAW );
	}
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupCameras()
{
	m_mat4ProjectionLeft = GetHMDMatrixProjectionEye( vr::Eye_Left );
	m_mat4ProjectionRight = GetHMDMatrixProjectionEye( vr::Eye_Right );
	m_mat4eyePosLeft = GetHMDMatrixPoseEye( vr::Eye_Left );
	m_mat4eyePosRight = GetHMDMatrixPoseEye( vr::Eye_Right );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::CreateFrameBuffer( int nWidth, int nHeight, FramebufferDesc &framebufferDesc )
{
	glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

	glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
	glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
	glRenderbufferStorage/*Multisample*/(GL_RENDERBUFFER,/* 1, */GL_DEPTH_COMPONENT32, nWidth, nHeight );
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,	framebufferDesc.m_nDepthBufferId );

	glGenTextures(1, &framebufferDesc.m_nRenderTextureId );
	glBindTexture(GL_TEXTURE_2D/*_MULTISAMPLE*/, framebufferDesc.m_nRenderTextureId );
	//glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 1, GL_RGBA8, nWidth, nHeight, true);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D/*_MULTISAMPLE*/, framebufferDesc.m_nRenderTextureId, 0);

	glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId );
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

	glGenTextures(1, &framebufferDesc.m_nResolveTextureId );
	glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

	// check FBO status
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		return false;
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CMainApplication::SetupStereoRenderTargets()
{
	if ( !m_pHMD )
		return false;

	m_pHMD->GetRecommendedRenderTargetSize( &m_nRenderWidth, &m_nRenderHeight );

	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, leftEyeDesc );
	CreateFrameBuffer( m_nRenderWidth, m_nRenderHeight, rightEyeDesc );
	
	pixels = new char [m_nRenderWidth*m_nRenderHeight*3];

	return true;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::SetupDistortion()
{
	if ( !m_pHMD )
		return;

	GLushort m_iLensGridSegmentCountH = 43;
	GLushort m_iLensGridSegmentCountV = 43;

	float w = (float)( 1.0/float(m_iLensGridSegmentCountH-1));
	float h = (float)( 1.0/float(m_iLensGridSegmentCountV-1));

	float u, v = 0;

	std::vector<VertexDataLens> vVerts(0);
	VertexDataLens vert;

	//left eye distortion verts
	float Xoffset = -1;
	for( int y=0; y<m_iLensGridSegmentCountV; y++ )
	{
		for( int x=0; x<m_iLensGridSegmentCountH; x++ )
		{
			u = x*w; v = 1-y*h;
			vert.position = Vector2( Xoffset+u, -1+2*y*h );

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion(vr::Eye_Left, u, v);

			vert.texCoordRed = Vector2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen =  Vector2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = Vector2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back( vert );
		}
	}

	//right eye distortion verts
	Xoffset = 0;
	for( int y=0; y<m_iLensGridSegmentCountV; y++ )
	{
		for( int x=0; x<m_iLensGridSegmentCountH; x++ )
		{
			u = x*w; v = 1-y*h;
			vert.position = Vector2( Xoffset+u, -1+2*y*h );

			vr::DistortionCoordinates_t dc0 = m_pHMD->ComputeDistortion( vr::Eye_Right, u, v );

			vert.texCoordRed = Vector2(dc0.rfRed[0], 1 - dc0.rfRed[1]);
			vert.texCoordGreen = Vector2(dc0.rfGreen[0], 1 - dc0.rfGreen[1]);
			vert.texCoordBlue = Vector2(dc0.rfBlue[0], 1 - dc0.rfBlue[1]);

			vVerts.push_back( vert );
		}
	}

	std::vector<GLushort> vIndices;
	GLushort a,b,c,d;

	GLushort offset = 0;
	for( GLushort y=0; y<m_iLensGridSegmentCountV-1; y++ )
	{
		for( GLushort x=0; x<m_iLensGridSegmentCountH-1; x++ )
		{
			a = m_iLensGridSegmentCountH*y+x +offset;
			b = m_iLensGridSegmentCountH*y+x+1 +offset;
			c = (y+1)*m_iLensGridSegmentCountH+x+1 +offset;
			d = (y+1)*m_iLensGridSegmentCountH+x +offset;
			vIndices.push_back( a );
			vIndices.push_back( b );
			vIndices.push_back( c );

			vIndices.push_back( a );
			vIndices.push_back( c );
			vIndices.push_back( d );
		}
	}

	offset = (m_iLensGridSegmentCountH)*(m_iLensGridSegmentCountV);
	for( GLushort y=0; y<m_iLensGridSegmentCountV-1; y++ )
	{
		for( GLushort x=0; x<m_iLensGridSegmentCountH-1; x++ )
		{
			a = m_iLensGridSegmentCountH*y+x +offset;
			b = m_iLensGridSegmentCountH*y+x+1 +offset;
			c = (y+1)*m_iLensGridSegmentCountH+x+1 +offset;
			d = (y+1)*m_iLensGridSegmentCountH+x +offset;
			vIndices.push_back( a );
			vIndices.push_back( b );
			vIndices.push_back( c );

			vIndices.push_back( a );
			vIndices.push_back( c );
			vIndices.push_back( d );
		}
	}
	m_uiIndexSize = vIndices.size();

	glGenVertexArrays( 1, &m_unLensVAO );
	glBindVertexArray( m_unLensVAO );

	glGenBuffers( 1, &m_glIDVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glIDVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, vVerts.size()*sizeof(VertexDataLens), &vVerts[0], GL_STATIC_DRAW );

	glGenBuffers( 1, &m_glIDIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glIDIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, vIndices.size()*sizeof(GLushort), &vIndices[0], GL_STATIC_DRAW );

	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof( VertexDataLens, position ) );

	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof( VertexDataLens, texCoordRed ) );

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof( VertexDataLens, texCoordGreen ) );

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexDataLens), (void *)offsetof( VertexDataLens, texCoordBlue ) );

	

	glBindVertexArray( 0 );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderStereoTargets()
{
	glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1);
	//glEnable( GL_MULTISAMPLE );

	// Left Eye
	glBindFramebuffer( GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Left );

	//rgh: save to disk
	//this will kill the performance
	char name[100];

	if (savetodisk) {
		sprintf(name, "%sL%05d.bmp", SCREENSHOT, framecounter);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glReadPixels(0, 0, m_nRenderWidth, m_nRenderHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		//little endian machine, R and B are flipped
		SDL_Surface *s = SDL_CreateRGBSurfaceFrom(pixels, m_nRenderWidth, m_nRenderHeight, 24, 3 * m_nRenderWidth, 0xff, 0xff00, 0xff0000, 0);
		SDL_SaveBMP(s, name);
	}

 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	
	glDisable( GL_MULTISAMPLE );
	 	
 	glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId );

    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);	

	//glEnable( GL_MULTISAMPLE );

	// Right Eye
	glBindFramebuffer( GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
 	glViewport(0, 0, m_nRenderWidth, m_nRenderHeight );
 	RenderScene( vr::Eye_Right );
 	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
 	
	glDisable( GL_MULTISAMPLE );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId );
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId );
	
    glBlitFramebuffer( 0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight, 
		GL_COLOR_BUFFER_BIT,
 		GL_LINEAR  );

 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0 );

	if (savetodisk) {
		framecounter++;
	}
}

void CMainApplication::PaintGrid(const vr::Hmd_Eye &nEye, int iso) {
	int c=0;
			
	Matrix4 trans;
	Vector3 iPos = Vector3(0, 0, 0);
		
	trans.translate(iPos).translate(UserPosition);
	Matrix4 transform = GetCurrentViewProjectionMatrix(nEye)*trans;
	glUniformMatrix4fv(m_nSceneMatrixLocation, 1, GL_FALSE, transform.get());

	const int currentlod = 0;
	int actualset = currentset;
	glBindVertexArray(m_unSceneVAO[currentlod][ISOS * actualset + iso]);
	glDrawElements(GL_TRIANGLES, m_uiVertcount[currentlod][ISOS * actualset + iso], 
#ifndef INDICESGL32				
		GL_UNSIGNED_SHORT, 
#else
		GL_UNSIGNED_INT,
#endif
				
		0);

	int e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after Paintgrid iso=%d: %d, %s\n", iso, e, gluErrorString(e));

}

void CMainApplication::RenderUnitCell(const vr::Hmd_Eye &nEye)
{
	if (!has_abc)
		return;
	int e;
	glUseProgram(m_unUnitCellProgramID);
	glBindVertexArray(m_unUnitCellVAO);
	if (m_unUnitCellVAO==0)
		dprintf ("Error, Unit Cell VAO not loaded");
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after glBindVertexArray RenderUnitCell: %d, %s\n", e, gluErrorString(e));

	Matrix4 trans;
	int p[3];
	for (p[0]=0;p[0]<repetitions[0];(p[0])++)
		for (p[1]=0;p[1]<repetitions[1];(p[1])++)
			for (p[2]=0;p[2]<repetitions[2];(p[2])++)
				{
					float delta[3];
					::GetDisplacement(p, delta);
					Vector3 iPos(delta[0], delta[1], delta[2]);
					trans.identity();
		
					trans.translate(iPos).rotateX(-90).translate(UserPosition);
					Matrix4 transform = GetCurrentViewProjectionMatrix(nEye)*trans;
					glUniformMatrix4fv(m_nUnitCellMatrixLocation, 1, GL_FALSE, transform.get());
					if ((e = glGetError()) != GL_NO_ERROR)
						dprintf("Gl error after glUniform4fv 1 RenderUnitCell: %d, %s\n", e, gluErrorString(e));
					float color[4]={1,1,1,1};
					glUniform4fv(m_nUnitCellColourLocation, 1, color);
					if ((e = glGetError()) != GL_NO_ERROR)
						dprintf("Gl error after glUniform4fv 2 RenderUnitCell: %d, %s\n", e, gluErrorString(e));
					glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
					if ((e = glGetError()) != GL_NO_ERROR)
						dprintf("Gl error after RenderUnitCell: %d, %s\n", e, gluErrorString(e));
				}
}

/*
Vector3 CMainApplication::GetDisplacement(int p[3])
{
Vector3 delta[3];
for (int ss=0;ss<3;ss++)
	delta[ss]=static_cast<float>(p[ss])*Vector3(abc[ss][0], abc[ss][1], abc[ss][2]);
Vector3 f=delta[0]+delta[1]+delta[2];
return (f);
}*/

void CMainApplication::RenderAtomsUnitCell(const vr::Hmd_Eye &nEye, int p[3])
{
	int e;
Matrix4 trans;
float delta[3];
::GetDisplacement(p, delta);
Vector3 iPos(delta[0], delta[1], delta[2]);
glUseProgram(m_unAtomsProgramID);

float levelso[4] = { TESSSUB, TESSSUB, TESSSUB, TESSSUB };
float levelsi[2] = { TESSSUB, TESSSUB};

glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,levelso);
glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,levelsi);
glPatchParameteri(GL_PATCH_VERTICES, 1);
glBindVertexArray(m_unAtomVAO[0]);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
glBindBuffer(GL_ARRAY_BUFFER, m_glAtomVertBuffer[0]);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(0));
glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void *)(3 * sizeof(float)));

trans.translate(iPos).rotateX(-90).translate(UserPosition);
Matrix4 transform = GetCurrentViewProjectionMatrix(nEye)*trans;
Matrix4 mv=GetCurrentViewMatrix(nEye)*trans;
glUniformMatrix4fv(m_nAtomMatrixLocation, 1, GL_FALSE, transform.get());
//glUniformMatrix4fv(m_nAtomMVLocation, 1, GL_FALSE, mv.get());
if ((e = glGetError()) != GL_NO_ERROR)
	dprintf("Gl error 4 timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));
if (currentset==0)
	glDrawArrays(GL_PATCHES, 0, numAtoms[0]);
else
	glDrawArrays(GL_PATCHES, numAtoms[currentset-1], numAtoms[currentset]-numAtoms[currentset-1]);
	
if ((e = glGetError()) != GL_NO_ERROR)
	dprintf("Gl error after RenderAtoms timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));

//now cloned atoms
if (numClonedAtoms!=0 && currentset==0) {
	glBindVertexArray(m_unAtomVAO[1]);
	glDrawArrays(GL_PATCHES, 0, numClonedAtoms);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after Render cloned Atom timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));
}

//now trajectories
if (!showTrajectories)
	return;

glUseProgram(m_unUnitCellProgramID);
glUniformMatrix4fv(m_nUnitCellMatrixLocation, 1, GL_FALSE, transform.get());
float color[4]={1,0,0,1};
glUniform4fv(m_nUnitCellColourLocation, 1, color);
if ((e = glGetError()) != GL_NO_ERROR)
	dprintf("Gl error after glUniform4fv 2 RenderUnitCell: %d, %s\n", e, gluErrorString(e));
glEnableVertexAttribArray(0);
glDisableVertexAttribArray(1);

for (int i=0;i<atomtrajectories.size();i++) {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float)*numAtoms[0], (const void *)(0+4*sizeof(float)*atomtrajectories[i]));
	for (int j=1;j<atomtrajectoryrestarts[i].size();j++) {
		int orig=atomtrajectoryrestarts[i][j-1];
		int count=atomtrajectoryrestarts[i][j]-atomtrajectoryrestarts[i][j-1];
		glDrawArrays(GL_LINE_STRIP, orig, count);
	}
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error after Render Atom trajectories timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));
}
}

void CMainApplication::RenderAtoms(const vr::Hmd_Eye &nEye)
{
	//glDisable(GL_DEPTH_TEST);
	//glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	int e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error before RenderAtoms timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));




	
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error 0 timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));
	glBindTexture(GL_TEXTURE_2D, m_iTexture[3+ZLAYERS]);
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error 2 timestep =%d: %d, %s\n", currentset, e, gluErrorString(e));

	int p[3];
	for (p[0]=0;p[0]<repetitions[0];(p[0])++)
		for (p[1]=0;p[1]<repetitions[1];(p[1])++)
			for (p[2]=0;p[2]<repetitions[2];(p[2])++)
				RenderAtomsUnitCell(nEye, p);
	
	glUseProgram(0);
}

void CMainApplication::CleanDepthTexture ()
{
int e;
GLuint clearColor = 0;
glBindTexture(GL_TEXTURE_2D, m_iTexture[1]);
if ((e = glGetError()) != GL_NO_ERROR)
	dprintf("Gl error after bindtexture: %d, %s\n", e, gluErrorString(e));
glClearTexImage(m_iTexture[1], 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, &clearColor);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderScene(vr::Hmd_Eye nEye)
{
	int e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error previous to renderscene: %d, %s\n", e, gluErrorString(e));

	glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1);
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (ISOS==0) {
		RenderAtoms(nEye);
		RenderUnitCell(nEye);
		return;
	}

	if (m_bShowCubes)
	{
		glDisable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

		//iso loop needs to go first; otherwise rendering problems appear in the borders, as the atom is rendered after half its 
		//surroundings


		//isosurface

		if (currentiso == ISOS) {
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
			//do depth peeling
			CleanDepthTexture();
			GLint dfb;
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dfb);

			for (int zl = 0; zl < ZLAYERS; zl++) {
				glUseProgram(m_unSceneProgramID);

				if ((e = glGetError()) != GL_NO_ERROR)
					dprintf("Gl error after useprogram: %d, %s\n", e, gluErrorString(e));


				glBindFramebuffer(GL_FRAMEBUFFER, peelingFramebuffer);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iTexture[3 + zl], 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_iTexture[2-zl%2], 0);
				glBindTexture(GL_TEXTURE_2D, m_iTexture[1+zl%2]);
				glClearColor(BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1);
				glDepthMask(GL_TRUE);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				for (int i = ISOS - 1; i >= 0; i--) {
					PaintGrid(nEye, i);
				} //for all isos in descending order
				if ((e = glGetError()) != GL_NO_ERROR)
					dprintf("Gl error after paintgrid: %d, %s\n", e, gluErrorString(e));
				if (numAtoms!=0) {
					RenderAtoms(nEye);
					RenderUnitCell(nEye);
				}
				RenderAllTrackedRenderModels(nEye);
			} // for zl

			glBindFramebuffer(GL_FRAMEBUFFER, dfb);
			//now blend all together, back to front
			glDisable(GL_DEPTH_TEST);

			glUseProgram(m_unRenderModelProgramID);
			float mat[] = { 1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1 };

			glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, mat);

			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s\n", e, gluErrorString(e));
			glBindTexture(GL_TEXTURE_2D, m_iTexture[1]);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s\n", e, gluErrorString(e));

			glDisable(GL_CULL_FACE);
			float z = 0.0f; //(m_fNearClip + m_fFarClip) / 2.0f;
			float points[] = {
				-1, -1, z, 1, 0, 0, -1, 0, 0,
				-1, 1, z, 1, 0, 0, -1, 0, 1,
				1, 1, z, 1, 0, 0, -1, 1, 1,
				1, -1, z, 1, 0, 0, -1, 1, 0, };

			GLuint myvao;
			glGenVertexArrays(1, &myvao);
			glBindVertexArray(myvao);
			GLuint testBuf;
			glGenBuffers(1, &testBuf);
			glBindBuffer(GL_ARRAY_BUFFER, testBuf);
			glBufferData(GL_ARRAY_BUFFER, 4 * 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);

			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glEnableVertexAttribArray(2);
			glDisableVertexAttribArray(3);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__); //<- fails ?!
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(4 * sizeof(float)));
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(7 * sizeof(float)));
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);

			GLuint iData[] = { 0, 1, 2,
				2, 3, 0 };

			GLuint indexBufferID;
			glGenBuffers(1, &indexBufferID);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(iData), iData, GL_STATIC_DRAW);

			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			for (int zl = ZLAYERS - 1; zl >= 0; zl--) {
				glBindTexture(GL_TEXTURE_2D, m_iTexture[3+zl]);
				glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			}

			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glDeleteBuffers(1, &testBuf);
			glDeleteBuffers(1, &indexBufferID);
			glDeleteVertexArrays(1, &myvao);
			if ((e = glGetError()) != GL_NO_ERROR)
				dprintf("Gl error after zlayer: %d, %s, l %d\n", e, gluErrorString(e), __LINE__);
			glUseProgram(0);
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
		} // if currentiso
		else {
			if (numAtoms!=0) {
				RenderAtoms(nEye);
				RenderUnitCell(nEye);
			}
			glEnable(GL_DEPTH_TEST);
			glDisable(GL_BLEND);
			CleanDepthTexture();
			glUseProgram(m_unSceneProgramID);
			PaintGrid(nEye, currentiso);
			RenderAllTrackedRenderModels(nEye);
		} //else currentiso =isos


		if ((e = glGetError()) != GL_NO_ERROR)
			dprintf("Gl error after rendering: %d, %s\n", e, gluErrorString(e));
	} //show cubes 
}

void CMainApplication::RenderAllTrackedRenderModels(vr::Hmd_Eye nEye)
{
	// ----- Render Model rendering -----
	glUseProgram(m_unRenderModelProgramID);

	for (uint32_t unTrackedDevice = 0; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++)
	{
		if (!m_rTrackedDeviceToRenderModel[unTrackedDevice] || !m_rbShowTrackedDevice[unTrackedDevice])
			continue;

		const vr::TrackedDevicePose_t & pose = m_rTrackedDevicePose[unTrackedDevice];

		const Matrix4 & matDeviceToTracking = m_rmat4DevicePose[unTrackedDevice];
		Matrix4 matMVP = GetCurrentViewProjectionMatrix(nEye) * matDeviceToTracking;
		glUniformMatrix4fv(m_nRenderModelMatrixLocation, 1, GL_FALSE, matMVP.get());

		m_rTrackedDeviceToRenderModel[unTrackedDevice]->Draw();
	}
	GLuint e;
	if ((e = glGetError()) != GL_NO_ERROR)
		dprintf("Gl error marker 2: %d, %s\n", e, gluErrorString(e));

	//glUseProgram(0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::RenderDistortion()
{
	glDisable(GL_DEPTH_TEST);
	glViewport( 0, 0, m_nWindowWidth, m_nWindowHeight );

	glBindVertexArray( m_unLensVAO );
	glUseProgram( m_unLensProgramID );

	//render left lens (first half of index array )
	glBindTexture(GL_TEXTURE_2D, leftEyeDesc.m_nResolveTextureId );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, 0 );

	//render right lens (second half of index array )
	glBindTexture(GL_TEXTURE_2D, rightEyeDesc.m_nResolveTextureId  );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glDrawElements( GL_TRIANGLES, m_uiIndexSize/2, GL_UNSIGNED_SHORT, (const void *)(m_uiIndexSize) );

	glBindVertexArray( 0 );
	glUseProgram( 0 );
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixProjectionEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix( nEye, m_fNearClip, m_fFarClip, vr::API_OpenGL);

	return Matrix4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1], 
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2], 
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetHMDMatrixPoseEye( vr::Hmd_Eye nEye )
{
	if ( !m_pHMD )
		return Matrix4();

	vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform( nEye );
	Matrix4 matrixObj(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0, 
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

	return matrixObj.invert();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewMatrix( vr::Hmd_Eye nEye )
{
	Matrix4 matMV;
	if( nEye == vr::Eye_Left )
	{
		matMV = m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMV = m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMV;
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::GetCurrentViewProjectionMatrix( vr::Hmd_Eye nEye )
{
	Matrix4 matMVP;
	if( nEye == vr::Eye_Left )
	{
		matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
	}
	else if( nEye == vr::Eye_Right )
	{
		matMVP = m_mat4ProjectionRight * m_mat4eyePosRight *  m_mat4HMDPose;
	}

	return matMVP;
}


//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CMainApplication::UpdateHMDMatrixPose()
{
	if ( !m_pHMD )
		return;

	vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0 );

	m_iValidPoseCount = 0;
	m_strPoseClasses = "";
	for ( int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice )
	{
		if ( m_rTrackedDevicePose[nDevice].bPoseIsValid )
		{
			m_iValidPoseCount++;
			m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4( m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking );
			if (m_rDevClassChar[nDevice]==0)
			{
				switch (m_pHMD->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_Other:             m_rDevClassChar[nDevice] = 'O'; break;
				case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
				default:                                       m_rDevClassChar[nDevice] = '?'; break;
				}
			}
			m_strPoseClasses += m_rDevClassChar[nDevice];
		}
	}

	if ( m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid )
	{
		m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd].invert();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Finds a render model we've already loaded or loads a new one
//-----------------------------------------------------------------------------
CGLRenderModel *CMainApplication::FindOrLoadRenderModel( const char *pchRenderModelName )
{
	CGLRenderModel *pRenderModel = NULL;
	for( std::vector< CGLRenderModel * >::iterator i = m_vecRenderModels.begin(); i != m_vecRenderModels.end(); i++ )
	{
		if( !stricmp( (*i)->GetName().c_str(), pchRenderModelName ) )
		{
			pRenderModel = *i;
			break;
		}
	}

	//http://steamcommunity.com/app/358720/discussions/0/357284131801141236/
	// load the model if we didn't find one
	if (!pRenderModel)
	{
		// Mesh laden
		vr::RenderModel_t *pModel = NULL;

		while (vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel) == vr::VRRenderModelError_Loading) {
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
			Sleep(50);
		}

		if (vr::VRRenderModels()->LoadRenderModel_Async(pchRenderModelName, &pModel) || pModel == NULL)
		{
			dprintf(" Unable to load render model %s\n", pchRenderModelName);
			return NULL; // move on to the next tracked device
		}

		vr::RenderModel_TextureMap_t *pTexture = NULL;

		while (vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture) == vr::VRRenderModelError_Loading) {
			//std::this_thread::sleep_for(std::chrono::milliseconds(50));
			Sleep(50);
		}

		if (vr::VRRenderModels()->LoadTexture_Async(pModel->diffuseTextureId, &pTexture) || pTexture == NULL)
		{
			dprintf("Unable to load render texture id:%d for render model %s\n", pModel->diffuseTextureId, pchRenderModelName);
			vr::VRRenderModels()->FreeRenderModel(pModel);
			return NULL; // move on to the next tracked device
		}

		pRenderModel = new CGLRenderModel( pchRenderModelName );
		if ( !pRenderModel->BInit( *pModel, *pTexture ) )
		{
			dprintf( "Unable to create GL model from render model %s\n", pchRenderModelName );
			delete pRenderModel;
			pRenderModel = NULL;
		}
		else
		{
			m_vecRenderModels.push_back( pRenderModel );
		}
		vr::VRRenderModels()->FreeRenderModel( pModel );
		vr::VRRenderModels()->FreeTexture( pTexture );
	}
	return pRenderModel;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL a Render Model for a single tracked device
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModelForTrackedDevice( vr::TrackedDeviceIndex_t unTrackedDeviceIndex )
{
	if( unTrackedDeviceIndex >= vr::k_unMaxTrackedDeviceCount )
		return;

	// try to find a model we've already set up
	std::string sRenderModelName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_RenderModelName_String );
	CGLRenderModel *pRenderModel = FindOrLoadRenderModel( sRenderModelName.c_str() );
	if( !pRenderModel )
	{
		std::string sTrackingSystemName = GetTrackedDeviceString( m_pHMD, unTrackedDeviceIndex, vr::Prop_TrackingSystemName_String );
		dprintf( "Unable to load render model for tracked device %d (%s.%s)\n", unTrackedDeviceIndex, sTrackingSystemName.c_str(), sRenderModelName.c_str() );
	}
	else
	{
		m_rTrackedDeviceToRenderModel[ unTrackedDeviceIndex ] = pRenderModel;
		m_rbShowTrackedDevice[ unTrackedDeviceIndex ] = true;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
void CMainApplication::SetupRenderModels()
{
	memset( m_rTrackedDeviceToRenderModel, 0, sizeof( m_rTrackedDeviceToRenderModel ) );

	if( !m_pHMD )
		return;

	for( uint32_t unTrackedDevice = vr::k_unTrackedDeviceIndex_Hmd + 1; unTrackedDevice < vr::k_unMaxTrackedDeviceCount; unTrackedDevice++ )
	{
		if( !m_pHMD->IsTrackedDeviceConnected( unTrackedDevice ) )
			continue;

		SetupRenderModelForTrackedDevice( unTrackedDevice );
	}

}


//-----------------------------------------------------------------------------
// Purpose: Converts a SteamVR matrix to our local matrix class
//-----------------------------------------------------------------------------
Matrix4 CMainApplication::ConvertSteamVRMatrixToMatrix4( const vr::HmdMatrix34_t &matPose )
{
	Matrix4 matrixObj(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
		);
	return matrixObj;
}


//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
CGLRenderModel::CGLRenderModel( const std::string & sRenderModelName )
	: m_sModelName( sRenderModelName )
{
	m_glIndexBuffer = 0;
	m_glVertArray = 0;
	m_glVertBuffer = 0;
	m_glTexture = 0;
}


CGLRenderModel::~CGLRenderModel()
{
	Cleanup();
}


//-----------------------------------------------------------------------------
// Purpose: Allocates and populates the GL resources for a render model
//-----------------------------------------------------------------------------
bool CGLRenderModel::BInit( const vr::RenderModel_t & vrModel, const vr::RenderModel_TextureMap_t & vrDiffuseTexture )
{
	// create and bind a VAO to hold state for this model
	glGenVertexArrays( 1, &m_glVertArray );
	glBindVertexArray( m_glVertArray );

	// Populate a vertex buffer
	glGenBuffers( 1, &m_glVertBuffer );
	glBindBuffer( GL_ARRAY_BUFFER, m_glVertBuffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof( vr::RenderModel_Vertex_t ) * vrModel.unVertexCount, vrModel.rVertexData, GL_STATIC_DRAW );

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray( 0 );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vPosition ) );
	glEnableVertexAttribArray( 1 );
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, vNormal ) );
	glEnableVertexAttribArray( 2 );
	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( vr::RenderModel_Vertex_t ), (void *)offsetof( vr::RenderModel_Vertex_t, rfTextureCoord ) );

	// Create and populate the index buffer
	glGenBuffers( 1, &m_glIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_glIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( uint16_t ) * vrModel.unTriangleCount * 3, vrModel.rIndexData, GL_STATIC_DRAW );

	glBindVertexArray( 0 );

	// create and populate the texture
	glGenTextures(1, &m_glTexture );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, vrDiffuseTexture.unWidth, vrDiffuseTexture.unHeight,
		0, GL_RGBA, GL_UNSIGNED_BYTE, vrDiffuseTexture.rubTextureMapData );

	// If this renders black ask McJohn what's wrong.
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	GLfloat fLargest;
	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest );

	glBindTexture( GL_TEXTURE_2D, 0 );

	m_unVertexCount = vrModel.unTriangleCount * 3;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Frees the GL resources for a render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Cleanup()
{
	if( m_glVertBuffer )
	{
		glDeleteBuffers(1, &m_glIndexBuffer);
		glDeleteBuffers(1, &m_glVertArray);
		glDeleteBuffers(1, &m_glVertBuffer);
		m_glIndexBuffer = 0;
		m_glVertArray = 0;
		m_glVertBuffer = 0;
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draws the render model
//-----------------------------------------------------------------------------
void CGLRenderModel::Draw()
{
	glBindVertexArray( m_glVertArray );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_glTexture );

	glDrawElements( GL_TRIANGLES, m_unVertexCount, GL_UNSIGNED_SHORT, 0 );

	glBindVertexArray( 0 );
}








void cleanConfig()
{
	for (int i = 0; i < ISOS; i++) {
		delete[] isocolours[i];
		delete[] translations[i];
	}
	delete[] isocolours;
	delete[] translations;
	if (plyfiles) {
		for (int i=0;i<ISOS;i++)
			free ((void*)(plyfiles[i])); //strdup
		delete[] plyfiles;
	}
	
	free((void*)PATH);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
char * MainErrors [] = {
	"No error, successful exit",
	"Exactly one parameter should be provided",
	"Out of memory starting application",
	"Could not init application"
};
int main(int argc, char *argv[])
{
	//http://stackoverflow.com/questions/4991967/how-does-wsastartup-function-initiates-use-of-the-winsock-dll
	WSADATA wsaData;
    if(WSAStartup(0x202, &wsaData) != 0)
		return -10;
	
	if (argc != 2) {
		fprintf(stderr, "Use: %s <config file>\n", argv[0]);
		MessageBoxA(0, MainErrors[1], nullptr, 0);
		return -1;
	}

	{
	//change cwd so that relative paths work
	std::string s(argv[1]);
	SetCurrentDirectoryA(s.substr(0, s.find_last_of("\\/")).c_str());
	}

	int r;
	if ((r=loadConfigFile(argv[1]))<0) {
		if (-100<r)
			MessageBoxA(0, loadConfigFileErrors[-r], "Config file reading error", 0);
		else if (-200<r)
			MessageBoxA(0, readAtomsXYZErrors[-r-100], "XYZ file reading error", 0);
		else if (-300<r)
			MessageBoxA(0, readAtomsCubeErrors[-r-200], "Cube file reading error", 0);
		else MessageBoxA(0, readAtomsJsonErrors[-r-300], "Json reading error", 0);
		return -100+r;
	}

	if (solid)
		MessageBoxA(0, "Only spheres implemented as atom glyphs in HTC Vive", "Atom Glyph", 0);

	CMainApplication *pMainApplication = new CMainApplication( argc, argv );

	if (pMainApplication == nullptr) {
		MessageBoxA(0, MainErrors[2], nullptr, 0);
		return -2;
	}

	if (!pMainApplication->BInit())
	{
		pMainApplication->Shutdown();
		MessageBoxA(0, MainErrors[3], nullptr, 0);
		return -3;
	}

	pMainApplication->RunMainLoop();

	pMainApplication->Shutdown();
	delete pMainApplication;

	cleanConfig();
	_CrtDumpMemoryLeaks();
	return 0;
}


static int vertex_cb(p_ply_argument argument) {
    long what;
    int ret=ply_get_argument_user_data(argument, NULL, &what);
	if (!ret)
		return 0;
	if (what == 0)
		CurrentVertex++;
    if (what <=5 /* || what >=10*/ ){ //no uvs in these meshes.
		CubeVertices[CurrentVertex*numComponents+what]=(float)ply_get_argument_value(argument);
    } else {
        CubeVertices[CurrentVertex*numComponents+what]=(float)(ply_get_argument_value(argument)/255.0);
    }

    return 1;
}

static int face_cb(p_ply_argument argument) {
    long length, value_index;
//	int v;
    ply_get_argument_property(argument, NULL, &length, &value_index);
    //discard the first call with a 3
	//if (value_index == 0 && 3 != (v = (int)ply_get_argument_value(argument)))
	//	dprintf("Non-triangular face: %d vertices\n", v);
    if (value_index>=0 && value_index<=2)
        CubeIndices[CurrentIndex++]=(int)(ply_get_argument_value(argument));

    return 1;
}