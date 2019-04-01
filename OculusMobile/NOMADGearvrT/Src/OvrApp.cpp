/************************************************************************************

Filename    :   OvrApp.cpp
Content     :   Trivial use of the application framework.
Created     :   
Authors     :   

Copyright   :   Copyright 2014 Oculus VR, LLC. All Rights reserved.

*************************************************************************************/

#include <unistd.h>

#include "OvrApp.h"
#include "GuiSys.h"
#include "OVR_Locale.h"

#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/ConfigFile.h"
#include "NOMADVRLib/atomsGL.h"
#include "NOMADVRLib/UnitCellShaders.h"
#include "NOMADVRLib/TessShaders.h"
#include "NOMADVRLib/polyhedron.h"
#include "NOMADVRLib/IsosurfacesGL.h"

using namespace OVR;

void eprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );
	va_end( args );

    LOG("Message from NOMADGearvrT");
	if (*fmt=='\0')
		LOG("Empty format");
	LOG("<%s>", buffer);
}

#if defined( OVR_OS_ANDROID )
extern "C" {

jlong Java_oculus_MainActivity_nativeSetAppInterface( JNIEnv * jni, jclass clazz, jobject activity,
		jstring fromPackageName, jstring commandString, jstring uriString )
{
	LOG( "nativeSetAppInterface" );
	return (new OvrTemplateApp::OvrApp())->SetActivity( jni, clazz, activity, fromPackageName, commandString, uriString );
}

} // extern "C"

#endif

#if 0
    #define GL( func )		func; EglCheckErrors();
#else
    #define GL( func )		func;
#endif

#include "rply/rply.h"

//static int vertex_cb(p_ply_argument argument);
//static int face_cb(p_ply_argument argument);

#include "stb_image.h"

namespace OvrTemplateApp
{
static const int CPU_LEVEL			= 2;
static const int GPU_LEVEL			= 3;
static const int NUM_INSTANCES		= 1;


//static int currentVertex=-1;

//static unsigned int currentCubeIndex=0;
//static unsigned short *cubeIndices;

OvrApp::OvrApp()
	: SoundEffectContext( NULL )
	, SoundEffectPlayer( NULL )
	, GuiSys( OvrGuiSys::Create() )
	, Locale( NULL )
{
    CenterEyeViewMatrix = ovrMatrix4f_CreateIdentity();
}

OvrApp::~OvrApp()
{
	OvrGuiSys::Destroy( GuiSys );
}

void OvrApp::Configure( ovrSettings & settings )
{
    settings.PerformanceParms.CpuLevel = CPU_LEVEL;
    settings.PerformanceParms.GpuLevel = GPU_LEVEL;
    settings.EyeBufferParms.multisamples = 4;
}

void OvrApp::OneTimeInit( const char * fromPackage, const char * launchIntentJSON, const char * launchIntentURI )
{
	OVR_UNUSED( fromPackage );
	OVR_UNUSED( launchIntentJSON );

    LOG("NOMADGearVRT, OneTimeInit, launchintentURI");
    LOG(launchIntentURI);
	LOG("NOMADGearVRT, launchIntentURI==null");
	LOG("%d", launchIntentURI==nullptr);
	//LOG("length of launchIntentURI=%d", strlen(launchIntentURI));
	//for (unsigned int i=0;i<strlen(launchIntentURI);i++) {
	//	LOG("<%c>: [%d]", launchIntentURI[i], launchIntentURI[i]);
	//}
	
	const char *configLocation="/sdcard/NOMAD/NOMADGearVR.cfg";
	const char *defaultURI="/sdcard/Oculus/NomadGearVR/cytosine.ncfg";

	char finalURI[1024];
	FILE *cl = fopen (configLocation, "r");
	if (cl==nullptr) {
		LOG("NOMADGearVRT, /sdcard/NOMAD/NOMADGearVR.cfg not found, using default at /sdcard/Oculus/NomadGearVR/cytosine.ncfg");
		strcpy(finalURI, defaultURI);
	} else {
        fgets(finalURI, 1024, cl);
        //discard newline
        int l=strlen(finalURI);
        if (finalURI[l-1]=='\n')
            finalURI[l-1]='\0';
		LOG("NOMADGearVRT, /sdcard/NOMAD/NOMADGearVR.cfg found, using contents=%s", finalURI);
		fclose (cl);
	}
	
	

	//open a default file if none given
	//if (!launchIntentURI || !strcmp(launchIntentURI, "")) {
	//	LOG(" OneTimeInit changing launchIntentURI");
	//	launchIntentURI=finalURI;
	//}
	
	LOG("NOMADGearVRT OneTimeInit, launchintentURI");
    LOG(launchIntentURI);
	
	
	const char *configfile=finalURI; 
	
	//change cwd so that relative paths work
	std::string s(configfile);
	chdir(s.substr(0, s.find_last_of("\\/")).c_str());
	
	LOG("NOMADGEARVRT, configfile=%s", configfile);
	
	int r;
	LOG("NOMADGearVRT OneTimeInit, 8");	
	if ((r=loadConfigFile(configfile))<0) {
		if (-100<r) {
			eprintf(loadConfigFileErrors[-r]);
			eprintf("Config file reading error");
		} else if (-200<r){
			
			eprintf(readAtomsXYZErrors[-r-100]);
			eprintf("XYZ file reading error");
		} else if (-300<r) {
			eprintf(readAtomsCubeErrors[-r-200]);
			eprintf("Cube file reading error");
		} else {
			eprintf(readAtomsJsonErrors[-r-300]);
			eprintf("Json reading error");
		}
	
	}
	if (!solid) {
		LOG("NOMADGearVRT No atom glyph specified, using Icosahedron");
		solid=new Solid(Solid::Type::Icosahedron);
	}
	
	UserTranslation[0]=-userpos[0];
	UserTranslation[1]=-userpos[2];
	UserTranslation[2]=userpos[1];
	
//	LOG("OneTimeInit, 2");
	const ovrJava * java = app->GetJava();
	SoundEffectContext = new ovrSoundEffectContext( *java->Env, java->ActivityObject );
	SoundEffectContext->Initialize();
	SoundEffectPlayer = new OvrGuiSys::ovrDummySoundEffectPlayer();
//LOG("OneTimeInit, 3");
	Locale = ovrLocale::Create( *app, "default" );
//LOG("OneTimeInit, 4");
	String fontName;
	GetLocale().GetString( "@string/font_name", "efigs.fnt", fontName );
	GuiSys->Init( this->app, *SoundEffectPlayer, fontName.ToCStr(), &app->GetDebugLines() );
//LOG("OneTimeInit, 5");
    GL(glGenTextures(2, textures));
    //white
    unsigned char data2[4]={255,255,255,255}; //white texture for non-textured meshes
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
//LOG("OneTimeInit, 6");
    // Create the programs
	//Leave atoms until we check if android 7 has gles 3.2 or if we use the old icosahedron method with no tesselation
	if (!PrepareUnitCellShader (&UnitCellP, &UnitCellMatrixLoc, &UnitCellColourLoc)) {
		LOG("OneTimeInit, failure compiling Unit Cell Shader");
		return ;
	}
	
	//rgh: for now, we don't have any tess-ready phones
	//if (!PrepareAtomShader(&AtomsP, &AtomMatrixLoc)) {
		hasTess=false;
        if (!PrepareAtomShaderNoTess(&AtomsP, &AtomMatrixLoc, &TotalatomsLoc))
			eprintf ("PrepareAtomShaderNoTess failed");
	//};
//LOG("OneTimeInit, 7");	
	//atom texture
	//FIXME, add atom texture
	int e;
	
	e=atomTexture(textures[1]);
	if (e!=GL_NO_ERROR)
		eprintf ("atomTexture error %d", e);
//LOG("OneTimeInit, 7A");	
    e=SetupAtoms(&AtomTVAO, &AtomTBuffer, &BondIndices);
	if (e!=GL_NO_ERROR)
		eprintf ("SetupAtoms error %d", e);
//LOG("OneTimeInit, 7B");	
	if (!hasTess)
		e=SetupAtomsNoTess(&AtomVAO, &AtomBuffer, &AtomIndices);
//LOG("OneTimeInit, 7C");	
	if (e!=GL_NO_ERROR)
		eprintf ("SetupAtomsNoTess error %d, tess=%d", e, hasTess);
	e=SetupUnitCell(&UnitCellVAO, &UnitCellBuffer, &UnitCellIndexBuffer);
	if (e!=GL_NO_ERROR)
		eprintf ("SetupUnitCell error %d", e);

    e=SetupMarkerNoTess(&MarkerVAO, &MarkerVertBuffer, &MarkerIndexBuffer);
    if (e!=GL_NO_ERROR) {
        eprintf ("SetupMarkerNoTess error %d", e);
    }

    if (ISOS || markers) {
        PrepareISOShader(&ISOP, &ISOMatrixLoc);
		currentISO=ISOS;
    }

    if (ISOS) {
        float m=BACKGROUND[0];
        m=std::max(m, BACKGROUND[1]);
        m=std::max(m, BACKGROUND[2]);
        if (m>0.2f) { //make background darker, preserving colour, so additive blending works
              for (int i=0;i<3;i++)
                   BACKGROUND[i]*=0.2f / m;
        }


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

            Matrix4f matFinal;
            /*matFinal=Matrix4f::RotationX(-M_PI_2) *
				Matrix4f::Translation(translations[p%ISOS][0],
					translations[p%ISOS][1],
					translations[p%ISOS][2]);
*/
//gvr
            if (voxelSize[0]!=-1 ||has_abc) {
                Matrix4f mvs, abcm, matcubetrans, sctrans, sc;
                if (voxelSize[0]==-1)
                    mvs=Matrix4f::Scaling(scaling);
                else
                    mvs=Matrix4f::Scaling(scaling/(float)voxelSize[0], scaling/(float)voxelSize[1],
                        scaling/(float)voxelSize[2]);
                matcubetrans=Matrix4f::Translation(cubetrans[0], cubetrans[1], cubetrans[2]);

                for (int i=0;i<3;i++) {
                    for(int j=0;j<3;j++)
                           abcm.M[i][j]=abc[i][j];
                    abcm.M[i][3]=0;
                }
                abcm.M[4][4]=1;

                sc=Matrix4f::Scaling(supercell[0], supercell[1], supercell[2]);
                sctrans=Matrix4f::Translation(-translations[p%ISOS][2],
                        -translations[p%ISOS][1], -translations[p%ISOS][0]);
                matFinal = abcm*sctrans*sc*mvs;
            } else {
                matFinal=Matrix4f::Translation(translations[p%ISOS][0], translations[p%ISOS][1],
                            translations[p%ISOS][2]);
                matFinal=Matrix4f::Scaling(scaling)*matFinal;
            }
//end gvr



			float t[16];
			for (int i=0;i<4;i++)
				for (int j=0;j<4;j++)
					t[j*4+i]=matFinal.M[i][j];
				
			if (!AddModelToScene(t, vertices, indices, tmpname, false, 
				isocolours[p%ISOS][0]<0, p%ISOS))
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
				eprintf ("timestep %d", timestep);
				timestep++;
			}
		}
	}
	
//	LOG("End of OneTimeInit");
}

void OvrApp::OneTimeShutdown()
{
//	LOG("OneTimeShutdown 1");
	delete SoundEffectPlayer;
	SoundEffectPlayer = NULL;

	delete SoundEffectContext;
	SoundEffectContext = NULL;

	glDeleteProgram(AtomsP);
	glDeleteProgram(UnitCellP);
	
    GL(glDeleteTextures(2, textures));
//	LOG("OneTimeShutdown 2");
}

bool OvrApp::OnKeyEvent( const int keyCode, const int repeatCount, const KeyEventType eventType )
{
	if (eventType==KEY_EVENT_SHORT_PRESS) {
		if (backbuttonTimesteps) {
			animateTimesteps=!animateTimesteps;
		} else {
			currentISO++;
			if (currentISO==ISOS+1)
				currentISO=0;
		}
	}
    //eprintf("OnKeyEvent called! keycode=%d", keyCode);
    return true;
    /*if ( GuiSys->OnKeyEvent( keyCode, repeatCount, eventType ) )
    {
        return true;
    }*/
}

Matrix4f OvrApp::Frame( const VrFrame & vrFrame )
{
	
	LOG("NOMADGearVRT Start Frame");
    CenterEyeViewMatrix = vrapi_GetCenterEyeViewMatrix( &app->GetHeadModelParms(), &vrFrame.Tracking, NULL );

    if (animateTimesteps) {
        animateCounter++;
        if (animateCounter>1/*0*/) {
            animateCounter=0;
            currentSet++;
            if (currentSet>TIMESTEPS-1)
                currentSet=0;
			LOG("NOMADGearVRT currentSet updated, animate timesteps %d", currentSet);
        }
    }
    Matrix4f rot;
    float speed=0.1;
    unsigned int pb=vrFrame.Input.buttonState;
	if (pb&BUTTON_SWIPE_FORWARD && eyedir[2]<speed*0.5) {//move
	   eyedir[2]+=speed;
	   eyedir[1]=0;
	} else if (pb & BUTTON_SWIPE_BACK &&eyedir[2]>speed*-0.5) {
	   eyedir[2]-=speed;
	   eyedir[1]=0;
	} else if (pb & BUTTON_SWIPE_UP) {//next, previous timestep
		currentSet++;
		if (currentSet>TIMESTEPS-1)
			currentSet=0;
		//LOG("currentSet updated, swipe, %d", currentSet);
	} else if (pb & BUTTON_SWIPE_DOWN) {
		currentSet--;
		if (currentSet<0)
			currentSet=TIMESTEPS-1;
		//LOG("currentSet updated, swipe %d", currentSet);
	} /*else dir=Vector4f(0,0,0,0);*/
	if (fabs(eyedir[1]) < 0.5*speed && fabs(eyedir[2]) < 0.5*speed)
		eyedir[1]=eyedir[2]=0.0f;
	if (eyedir[1]!=0 || eyedir[2]!=0) {
	   rot=CenterEyeViewMatrix;
	   rot.Invert();
	}
	Vector4f dir=rot.Transform(eyedir);

	UserTranslation+=dir;
    
    // Update GUI systems last, but before rendering anything.
    GuiSys->Frame( vrFrame, CenterEyeViewMatrix );

    return CenterEyeViewMatrix;
}

void OvrApp::RenderOneIso(int i) {
	GLenum e;
		//rgh FIXME, redo this when we have the new rendering code for atom trajectories
	glBindVertexArray(ISOVAO[currentSet*ISOS+i]);
/*	glBindBuffer(GL_ARRAY_BUFFER, ISOBuffer[currentSet*ISOS+i]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ISOIndices[currentSet*ISOS+i]);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), (const void *)(0*sizeof(float)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 10*sizeof(float), (const void *)(3*sizeof(float)));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 10*sizeof(float), (const void *)(6*sizeof(float)));*/
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("5 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
	eprintf ("Drawing %d vertices, isos", numISOIndices[currentSet*ISOS+i]);
	glDrawElements(GL_TRIANGLES,numISOIndices[currentSet*ISOS+i] , GL_UNSIGNED_INT, 0);		
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("6 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
}

void OvrApp::RenderIsos(const OVR::Matrix4f eyeViewProjection, int iso) {
	GLenum e;
	Matrix4f trans=Matrix4f::Translation(UserTranslation[0], UserTranslation[1], UserTranslation[2]);
	//trans.translate(iPos).rotateX(-90).translate(UserPosition);
    //rgh: prescaled on ply load
    Matrix4f transform = eyeViewProjection*trans/**Matrix4f::Scaling(scaling)*/*Matrix4f::RotationX(-M_PI_2);
	float t[16];
	for (int i=0;i<4;i++)
		for (int j=0;j<4;j++)
			t[j*4+i]=transform.M[i][j];
	glUseProgram(ISOP);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
    glUniformMatrix4fv(ISOMatrixLoc, 1, GL_FALSE, t);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("2 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);

	if (iso!=ISOS) {
		glBindVertexArray(ISOVAO[currentSet*ISOS+iso]);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("3 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
		eprintf ("Drawing %d vertices, isos", numISOIndices[currentSet*ISOS+iso]);
		glDrawElements(GL_TRIANGLES,numISOIndices[currentSet*ISOS+iso], GL_UNSIGNED_INT, 0);
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("4 Gl error RenderIsos timestep =%d: %d\n", currentSet, e);
	} else {
		//rgh: render explicitly opaque surfaces first, then rest
		//rgh: isocolours alpha == 1 for these surfaces
		for (int i=0;i<ISOS;i++) {
			if (isocolours[i][3]==1) {
				RenderOneIso(i);
			}
		}
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
		for (int i=0;i<ISOS;i++) {
			if (isocolours[i][3]==1)
				continue;
			RenderOneIso(i);
		}
        glDisable(GL_BLEND);
        glDepthMask(GL_TRUE);
	}
    glBindVertexArray(0);
}

void OvrApp::RenderMarker(const float *m) //m[16]
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

void OvrApp::RenderAtoms(const float *m) //m[16]
{
    //eprintf ("RenderAtoms start numatoms %d", numAtoms);
	int e;
	if (numAtoms==0)
		return;
	
	if (hasTess) {
	//FIXME, unimplemented
		LOG("NOMADGearVRT FIXME, No Tess code for atoms yet!");
		return;
	} else { //no tess
		glBindVertexArray(AtomVAO[0]);
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUseProgram(AtomsP);
		glUniformMatrix4fv(AtomMatrixLoc, 1, GL_FALSE, m);
        glUniform1f(TotalatomsLoc, (float)getTotalAtomsInTexture());

		if (currentSet==0) {
			glDrawElements(GL_TRIANGLES, numAtoms[currentSet]* 3 * solid->nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT,
#else
				GL_UNSIGNED_INT,
#endif	
				0);
		} else {
			glDrawElements(GL_TRIANGLES, (numAtoms[currentSet]-numAtoms[currentSet-1]) * 3 * solid->nFaces,
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT, (void*)(numAtoms[currentSet-1]*sizeof(unsigned short)*3*solid->nFaces)
#else
				GL_UNSIGNED_INT, (void*)(numAtoms[currentSet-1]*sizeof(unsigned int)*3*solid->nFaces)
#endif
				);
		}
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render  Atom timestep =%d: %d\n", currentSet, e);
		//now cloned atoms
		if (numClonedAtoms!=0 && currentSet==0) {
			glBindVertexArray(AtomVAO[1]);
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

void OvrApp::RenderAtomTrajectoriesUnitCell()
{
	//now trajectories
if (!showTrajectories)
	return;

int e;
if (!AtomTVAO) {
	LOG("NOMADGearVRT RenderAtomTrajectoriesUnitCell, no atoms");
	return;
}
glBindVertexArray(AtomTVAO[0]);

glUniform4fv(UnitCellColourLoc, 1, atomtrajectorycolour);
if ((e = glGetError()) != GL_NO_ERROR)
    eprintf("Gl error after glUniform4fv 2 RenderAtomTrajectoriesUnitCell: %d\n", e);

//LOG("atomtrajectories.size()=%d", atomtrajectories.size());
//rgh FIXME, old code which does not work with large atom sets!
glBindBuffer(GL_ARRAY_BUFFER, AtomTBuffer[0]);

for (unsigned int i=0;i<atomtrajectories.size();i++) {
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float)*numAtoms[0], 
		(const void *)(0+4*sizeof(float)*atomtrajectories[i]));
		//LOG("atomtrajectoryrestarts[%d].size()=%d", i, atomtrajectoryrestarts[i].size());
	for (unsigned int j=1;j<atomtrajectoryrestarts[i].size();j++) {
		int orig=atomtrajectoryrestarts[i][j-1];
		int count=atomtrajectoryrestarts[i][j]-atomtrajectoryrestarts[i][j-1];
		glDrawArrays(GL_LINE_STRIP, orig, count);

	} //j
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error after Render Atom trajectories: %d\n", e);
} //i

} //OvrApp::RenderAtomTrajectoriesUnitCell()


void OvrApp::RenderUnitCell(const Matrix4f eyeViewProjection)
{
	eprintf ("RenderUnitCell, has_abc=%d", has_abc);
	if (!has_abc)
		return;
	if (UnitCellVAO==0)
		eprintf ("Error, Unit Cell VAO not loaded");
	int e;
    Matrix4f sc=Matrix4f::Scaling(scaling);
	int p[3];
	for (p[0]=0;p[0]<repetitions[0];(p[0])++)
		for (p[1]=0;p[1]<repetitions[1];(p[1])++)
			for (p[2]=0;p[2]<repetitions[2];(p[2])++)
				{
					float delta[3];
					GetDisplacement(p, delta);
                    //delta is in atom coordinates, need rotateX(-90)
                    Vector3f iPos(delta[0]+UserTranslation[0]/scaling, delta[2]+UserTranslation[1]/scaling,
                            -delta[1]+UserTranslation[2]/scaling);
					Matrix4f trans=Matrix4f::Translation(iPos);
                    Matrix4f transform = eyeViewProjection*sc*trans*Matrix4f::RotationX(-M_PI_2);
					float t[16];
					for (int i=0;i<4;i++)
						for (int j=0;j<4;j++)
							t[j*4+i]=transform.M[i][j];
					glUseProgram(UnitCellP);
					glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
                    if ((e = glGetError()) != GL_NO_ERROR)
						eprintf("Gl error after glUniform4fv 1 RenderUnitCell: %d\n", e);
                    if (displayunitcell) {
                        glUniform4fv(UnitCellColourLoc, 1, unitcellcolour);
                        if ((e = glGetError()) != GL_NO_ERROR)
                            eprintf("Gl error after glUniform4fv 2 RenderUnitCell: %d\n", e);
                        glBindVertexArray(UnitCellVAO);
                        if ((e = glGetError()) != GL_NO_ERROR)
                            eprintf("Gl error after glBindVertexArray RenderUnitCell: %d\n", e);
                        glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
                        if ((e = glGetError()) != GL_NO_ERROR)
                            eprintf("Gl error after RenderUnitCell: %d\n", e);
                    }
					//atom trajectories
					RenderAtomTrajectoriesUnitCell();
					RenderAtoms(t);
                    RenderMarker(t);
				}
}

void OvrApp::RenderAtomTrajectories (const Matrix4f eyeViewProjection)
{
    if (!numAtoms)
        return;
    const Matrix4f sc=Matrix4f::Scaling(scaling);
    const Matrix4f trans=Matrix4f::Translation(UserTranslation[0]/scaling,UserTranslation[1]/scaling,
                UserTranslation[2]/scaling );
    const Matrix4f transform = eyeViewProjection*sc*trans*Matrix4f::RotationX(-M_PI_2);

    float t[16];
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            t[j*4+i]=transform.M[i][j];

    glUseProgram(UnitCellP);
    glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
    RenderAtomTrajectoriesUnitCell();
    RenderAtoms(t);
    RenderMarker(t);
}

Matrix4f OvrApp::DrawEyeView( const int eye, const float fovDegreesX, const float fovDegreesY, ovrFrameParms & frameParms )
{
    OVR_UNUSED( frameParms );
    const Matrix4f eyeViewMatrix = vrapi_GetEyeViewMatrix( &app->GetHeadModelParms(), &CenterEyeViewMatrix, eye );
    const Matrix4f eyeProjectionMatrix = ovrMatrix4f_CreateProjectionFov( fovDegreesX, fovDegreesY, 0.0f, 0.0f, 1,100000/*1.0f, 0.0f*/ );
    const Matrix4f eyeViewProjection = eyeProjectionMatrix * eyeViewMatrix;

    GL( glClearColor( BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1.0f ) );
    GL( glClear( GL_COLOR_BUFFER_BIT ) );

    GL( glDepthFunc(GL_LEQUAL));

    if (has_abc)
        RenderUnitCell(eyeViewProjection);
    else
        RenderAtomTrajectories(eyeViewProjection);
	
	if (ISOS)
		RenderIsos(eyeViewProjection, currentISO);
	
    GL( glBindVertexArray( 0 ) );
    GL( glUseProgram( 0 ) );

    GuiSys->RenderEyeView( CenterEyeViewMatrix, eyeViewMatrix, eyeProjectionMatrix );

    frameParms.Layers[VRAPI_FRAME_LAYER_TYPE_WORLD].Flags |= VRAPI_FRAME_LAYER_FLAG_CHROMATIC_ABERRATION_CORRECTION;
    return eyeViewProjection;
}

} // namespace OvrTemplateApp


