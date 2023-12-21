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


#include "atomVectors.h"
#include <array>


#include "NOMADVRLib/CompileGLShader.h"

#include "shared/Vectors.h"

#include "NOMADVRLib/MyGL.h"
#include "NOMADVRLib/eprintf.h"
#include "NOMADVRLib/atoms.hpp"
#include "NOMADVRLib/atomsGL.h"

#include "NOMADVRLib/ConfigFile.h"

//png
#include "shared\lodepng.h"
#include "LoadPNG.h"


#define arrow_vertex_number 6 // indexBuffer for each arrow
#define arrow_vertexdata_size 18  //Each arrow has arrow_vertex_number (6) Vertices with 3 components (xyz) each
#define arrow_line_pair_points 10 //to draw an arrow we need 5 pairs with 2 points each

//moved to ConfigFile.h/cpp
//#define arrowScalingVelocity (1.0/0.016) //scaling for arrow size
//#define arrowScalingAcceleration (1.0/0.016)
//#define arrowScalingCross (1.0/0.016)

#define textureVertSize 864 // 9 dimensions, 4 points for two triangles, 24 textures
#define textureBufferSize 144 //3 points for triangle, 2 triangles per texture, 24 textures


float* getVelocities(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms) {
	float* velocityVector = new float[arrow_vertexdata_size * totalatoms]; // (float*)malloc(sizeof(float)*totalatoms*18); 
	int mycurrent = 0;
	int myAtom = 0;
	int sumAtoms = 0;
	float aRad;

	// Time[0..t]{Atoms[0..totalatoms]}
	for (int i = 0; i < TIMESTEPS; i++){
		mycurrent = i;
		for (int myAtom = 0; myAtom < (i==0?numAtoms[i]:numAtoms[i]-numAtoms[i-1]); myAtom++){
			//get atomRaduis
			aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);
			
			//Vector tmp1 = r, starting Point
			Vector3 tmp1;
			tmp1[0] = atoms[mycurrent][(myAtom * 4)];
			tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
			tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];
			// Check if next Timestep exists, if not tmp2=tmp1
			Vector3 tmp2;
			if (mycurrent == TIMESTEPS - 1){
				tmp2 = tmp1;
			}
			else{
				tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
				tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
				tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];
			}
			// tmp2 = v, direction Vector
			tmp2 = tmp2 - tmp1;
			// add atomRadius*scaling
			if(tmp2!=Vector3(0,0,0) ){
				Vector3 norm_vel = tmp2;//.normalize();
				norm_vel.normalize();
				if(arrowScalingVelocity < 0){
					tmp2 += norm_vel * 4 * atomScaling*aRad;
				} else {
					tmp2 *= arrowScalingVelocity;
				}
				
			}
			// B = r + v, vector head
			Vector3 B;
			B = tmp1 + tmp2;

			// E = r + v*0.7
			Vector3 E;
			E = tmp1 + (tmp2*0.7);
			// if v_x > 0.5
			// S = tmp5, make sure tmp5 is perpendicular to direction Vector tmp2
			Vector3 tmp5;
			if (tmp2[0] > 0.5 || tmp2[0] < -0.5){
				tmp5.set(0, 0, 1);	
			}
			else{
				tmp5.set(1, 0, 0);
			}

			// tmp5 = v x S = N
			if(tmp2 == Vector3(0,0,0) )
				tmp2.set(1,1,1);
			
			tmp5 = tmp5.cross(tmp2);

			tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E1 = E + tmp5;
			Vector3 E2 = E - tmp5;

			// v x N = M = tmp5
			tmp5 = tmp5.cross(tmp2);

			tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E3 = E + tmp5;
			Vector3 E4 = E - tmp5;


			//fill velocityVector
			// Vector array of tmp1,B,E1,E2,E3,E4
			for (int k = 0; k < 3; k++){
			velocityVector[k + sumAtoms + myAtom*arrow_vertexdata_size] = tmp1[k];
			velocityVector[k + 3 + sumAtoms + myAtom*arrow_vertexdata_size] = B[k];
			velocityVector[k+6+ sumAtoms + myAtom*arrow_vertexdata_size] = E1[k];
			velocityVector[k+9+ sumAtoms + myAtom*arrow_vertexdata_size] = E2[k];
			velocityVector[k+12+ sumAtoms + myAtom*arrow_vertexdata_size] = E3[k];
			velocityVector[k+15+ sumAtoms + myAtom*arrow_vertexdata_size] = E4[k];
			}
			
		}
		sumAtoms = numAtoms[i]*arrow_vertexdata_size;
	}
	//Debug
	/*
	for(int i=0; i< 54	;i++){
	eprintf(" current atom i = %d \nvelocityVector[i+0-2]: %f %f %f \n velocityVector[i+3-5]: %f %f %f \n"
	"velocityVector[i+6-8]: %f %f %f \n velocityVector[i+9-11]: %f %f %f \n"
		"velocityVector[i+12-14]: %f %f %f \n velocityVector[i+15-17]: %f %f %f", 
		i,velocityVector[i*18],velocityVector[i*18+1],velocityVector[i*18+2]
	, velocityVector[i*18+3],velocityVector[i*18+4],velocityVector[i*18+5]
	, velocityVector[i*18+6],velocityVector[i*18+7],velocityVector[i*18+8]
	, velocityVector[i*18+9],velocityVector[i*18+10],velocityVector[i*18+11]
	, velocityVector[i*18+12],velocityVector[i*18+13],velocityVector[i*18+14]
	, velocityVector[i*18+15],velocityVector[i*18+16],velocityVector[i*18+17]);
	}
	*/
	return velocityVector;
}


float* getAccelerations(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms) {
	float *accelerationVector = new float[arrow_vertexdata_size * totalatoms]; //[timestep 0 ][x0 y1 z2] timestep[]
	int mycurrent = 0;
	int myAtom = 0;
	int sumAtoms = 0;
	float aRad;
	// get correct loops
	for (int i = 0; i < TIMESTEPS; i++){
		mycurrent = i;
		
		for (int myAtom = 0; myAtom < (i==0?numAtoms[i]:numAtoms[i]-numAtoms[i-1]); myAtom++){
			//get atomRaduis
			aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);

			//Vector A = tmp1 x(t) = r 
			Vector3 tmp1;
			tmp1[0] = atoms[mycurrent][(myAtom * 4)];
			tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
			tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];
			// v0 = tmp0
			Vector3 tmp0;
			Vector3 tmp2;
			if (mycurrent == 0 || mycurrent == TIMESTEPS - 1){
				tmp0 = tmp1;
				tmp2 = tmp1;
			}
			else{	
			tmp0[0] = atoms[mycurrent-1][(myAtom * 4)];
			tmp0[1] = atoms[mycurrent-1][(myAtom * 4) + 1];
			tmp0[2] = atoms[mycurrent-1][(myAtom * 4) + 2];

			tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
			tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
			tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];
			}
			// v0 = tmp2
			tmp0 = tmp1 - tmp0;
			
			// v1 = tmp2
			tmp2 = tmp2 - tmp1;
			
			// a = v1 - v0
			Vector3 acceleration;
			acceleration = tmp2 - tmp0;

			// add atomRadius*scaling
			if(acceleration!=Vector3(0,0,0) ){
				Vector3 norm_accel = acceleration;
				norm_accel.normalize();
				if(arrowScalingAcceleration < 0){ 
					acceleration += norm_accel * 4 * atomScaling*aRad; 
				} else {
					acceleration *= arrowScalingAcceleration;
				}
			}
			


			// B = r + acceleration
			Vector3 B;
				B = tmp1 + acceleration;
			// E = r + v*0.7
			Vector3 E;
			E = tmp1 + acceleration*0.7;
			// if v_x > 0.5
			// S = tmp5
			Vector3 tmp5;
			if (tmp2[0] > 0.5 || tmp2[0] < -0.5)
			{
				tmp5.set(0, 0, 1);	
			}
			else{
				tmp5.set(1, 0, 0);
			}

			// tmp5 = acceleration x S = N
			tmp5 = tmp5.cross(acceleration);

			if(tmp5 != Vector3(0,0,0) )
				tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E1 = E + tmp5;
			Vector3 E2 = E - tmp5;

			// v x N = M = tmp5
			tmp5 = tmp5.cross(acceleration);

			if(tmp5 != Vector3(0,0,0) )
				tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E3 = E + tmp5;
			Vector3 E4 = E - tmp5;
			//fill accelerationVector[time][Atom][Values]
			for (int k = 0; k < 3; k++){
				accelerationVector[k + sumAtoms + myAtom*arrow_vertexdata_size] = tmp1[k];
				accelerationVector[k + 3 + sumAtoms + myAtom*arrow_vertexdata_size] = B[k];
				accelerationVector[k+6+ sumAtoms + myAtom*arrow_vertexdata_size] = E1[k];
				accelerationVector[k+9+ sumAtoms + myAtom*arrow_vertexdata_size] = E2[k];
				accelerationVector[k+12+ sumAtoms + myAtom*arrow_vertexdata_size] = E3[k];
				accelerationVector[k+15+ sumAtoms + myAtom*arrow_vertexdata_size] = E4[k];
			}
			// Vector array of A(tmp1),B,E1,E2,E3,E4
			
		}
		sumAtoms = numAtoms[i]*arrow_vertexdata_size;
	}
	//Debug
	/*
	for(int i=0; i< 54	;i++){
	eprintf(" current atom i = %d \naccelerationVector[i+0-2]: %f %f %f \naccelerationVector[i+3-5]: %f %f %f \n"
	"accelerationVector[i+6-8]: %f %f %f \n accelerationVector[i+9-11]: %f %f %f \n"
		"accelerationVector[i+12-14]: %f %f %f \n accelerationVector[i+15-17]: %f %f %f", 
		i,accelerationVector[i*18],accelerationVector[i*18+1],accelerationVector[i*18+2]
	, accelerationVector[i*18+3],accelerationVector[i*18+4],accelerationVector[i*18+5]
	, accelerationVector[i*18+6],accelerationVector[i*18+7],accelerationVector[i*18+8]
	, accelerationVector[i*18+9],accelerationVector[i*18+10],accelerationVector[i*18+11]
	, accelerationVector[i*18+12],accelerationVector[i*18+13],accelerationVector[i*18+14]
	, accelerationVector[i*18+15],accelerationVector[i*18+16],accelerationVector[i*18+17]);
	}
	*/
	return accelerationVector;
}


float* getVeloCrossAccel(int TIMESTEPS, float **atoms, int *numAtoms,int totalatoms) {
	float *crossVector= new float[arrow_vertexdata_size * totalatoms]; //[timestep 0 ][x0 y1 z2] timestep[]
	int mycurrent = 0;
	int myAtom = 0;
	int sumAtoms = 0;
	float aRad;
	// get correct loops
	for (int i = 0; i < TIMESTEPS; i++){
		mycurrent = i;
		
		for (int myAtom = 0; myAtom < (i==0?numAtoms[i]:numAtoms[i]-numAtoms[i-1]); myAtom++){
			//get atomRaduis
			aRad = atomRadius(atoms[mycurrent][(myAtom * 4) + 3]);

			//Vector A = tmp1 x(t) = r 
			Vector3 tmp1;
			tmp1[0] = atoms[mycurrent][(myAtom * 4)];
			tmp1[1] = atoms[mycurrent][(myAtom * 4) + 1];
			tmp1[2] = atoms[mycurrent][(myAtom * 4) + 2];
			// v0 = tmp0
			Vector3 tmp0;
			Vector3 tmp2;
			if (mycurrent == 0 || mycurrent == TIMESTEPS - 1){
				tmp0 = tmp1;
				tmp2 = tmp1;
			}
			else{	
			tmp0[0] = atoms[mycurrent-1][(myAtom * 4)];
			tmp0[1] = atoms[mycurrent-1][(myAtom * 4) + 1];
			tmp0[2] = atoms[mycurrent-1][(myAtom * 4) + 2];

			tmp2[0] = atoms[mycurrent + 1][(myAtom * 4)];
			tmp2[1] = atoms[mycurrent + 1][(myAtom * 4) + 1];
			tmp2[2] = atoms[mycurrent + 1][(myAtom * 4) + 2];
			}
			// v0 = tmp0
			tmp0 = tmp1 - tmp0;

			// v1 = tmp2
			tmp2 = tmp2 - tmp1;
			// v & a = v1 - v0
			Vector3 velocity;
			velocity = tmp2;
			Vector3 acceleration;
			acceleration = tmp2 - tmp0;

			//crossproduct between velocity and acceleration
			Vector3 vel_cross_accel;
			vel_cross_accel= velocity.cross(acceleration); 
			// add atomRadius*scaling
			if(vel_cross_accel!=Vector3(0,0,0) ){

				Vector3 norm_crossp = vel_cross_accel;
				norm_crossp.normalize();
				if(arrowScalingCross < 0){
					vel_cross_accel += norm_crossp * 4 * atomScaling*aRad;
				} else { 
					vel_cross_accel *= arrowScalingCross;
				}
			}

			// B = r + crossproduct
			Vector3 B;
			B = tmp1 + vel_cross_accel;
			// E = r + v*0.7
			Vector3 E;
			E = tmp1 + vel_cross_accel*0.7;
			// if v_x > 0.5
			// S = tmp5
			Vector3 tmp5;
			if (tmp2[0] > 0.5 || tmp2[0] < -0.5)
			{
				tmp5.set(0, 0, 1);	
			}
			else{
				tmp5.set(1, 0, 0);
			}

			// tmp5 = vel_cross_accel x S = N
			tmp5 = tmp5.cross(vel_cross_accel);

			if(tmp5 != Vector3(0,0,0) )
				tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E1 = E + tmp5;
			Vector3 E2 = E - tmp5;

			// v x N = M = tmp5
			tmp5 = tmp5.cross(vel_cross_accel);

			if(tmp5 != Vector3(0,0,0) )
				tmp5.normalize();
			tmp5 *=aRad*atomScaling*1.2;
			Vector3 E3 = E + tmp5;
			Vector3 E4 = E - tmp5;
			//fill accelerationVector[time][Atom][Values]
			for (int k = 0; k < 3; k++){
				crossVector[k + sumAtoms + myAtom*arrow_vertexdata_size] = tmp1[k];
				crossVector[k + 3 + sumAtoms + myAtom*arrow_vertexdata_size] = B[k];
				crossVector[k+6+ sumAtoms + myAtom*arrow_vertexdata_size] = E1[k];
				crossVector[k+9+ sumAtoms + myAtom*arrow_vertexdata_size] = E2[k];
				crossVector[k+12+ sumAtoms + myAtom*arrow_vertexdata_size] = E3[k];
				crossVector[k+15+ sumAtoms + myAtom*arrow_vertexdata_size] = E4[k];
			}
			// Vector array of A(tmp1),B,E1,E2,E3,E4
			
		}
		sumAtoms = numAtoms[i]*arrow_vertexdata_size;
	}
	//Debug
	/*
	for(int i=0; i< 54	;i++){
	eprintf(" current atom i = %d \ncrossVector[i+0-2]: %f %f %f \ncrossVector[i+3-5]: %f %f %f \n"
	"crossVector[i+6-8]: %f %f %f \n crossVector[i+9-11]: %f %f %f \n"
		"crossVector[i+12-14]: %f %f %f \n crossVector[i+15-17]: %f %f %f", 
		i,crossVector[i*18],crossVector[i*18+1],crossVector[i*18+2]
	, crossVector[i*18+3],crossVector[i*18+4],crossVector[i*18+5]
	, crossVector[i*18+6],crossVector[i*18+7],crossVector[i*18+8]
	,crossVector[i*18+9],crossVector[i*18+10],crossVector[i*18+11]
	,crossVector[i*18+12],crossVector[i*18+13],crossVector[i*18+14]
	,crossVector[i*18+15],crossVector[i*18+16],crossVector[i*18+17]);
	}
	*/
	return crossVector;
}



GLuint loadAtomVectorsGL::SetupAtomVelocity()
{
	if (!numAtoms){
		return glGetError();}
	//rgh FIXME: put this all in the same vao

	//http://prideout.net/blog/?p=48 //public domain code
	//xyz u=atom type ; 4 floats
	e=0;
	totalatoms = numAtoms[TIMESTEPS-1];
	
	velocityVectors = new float[18 * totalatoms];
	velocityVectors = getVelocities(TIMESTEPS, atoms, numAtoms,totalatoms);

	accelerationVectors = new float[18 * totalatoms];
	accelerationVectors = getAccelerations(TIMESTEPS, atoms, numAtoms,totalatoms);

	veloCrossAccel = new float[18 * totalatoms];
	veloCrossAccel = getVeloCrossAccel(TIMESTEPS, atoms, numAtoms,totalatoms);

	/*Debug
	for(int i=0; i< 54 ;i++){
	eprintf("Current atom i: %d\n velocityVector[i+0-2]: %f %f %f \n velocityVector[i+3-5]: %f %f %f \n" 
		"velocityVector[i+6-8]: %f %f %f \n velocityVector[i+9-11]: %f %f %f \n"
		"velocityVector[i+12-14]: %f %f %f \n velocityVector[i+15-17]: %f %f %f \n In SetupAvelocityVectors", 
		i,velocityVectors[i*18],velocityVectors[i*18+1],velocityVectors[i*18+2]
	, velocityVectors[i*18+3],velocityVectors[i*18+4],velocityVectors[i*18+5]
	, velocityVectors[i*18+6],velocityVectors[i*18+7],velocityVectors[i*18+8]
	, velocityVectors[i*18+9],velocityVectors[i*18+10],velocityVectors[i*18+11]
	, velocityVectors[i*18+12],velocityVectors[i*18+13],velocityVectors[i*18+14]
	, velocityVectors[i*18+15],velocityVectors[i*18+16],velocityVectors[i*18+17]);
	}
	*/
	
	
	//indexBuffer x coords, pairs of 5 
	int* indexBuffer = new int[arrow_line_pair_points * totalatoms];
	// i 0..totalatoms, n+i*18

	for (int i = 0; i < totalatoms;i++){
	indexBuffer[i * arrow_line_pair_points + 0] = 0 + (i * arrow_vertex_number); 
	indexBuffer[i * arrow_line_pair_points + 1] = 1 + (i * arrow_vertex_number);
	indexBuffer[i * arrow_line_pair_points + 2] = 1 + (i * arrow_vertex_number); 
	indexBuffer[i * arrow_line_pair_points + 3] = 2 + (i * arrow_vertex_number);
	indexBuffer[i * arrow_line_pair_points + 4] = 1 + (i * arrow_vertex_number); 
	indexBuffer[i * arrow_line_pair_points + 5] = 3 + (i * arrow_vertex_number);
	indexBuffer[i * arrow_line_pair_points + 6] = 1 + (i * arrow_vertex_number); 
	indexBuffer[i * arrow_line_pair_points + 7] = 4 + (i * arrow_vertex_number);
	indexBuffer[i * arrow_line_pair_points + 8] = 1 + (i * arrow_vertex_number); 
	indexBuffer[i * arrow_line_pair_points + 9] = 5 + (i * arrow_vertex_number);
	}

	
	/*DEBUG
	
	for (int i = 0; i < (totalatoms*10) ;i+=(10) ){
	eprintf("IndexBuffer i %d, indexBuffer[i + 0] = %d , \n indexBuffer[i + 1] = %d\n indexBuffer[i + 2] = %d , indexBuffer[i + 3] = %d \n"
		"indexBuffer[i + 4] = %d , indexBuffer[i + 5] = %d\n indexBuffer[i + 6] = %d , indexBuffer[i + 7] = %d \n"
		"indexBuffer[i + 8] = %d ,  indexBuffer[i + 9] = %d\n Current Atom %d",
	i, indexBuffer[i + 0], indexBuffer[i + 1],
	indexBuffer[i + 2], indexBuffer[i  + 3],
	indexBuffer[i + 4], indexBuffer[i + 5],
	indexBuffer[i + 6], indexBuffer[i  + 7],
	indexBuffer[i  + 8], indexBuffer[i  + 9],i/10 );
	}

	for (int i = 0; i < totalatoms;i++){
	indexBuffer[i * 10 + 0] = 0 + (i * 18); indexBuffer[i * 10 + 1] = 1 + (i * 18);
	indexBuffer[i * 10 + 2] = 1 + (i * 18); indexBuffer[i * 10 + 3] = 2 + (i * 18);
	indexBuffer[i * 10 + 4] = 1 + (i * 18); indexBuffer[i * 10 + 5] = 3 + (i * 18);
	indexBuffer[i * 10 + 6] = 1 + (i * 18); indexBuffer[i * 10 + 7] = 4 + (i * 18);
	indexBuffer[i * 10 + 8] = 1 + (i * 18); indexBuffer[i * 10 + 9] = 5 + (i * 18);
	}


	for (int i = 0; i < (totalatoms*10);i+=10){
	indexBuffer[i + 0] = 0 + ((i/10) * 6); indexBuffer[i  + 1] = 1 + (i/10 * 6);
	indexBuffer[i + 2] = 1 + (i/10 * 6); indexBuffer[i  + 3] = 2 + (i/10 * 6);
	indexBuffer[i + 4] = 1 + (i/10 * 6); indexBuffer[i  + 5] = 3 + (i/10 * 6);
	indexBuffer[i + 6] = 1 + (i/10 * 6); indexBuffer[i  + 7] = 4 + (i/10 * 6);
	indexBuffer[i + 8] = 1 + (i/10 * 6); indexBuffer[i  + 9] = 5 + (i/10 * 6);
	}

	eprintf("velo[0]=%d, velo[3]=%d, velo[6]=%d, velo[9]=%d, velo[12]=%d, velo[15]=%d,\n velo[18]=%d, velo[21]=%d, velo[24]=%d, velo[27]=%d, velo[30]=%d, velo[33]=%d, ",
		velocityVectors[3],velocityVectors[6],velocityVectors[9],velocityVectors[12],velocityVectors[15],velocityVectors[18],velocityVectors[21],
		velocityVectors[24],velocityVectors[27],velocityVectors[30],velocityVectors[33]);

	
	*/
	

	
	//InitGL
	AtomVAO = new GLuint[3]; //
	AtomVertBuffer = new GLuint[3]; //
	Indices = new GLuint[3]; //

	glGenVertexArrays(3, AtomVAO);
	glGenBuffers(3, AtomVertBuffer);
	glGenBuffers(3, Indices);

	//Velocity Arrows
	glBindVertexArray((AtomVAO)[0]);
	glBindBuffer(GL_ARRAY_BUFFER, (AtomVertBuffer)[0]);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* totalatoms * arrow_vertexdata_size, velocityVectors,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *Indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* arrow_line_pair_points * totalatoms, indexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of Velocity Arrows, l %d\n", e, __LINE__);

	//Acceleration Arrows
	glBindVertexArray((AtomVAO)[1]);
	glBindBuffer(GL_ARRAY_BUFFER, (AtomVertBuffer)[1]);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* totalatoms * arrow_vertexdata_size, accelerationVectors,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *Indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* arrow_line_pair_points * totalatoms, indexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of Acceleration Arrows, l %d\n", e, __LINE__);

	//crossproduct Arrows
	glBindVertexArray((AtomVAO)[2]);
	glBindBuffer(GL_ARRAY_BUFFER, (AtomVertBuffer)[2]);

	glEnableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (const void *)(0));
	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* totalatoms * arrow_vertexdata_size, veloCrossAccel,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *Indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* arrow_line_pair_points * totalatoms, indexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of Crossproduct Arrows, l %d\n", e, __LINE__);

	//clean
	delete[] velocityVectors;
	delete[] accelerationVectors;
	delete[] veloCrossAccel;
	delete[] indexBuffer;

	return e;
}


GLuint loadAtomVectorsGL::SetupTextures(){
	
	int points = 36; // 4 points with 9 dimensions each.
	int tex = 24; //amount of textures

	e = 0;

	//fill vertice buffer
	textureVertices = new float[textureVertSize];
	textureIndexBuffer = new int[textureBufferSize];

	//indices of triangle points (012,023) (456,467) .....
	int tmp = 0; //tmp+=4 after each step
	for(int i = 0; i< tex;i++){ //
		textureIndexBuffer[i*6]  =tmp;
		textureIndexBuffer[i*6+1]=tmp+1;
		textureIndexBuffer[i*6+2]=tmp+2;
		textureIndexBuffer[i*6+3]=tmp;
		textureIndexBuffer[i*6+4]=tmp+2;
		textureIndexBuffer[i*6+5]=tmp+3;
		tmp+=4;
	}

	//most entries of textureVertices are 0
	for(int i = 0; i<textureVertSize;i++){
		textureVertices[i]=0;
	}


	// 21 Triangles to paint
	// 2Dcoordinates are flipped, b/c of rendering in main
	for(int i=0;i < tex;i++){
		//bottom left point
		// realspace x,z = 0
		textureVertices[5+(i*points)] = 1; //texture orientation y axis
		textureVertices[7+(i*points)] = (float)(i+1)/tex;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[8+(i*points)] = 1;
		//top left point
		//realspace x 0,z 1
		textureVertices[11+(i*points)] = 0.1; // realspace z
		textureVertices[14+(i*points)] = 1; //texture orientation y axis
		textureVertices[16+(i*points)] = (float)(i+1)/tex;;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[17+(i*points)] = 0;
		//top right point
		//realspace x 1, z 1
		textureVertices[18+(i*points)] = 0.1; //realspace x
		textureVertices[20+(i*points)] = 0.1; // realspace z
		textureVertices[23+(i*points)] = 1; //texture orientation y axis
		textureVertices[25+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[26+(i*points)] = 0;
		//bottom right point
		//realspace x 1, z 0
		textureVertices[27+(i*points)] = 0.1; //realspace x
		textureVertices[32+(i*points)] = 1; //texture orientation y axis
		textureVertices[34+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[35+(i*points)] = 1;
	}


	//InitGL
	textureVAO = new GLuint[1]; //
	textureBuffer= new GLuint[1]; //
	textureIndices = new GLuint[1]; //

	glGenVertexArrays(1, textureVAO);
	glGenBuffers(1, textureBuffer);
	glGenBuffers(1, textureIndices);

	//Velocity Arrows
	glBindVertexArray(textureVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	int offset=0;

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	// realspace x,y,z,w
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	// Orientation x,y,z
	offset += 4*sizeof(GLfloat);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	//Texture2D coords u,v
	offset += 3*sizeof(GLfloat);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* textureVertSize, textureVertices,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *textureIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* textureBufferSize, textureIndexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData in setupTextures, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of setup Texture, l %d\n", e, __LINE__);

	delete[] textureVertices;
	delete[] textureIndexBuffer;
	
	//setup Textures
	m_iTexture = new GLuint[1];
	/*rgh: moved to a function to be able to use it in the config file*/
	/* Moved to LoadPNG
	glGenTextures(1, m_iTexture);
	glBindTexture(GL_TEXTURE_2D, m_iTexture[0]);
	*/
	//add path to texture.png
	const char * images[] = {
		//Texture 32 Pixels
		//PATH Y:\software\openvr\openvr-0.9.19\samples\bin\

		"Interface_v4_768x32.png"
	};
	/* Moved to LoadPNG
	std::vector<unsigned char> imageRGBA;
	unsigned nImageWidth, nImageHeight;
	unsigned nError = lodepng::decode(imageRGBA, nImageWidth, nImageHeight,
			images[0]);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nImageWidth, nImageHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0]);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//rgh fixme: revise this if texture sampling is too slow
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	
	glBindTexture( GL_TEXTURE_2D, 0 );
	*/
	m_iTexture[0]=LoadPNG(images[0],0);

	return e;
}

GLuint loadAtomVectorsGL::SetupSimpleTextures(){
	
	int points = 36; // 4 points with 9 dimensions each.
	int tex = 21; //amount of textures

	int simpleTextureVertSize = 756;  //4*9*21
	int simpleTextureBufferSize = 126; // 3*2*21

	e = 0;

	//fill vertice buffer
	textureVertices = new float[simpleTextureVertSize];
	textureIndexBuffer = new int[simpleTextureBufferSize];

	//indices of triangle points (012,023) (456,467) .....
	int tmp = 0; //tmp+=4 after each step
	for(int i = 0; i< tex;i++){
		textureIndexBuffer[i*6]  =tmp;
		textureIndexBuffer[i*6+1]=tmp+1;
		textureIndexBuffer[i*6+2]=tmp+2;
		textureIndexBuffer[i*6+3]=tmp;
		textureIndexBuffer[i*6+4]=tmp+2;
		textureIndexBuffer[i*6+5]=tmp+3;
		tmp+=4;
	}

	//most entries of textureVertices are 0
	for(int i = 0; i<simpleTextureVertSize;i++){
		textureVertices[i]=0;
	}


	// 40 Triangles to paint
	// 2Dcoordinates are flipped, because of rendering in main
	for(int i=0;i < tex;i++){
		//bottom left point
		// realspace x,z = 0
		textureVertices[5+(i*points)] = 1; //texture orientation y axis
		textureVertices[7+(i*points)] = (float)(i+1)/tex;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[8+(i*points)] = 1;
		//top left point
		//realspace x 0,z 1
		textureVertices[11+(i*points)] = 0.1; // realspace z
		textureVertices[14+(i*points)] = 1; //texture orientation y axis
		textureVertices[16+(i*points)] = (float)(i+1)/tex;;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[17+(i*points)] = 0;
		//top right point
		//realspace x 1, z 1
		textureVertices[18+(i*points)] = 0.1; //realspace x
		textureVertices[20+(i*points)] = 0.1; // realspace z
		textureVertices[23+(i*points)] = 1; //texture orientation y axis
		textureVertices[25+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[26+(i*points)] = 0;
		//bottom right point
		//realspace x 1, z 0
		textureVertices[27+(i*points)] = 0.1; //realspace x
		textureVertices[32+(i*points)] = 1; //texture orientation y axis
		textureVertices[34+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[35+(i*points)] = 1;
	}


	//InitGL
	textureVAO = new GLuint[1]; //
	textureBuffer= new GLuint[1]; //
	textureIndices = new GLuint[1]; //

	glGenVertexArrays(1, textureVAO);
	glGenBuffers(1, textureBuffer);
	glGenBuffers(1, textureIndices);

	//Velocity Arrows
	glBindVertexArray(textureVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, textureBuffer[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	int offset=0;

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	// realspace x,y,z,w
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	// Orientation x,y,z
	offset += 4*sizeof(GLfloat);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	//Texture2D coords u,v
	offset += 3*sizeof(GLfloat);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* simpleTextureVertSize, textureVertices,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *textureIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* simpleTextureBufferSize, textureIndexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData in setupTextures, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of setup Texture, l %d\n", e, __LINE__);

	delete[] textureVertices;
	delete[] textureIndexBuffer;
	
	//setup Textures
	m_iTexture = new GLuint[1];
	//rgh: made a function
	/* Moved to LoadPNG
	//glGenTextures(1, m_iTexture);
	//glBindTexture(GL_TEXTURE_2D, m_iTexture[0]);
	*/
	//path to texture.png
	//if no path, *.png needs to be where config file is, even for debugging
	const char * images[] = {
		//Texture 32 Pixels
		//PATH Y:\software\openvr\openvr-0.9.19\samples\bin\

		"SimpleInterface.png"
	};
	m_iTexture[0]=LoadPNG(images[0],0);
	/* Moved to LoadPNG
	std::vector<unsigned char> imageRGBA;
	unsigned nImageWidth, nImageHeight;
	unsigned nError = lodepng::decode(imageRGBA, nImageWidth, nImageHeight,
			images[0]);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nImageWidth, nImageHeight,
			0, GL_RGBA, GL_UNSIGNED_BYTE, &imageRGBA[0]);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//rgh fixme: revise this if texture sampling is too slow
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
	


	glBindTexture( GL_TEXTURE_2D, 0 );
	*/
	
	return e;
}

GLuint loadAtomVectorsGL::SetupDigitTextures(){
	
	int points = 36; // 4 points with 9 dimensions each.
	int tex = 16; //amount of textures

	int digitTextureVertSize = 576;  //4*9*16
	int digitTextureBufferSize = 96; // 3*2*16

	e = 0;

	//fill vertice buffer
	textureVertices = new float[digitTextureVertSize];
	textureIndexBuffer = new int[digitTextureBufferSize];

	//indices of triangle points (012,023) (456,467) .....
	int tmp = 0; //tmp+=4 after each step
	for(int i = 0; i< tex;i++){
		textureIndexBuffer[i*6]  =tmp;
		textureIndexBuffer[i*6+1]=tmp+1;
		textureIndexBuffer[i*6+2]=tmp+2;
		textureIndexBuffer[i*6+3]=tmp;
		textureIndexBuffer[i*6+4]=tmp+2;
		textureIndexBuffer[i*6+5]=tmp+3;
		tmp+=4;
	}

	//most entries of textureVertices are 0
	for(int i = 0; i<digitTextureVertSize;i++){
		textureVertices[i]=0;
	}


	// 40 Triangles to paint
	// 2Dcoordinates are flipped, because of rendering in main
	for(int i=0;i < tex;i++){
		//bottom left point
		// realspace x,z = 0
		textureVertices[5+(i*points)] = 1; //texture orientation y axis
		textureVertices[7+(i*points)] = (float)(i+1)/tex;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[8+(i*points)] = 1;
		//top left point
		//realspace x 0,z 1
		textureVertices[11+(i*points)] = 0.1; // realspace z
		textureVertices[14+(i*points)] = 1; //texture orientation y axis
		textureVertices[16+(i*points)] = (float)(i+1)/tex;;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[17+(i*points)] = 0;
		//top right point
		//realspace x 1, z 1
		textureVertices[18+(i*points)] = 0.1; //realspace x
		textureVertices[20+(i*points)] = 0.1; // realspace z
		textureVertices[23+(i*points)] = 1; //texture orientation y axis
		textureVertices[25+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[26+(i*points)] = 0;
		//bottom right point
		//realspace x 1, z 0
		textureVertices[27+(i*points)] = 0.1; //realspace x
		textureVertices[32+(i*points)] = 1; //texture orientation y axis
		textureVertices[34+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[35+(i*points)] = 1;
	}


	//InitGL
	digitVAO = new GLuint[1]; //
	digitBuffer= new GLuint[1]; //
	digitIndices = new GLuint[1]; //

	glGenVertexArrays(1, digitVAO);
	glGenBuffers(1, digitBuffer);
	glGenBuffers(1, digitIndices);

	//Velocity Arrows
	glBindVertexArray(digitVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, digitBuffer[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	int offset=0;

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	// realspace x,y,z,w
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	// Orientation x,y,z
	offset += 4*sizeof(GLfloat);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	//Texture2D coords u,v
	offset += 3*sizeof(GLfloat);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* digitTextureVertSize, textureVertices,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *digitIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* digitTextureBufferSize, textureIndexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData in setupTextures, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of setup Texture, l %d\n", e, __LINE__);

	delete[] textureVertices;
	delete[] textureIndexBuffer;
	
	//setup Textures
	digit_m_iTexture = new GLuint[1];
	//rgh: made a function

	//path to texture.png
	//if no path, *.png needs to be where config file is, even for debugging
	const char * images[] = {
		//Texture 32 Pixels
		//PATH Y:\software\openvr\openvr-0.9.19\samples\bin\

		"digits_64x7_l_blank.png"
	};
	digit_m_iTexture[0]=LoadPNG(images[0],1);
	
	
	return e;

}

GLuint loadAtomVectorsGL::SetupLoadingIsosTexture(){
	
	int points = 36; // 4 points with 9 dimensions each.
	int tex = 1; //amount of textures

	int loadingTextureVertSize = 36;  //4*9
	int loadingTextureBufferSize = 6; // 3*2

	e = 0;

	//fill vertice buffer
	textureVertices = new float[loadingTextureVertSize];
	textureIndexBuffer = new int[loadingTextureBufferSize];

	//indices of triangle points (012,023) (456,467) .....
	int tmp = 0; //tmp+=4 after each step
	for(int i = 0; i< tex;i++){
		textureIndexBuffer[i*6]  =tmp;
		textureIndexBuffer[i*6+1]=tmp+1;
		textureIndexBuffer[i*6+2]=tmp+2;
		textureIndexBuffer[i*6+3]=tmp;
		textureIndexBuffer[i*6+4]=tmp+2;
		textureIndexBuffer[i*6+5]=tmp+3;
		tmp+=4;
	}

	//most entries of textureVertices are 0
	for(int i = 0; i<loadingTextureVertSize;i++){
		textureVertices[i]=0;
	}


	// 40 Triangles to paint
	// 2Dcoordinates are flipped, because of rendering in main
	for(int i=0;i < tex;i++){
		//bottom left point
		// realspace x,z = 0
		textureVertices[5+(i*points)] = 1; //texture orientation y axis
		textureVertices[7+(i*points)] = (float)(i+1)/tex;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[8+(i*points)] = 1;
		//top left point
		//realspace x 0,z 1
		textureVertices[11+(i*points)] = 0.1; // realspace z
		textureVertices[14+(i*points)] = 1; //texture orientation y axis
		textureVertices[16+(i*points)] = (float)(i+1)/tex;;//(float)i/tex; //x,y 2D Coords of texture
		textureVertices[17+(i*points)] = 0;
		//top right point
		//realspace x 1, z 1
		textureVertices[18+(i*points)] = 0.1; //realspace x
		textureVertices[20+(i*points)] = 0.1; // realspace z
		textureVertices[23+(i*points)] = 1; //texture orientation y axis
		textureVertices[25+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[26+(i*points)] = 0;
		//bottom right point
		//realspace x 1, z 0
		textureVertices[27+(i*points)] = 0.1; //realspace x
		textureVertices[32+(i*points)] = 1; //texture orientation y axis
		textureVertices[34+(i*points)] = (float)i/tex;//(float)(i+1)/tex; //x,y 2D Coords of texture
		textureVertices[35+(i*points)] = 1;
	}


	//InitGL
	loadingTextureVAO = new GLuint[1]; //
	loadingTextureBuffer= new GLuint[1]; //
	loadingTextureIndices = new GLuint[1]; //

	glGenVertexArrays(1, loadingTextureVAO);
	glGenBuffers(1, loadingTextureBuffer);
	glGenBuffers(1, loadingTextureIndices);

	//Velocity Arrows
	glBindVertexArray(loadingTextureVAO[0]);
	glBindBuffer(GL_ARRAY_BUFFER, loadingTextureBuffer[0]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	int offset=0;

	e = glGetError();
	if (e != GL_NO_ERROR)
		eprintf("gl error %d, %s %d", e, __FILE__, __LINE__);

	// realspace x,y,z,w
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	// Orientation x,y,z
	offset += 4*sizeof(GLfloat);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	//Texture2D coords u,v
	offset += 3*sizeof(GLfloat);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (const void *)offset);

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)* loadingTextureVertSize, textureVertices,
		GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *loadingTextureIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)* loadingTextureBufferSize, textureIndexBuffer, GL_STATIC_DRAW);

	
	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, glBufferData in setupTextures, l %d\n", e, __LINE__);

	glBindVertexArray(0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	if ((e = glGetError()) != GL_NO_ERROR)
		eprintf("opengl error %d, end of setup Texture, l %d\n", e, __LINE__);

	delete[] textureVertices;
	delete[] textureIndexBuffer;
	
	//setup Textures
	loading_m_iTexture = new GLuint[1];
	//rgh: made a function

	//path to texture.png
	//if no path, *.png needs to be where config file is, even for debugging
	const char * images[] = {
		//Texture 32 Pixels
		//PATH Y:\software\openvr\openvr-0.9.19\samples\bin\

		"LoadingIsos_64x7.png"
	};
	loading_m_iTexture[0]=LoadPNG(images[0],1);
	
	
	return e;

}