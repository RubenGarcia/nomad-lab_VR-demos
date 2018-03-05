/*
# Copyright 2016-2018 The NOMAD Developers Group
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

#define GLM_SWIZZLE
#include <glm/gtx/matrix_decompose.hpp>
#include <m3drenderer.h>
#include <functional>
#include <iostream>
#include <chrono>
#include <renderNode.h>
#include <renderServer.h>

#include <synchObject.h>
//#include <m3dshader.h>
#include <future>
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdio.h>
#include <m3dutils.h>
#include <ctime>

#include "selectedPoints.hpp"

typedef std::chrono::high_resolution_clock Clock;

#include "defines.h"


#define bunge_maxSurfaces 1
//#define bunge_maxTimeSteps TIMESTEPS


class sceneManager{
public:
    sceneManager(m3d::Renderer* ren,  synchlib::renderServer* serv) {
    	m_ren = ren;
    	m_serv = serv;
    	cameraFile.open("/home/di73yad/demos/demoBunge/cameraTrafos.txt",std::fstream::out | std::fstream::app);
    	std::time_t curTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    	cameraFile<<std::ctime(&curTime)<<std::endl;
    }
    ~sceneManager();
    void displayFunction();
    void keyboardFunction(char key, int x, int y);
  //  void keyReleaseFunction(char key, int x, int y);

    void startThreads();
    std::shared_ptr<synchlib::SynchObject<int> > m_pCurrentDataPosSyncher;
    std::shared_ptr<synchlib::SynchObject<int> > m_pCurrentDataTimeSyncher;
	std::shared_ptr<synchlib::SynchObject<SelectedPoints> >
		m_pCurrentDataPointSyncher;

private:
    void updateButtonActions();
    void buttonEval();
	void synch();

    m3d::Renderer* m_ren;
    synchlib::renderServer* m_serv;


    Clock::time_point m_oldTimeForSwap = Clock::now();
    float m_maxAnimTime;

    bool m_stop = false;
    int m_swapIsoSurface = 0;
    int m_curData = -1; //ISOS;
    std::thread* m_updateButtonsThread;
    int m_mode = 0; //0 both, 1 negative and 2 positive iso swap
    bool m_pressB0 = false;
    bool m_pressB2 = false;
    int m_curTime = 0;
	SelectedPoints selectedPoints={0};
    std::fstream cameraFile;
 	int TIMESTEPS=1;

};


sceneManager::~sceneManager(){
	cameraFile.close();
    m_updateButtonsThread->join();

}



void sceneManager::startThreads(){
    //initialize first data as zero
    m_pCurrentDataPosSyncher->setData(m_curData);
    m_pCurrentDataPosSyncher->send();
    m_pCurrentDataTimeSyncher->setData(0);
    m_pCurrentDataTimeSyncher->send();
	SelectedPoints p={0};
    m_pCurrentDataPointSyncher->setData(p);
    m_pCurrentDataPointSyncher->send();


    m_updateButtonsThread  = new std::thread( std::bind(&sceneManager::updateButtonActions,this));
}

void sceneManager::synch()
{
	m_pCurrentDataPosSyncher->setData(m_curData);
	m_pCurrentDataPosSyncher->send();
	m_pCurrentDataTimeSyncher->setData(m_curTime);
	m_pCurrentDataTimeSyncher->send();
	m_pCurrentDataPointSyncher->setData(selectedPoints);
	m_pCurrentDataPointSyncher->send();
	m_oldTimeForSwap = Clock::now();
}

void sceneManager::updateButtonActions()
{
bool longpress=false;
    while (!m_stop) {
        buttonEval();
         auto t2 = Clock::now();
	int dur;
        if(abs(m_swapIsoSurface) > 0 || m_pressB0 || m_pressB2)
             dur = std::chrono::duration_cast<std::chrono::milliseconds>(t2-m_oldTimeForSwap).count();
	else {
		longpress=false;
		}
        if(abs(m_swapIsoSurface) > 0){
              	 if(!m_pressB2){
			if (dur>100) {//ms
					//make modulo in client; avoids having to read the file here.
					std::cout << "Current timestep " << m_curTime << std::endl;
					 m_curTime += m_swapIsoSurface;
					 synch();
			}
            	 }else {
			if ((!longpress && dur>100) || (longpress && dur>500)) 
			{//ms //rgh: change isos in a more discrete manner.
					std::cout << "Changing iso!!\n";
					 m_curData += m_swapIsoSurface;
//rgh: isos only in clients
//					 m_curData = (m_curData > ISOS) ? 0 : m_curData;
//					 m_curData = (m_curData < 0) ? ISOS : m_curData;
					 synch();
					 longpress=true;
			std::cout << "longpress true in B2\n";
			}



             }//m_pressB2
        } 
	if (m_pressB0){
		if ((!longpress & dur>100) ||(longpress && dur>500)) {
			selectedPoints.number++;


			glm::vec3 scale;
			glm::quat rotation;
			glm::vec3 translation;
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(m_serv->getWandTransform(), 
				scale, rotation, translation, skew, perspective);
			glm::mat4 st;
			m_serv->getSceneTransform(st);
			st=glm::inverse(st);
			translation=(st*glm::vec4(translation, 1)).xyz();
			if (selectedPoints.number>4)
				selectedPoints.number=0;
			else
				selectedPoints.p[selectedPoints.number-1]=translation;
			synch();
			longpress=true;
			std::cout << "longpress true in B0\n";
		}//dur
	}//m_pressB0
	
        //wait 5 ms
        usleep(5000);

    }

}


void sceneManager::buttonEval(){
    std::list<glm::ivec2> queue;
    m_serv->getButtonQueue(queue);
    for(std::list<glm::ivec2>::iterator it = queue.begin(); it != queue.end();++it){
	if ((*it)[1]==1)
		std::cout << "button " << (*it)[0] << "pressed\n";
        if((*it)[0] == 1){ // if button 1
            if((*it)[1] == 1){ //if pressed
                m_swapIsoSurface = +1;
                m_oldTimeForSwap = Clock::now();

            } else {
                m_swapIsoSurface = 0;
            }
        }
        if((*it)[0] == 3){ // if button 3
            if((*it)[1] == 1){ //if pressed
                m_swapIsoSurface = -1;
                m_oldTimeForSwap = Clock::now();

            } else {
                m_swapIsoSurface = 0;
            }
        }
        if((*it)[0] == 2){ // if button 2 (mode for isosurfaces: all, one by one)
            if((*it)[1] == 1){ //if pressed
                m_oldTimeForSwap = Clock::now();
            	m_pressB0 = true;
            } else {
            	if(m_pressB0){
			std::cout << " B0 true\n";
            		m_mode = (m_mode +1) % 3;
            		m_pressB0 = false;
            	}

            }
        }
        if((*it)[0] == 0){ // if button 0 (switch time on/off)
            if((*it)[1] == 1){ //if pressed
            	m_pressB2 = true;

            } else {
            	if(m_pressB2){
			std::cout << "b2 true\n";
            		m_pressB2 = false;
            	}

            }
        }
    }

}






void sceneManager::keyboardFunction(char key, int x, int y){
    switch(key){
    case 27:
        m_stop = true;
        m_ren->stopMainLoop();

        break;
    case 'c':
    case 'C':
//    	if(cameraFile.is_open())
//    		cameraFile<<glm::transpose(m_serv->getSceneTrafo())<<std::endl;
//    	std::cout<<glm::transpose(m_serv->getSceneTrafo())<<std::endl;
//    	break;
    default:
        std::cout<<"got key: "<<key<<std::endl;
        break;


    }
}

void sceneManager::displayFunction(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_ren->swapBuffer();

}


void InitGraphics(void)
{


    GLfloat LightAmbient[]= { 0.f, 0.0f, 0.0f, 1.0f };
    GLfloat LightDiffuse[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightSpecular[]= { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat LightPosition[]= { 0.0f, 20.0f, 0.0f};

    glEnable(GL_TEXTURE_2D);

    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0f);				// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST);		// Enables Depth Testing
    glDepthFunc(GL_LESS);			// The Type Of Depth Test To Do

    glDisable(GL_CULL_FACE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


}


int main( int argc, char **argv )
{
	if(argc < 3){
		std::cout<<"Not enough arguments! Start with "<<argv[0]<<
			" <path to mlib configfile> <path to NOMAD configfile>"<<std::endl
			<<"Example:\n"<<argv[0]<<" /sw/config/mlib/cave_1_multicast.conf "
			"/home/demos/nomad_rgarcia-ubuntu/IrO2_1.ncfg"<<std::endl;
		exit(0);
	}
    std::string file =argv[1];
    synchlib::caveConfig conf(file);
    m3d::Renderer* ren = new m3d::Renderer(false,true);
    ren->createWindow("demoServer",800,600,false,false);
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));

    }

    synchlib::renderServer server(file,argc,argv);
    {
        sceneManager sceneM(ren,&server);

        std::shared_ptr<synchlib::SynchObject<int> > currentDataPosSyncher = synchlib::SynchObject<int>::create();
        sceneM.m_pCurrentDataPosSyncher = currentDataPosSyncher;
        server.addSynchObject(currentDataPosSyncher,synchlib::renderServer::SENDER,0,0); //manual sending

        std::shared_ptr<synchlib::SynchObject<int> > currentDataTimeSyncher = synchlib::SynchObject<int>::create();
        sceneM.m_pCurrentDataTimeSyncher = currentDataTimeSyncher;
        server.addSynchObject(currentDataTimeSyncher,synchlib::renderServer::SENDER,0,0); //manual sending

	std::shared_ptr<synchlib::SynchObject<SelectedPoints> > 
		currentDataPointsSyncher = 
			synchlib::SynchObject<SelectedPoints>::create();

	sceneM.m_pCurrentDataPointSyncher=currentDataPointsSyncher;
	server.addSynchObject(currentDataPointsSyncher, synchlib::renderServer::SENDER,0,0); //manual sending


        std::function<void(void)> dispFunc = std::bind(&sceneManager::displayFunction,&sceneM);
        std::function<void(char,int,int)>  keyFunc = std::bind(&sceneManager::keyboardFunction,&sceneM,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);

        ren->setDisplayFunction(dispFunc);
        ren->setIdleFunction(dispFunc);
        ren->setKeyPressFunction(keyFunc);
        ren->setAllowKeyboardAutoRepressing(false);


        server.init();

        server.startSynching();


        sceneM.startThreads();

        ren->mainLoop();
    }
    server.stopSynching();
    delete ren;



        return 0;
}


