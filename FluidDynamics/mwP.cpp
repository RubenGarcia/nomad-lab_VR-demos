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

#include <cmath>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <array>
#include <vector>

#include "shared/Vectors.h"
#include "shared/Matrices.h"

#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/atomsGL.h"

#include "NOMADVRLib/ConfigFile.h"
#include "mwP.h"
#include "NOMADVRLib/eprintf.h"
#include <assert.h>


//mwP : move with Particle

//UserPosition actually coords of world translation (inversion with respect to 0)
// function that jumps to particle position    
Vector3 jumpToAtom(int currentset, std::vector<int> atomtrajectories,int currentAtomtrajectoryIndex, float **atoms, const Matrix4& m_mat4HMDPose,
				   const Matrix4& rotatePV){
    int mycurrent = currentset;

	assert (atomtrajectories.size() > 0);

        Matrix4 mat_tTmp = m_mat4HMDPose;
		mat_tTmp.invert();
		int myAtom = atomtrajectories[0];
		if(currentAtomtrajectoryIndex < atomtrajectories.size()-1 )
			//eprintf("jumpTo:currentAtomtrajIndex %d, atomtrajsize()-1 %d",currentAtomtrajectoryIndex,atomtrajectories.size()-1 );
			myAtom = atomtrajectories[currentAtomtrajectoryIndex];

		Vector4 tmp;
		tmp[0] = -atoms[mycurrent][(myAtom * 4)] ; //change to for loop
		tmp[1] = -atoms[mycurrent][(myAtom * 4) + 2]; // y & z are flipped
		tmp[2] = atoms[mycurrent][(myAtom * 4) + 1];
		tmp[3] = 1.0f;

		Vector4 tmp2=Vector4(mat_tTmp[12], mat_tTmp[13], mat_tTmp[14], 1)*rotatePV;
		
		tmp2= tmp2+tmp;
		return Vector3 (tmp2[0], tmp2[1], tmp2[2]);


}      

//jumps behind particle position depending on particle position(t+1)
//Needs some more Scaling
Vector3 jumpBehindAtom(int currentset, float atomScaling, int currentAtomtrajectoryIndex, int TIMESTEPS, std::vector<int> atomtrajectories, 
					   float **atoms,  const Matrix4& m_mat4HMDPose, const Matrix4& rotatePV, int distanceToAtom){
	int mycurrent = currentset;
	int next = currentset+1;
	if ( currentset == TIMESTEPS-1){
	    mycurrent = currentset - 1;
		next = currentset;
	}
	assert (atomtrajectories.size() > 0);
		Matrix4 mat_tTmp = m_mat4HMDPose;
		mat_tTmp.invert();
		int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
		float aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
		//eprintf("atomRadius = %f", atomRadius);
		Vector3 a; //current location of atom
		Vector3 b; // t+1 of atom

		// ||b-a|| atomRad*(-2) + a

		a[0] = atoms[mycurrent][(myAtom * 4)];
		a[1] = atoms[mycurrent][(myAtom * 4) + 1]; // y & z are flipped
		a[2] = atoms[mycurrent][(myAtom * 4) + 2];

		b[0] = atoms[next][myAtom * 4];
		b[1] = atoms[next][(myAtom * 4) + 1];
		b[2] = atoms[next][(myAtom * 4) + 2];
		b-= a;
		//check
		if( b!=Vector3(0,0,0) )
			b.normalize();

		b *=-(10+distanceToAtom)*aRad*atomScaling;
		a +=(b);
		b = a;
		
		a[0] = -b[0] ;
		a[1] = -b[2] ; // y & z are flipped
		a[2] = b[1];
		//eprintf("Vector UserPosition: %f %f %f ", a[0], a[1], a[2]);
		Vector4 tmp (a[0],a[1],a[2],1.0f);
		
		Vector4 tmp2=Vector4(mat_tTmp[12], mat_tTmp[13], mat_tTmp[14], 1)*rotatePV;
		
		tmp2=tmp2+tmp;
		return Vector3 (tmp2[0], tmp2[1], tmp2[2]);

	}

// jump behind and above particle
Vector3 floatAbove(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms,
	const Matrix4& m_mat4HMDPose, const Matrix4& rotatePV, int distanceToAtom){

	Vector3 tmp;
	int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
	int mycurrent = currentset;
	float aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
	float amountUp;
	amountUp = (5+distanceToAtom*0.5)*atomScaling*aRad;
	tmp = jumpBehindAtom(currentset, atomScaling,currentAtomtrajectoryIndex, TIMESTEPS, 
		atomtrajectories, atoms, m_mat4HMDPose, rotatePV, distanceToAtom);

	tmp[1] -= amountUp;
	return tmp;
}

// angle between userView and Atom movement in X,Y
// 2D rotation
float angleAtan2(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, std::vector<int> atomtrajectories, 
				 float **atoms,const Matrix4& m_mat4HMDPose){
	float angle = 0;
	float angle1 = 0;
	float angle2 = 0;

	Matrix4 mat_tmp = m_mat4HMDPose;
	mat_tmp.invert();
	Vector3 tmp = mat_tmp * Vector3(0,0,1); //12,13,14 oder 0,1,2
	tmp[1] = 0;
	tmp.normalize();

	int mycurrent = currentset;
	if (currentset == TIMESTEPS - 1){
		 mycurrent = currentset - 1;
	}
	int myAtom = atomtrajectories[currentAtomtrajectoryIndex]; //change later

	// tmp1, tmp2: x,0,z
	Vector3 tmp1;
	tmp1[0] = atoms[mycurrent][(myAtom * 4)];
	tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
	tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];

	Vector3 tmp2;
	tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
	tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
	tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];

	// ||b-a||
	tmp1 = tmp2 - tmp1;
	tmp2 = tmp1;
	tmp1[0] = -tmp2[0];
	tmp1[1] = 0; //-tmp2[2];
	tmp1[2]	=  tmp2[1];
	if(tmp1!=Vector3(0,0,0))
		tmp1.normalize();
	//angle
	angle1 = atan2(tmp[2], tmp[0]); //X,Z Userorientation angle
	angle2 = atan2(tmp1[2], tmp1[0]);
	
	angle = (angle2 - angle1) * 180.0 / M_PI;

	return angle;

}

//Angle and axis between Userorientation and Velocity
//crazy rotation, not used
Vector4 getVelocityUserOrientaionRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, std::vector<int> atomtrajectories, 
				 float **atoms,const Matrix4& m_mat4HMDPose){
	Vector4 axisRotation(0,0,0,0);
	float angle = 0;

	Matrix4 mat_tmp = m_mat4HMDPose;
	mat_tmp.invert();
	Vector3 tmp = mat_tmp * Vector3(0,0,1); //12,13,14 oder 0,1,2
	if(tmp != Vector3(0,0,0))
		tmp.normalize();

	int mycurrent = currentset;
	if (currentset == TIMESTEPS - 1){
		 mycurrent = currentset - 1;
	}
	int myAtom = atomtrajectories[currentAtomtrajectoryIndex];

	// tmp1, tmp2: x,0,z
	Vector3 tmp1;
	tmp1[0] = atoms[mycurrent][(myAtom * 4)];
	tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
	tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];

	Vector3 tmp2;
	tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
	tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
	tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];

	// ||b-a||
	tmp1 = tmp2 - tmp1;
	tmp2 = tmp1; //rearragen x,y,z to -x,-z,y 
	tmp1[0] = -tmp2[0];
	tmp1[1] = -tmp2[2];
	tmp1[2] =  tmp2[1];
	if(tmp1!=Vector3(0,0,0))
		tmp1.normalize();
	

	//angle
	angle = acos(tmp.dot(tmp1)) * 180/M_PI;
	//eprintf("angle %f",angle);
	if(abs(angle)<0.0001){
		tmp1.set(1,0,0);
	}else{
	// tmp2 = rotation axis between Userorientation(tmp) and velocity(tmp1)
		tmp2 = tmp1.cross(tmp).normalize();
	}
	axisRotation.set(angle,tmp2[0],tmp2[1],tmp2[2]);
	
	return axisRotation;

}



//change distance between head and atom
int distancePlus(int distance){
	int tmp_distance = distance;

	if (tmp_distance<10){
		tmp_distance++;
	} else if(tmp_distance<50){
		tmp_distance+=5;
	} else if(tmp_distance<100){ 
		tmp_distance+=10;
	} else{
		tmp_distance = 0;	
	}
	return tmp_distance;
}

int distanceMinus(int distance){
	int tmp_distance = distance;

	if (tmp_distance > 0 && tmp_distance<=10){
		tmp_distance--;
	} else if(tmp_distance > 10 && tmp_distance<=50){
		tmp_distance-=5;
	} else if(tmp_distance > 50 && tmp_distance<=100){ 
		tmp_distance-=10;
	} else{
		tmp_distance = 100;	
	}
	return tmp_distance;
}


Matrix4 getFullRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, std::vector<int> atomtrajectories, 
						float **atoms,const Matrix4& m_mat4HMDPose){

	Matrix4 axisRotation;
	axisRotation.identity();
	
	Matrix4 mat_tmp = m_mat4HMDPose;
	mat_tmp.invert();

	
	int mycurrent = currentset;
	if (currentset == TIMESTEPS - 1){
		 mycurrent = currentset - 1;
	}
	if(currentset == 0){
		eprintf("currentset == 0");
		return axisRotation;
	}
	
	int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
	//Preveious location, used for acceleration
	Vector3 tmp0;
	tmp0[0] = atoms[mycurrent-1][(myAtom * 4)];
	tmp0[1] = atoms[mycurrent-1][(myAtom * 4) + 1];
	tmp0[2] = atoms[mycurrent-1][(myAtom * 4) + 2];

	// current location
	Vector3 tmp1;
	tmp1[0] = atoms[mycurrent][(myAtom * 4)];
	tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
	tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];

	//next location, used for velocity and acceleration
	Vector3 tmp2;
	tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
	tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
	tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];


	//rearrange vectors from xyz file
	Vector3 tmp_swap;
	tmp_swap = tmp0;
	tmp0.set(-tmp_swap[0],-tmp_swap[2],tmp_swap[1]);
	tmp_swap = tmp1;
	tmp1.set(-tmp_swap[0],-tmp_swap[2],tmp_swap[1]);
	tmp_swap = tmp2;
	tmp2.set(-tmp_swap[0],-tmp_swap[2],tmp_swap[1]);

	// Initialize Vectors
	Vector3 velocity;
	Vector3 acceleration;
	Vector3 velCrossAccel; // velocity x acceleration
	Vector3 xAxis; // perpendicualr to velocity and crossproduct
 
	velocity = tmp2 - tmp1;
	acceleration = -(tmp2 - tmp0);
	velCrossAccel = velocity.cross(acceleration); 
	xAxis = velCrossAccel.cross(velocity);

	//Normalize for Rotation
	if(velocity!=Vector3(0,0,0))
		velocity.normalize();
	if(velCrossAccel!=Vector3(0,0,0))
		velCrossAccel.normalize();
	if(xAxis!=Vector3(0,0,0))
		xAxis.normalize();

	axisRotation.set(
		xAxis[0],xAxis[1],xAxis[2],0,
		velCrossAccel[0],velCrossAccel[1],velCrossAccel[2],0,
		velocity[0],velocity[1],velocity[2],0,	 
		0,0,0,1);
	//axisRotation = axisRotation*mat_tmp;
	axisRotation = mat_tmp*(axisRotation.invert());
	
	return axisRotation;

}

Matrix4 getPartialRotation(int currentset, int TIMESTEPS,int currentAtomtrajectoryIndex, 
						   std::vector<int> atomtrajectories, float **atoms,const Matrix4& m_mat4HMDPose){
	Matrix4 axisRotation;
	axisRotation.identity();
	
	Matrix4 mat_tmp = m_mat4HMDPose;
	mat_tmp.invert();

	Vector3 tmp0;
	tmp0 = mat_tmp * Vector3(1,0,0);
	
	int mycurrent = currentset;
	if (currentset == TIMESTEPS - 1){
		 mycurrent = currentset - 1;
	}
	int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
	//Preveious location, used for acceleration


	// current location
	Vector3 tmp1;
	tmp1[0] = atoms[mycurrent][(myAtom * 4)];
	tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
	tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];

	//next location, used for velocity and acceleration
	Vector3 tmp2;
	tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
	tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
	tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];


	//rearrange vectors from xyz file
	Vector3 tmp_swap;

	tmp_swap = tmp1;
	tmp1.set(-tmp_swap[0],-tmp_swap[2],tmp_swap[1]);
	tmp_swap = tmp2;
	tmp2.set(-tmp_swap[0],-tmp_swap[2],tmp_swap[1]);

	// Initialize Vectors
	Vector3 velocity;
	Vector3 uoCross; // velocity x User's x-axis (left to right)
	Vector3 crossCross; // perpendicualr to velocity and cross
 
	velocity = tmp2 - tmp1;

	uoCross = velocity.cross(tmp0); 

	if(uoCross==Vector3(0,0,0)){
		tmp0 = mat_tmp * Vector3(0,0,1);
		uoCross = velocity.cross(tmp0);
	}

	crossCross = uoCross.cross(velocity);

	if(velocity!=Vector3(0,0,0))
		velocity.normalize();
	if(uoCross!=Vector3(0,0,0))
		uoCross.normalize();
	if(crossCross!=Vector3(0,0,0))
		crossCross.normalize();

	axisRotation.set(
		crossCross[0],crossCross[1],crossCross[2],0,
		uoCross[0],uoCross[1],uoCross[2],0,
		velocity[0],velocity[1],velocity[2],0,	 
		0,0,0,1);
	//axisRotation = axisRotation*mat_tmp;
	axisRotation = mat_tmp*(axisRotation.invert());
	
	return axisRotation;

}

Vector3 jumpToAtomFullRotation(int currentset, std::vector<int> atomtrajectories,int currentAtomtrajectoryIndex, float **atoms){
    int mycurrent = currentset;
	Vector3 tmp;
	tmp.set(0,0,0);
	assert (atomtrajectories.size() > 0);

		int myAtom = atomtrajectories[0];
		if(currentAtomtrajectoryIndex < atomtrajectories.size()-1 )
			myAtom = atomtrajectories[currentAtomtrajectoryIndex];

		tmp[0] = -atoms[mycurrent][(myAtom * 4)] ; //change to for loop
		tmp[1] = -atoms[mycurrent][(myAtom * 4) + 2]; // y & z are flipped
		tmp[2] = atoms[mycurrent][(myAtom * 4) + 1];

		return tmp;
}      

//jumps behind particle position depending on particle position(t+1)
//is used by partial and full rotation
Vector3 jumpBehindAtomFullRotation(int currentset, float atomScaling, int currentAtomtrajectoryIndex, int TIMESTEPS, std::vector<int> atomtrajectories, 
					   float **atoms, int distanceToAtom){
	int mycurrent = currentset;
	int next = currentset+1;
	if ( currentset == TIMESTEPS-1){
	    mycurrent = currentset - 1;
		next = currentset;
	}
	assert (atomtrajectories.size() > 0);
		int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
		float aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
		//eprintf("atomRadius = %f", atomRadius);
		Vector3 a; //current location of atom
		Vector3 b; // t+1 of atom

		// ||b-a|| atomRad*(-2) + a

		a[0] = atoms[mycurrent][(myAtom * 4)];
		a[1] = atoms[mycurrent][(myAtom * 4) + 1]; // y & z are flipped
		a[2] = atoms[mycurrent][(myAtom * 4) + 2];

		b[0] = atoms[next][myAtom * 4];
		b[1] = atoms[next][(myAtom * 4) + 1];
		b[2] = atoms[next][(myAtom * 4) + 2];
		b-= a;
		//check
		if( b!=Vector3(0,0,0) )
			b.normalize();

		b *=-(10+distanceToAtom)*aRad*atomScaling;
		a +=(b);
		b = a;
		
		a[0] = -b[0] ;
		a[1] = -b[2] ; // y & z are flipped
		a[2] = b[1];
		
		return a;

	}

// jump behind and above particle
Vector3 floatAboveFullRotation(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms, int distanceToAtom){

	int mycurrent = currentset;

	if ( currentset == TIMESTEPS-1){
	   mycurrent = currentset - 1;
	}else if(currentset == 0){
	   mycurrent = currentset+1;
	}
	assert (atomtrajectories.size() > 0);
		int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
		float aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
		float amountUp;
		
		//eprintf("atomRadius = %f", atomRadius);
		Vector3 tmp0;
		Vector3 tmp1;
		Vector3 tmp2;
		Vector3 acceleration; 
		Vector3 velocity; 
		Vector3 crossproduct;

		tmp0[0] = -atoms[mycurrent-1][(myAtom * 4)] ; 
		tmp0[1] = -atoms[mycurrent-1][(myAtom * 4) + 2]; // y & z are flipped
		tmp0[2] = atoms[mycurrent-1][(myAtom * 4) + 1];

		tmp1[0] = -atoms[mycurrent][(myAtom * 4)] ; 
		tmp1[1] = -atoms[mycurrent][(myAtom * 4) + 2]; // y & z are flipped
		tmp1[2] = atoms[mycurrent][(myAtom * 4) + 1];

		tmp2[0] = -atoms[mycurrent+1][(myAtom * 4)] ; 
		tmp2[1] = -atoms[mycurrent+1][(myAtom * 4) + 2]; // y & z are flipped
		tmp2[2] = atoms[mycurrent+1][(myAtom * 4) + 1];
		
		//velocity
		velocity = tmp2-tmp1;
		acceleration = tmp2-tmp0;
		crossproduct = velocity.cross(acceleration);
		//check
		if( velocity!=Vector3(0,0,0) )
			velocity.normalize();
		if( acceleration!=Vector3(0,0,0) )
			acceleration.normalize();
		if(crossproduct != Vector3(0,0,0))
			crossproduct.normalize();
		
		amountUp = (3+distanceToAtom*0.25)*atomScaling*aRad;
		crossproduct *= amountUp;
		velocity *=-(10+distanceToAtom)*aRad*atomScaling;
		
		
		tmp1 = tmp1 + velocity + crossproduct;
		
		return tmp1;
}


//used for getPartialRotation
Vector3 floatAbovePartialRotation(int currentset, float atomScaling,int currentAtomtrajectoryIndex, int TIMESTEPS,
	std::vector<int> atomtrajectories, float **atoms, const Matrix4& m_mat4HMDPose, int distanceToAtom){

	int mycurrent = currentset;
	Matrix4 mat_tmp = m_mat4HMDPose;
	mat_tmp.invert();

	Vector3 tmp0 = mat_tmp * Vector3(1,0,0);

	if ( currentset == TIMESTEPS-1){
	   mycurrent = currentset - 1;
	}else if(currentset == 0){
	   mycurrent = currentset+1;
	}
	assert (atomtrajectories.size() > 0);
		int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
		float aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
		float amountUp;
		
		//eprintf("atomRadius = %f", atomRadius);

		Vector3 tmp1;
		Vector3 tmp2;
		Vector3 velocity;
		Vector3 uoCross; // velocity x User's x-axis (left to right)


		tmp1[0] = -atoms[mycurrent][(myAtom * 4)] ; 
		tmp1[1] = -atoms[mycurrent][(myAtom * 4) + 2]; // y & z are flipped
		tmp1[2] = atoms[mycurrent][(myAtom * 4) + 1];

		tmp2[0] = -atoms[mycurrent+1][(myAtom * 4)] ; 
		tmp2[1] = -atoms[mycurrent+1][(myAtom * 4) + 2]; // y & z are flipped
		tmp2[2] = atoms[mycurrent+1][(myAtom * 4) + 1];
		
		//velocity
		velocity = tmp2-tmp1;
		uoCross = velocity.cross(tmp0);
		//check
		if(uoCross == Vector3(0,0,0)){
			tmp0 = mat_tmp * Vector3(1,0,0);
			uoCross = velocity.cross(tmp0);
		}


		if( velocity!=Vector3(0,0,0) )
			velocity.normalize();
		if( uoCross!=Vector3(0,0,0) )
			uoCross.normalize();

		
		amountUp = -(3+distanceToAtom*0.25)*atomScaling*aRad;
		uoCross *= amountUp;
		velocity *=-(10+distanceToAtom)*aRad*atomScaling;
		
		
		tmp1 = tmp1 + velocity + uoCross;
		
		return tmp1;
}

//check if next texture is correct
int checkdpadMode(int dPadMode, int currentdpadMode){
	int currentMode = currentdpadMode;
	int tmp = dPadMode;

	//Atoms all/single
	if(currentMode == 0){
		if(tmp<0)
			tmp=0;
		else if(tmp>1)
			tmp=1;
	//trajectories all/single
	}else if(currentMode == 1){
		if(tmp<2)
			tmp=2;
		else if(tmp>3)
			tmp=3;
	//change current trajectory
	}else if(currentMode == 2){
		if(tmp!=4)
			tmp=4;	
	//camera Modes
	}else if(currentMode == 3){
		if(tmp<5)
			tmp=5;
		else if(tmp>10)
			tmp=10;
	//atomVectors, arrows
	}else if(currentMode == 4){
		if(tmp<11)
			tmp=11;
		else if(tmp>17)
			tmp=17;
	//distance
		/*textures are at 18 19 20, but distance ranges from 0 to 100, 10,50,100*/
	}else if(currentMode == 5){
		if(tmp<=10)
			tmp=18;
		else if(tmp<=50)
			tmp=19;
		else
			tmp=20;
	//video speed
	}else if(currentMode == 6){
		if(tmp<21)
			tmp=21;
		else if(tmp>23)
			tmp=23;
	}

	return tmp;
}

int checkSimpledpadMode(int simpleMode, int currentSimpleMode){
	int currentMode = currentSimpleMode;
	int tmp = simpleMode;

	//Atoms all/single
	if(currentMode == 0){
		if(tmp<0)
			tmp=0;
		else if(tmp>1)
			tmp=1;
	//trajectories all/single
	}else if(currentMode == 1){
		if(tmp<2)
			tmp=2;
		else if(tmp>3)
			tmp=3;
	//change current trajectory
	}else if(currentMode == 2){
		if(tmp!=4)
			tmp=4;	
	//Zoom
	}else if(currentMode == 3){
		tmp=20;
	//Pick Particle
	}else if(currentMode == 4){
		tmp = 5;
	//MoveIsos
	}else if(currentMode == 5){
			tmp=6;
	//Vectors
	}else if(currentMode == 6){
		if(tmp<7)
			tmp=7;
		else if(tmp>13)
			tmp=13;
	//videoSpeed
	}else if(currentMode == 7){
		if(tmp<14)
			tmp=14;
		else if(tmp>16)
			tmp=16;
	//Camera
	}else if(currentMode == 8){
		if(tmp<18)
			tmp=18;
		else if(tmp>19)
			tmp=19;
	}

	return tmp;
}

//cehmistry
//moves with particle positition
Vector3 chemistryJump(int currentset, int currentAtomtrajectoryIndex, int TIMESTEPS, std::vector<int> atomtrajectories, 
					   float **atoms){
	int mycurrent = currentset;
	if ( currentset == TIMESTEPS-1){
	    mycurrent = currentset - 1;
	}
	assert (atomtrajectories.size() > 0);
		int myAtom = atomtrajectories[currentAtomtrajectoryIndex];
		Vector3 a; //current location of atom
		Vector3 b; // t+1 of atom

		// ||b-a|| atomRad*(-2) + a

		a[0] = atoms[mycurrent][(myAtom * 4)];
		a[1] = atoms[mycurrent][(myAtom * 4) + 1];
		a[2] = atoms[mycurrent][(myAtom * 4) + 2];

		
		b[0] = atoms[mycurrent+1][myAtom * 4];
		b[1] = atoms[mycurrent+1][(myAtom * 4) + 1];
		b[2] = atoms[mycurrent+1][(myAtom * 4) + 2];
		
		a = b-a;
		b = a;
		
		a[0] = -b[0] ;
		a[1] = -b[2] ; // y & z are flipped
		a[2] = b[1];
		//eprintf("Vector UserPosition: %f %f %f ", a[0], a[1], a[2]);
		
		return a;

	}

//get max(abs(x,y,z) ) and return e.g. (1,0,0) or (0,-1,0)
//controls are reversed
// box scaling "tmp *= (2*M_PI);"
Vector3 getMinMaxCrontrollerOrientation(Vector3 controller){
	float x = controller[0];
	float y = controller[1];
	float z = controller[2];
	Vector3 tmp = Vector3(-1,-1,-1);
	if(x < 0)
		tmp[0] = 1;
	if(y < 0)
		tmp[1] = 1;
	if(z < 0)
		tmp[2] = 1;
	tmp *= (2*M_PI);
	if(abs(x)>abs(y)&&abs(x)>abs(z))
		return Vector3(tmp[0],0,0);
	if(abs(y)>abs(x)&&abs(y)>abs(z))
		return Vector3(0,tmp[1],0);
	if(abs(z)>abs(x)&&abs(z)>abs(y))
		return Vector3(0,0,tmp[2]);
	else
		return Vector3(0,0,0);
}

int* userposToTexture(int x, int y, int z){
	int *posXYZ = new int[16];
	for(int i=0;i<16;i++){
		posXYZ[i]=15;
	}
	//dots:
	posXYZ[2]=11;
	posXYZ[7]=11;
	posXYZ[12]=11;
	//black space at the end
	posXYZ[15]=12;
	// x
	posXYZ[0]=13; //signum +
	if(x <0){
		x *= -1;
		posXYZ[0]=10; // change signum to -
	}
	if(x<1000){
		posXYZ[4] = x % 10;
		x /= 10;
		posXYZ[3] = x % 10;
		x /= 10;
		posXYZ[1] = x % 10;
	}
	//y
	posXYZ[5]=13; //signum +
	if(y <0){
		y *= -1;
		posXYZ[5]=10; // change signum to -
	}
	if(y<1000){
		posXYZ[9] = y % 10;
		y /= 10;
		posXYZ[8] = y % 10;
		y /= 10;
		posXYZ[6] = y % 10;
	}
	//z
	posXYZ[10]=13; //signum +
	if(z <0){
		z *= -1;
		posXYZ[10]=10; // change signum to -
	}
	if(z<1000){
		posXYZ[14] = z % 10;
		z /= 10;
		posXYZ[13] = z % 10;
		z /= 10;
		posXYZ[11] = z % 10;
	}
	

return posXYZ;
}