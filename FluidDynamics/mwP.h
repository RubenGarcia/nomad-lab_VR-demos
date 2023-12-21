/*
# Copyright 2016-2018 Ruben Jesus Garcia-Hernandez, Matthias Chrisopher Albert
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

#ifndef __MWP_H
#define __MWP_H

//jump functions for angleAtan2
/* Example:
angle = angleAtan2(currentset, TIMESTEPS, currentAtomtrajectoryIndex,atomtrajectories, atoms, m_mat4HMDPose);
rotatePV.identity().rotateY(angle);
UserPosition = jumpBehindAtom()
*/
Vector3 jumpToAtom(int currentset, std::vector<int> atomtrajectories, int currentAtomtrajectoryIndex, 
				   float **atoms, const Matrix4& m_mat4HMDPose,const Matrix4& rotatePV);

Vector3 jumpBehindAtom(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS, 
	std::vector<int> atomtrajectories, float **atoms, 
	const Matrix4& m_mat4HMDPose,const Matrix4& rotatePV,int distanceToAtom);

Vector3 floatAbove(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms,
	const Matrix4& m_mat4HMDPose, const Matrix4& rotatePV,int distanceToAtom);

float angleAtan2(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, 
				 std::vector<int> atomtrajectories, float **atoms, const Matrix4& m_mat4HMDPose);

Vector4 getVelocityUserOrientaionRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, std::vector<int> atomtrajectories, 
				 float **atoms,const Matrix4& m_mat4HMDPose);


int distancePlus(int distance);

int distanceMinus(int distance);

Matrix4 getFullRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, 
						std::vector<int> atomtrajectories, float **atoms,const Matrix4& m_mat4HMDPose);

Matrix4 getPartialRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, 
						std::vector<int> atomtrajectories, float **atoms,const Matrix4& m_mat4HMDPose);

//jump functions for getFullRotation
/*Example:
rotatePV.identity();
if(currentset!=0)
rotatePV = getFullRotation(currentset, TIMESTEPS, currentAtomtrajectoryIndex, atomtrajectories, atoms, m_mat4HMDPose);
UserPosition = jumpBehindAtomFullRotation();
*/

Vector3 jumpToAtomFullRotation(int currentset, std::vector<int> atomtrajectories, int currentAtomtrajectoryIndex, 
				   float **atoms);

// used by getFull and getPartial Rotation 
Vector3 jumpBehindAtomFullRotation(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS, 
	std::vector<int> atomtrajectories, float **atoms, int distanceToAtom);

Vector3 floatAboveFullRotation(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms, int distanceToAtom);


// partial Rotation. 
Vector3 floatAbovePartialRotation(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms, const Matrix4& m_mat4HMDPose, int distanceToAtom);


//dpad Controls check:
int checkdpadMode(int dPadMode, int currentdpadMode);

int checkSimpledpadMode(int simpleMode, int currentSimpleMode);
//chemistry
Vector3 chemistryJump(int currentset, int currentAtomtrajectoryIndex, int TIMESTEPS, std::vector<int> atomtrajectories, 
					   float **atoms);

//get biggest direction value of controller and return 1 or -1 for controller
Vector3 getMinMaxCrontrollerOrientation(Vector3 controller);

//Userposition to texture needs to be called 3 times for x,y,z
int* userposToTexture(int x, int y, int z);

#endif
