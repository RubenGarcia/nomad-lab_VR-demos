/*Uses code from Philip Rideout which uses the public domain license*/
//http://prideout.net/blog/?p=48
/*This license is compatible with Apache 2.0*/

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

#include <math.h>
#include "polyhedron.h"

const int Icosahedron::nFaces=20;
const int Icosahedron::Faces[] = { //20 faces, 60 ints
	2, 1, 0,
	3, 2, 0,
	4, 3, 0,
	5, 4, 0,
	1, 5, 0,
	11, 6, 7,
	11, 7, 8,
	11, 8, 9,
	11, 9, 10,
	11, 10, 6,
	1, 2, 6,
	2, 3, 7,
	3, 4, 8,
	4, 5, 9,
	5, 1, 10,
	2, 7, 6,
	3, 8, 7,
	4, 9, 8,
	5, 10, 9,
	1, 6, 10 };
const int Icosahedron::nVerts=12;
const float Icosahedron::Verts[] = { //12 verts, 36 floats
	0.000f, 0.000f, 1.000f,
	0.894f, 0.000f, 0.447f,
	0.276f, 0.851f, 0.447f,
	-0.724f, 0.526f, 0.447f,
	-0.724f, -0.526f, 0.447f,
	0.276f, -0.851f, 0.447f,
	0.724f, 0.526f, -0.447f,
	-0.276f, 0.851f, -0.447f,
	-0.894f, 0.000f, -0.447f,
	-0.276f, -0.851f, -0.447f,
	0.724f, -0.526f, -0.447f,
	0.000f, 0.000f, -1.000f };

const int Octahedron::nFaces=8;
const int Octahedron::Faces[] = {
    0,2,4,
    0,4,3,
    0,3,5,
    0,5,2,
    1,2,4,
    1,4,3,
    1,3,5,
    1,5,2
};

const int Octahedron::nVerts=6;
const float Octahedron::Verts[] = {
       1,0,0,
        -1,0,0,
        0,1,0,
        0,-1,0,
        0,0,1,
        0,0,-1
};

const int Tetrahedron::nFaces = 4;
const int Tetrahedron::Faces[] = {
	0,1,2,
	0,1,3,
	0,3,2,
	1,2,3
};

const int Tetrahedron::nVerts = 4;
const float Tetrahedron::Verts[] = {
	sqrt(2.0f)/sqrt(3.0f)*1,		0,	-1.0f/sqrt(3.0f),
	-sqrt(2.0f) / sqrt(3.0f) * 1, 0, -1.0f / sqrt(3.0f),
	0,			sqrt(2.0f)/sqrt(3.0f), 1.0f / sqrt(3.0f),
	0,			-sqrt(2.0f) / sqrt(3.0f), 1.0f / sqrt(3.0f),
};