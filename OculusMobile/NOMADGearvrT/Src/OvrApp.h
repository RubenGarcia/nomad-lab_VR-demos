/************************************************************************************

Filename    :   OvrApp.h
Content     :   Trivial use of the application framework.
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

************************************************************************************/

#ifndef OVRAPP_H
#define OVRAPP_H

#include "App.h"
#include "SceneView.h"
#include "SoundEffectContext.h"
#include <memory>
#include "GuiSys.h"

namespace OvrTemplateApp
{

class OvrApp : public OVR::VrAppInterface
{
public:
						OvrApp();
	virtual				~OvrApp();

	virtual void 		Configure( OVR::ovrSettings & settings );
	virtual void		OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI );
	virtual void		OneTimeShutdown();
	virtual bool 		OnKeyEvent( const int keyCode, const int repeatCount, const OVR::KeyEventType eventType );
	virtual OVR::Matrix4f Frame( const OVR::VrFrame & vrFrame );
	virtual OVR::Matrix4f DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms );

	class OVR::ovrLocale &	GetLocale() { return *Locale; }

private:
	OVR::ovrSoundEffectContext * SoundEffectContext;
	OVR::OvrGuiSys::SoundEffectPlayer * SoundEffectPlayer;
	OVR::OvrGuiSys *		GuiSys;
	class OVR::ovrLocale *	Locale;

    GLuint			AtomsP, UnitCellP; // framework does not provide support for tesselation and provides many things we don't need.
	GLint		AtomMatrixLoc, UnitCellMatrixLoc, UnitCellColourLoc;
    GLint				VertexTransformAttribute, VTAGrid;

    ovrMatrix4f			CenterEyeViewMatrix;
    OVR::Vector4f         UserTranslation=OVR::Vector4f(0,0,-30,0);
    OVR::Vector4f eyedir=OVR::Vector4f(0,0,0,0);
    int currentSet=0;
    bool animateTimesteps=false;
    int animateCounter=0;
    GLuint textures[2]; // white, atoms
	//if no tesselation is available, we still need the tess atoms for the trajectories!
	GLuint *AtomTVAO=nullptr, *AtomTBuffer=nullptr, *AtomVAO=nullptr, *AtomBuffer=nullptr, *AtomIndices=nullptr,//[2], atoms, extraatoms
		UnitCellVAO, UnitCellBuffer, UnitCellIndexBuffer;

	GLuint *ISOVAO=nullptr/*[ISOS*TIMESTEPS]*/, *ISOBuffer=nullptr/*[ISOS*TIMESTEPS]*/,
		*ISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;
	GLuint ISOP;
	GLint ISOMatrixLoc;
	int *numISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;
		
	void RenderAtoms(const float *m);
	void RenderUnitCell(const OVR::Matrix4f eyeViewProjection);
	void RenderAtomTrajectoriesUnitCell();
	bool hasTess=true;
	void RenderIsos(const OVR::Matrix4f eyeViewProjection, int iso);
};

} // namespace OvrTemplateApp

#endif	// OVRAPP_H
