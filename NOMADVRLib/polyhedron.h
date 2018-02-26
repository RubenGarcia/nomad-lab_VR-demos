/*Uses code from Stackoverflow which uses the MIT license and the CC BY-SA 3.0*/
/*These licenses are compatible with Apache 2.0*/

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

#ifndef __POLYHEDRON_H
#define __POLYHEDRON_H

class Icosahedron {
public:
    static const int nFaces;
	static const int Faces[];
    static const int nVerts;
	static const float Verts[];
};

class Octahedron {
public:
    static const int nFaces;
    static const int Faces[];
    static const int nVerts;
    static const float Verts[];
};

class Tetrahedron {
public:
    static const int nFaces;
    static const int Faces[];
    static const int nVerts;
    static const float Verts[];
};


class Solid {
public:
	enum Type {
		Icosahedron,
		Octahedron,
		Tetrahedron,
		Sphere
	};
	Solid(Type t, int SphereFacets=15) {
		switch (t) {
		case Icosahedron:
			nFaces=Icosahedron::nFaces;
			Faces=Icosahedron::Faces;
			nVerts=Icosahedron::nVerts;
			Verts=Icosahedron::Verts;
			break;
		case Octahedron:
			nFaces=Octahedron::nFaces;
			Faces=Octahedron::Faces;
			nVerts=Octahedron::nVerts;
			Verts=Octahedron::Verts;
			break;
		case Tetrahedron:
			nFaces=Tetrahedron::nFaces;
			Faces=Tetrahedron::Faces;
			nVerts=Tetrahedron::nVerts;
			Verts=Tetrahedron::Verts;
			break;
		case Sphere:
//https://stackoverflow.com/questions/23143921/python-program-to-create-sphere-coordinates-not-working
			nVerts=SphereFacets*SphereFacets;
			
			float *tVerts=new float[nVerts*3];
			for (int i=0;i<SphereFacets;i++)
				for (int j=0;j<SphereFacets;j++) {
					float t, f;
					t=i*2.0f*(float)M_PI/(float)(SphereFacets-1);
					f=j*(float)M_PI/(float)(SphereFacets-1);
					
					tVerts[i*SphereFacets*3 + j*3]=sin(f)*sin(t);
					tVerts[i*SphereFacets*3 + j*3+1]=sin(f)*cos(t);
					tVerts[i*SphereFacets*3 + j*3+2]=cos(f);
				}
			nFaces=2*(SphereFacets-1)*(SphereFacets-1);
			int *tFaces=new int[3*nFaces];
			for (int i=0;i<SphereFacets-1;i++)
				for (int j=0;j<SphereFacets-1;j++) {
					tFaces[i*(SphereFacets-1)*6 + j*6]=i*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+1]=((i+1)%SphereFacets)*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+2]=(i)*SphereFacets+(j+1)%SphereFacets;
					tFaces[i*(SphereFacets-1)*6 + j*6+3]=((i+1)%SphereFacets)*SphereFacets+j;
					tFaces[i*(SphereFacets-1)*6 + j*6+4]=(i)*SphereFacets+(j+1)%SphereFacets;
					tFaces[i*(SphereFacets-1)*6 + j*6+5]=((i+1)%SphereFacets)*SphereFacets+(j+1)%SphereFacets;
				}
			Faces=tFaces; Verts=tVerts;
		} // switch
	} // Solid (constructor)
	
int nFaces;
const int *Faces;
int nVerts;
const float *Verts;
}; // class Solid

#endif // __POLYHEDRON_H
