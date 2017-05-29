#include <string>

#include <m3drenderer.h>
#include <functional>
#include <iostream>
#include <m3dtextureHandler.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp> 
#include <m3dnode.h>
#include <chrono>
#include <renderNode.h>
#include <synchObject.h>
#include <m3dFileIO.h>
#include <m3dtextureHandler.h>
#include <m3dmeshHandler.h>
#include <m3duploadingInterface.h>
#include <m3ddefaultShader.h>

#include "textRendering.hpp"
#include "selectedPoints.hpp"

#include "NOMADVRLib/ConfigFile.h"
#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/atomsGL.h"

#include "NOMADVRLib/TessShaders.h"
#include "NOMADVRLib/UnitCellShaders.h"

#include "NOMADVRLib/CompileGLShader.h"

#include "NOMADVRLib/IsosurfacesGL.h"

#include "defines.h"

#define TESSSUB 16

void eprintf( const char *fmt, ... )
{
	va_list args;
	char buffer[ 2048 ];

	va_start( args, fmt );
	vsprintf( buffer, fmt, args );
	va_end( args );

	fprintf(stderr, "%s\n", buffer );
}

typedef std::chrono::high_resolution_clock Clock;
 using namespace m3d;

class sceneManager{
public:
    sceneManager(m3d::Renderer* ren, synchlib::renderNode* node, 
	const char *NOMADconfigFile, const unsigned int geo[2]);
    ~sceneManager();
    void displayFunction();
    void keyboardFunction(char key, int x, int y);
    void keyReleaseFunction(char key, int x, int y);
    void setCDPSyncher(std::shared_ptr<synchlib::SynchObject<int> > sy){m_pCurrentDataPosSyncher = sy; m_pCurrentDataPosSyncher->setData(-1);}
    void setCDTPSyncher(std::shared_ptr<synchlib::SynchObject<int> > sy){m_pCurrentDataTimeSyncher = sy; m_pCurrentDataTimeSyncher->setData(0);}
    void setCDPtSyncher(std::shared_ptr<synchlib::SynchObject<SelectedPoints> > sy)
	{
	m_pCurrentDataPointSyncher = sy; 
	SelectedPoints p={0};
	m_pCurrentDataPointSyncher->setData(p);
	}

private:
    m3d::Renderer* m_ren;
    synchlib::renderNode* m_node;

    int counter = 0;
    Clock::time_point t1 = Clock::now();




    glm::mat4 m_preMat;
    glm::mat4 m_scalemat;
    glm::mat4 m_scalematSky;
    std::shared_ptr<UploadingInterface> m_uploading;

TextRendering::Text text;

GLuint PointVAO, PointVBO;

std::shared_ptr<synchlib::SynchObject<int> > m_pCurrentDataPosSyncher;
std::shared_ptr<synchlib::SynchObject<int> > m_pCurrentDataTimeSyncher;
std::shared_ptr<synchlib::SynchObject<SelectedPoints> > m_pCurrentDataPointSyncher;

int m_oldDataPos = 0;
int m_oldTime = 0;

void glDraw(glm::mat4 pvmat, glm::mat4 viewmat, int curDataPos, 
	const SelectedPoints& sp);

const char * configFile="/home/demos/nomad_rgarcia-ubuntu/shell.ncfg";
void SetConfigFile (const char * f) {
configFile=f;
}
int error=0;
GLuint textures[2]; // white, atoms
GLuint textDepthPeeling[ZLAYERS+2]; 
GLuint peelingFramebuffer;
unsigned int geo[0]; //window width, height
	//if no tesselation is available, we still need the tess atoms for the trajectories!
GLuint *AtomTVAO=nullptr, *AtomTBuffer=nullptr, BondIndices=0,
	*AtomVAO=nullptr, *AtomBuffer=nullptr, *AtomIndices=nullptr,//[2], atoms, extraatoms
	UnitCellVAO, UnitCellBuffer, UnitCellIndexBuffer;
GLuint	AtomsP, UnitCellP; 
GLint	AtomMatrixLoc, UnitCellMatrixLoc, UnitCellColourLoc;
GLuint	TransP=0, BlendP=0;
GLint	TransMatrixLoc=-1;
bool hasTess=true;

GLuint *ISOVAO=nullptr/*[ISOS*TIMESTEPS]*/, *ISOBuffer=nullptr/*[ISOS*TIMESTEPS]*/,
	*ISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;
GLuint ISOP;
GLint ISOMatrixLoc;
GLuint BlendVAO=0, BlendBuffer=0, BlendIndices=0;
int *numISOIndices=nullptr/*[ISOS*TIMESTEPS]*/;

void RenderAtoms(const float *m);
void RenderUnitCell(const glm::mat4 eyeViewProjection);
void RenderAtomTrajectoriesUnitCell();
void RenderAtomTrajectories(const glm::mat4 eyeViewProjection);
void RenderIsos(const glm::mat4 eyeViewProjection, int curDataPos);
};


sceneManager::sceneManager(m3d::Renderer* ren, synchlib::renderNode* node,
	const char *c, const unsigned int geometry[2]){
GLenum err;

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "construktor 0: "<<__FUNCTION__<<" OpenGL error " << err << std::endl;
	}

	geo[0]=geometry[0];
	geo[1]=geometry[1];

     m_ren = ren;// m_th = th;
    m_node = node;
	if (c)
		SetConfigFile (c);

    m_preMat = glm::mat4_cast(glm::rotation(glm::vec3(0.,0.,1.),glm::vec3(0.,1.,0.)));
   m_scalemat = glm::scale(m_scalemat,glm::vec3(0.1,0.1,0.1));
   m_scalematSky = glm::scale(m_scalematSky,glm::vec3(0.05,0.05,0.05));
	
	std::string s(configFile);
	chdir(s.substr(0, s.find_last_of("\\/")).c_str());

	if ((error=loadConfigFile(configFile))<0) {
		if (-100<error) {
			fprintf (stderr, "%s", loadConfigFileErrors[-error]);
			fprintf (stderr, "Config file reading error");
		}else if (-200<error) {
			fprintf (stderr, "%s", readAtomsXYZErrors[-error-100]);
			fprintf (stderr, "XYZ file reading error");
		}else if (-300<error) {
			fprintf (stderr, "%s", readAtomsCubeErrors[-error-200]);
			fprintf (stderr, "Cube file reading error");
		}else {fprintf (stderr, "%s", readAtomsJsonErrors[-error-300]);
			fprintf (stderr, "Json reading error");
		}
	}

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
	if (!PrepareUnitCellShader (&UnitCellP, &UnitCellMatrixLoc, &UnitCellColourLoc)) {
		eprintf("OneTimeInit, failure compiling Unit Cell Shader");
		error=-401;
		return ;
	}
	
	if (!PrepareAtomShader(&AtomsP, &AtomMatrixLoc)) {
		hasTess=false;
		if (!solid)
			solid=new Solid(Solid::Type::Icosahedron);
		if (!PrepareAtomShaderNoTess(&AtomsP, &AtomMatrixLoc)) {
			error=-402;
			eprintf ("PrepareAtomShaderNoTess failed");
		}
	};

	//atom texture
	int e;
	
	e=atomTexture(textures[1]);
	if (e!=GL_NO_ERROR) {
		eprintf ("atomTexture error %d", e);
		error=-403;
	}

bool er;


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

//now isosurfaces
	if (ISOS) {
		er=::SetupDepthPeeling(geo[0], geo[1], ZLAYERS, 
			textDepthPeeling, &peelingFramebuffer);
		if (!er) {
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
		PrepareISOShader(&ISOP, &ISOMatrixLoc);

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
		//add the rotateX(-90)
			glm::mat4 trans;
			trans=glm::rotate(trans, (float)-M_PI_2, glm::vec3(1.f,0.f,0.f));
			glm::mat4 matFinal;
			matFinal=glm::translate(matFinal, 
				glm::vec3(translations[p%ISOS][0],
					translations[p%ISOS][1],
					translations[p%ISOS][2]));
			matFinal=trans*matFinal;
			float mat[16];
			for (int i=0;i<4;i++)
				for (int j=0;j<4;j++)
					mat[j*4+i]=matFinal[j][i];
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
				eprintf ("timestep %d", timestep);
				timestep++;
			}
		}
	}
//for painting selected points 
	glGenVertexArrays(1, &PointVAO);
	glGenBuffers(1, &PointVBO);
glBindVertexArray(PointVAO);
glBindBuffer(GL_ARRAY_BUFFER, PointVBO);
//x y z r g b (6), 5 points (wand + 4 selected points)
glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, NULL, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(0);
glEnableVertexAttribArray(1);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 
	(char*)0+3*sizeof(float));
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);      

//end for painting selected points

	M3DFileIO fileio;
	std::shared_ptr<Node> UploadingRoot = Node::create();


	
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "construktor 1: "<<__FUNCTION__<<" OpenGL error " << err << std::endl;
	}

	if (!text.init(FONT))
		std::cerr << "Font loading error " << FONT << std::endl;


	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "construktor nach shader zeug: "<<__FUNCTION__<<
			" OpenGL error" << err << std::endl;
	}

    std::shared_ptr<m3d::MeshHandler> mHandler = std::make_shared<MeshHandler>(ren->getDisplay(),*(ren->getContext()),true,false);
    std::shared_ptr<TextureHandler> tHandler = std::make_shared<TextureHandler>((ren->getDisplay()),*(ren->getContext()),true);
    mHandler->start();
    tHandler->start();



    m_uploading = std::make_shared<UploadingInterface>(mHandler,tHandler);
    m_uploading->setRootNode(UploadingRoot);

	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "construktor nach set root: "<<__FUNCTION__<<" OpenGL error " << err << std::endl;
	}
    std::cout<<"start"<<std::endl;
    m_uploading->start();
    std::cout<<"started"<<std::endl;
    sleep(2);
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "construktor nach start: "<<__FUNCTION__<<" OpenGL error " << err << std::endl;
	}
}

sceneManager::~sceneManager(){
m_uploading->quit();

}

void sceneManager::glDraw(glm::mat4 pvmat, glm::mat4 viewMat, int curDataPos,
	const SelectedPoints& selectedPoints) {
	GLenum err;

if ((err = glGetError()) != GL_NO_ERROR) 
	eprintf ("begin of glDraw, error %d\n", err);

       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
   	while ((err = glGetError()) != GL_NO_ERROR) {
   		std::cerr << "disp vor draw: "<<__FUNCTION__<<" OpenGL error " << err << std::endl;
   	}


//here the low-level gl code can be added
glm::mat4 wand;
m_node->getWandTrafo(wand);
//rgh FIXME this should be done only once.
glm::vec3 scale;
glm::quat rotation;
glm::vec3 translation;
glm::vec3 skew;
glm::vec4 perspective;
glm::decompose(wand, scale, rotation, translation, skew, perspective);

glm::mat4 st;
m_node->getSceneTrafo(st);
//fprintf (stderr, "st = ");
//for (int i=0;i<4;i++)
//	for (int j=0;j<4;j++)
//		fprintf (stderr, "%f ", st[i][j]);
//fprintf(stderr, "\n");
//rgh FIXME: cache these numbers, do not calculate twice per frame
if(error)
	return;
if (has_abc) {
	RenderUnitCell(pvmat*st);
} else {
	//atom trajectories
	RenderAtomTrajectories(pvmat*st);
}

if (ISOS)
	RenderIsos(pvmat*st, curDataPos);


if ((err = glGetError()) != GL_NO_ERROR) 
	eprintf ("end of glDraw, error %d\n", err);

}

void sceneManager::displayFunction(){
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR) {
		std::cerr << "disp 1: "<<__FUNCTION__<<" OpenGL error" << err << std::endl;
	}
	bool ttt = false;
    glm::mat4 pvmatL, pvmatR;

    m_node->getProjectionMatrices(pvmatL,pvmatR);
    glm::mat4 viewMat;
    m_node->getSceneTrafo(viewMat);
    int curDataPos;
    m_pCurrentDataPosSyncher->getData(curDataPos);
    curDataPos=curDataPos%(ISOS+1);
    if (curDataPos<0)
	curDataPos+=ISOS+1;
    int timePos;
    m_pCurrentDataTimeSyncher->getData(timePos);
    if(m_oldTime != timePos){
    	m_oldTime = timePos%TIMESTEPS;
	if (m_oldTime<0)
		m_oldTime+=TIMESTEPS;
	//eprintf ("m_oldtime=%d", m_oldTime);
    }
	SelectedPoints selectedPoints;
	m_pCurrentDataPointSyncher->getData(selectedPoints);

    auto t2 = Clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count();
    if(dur > 1000){
        std::cout<<"################################ fps: "<<(double)counter/((double) dur / (double) 1000)<<std::endl;
        counter = 0;
        t1 = Clock::now();
        std::cout<<"time: "<<timePos<<" iso: "<<curDataPos<<std::endl;
    }


    if(m_oldDataPos != curDataPos){
    	//rgh: no lods in this demo //m_posData->setLOD(curDataPos[0]);
    	m_oldDataPos = curDataPos;
    }
    glm::mat4 user; m_node->getUserTrafo(user);

//rgh: parameter not needed in this demo
//    m_posData->getShader()->setUserPos(glm::vec3(user[3]));
//    m_NegData->getShader()->setUserPos(glm::vec3(user[3]));


    glm::mat4 viewRotOnly = viewMat;
    viewRotOnly[3] = glm::vec4(0.,0.,0.,1.);



    glDrawBuffer(GL_BACK_LEFT);
    {
	glDraw(pvmatL, viewMat, curDataPos, selectedPoints);
    }

    glDrawBuffer(GL_BACK_RIGHT);
   {
	glDraw(pvmatR, viewMat, curDataPos, selectedPoints);
    }
   GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE,0);
   glClientWaitSync(fence,GL_SYNC_FLUSH_COMMANDS_BIT,GL_TIMEOUT_IGNORED);


    if(!m_node->synchFrame()){

       m_ren->stopMainLoop();
       return;
    }

    m_ren->swapBuffer();


    ++counter;

}


void sceneManager::keyboardFunction(char key, int x, int y){

switch(key){


case 27:
    m_ren->stopMainLoop();
    break;
}
}

void sceneManager::keyReleaseFunction(char key, int x, int y){
    switch(key){

    }
}



void InitGraphics(void)
{
	glClearColor (BACKGROUND[0], BACKGROUND[1], BACKGROUND[2], 1.0);
	glClearDepth(1.0f);				// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);			// The Type Of Depth Test To Do
	glDisable(GL_BLEND);

}





int main(int argc, char** argv){
	if(argc < 4){
		std::cout<<"Not enough arguments! Start with "<<argv[0]<<" <own hostname/IP> <path to mlib configfile> <path to ncfg configfile>"<<std::endl;
		exit(0);
	}

	TMPDIR="/tmp/"; //needs to be a non-shared directory, as we have 
	//concurrent loading from the web

    bool stereo = true;
    bool debug = true;
    m3d::Renderer* ren = new m3d::Renderer(stereo,debug);

    std::string file =argv[2];
    synchlib::caveConfig conf(file);
    std::stringstream ownIP;
    ownIP<< argv[1];
    if(argc > 3){
    	ownIP<<"_"<<argv[3];
    }

	synchlib::renderNode* node = new synchlib::renderNode(file,argc, argv);

const unsigned int geo[2]={static_cast<unsigned int>(conf.m_wall_conf[ownIP.str()].wall_geo[0]),
	static_cast<unsigned int>(conf.m_wall_conf[ownIP.str()].wall_geo[1])};

    std::cout<<geo[0]<<"    "<<geo[1]<<std::endl;

    ren->createWindow("NOMADCaveT",geo[0],geo[1]);
//http://stackoverflow.com/questions/8302625/segmentation-fault-at-glgenvertexarrays-1-vao
	glewExperimental = GL_TRUE; 
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    }



	{
		sceneManager sceneM(ren,node, argv[4], geo);


		std::shared_ptr<synchlib::SynchObject<int> > currentDataPosSyncher = synchlib::SynchObject<int>::create();
		sceneM.setCDPSyncher(currentDataPosSyncher);
		node->addSynchObject(currentDataPosSyncher,synchlib::renderNode::RECEIVER,0);

		std::shared_ptr<synchlib::SynchObject<int> > currentDataTimeSyncher = synchlib::SynchObject<int>::create();
		sceneM.setCDTPSyncher(currentDataTimeSyncher);
		node->addSynchObject(currentDataTimeSyncher,synchlib::renderNode::RECEIVER,0);

		std::shared_ptr<synchlib::SynchObject<SelectedPoints> > 
			currentDataPointsSyncher = synchlib::SynchObject<SelectedPoints>::create();
		sceneM.setCDPtSyncher(currentDataPointsSyncher);
		node->addSynchObject(currentDataPointsSyncher,synchlib::renderNode::RECEIVER,0);

		std::function<void(void)> dispFunc = std::bind(&sceneManager::displayFunction,&sceneM);
		std::function<void(char,int,int)>  keyFunc = std::bind(&sceneManager::keyboardFunction,&sceneM,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
		std::function<void(char,int,int)>  keyRFunc = std::bind(&sceneManager::keyReleaseFunction,&sceneM,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
		ren->setDisplayFunction(dispFunc);
		ren->setIdleFunction(dispFunc);
		ren->setKeyPressFunction(keyFunc);
		ren->setAllowKeyboardAutoRepressing(false);
		ren->setKeyReleaseFunction(keyRFunc);
//		ren->toggleFullscreen();
		node->init();

		node->startSynching();
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr <<" vor init: "<<__FUNCTION__<<" OpenGL error" << err << std::endl;
		}
		InitGraphics();
		while ((err = glGetError()) != GL_NO_ERROR) {
			std::cerr << "initGraphics: "<<__FUNCTION__<<" OpenGL error" << err << std::endl;
		}
		ren->mainLoop();
	}
    delete ren;

    node->stopSynching();
    delete node;

}

void sceneManager::RenderAtoms(const float *m) //m[16]
{
	//return;
	//eprintf ("RenderAtoms start numatoms %d, timestep %d", numAtoms[m_oldTime], m_oldTime);
	if (solid)
		eprintf ("solid nfaces %d", solid->nFaces);
	int e;
	if (numAtoms==0)
		return;
	
	if (hasTess) {
		glBindTexture(GL_TEXTURE_2D, textures[1]);
		glUseProgram(AtomsP);
		//eprintf ("1");
		float levelso[4] = { TESSSUB, TESSSUB, TESSSUB, TESSSUB };
		float levelsi[2] = { TESSSUB, TESSSUB};
		//eprintf ("2");
		glPatchParameterfv(GL_PATCH_DEFAULT_OUTER_LEVEL,levelso);
		glPatchParameterfv(GL_PATCH_DEFAULT_INNER_LEVEL,levelsi);
		glPatchParameteri(GL_PATCH_VERTICES, 1);
		//eprintf ("3");
		glBindVertexArray(AtomTVAO[0]);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		//eprintf ("4");
		glBindBuffer(GL_ARRAY_BUFFER, AtomTBuffer[0]);
		//eprintf ("5");
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
			4 * sizeof(float), (const void *)(0));
		//eprintf ("6");
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 
			4 * sizeof(float), (const void *)(3 * sizeof(float)));
		//eprintf ("7");
		//trans.translate(iPos).rotateX(-90).translate(UserPosition);
		//Matrix4 transform = GetCurrentViewProjectionMatrix(nEye)*trans;
		//Matrix4 mv=GetCurrentViewMatrix(nEye)*trans;

		glUniformMatrix4fv(AtomMatrixLoc, 1, GL_FALSE, m);

		//glUniformMatrix4fv(m_nAtomMVLocation, 1, GL_FALSE, mv.get());
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error 4 timestep =%d: %d, %s\n", m_oldTime, 
				e, gluErrorString(e));
		//eprintf ("8");
		if (m_oldTime==0 || fixedAtoms)
			glDrawArrays(GL_PATCHES, 0, numAtoms[0]);
		else
			glDrawArrays(GL_PATCHES, numAtoms[m_oldTime-1], 
				numAtoms[m_oldTime]-numAtoms[m_oldTime-1]);
		//eprintf ("9");
	
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after RenderAtoms timestep =%d: %d, %s\n", 
				m_oldTime, e, gluErrorString(e));

		//now cloned atoms
		if (numClonedAtoms!=0 && (m_oldTime==0 || fixedAtoms)) {
			//eprintf ("10");
			glBindVertexArray(AtomTVAO[1]);
			//eprintf ("11");
			glDrawArrays(GL_PATCHES, 0, numClonedAtoms);
			//eprintf ("12");
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("Gl error after Render cloned Atom timestep =%d: %d, %s\n", 
					m_oldTime, e, gluErrorString(e));
		}



	} else { //no tess
		glBindVertexArray(AtomVAO[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AtomIndices[0]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);
	glBindBuffer(GL_ARRAY_BUFFER, AtomBuffer[0]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("2 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("3 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("4 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("5 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		glBindTexture(GL_TEXTURE_2D, textures[1]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("6 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		glUseProgram(AtomsP);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("7 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		glUniformMatrix4fv(AtomMatrixLoc, 1, GL_FALSE, m);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("8 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		if (m_oldTime==0 || fixedAtoms) {
			glDrawElements(GL_TRIANGLES, numAtoms[0]* 3 * solid->nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT,
#else
				GL_UNSIGNED_INT,
#endif	
				0);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("9 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		} else {
			glDrawElements(GL_TRIANGLES, (numAtoms[m_oldTime]-numAtoms[m_oldTime-1]) * 3 * solid->nFaces,
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT, (void*)(numAtoms[m_oldTime-1]*sizeof(unsigned short)*3*solid->nFaces)
#else
				GL_UNSIGNED_INT, (void*)(numAtoms[m_oldTime-1]*sizeof(unsigned int)*3*solid->nFaces)
#endif
				);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("10 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);

		} //if (m_oldTime==0) 
		if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render  Atom timestep =%d: %d\n", m_oldTime, e);
		//now cloned atoms
		if (numClonedAtoms!=0 && m_oldTime==0) {
			glBindVertexArray(AtomVAO[1]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AtomIndices[1]);
			glBindBuffer(GL_ARRAY_BUFFER, AtomBuffer[1]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (const void *)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(3*sizeof(float)));
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (const void *)(6 * sizeof(float)));

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("5 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);
			glDrawElements(GL_TRIANGLES, numClonedAtoms* 3 * solid->nFaces, 
#ifndef INDICESGL32				
				GL_UNSIGNED_SHORT,
#else
				GL_UNSIGNED_INT,
#endif	
				0);			
			
			
			if ((e = glGetError()) != GL_NO_ERROR)
				eprintf("Gl error after Render cloned Atom timestep =%d: %d\n", m_oldTime, e);
		} // painting cloned atoms
	} // no tess

//eprintf ("RenderAtoms, end");
} // render atoms

void sceneManager::RenderAtomTrajectories(const glm::mat4 eyeViewProjection)
{
int e;
if (!numAtoms)
	return;
//eprintf ("RenderAtomTrajectories start");
/*glm::mat4 trans={1,0,0,UserTranslation[0],
		0,1,0,UserTranslation[1],
		0,0,1,UserTranslation[2],
		0,0,0,1};
*/					
//trans.translate(iPos).rotateX(-90).translate(UserPosition);
glm::mat4 transform = eyeViewProjection; //MatrixMul(eyeViewProjection,trans);
//gvr::Mat4f transform=eyeViewProjection;					
float t[16];
for (int i=0;i<4;i++)
	for (int j=0;j<4;j++)
		t[j*4+i]=transform[j][i];
glUseProgram(UnitCellP);
glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after glUniform4fv 1 RenderAtomTrajectories: %d\n", e);
RenderAtomTrajectoriesUnitCell();
RenderAtoms(t);
//now bonds
//return;
if (numBonds) {
	glBindVertexArray(AtomTVAO[2]);
	if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render Atom bonds glBindVertexArray timestep =%d: %d, %s\n", m_oldTime, e, gluErrorString(e));
	glUseProgram(UnitCellP);
	if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render Atom bonds glUseProgram timestep =%d: %d, %s\n", m_oldTime, e, gluErrorString(e));
	glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
	float color[4]={0.5,0.5,1,1};
	glUniform4fv(UnitCellColourLoc, 1, color);

	if (m_oldTime==0||fixedAtoms)
		glDrawElements(GL_LINES, numBonds[0],  GL_UNSIGNED_INT, (void*)0);
	else
		glDrawElements(GL_LINES, numBonds[m_oldTime]-numBonds[m_oldTime-1], GL_UNSIGNED_INT, 
			(void*)(sizeof(int)*numBonds[m_oldTime-1]) );

	if ((e = glGetError()) != GL_NO_ERROR)
			eprintf("Gl error after Render Atom bonds timestep =%d: %d, %s\n", m_oldTime, e, gluErrorString(e));
}
glBindVertexArray(0);

} //RenderAtomTrajectories

void sceneManager::RenderAtomTrajectoriesUnitCell()
{
//eprintf ("RenderAtomTrajectoriesUnitCell start");
	//now trajectories
if (!showTrajectories)
	return;

int e;
if (!AtomTVAO) {
	eprintf("RenderAtomTrajectoriesUnitCell, no atoms");
	return;
}
glBindVertexArray(AtomTVAO[0]);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderAtomTrajectoriesUnitCell: %d\n", e);
//glUseProgram(UnitCellP);
//glUniformMatrix4fv(m_nUnitCellMatrixLocation, 1, GL_FALSE, matrix);
float color[4]={1,0,0,1};
glUniform4fv(UnitCellColourLoc, 1, color);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after glUniform4fv 2 RenderAtomTrajectoriesUnitCell: %d\n", e);
//glEnableVertexAttribArray(0);
//glDisableVertexAttribArray(1);

//LOG("atomtrajectories.size()=%d", atomtrajectories.size());
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

} //sceneManager::RenderAtomTrajectoriesUnitCell()


void sceneManager::RenderIsos(const glm::mat4 eyeViewProjection, int curDataPos)
{
GLenum e;
float t[16];
for (int i=0;i<4;i++)
	for (int j=0;j<4;j++)
		t[j*4+i]=eyeViewProjection[j][i];


if (curDataPos!=ISOS) {
	glDisable(GL_BLEND);
	glUseProgram(ISOP);
	glUniformMatrix4fv(ISOMatrixLoc, 1, GL_FALSE, t);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("Gl error after glUniform4fv 1 RenderUnitCell: %d\n", e);
	glBindVertexArray(ISOVAO[m_oldTime*ISOS+curDataPos]);
	glDrawElements(GL_TRIANGLES,numISOIndices[m_oldTime*ISOS+curDataPos] , GL_UNSIGNED_INT, 0);
} else {//transparency
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	//do depth peeling
	CleanDepthTexture(textDepthPeeling[0]);
	GLint dfb;
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &dfb);
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error RenderIsos, before zl loop: %d\n", e);
	for (int zl = 0; zl < ZLAYERS; zl++) {
		EnableDepthFB(zl, TransP, 
			peelingFramebuffer, textDepthPeeling);
		glUniformMatrix4fv(TransMatrixLoc, 1, GL_FALSE, t);
		for (int i=0;i<ISOS;i++) {
			glBindVertexArray(ISOVAO[m_oldTime*ISOS+i]);
			glDrawElements(GL_TRIANGLES,numISOIndices[m_oldTime*ISOS+i] , GL_UNSIGNED_INT, 0);	
		}
	}
	glUseProgram(BlendP);
	glBindFramebuffer(GL_FRAMEBUFFER, dfb);
	glBindVertexArray(BlendVAO);
	BlendTextures(textDepthPeeling, ZLAYERS);
//old code for no transparency
//	for (int i=0;i<ISOS;i++) {
//		glBindVertexArray(ISOVAO[m_oldTime*ISOS+i]);
//		glDrawElements(GL_TRIANGLES,numISOIndices[m_oldTime*ISOS+i] , GL_UNSIGNED_INT, 0);		
//	}
}
if ((e = glGetError()) != GL_NO_ERROR)
	eprintf("Gl error after RenderIsos: %d\n", e);
}

void sceneManager::RenderUnitCell(const glm::mat4 eyeViewProjection)
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

	for (p[0]=0;p[0]<repetitions[0];(p[0])++)
		for (p[1]=0;p[1]<repetitions[1];(p[1])++)
			for (p[2]=0;p[2]<repetitions[2];(p[2])++)
				{
					float delta[3];
					GetDisplacement(p, delta);
					//eprintf ("delta %f %f %f", delta[0], delta[1], delta[2]);
					glm::mat4 scale={5,0,0,1,  
						0,5,0,1,
						0,0,5,1,
						0,0,0,1};
					glm::mat4 trans= glm::mat4(1.0f);
					trans=glm::translate(trans,glm::vec3(delta[0]*5, delta[1]*5, delta[2]*5));
					trans=glm::scale(trans, glm::vec3(5,5,5));
//						={1,0,0,delta[0]/*+UserTranslation[0]*/,
//						0,1,0,delta[1]/*+UserTranslation[1]*/,
//						0,0,1,delta[2]/*+UserTranslation[2]*/,
//						0,0,0,1};
					
					//trans.translate(iPos).rotateX(-90).translate(UserPosition);
					glm::mat4 transform = eyeViewProjection*trans;
					//gvr::Mat4f transform=eyeViewProjection;					
					float t[16];
					for (int i=0;i<4;i++)
						for (int j=0;j<4;j++)
							t[j*4+i]=transform[j][i];
					glUseProgram(UnitCellP);
					glUniformMatrix4fv(UnitCellMatrixLoc, 1, GL_FALSE, t);
					if ((e = glGetError()) != GL_NO_ERROR)
						eprintf("Gl error after glUniform4fv 1 RenderUnitCell: %d\n", e);
					float color[4]={1,1,1,1};
					glUniform4fv(UnitCellColourLoc, 1, color);
					if ((e = glGetError()) != GL_NO_ERROR)
						eprintf("Gl error after glUniform4fv 2 RenderUnitCell: %d\n", e);
					glBindVertexArray(UnitCellVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, UnitCellIndexBuffer);
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("1 Gl error RenderAtom timestep =%d: %d\n", m_oldTime, e);
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
					//atom trajectories
					//rgh: disable for now
					RenderAtomTrajectoriesUnitCell();
					RenderAtoms(t);
				}
}
